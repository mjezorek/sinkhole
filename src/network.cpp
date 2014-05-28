#include "sinkhole.hpp"
#include "network.hpp"
#include "io.hpp"
#include "string.hpp"

using namespace Sinkhole;

std::pair<char *, size_t> Sinkhole::NetBuffer()
{
	static char net_buffer[16384];
	return std::make_pair(net_buffer, sizeof(net_buffer));
}

sockaddrs::sockaddrs()
{
	this->clear();
}

void sockaddrs::clear()
{
	memset(this, 0, sizeof(*this));
}

size_t sockaddrs::size() const
{
	switch (sa.sa_family)
	{
		case AF_INET:
			return sizeof(this->sa4);
		case AF_INET6:
			return sizeof(this->sa6);
		case AF_UNIX:
			return sizeof(this->un);
		default:
			break;
	}

	return 0;
}

int sockaddrs::port() const
{
	switch (sa.sa_family)
	{
		case AF_INET:
			return ntohs(sa4.sin_port);
		case AF_INET6:
			return ntohs(sa6.sin6_port);
		default:
			break;
	}

	return -1;
}

std::string sockaddrs::addr() const
{
	char address[INET6_ADDRSTRLEN + 1] = "";

	switch (sa.sa_family)
	{
		case AF_INET:
			if (!inet_ntop(AF_INET, &sa4.sin_addr, address, sizeof(address)))
				throw SocketException(LastError());
			return address;
		case AF_INET6:
			if (!inet_ntop(AF_INET6, &sa6.sin6_addr, address, sizeof(address)))
				throw SocketException(LastError());
			return address;
		default:
			break;
	}

	return address;
}

bool sockaddrs::operator()() const
{
	return this->sa.sa_family != 0;
}

bool sockaddrs::operator==(const sockaddrs &other) const
{
	if (this->sa.sa_family != other.sa.sa_family)
		return false;
	switch (this->sa.sa_family)
	{
		case AF_INET:
			return (this->sa4.sin_port == other.sa4.sin_port) && (this->sa4.sin_addr.s_addr == other.sa4.sin_addr.s_addr);
		case AF_INET6:
			return (this->sa6.sin6_port == other.sa6.sin6_port) && !memcmp(this->sa6.sin6_addr.s6_addr, other.sa6.sin6_addr.s6_addr, 16);
		default:
			return !memcmp(this, &other, sizeof(*this));
	}

	return false;
}

bool sockaddrs::operator<(const sockaddrs &other) const
{
	if (this->sa.sa_family != other.sa.sa_family)
		return this->sa.sa_family < other.sa.sa_family;
	return memcmp(this, &other, sizeof(*this)) < 0;
}

void sockaddrs::pton(int type, const std::string &address, int pport)
{
	switch (type)
	{
		case AF_INET:
		{
			int i = inet_pton(type, address.c_str(), &sa4.sin_addr);
			if (i == 0)
				throw SocketException("Invalid host");
			else if (i <= -1)
				throw SocketException("Invalid host: " + Sinkhole::LastError());
			sa4.sin_family = type;
			sa4.sin_port = htons(pport);
			return;
		}
		case AF_INET6:
		{
			int i = inet_pton(type, address.c_str(), &sa6.sin6_addr);
			if (i == 0)
				throw SocketException("Invalid host");
			else if (i <= -1)
				throw SocketException("Invalid host: " + Sinkhole::LastError());
			sa6.sin6_family = type;
			sa6.sin6_port = htons(pport);
			return;
		}
		default:
			break;
	}

	throw Exception("Invalid socket type");
}

void sockaddrs::ntop(int type, const void *src)
{
	switch (type)
	{
		case AF_INET:
			sa4.sin_addr = *reinterpret_cast<const in_addr *>(src);
			sa4.sin_family = type;
			return;
		case AF_INET6:
			sa6.sin6_addr = *reinterpret_cast<const in6_addr *>(src);
			sa6.sin6_family = type;
			return;
		default:
			break;
	}

	throw Exception("Invalid socket type");
}

const unsigned char cidr::AF_INET_LEN = 32;
const unsigned char cidr::AF_INET6_LEN = 128;

cidr::cidr(const std::string &ip)
{
	if (ip.find_first_not_of("1234567890:./") != std::string::npos)
		throw SocketException("Invalid IP");

	bool ipv6 = ip.find(':') != std::string::npos;
	size_t sl = ip.find_last_of('/');
	if (sl == std::string::npos)
	{
		this->cidr_ip = ip;
		this->cidr_len = ipv6 ? AF_INET6_LEN : AF_INET_LEN;
		this->addr.pton(ipv6 ? AF_INET6 : AF_INET, ip);
	}
	else
	{
		std::string real_ip = ip.substr(0, sl);
		std::string cidr_range = ip.substr(sl + 1);
		if (cidr_range.find_first_not_of("1234567890") != std::string::npos)
			throw SocketException("Invalid CIDR range");

		this->cidr_ip = real_ip;
		this->cidr_len = atoi(cidr_range.c_str());
		this->addr.pton(ipv6 ? AF_INET6 : AF_INET, real_ip);
	}
}

cidr::cidr(const std::string &ip, unsigned char len)
{
	if (ip.find_first_not_of("1234567890:.") != std::string::npos)
		throw SocketException("Invalid IP");

	bool ipv6 = ip.find(':') != std::string::npos;
	this->addr.pton(ipv6 ? AF_INET6 : AF_INET, ip);
	this->cidr_ip = ip;
	this->cidr_len = len;
}

std::string cidr::mask() const
{
	return this->cidr_ip + "/" + stringify(this->cidr_len);
}

bool cidr::match(sockaddrs &other)
{
	if (this->addr.sa.sa_family != other.sa.sa_family)
		return false;

	unsigned char *ip, *their_ip, byte = this->cidr_len / 8;

	switch (this->addr.sa.sa_family)
	{
		case AF_INET:
			ip = reinterpret_cast<unsigned char *>(&this->addr.sa4.sin_addr);
			their_ip = reinterpret_cast<unsigned char *>(&other.sa4.sin_addr);
			break;
		case AF_INET6:
			ip = reinterpret_cast<unsigned char *>(&this->addr.sa6.sin6_addr);
			their_ip = reinterpret_cast<unsigned char *>(&other.sa6.sin6_addr);
			break;
		default:
			throw SocketException("Invalid address type");
	}
	
	if (memcmp(ip, their_ip, byte))
		return false;

	ip += byte;
	their_ip += byte;
	byte = this->cidr_len % 8;
	if ((*ip & byte) != (*their_ip & byte))
		return false;
	
	return true;
}

bool cidr::operator==(const cidr &other) const
{
	return (this->addr == other.addr && this->cidr_len == other.cidr_len);
}

bool cidr::operator<(const cidr &other) const
{
	return this->addr < other.addr;
}

