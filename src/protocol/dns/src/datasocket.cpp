#include "sinkhole.hpp"
#include "logger.hpp"
#include "network.hpp"
#include "io.hpp"
#include "timers.hpp"
#include "include/dns.hpp"
#include "include/class.hpp"
#include "include/datasocket.hpp"
#include "include/query.hpp"
#include "include/request.hpp"

using namespace Sinkhole::Protocol::DNS;

DNSDataSocket::DNSDataSocket(DNSServer *s, const std::string &bindip, int port, bool ipv6) : Sinkhole::Listener(bindip, port, ipv6 ? AF_INET6 : AF_INET, SOCK_DGRAM), server(s)
{
}

bool DNSDataSocket::ProcessRead()
{
	std::pair<char *, size_t> net_buffer = Sinkhole::NetBuffer();
	DNSPacket packet;
	socklen_t x = sizeof(packet.source);
	int length = recvfrom(this->fd, net_buffer.first, net_buffer.second, 0, &packet.source.sa, &x);
	if (length < DNSPacket::HeaderLength)
		return true;
	
	unsigned char *buffer = reinterpret_cast<unsigned char *>(net_buffer.first);
	packet.Fill(buffer);

	DNSClass *dnsclass = NULL;
	if ((packet.flags & DNS_QUERYFLAGS_QR) == 0)
	{
		for (unsigned i = this->server->classes.size(); i > 0; --i)
		{
			DNSClass &c = this->server->classes[i - 1];
			for (unsigned j = c.sources.size(); j > 0; --j)
			{
				Sinkhole::cidr &range = c.sources[j - 1];
				if (range.match(packet.source))
				{
					dnsclass = &c;
					break;
				}
			}
		}

		if (dnsclass == NULL)
			return true;

		DNSPacket *return_packet = new DNSPacket();
		return_packet->source = packet.source;
		return_packet->id = packet.id;
		return_packet->flags = DNS_QUERYFLAGS_QR;

		for (unsigned i = 0; i < packet.questions.size(); ++i)
		{
			Question &q = packet.questions[i];

			Sinkhole::Log(this->server->name, packet.source.addr(), "REQUEST") << q.name;

			DNSZone *zone = dnsclass->FindZoneFor(q.name);
			if (zone != NULL)
			{
				return_packet->soa = zone->soa;

				if (q.type == DNS_QUERY_SOA)
				{
					ResourceRecord return_record;

					return_record.name = q.name;
					return_record.type = q.type;
					return_record.qclass = 1;
					return_record.ttl = zone->soa.refresh;

					return_packet->flags |= DNS_QUERYFLAGS_AA;
					return_packet->answers.push_back(return_record);

					continue;
				}

				std::map<std::string, DNSRecord>::iterator it = zone->records.find(q.name);
				if (it != zone->records.end())
				{
					std::map<std::string, DNSRecord>::iterator upper = zone->records.upper_bound(q.name);
					for (; it != upper; ++it)
					{
						DNSRecord &record = it->second;

						if (record.type != q.type)
							continue;

						ResourceRecord return_record;
						return_record.name = record.name;
						return_record.type = record.type;
						return_record.qclass = 1;
						return_record.ttl = record.ttl;
						return_record.rdata = record.result;

						return_packet->flags |= DNS_QUERYFLAGS_AA;
						return_packet->answers.push_back(return_record);
					}
				}
			}
		}

		if (return_packet->answers.empty())
			return_packet->flags |= DNS_QUERYFLAGS_RCODE_UNKNOWN;

		this->write_packets.push_back(return_packet);
		SocketEngine::ModifySocket(this, this->flags | SF_WANT_WRITE);
	}
	else
	{
		DNSRequest::ProcessAnswer(packet);
	}

	return true;
}

bool DNSDataSocket::ProcessWrite()
{
	DNSPacket *packet = !this->write_packets.empty() ? this->write_packets.front() : NULL;
	if (packet != NULL)
	{
		std::pair<char *, size_t> net_buffer = Sinkhole::NetBuffer();

		unsigned char *buffer = reinterpret_cast<unsigned char *>(net_buffer.first);
		unsigned short len = 0;

		packet->Pack(buffer, len);

		sendto(this->fd, buffer, len, 0, &packet->source.sa, packet->source.size());

		delete packet;
		this->write_packets.pop_front();
	}

	if (this->write_packets.empty())
		SocketEngine::ModifySocket(this, this->flags & ~SF_WANT_WRITE);

	return true;
}

