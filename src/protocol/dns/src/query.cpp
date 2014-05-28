#include "sinkhole.hpp"
#include "network.hpp"
#include "io.hpp"
#include "string.hpp"
#include "include/dns.hpp"
#include "include/query.hpp"
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

using namespace Sinkhole::Protocol::DNS;

ResourceRecord::ResourceRecord()
{
}

ResourceRecord::ResourceRecord(Question q) : Question(q)
{
}

void DNSPacket::PackName(unsigned char *output, unsigned short &pos, const std::string &name)
{
	Sinkhole::sepstream sep(name, '.');
	std::string token;

	while (sep.GetToken(token))
	{
		output[pos++] = token.length();
		memcpy(&output[pos], token.c_str(), token.length());
		pos += token.length();
	}

	output[pos++] = 0;
}

std::string DNSPacket::UnpackName(const unsigned char *input, unsigned short &pos)
{
	std::string name;
	unsigned short packet_pos_ptr = pos;

	for (unsigned short offset = input[packet_pos_ptr]; offset; packet_pos_ptr += offset + 1, offset = input[packet_pos_ptr])
	{
		if (offset & 0xC0)
		{
			offset = (offset & 0x3F) << 8 | input[++packet_pos_ptr];
			packet_pos_ptr = offset;
			offset = input[packet_pos_ptr];
		}
		for (unsigned i = 1; i <= offset; ++i)
			name += input[packet_pos_ptr + i];
		name += ".";
	}
	name.erase(name.begin() + name.length() - 1);

	if (packet_pos_ptr < pos)
		pos += 2;
	else
		pos = packet_pos_ptr + 1;
	
	return name;
}

Question DNSPacket::UnpackQuestion(const unsigned char *input, unsigned short &pos)
{
	Question question;

	question.name = this->UnpackName(input, pos);

	question.type = static_cast<QueryType>(input[pos] << 8 | input[pos + 1]);
	pos += 2;

	question.qclass = input[pos] << 8 | input[pos + 1];
	pos += 2;

	return question;
}

void DNSPacket::Fill(const unsigned char *input)
{
	this->id = (input[0] << 8) | input[1];
	this->flags = (input[2] << 8) | input[3];

	if (this->flags & DNS_QUERYFLAGS_OPCODE)
		return;

	unsigned short qdcount = (input[4] << 8) | input[5];
	unsigned short ancount = (input[6] << 8) | input[7];
	unsigned short nscount = (input[8] << 8) | input[9];
	unsigned short arcount = (input[10] << 8) | input[11];

	unsigned short packet_pos = DNSPacket::HeaderLength;

	for (unsigned qcount = 0; qcount < qdcount; ++qcount)
		this->questions.push_back(this->UnpackQuestion(input, packet_pos));
	
	unsigned counts[] = { ancount, nscount, arcount };
	for (int i = 0; i < 3; ++i)
	{
		unsigned &count = counts[i];
		for (unsigned rcount = 0; rcount < count; ++rcount)
		{
			ResourceRecord record = static_cast<ResourceRecord>(this->UnpackQuestion(input, packet_pos));

			record.ttl = (input[packet_pos] << 24) | (input[packet_pos + 1] << 16) | (input[packet_pos + 2] << 8) | input[packet_pos + 3];
			packet_pos += 4;

			//record.rdlength = input[packet_pos] << 8 | input[packet_pos + 1];
			packet_pos += 2;

			switch (record.type)
			{
				case DNS_QUERY_A:
				{
					in_addr addr;
					addr.s_addr = input[packet_pos] | (input[packet_pos + 1] << 8) | (input[packet_pos + 2] << 16)  | (input[packet_pos + 3] << 24);
					packet_pos += 4;

					sockaddrs addrs;

					try
					{
						addrs.ntop(AF_INET, &addr);

						record.rdata = addrs.addr();
					}
					catch (const Sinkhole::SocketException &)
					{
						return;
					}
					break;
				}
				case DNS_QUERY_AAAA:
				{
					in6_addr addr;
					for (int j = 0; j < 16; ++j)
						addr.s6_addr[j] = input[packet_pos + j];
					packet_pos += 16;

					sockaddrs addrs;
					try
					{
						addrs.ntop(AF_INET6, &addr);

						record.rdata = addrs.addr();
					}
					catch (const Sinkhole::SocketException &)
					{
						return;
					}
					break;
				}
				case DNS_QUERY_NS:
				case DNS_QUERY_CNAME:
				case DNS_QUERY_PTR:
				{
					record.rdata = this->UnpackName(input, packet_pos);
					break;
				}
				case DNS_QUERY_SOA:
					break;
				default:
					continue;
			}

			if (i == 0)
				this->answers.push_back(record);
			else if (i == 1)
				this->authorities.push_back(record);
			else if (i == 2)
				this->additional.push_back(record);
		}
	}
}


void DNSPacket::Pack(unsigned char *output, unsigned short &len)
{
	output[0] = this->id >> 8;
	output[1] = this->id & 0xFF;
	output[2] = this->flags >> 8;
	output[3] = this->flags & 0xFF;
	output[4] = this->questions.size() >> 8;
	output[5] = this->questions.size() & 0xFF;
	output[6] = this->answers.size() >> 8;
	output[7] = this->answers.size() & 0xFF;
	output[8] = this->authorities.size() >> 8;
	output[9] = this->authorities.size() & 0xFF;
	output[10] = this->additional.size() >> 8;
	output[11] = this->additional.size() & 0xFF;

	len = DNSPacket::HeaderLength;
	for (unsigned i = 0; i < this->questions.size(); ++i)
	{
		Question &q = this->questions[i];

		if (q.type == DNS_QUERY_PTR)
		{
			if (q.name.find(':') != std::string::npos)
			{
				Sinkhole::sockaddrs ip;
				ip.pton(AF_INET6, q.name);

				static const char *const hex = "0123456789abcdef";
				char reverse_ip[128];
				unsigned reverse_ip_count = 0;
				for (int j = 15; j >= 0; --j)
				{
					reverse_ip[reverse_ip_count++] = hex[ip.sa6.sin6_addr.s6_addr[j] & 0xF];
					reverse_ip[reverse_ip_count++] = '.';
					reverse_ip[reverse_ip_count++] = hex[ip.sa6.sin6_addr.s6_addr[j] >> 4];
					reverse_ip[reverse_ip_count++] = '.';
				}
				reverse_ip[reverse_ip_count++] = 0;

				q.name = reverse_ip;
				q.name += "ip6.arpa";
			}
			else
			{
				Sinkhole::sockaddrs ip;
				ip.pton(AF_INET, q.name);

				unsigned long forward = ip.sa4.sin_addr.s_addr;
				in_addr reverse;
				reverse.s_addr = forward << 24 | (forward & 0xFF00) << 8 | (forward & 0xFF0000) >> 8 | forward >> 24;

				ip.ntop(AF_INET, &reverse);

				q.name = ip.addr() + ".in-addr.arpa";
			}
		}

		this->PackName(output, len, q.name);

		short ii = htons(q.type);
		memcpy(&output[len], &ii, 2);
		len += 2;

		ii = htons(q.qclass);
		memcpy(&output[len], &ii, 2);
		len += 2;
	}

	std::vector<ResourceRecord> record_types[] = { this->answers, this->authorities, this->additional };
	for (int i = 0; i < 3; ++i)
	{
		std::vector<ResourceRecord> &rrv = record_types[i];
		for (unsigned j = 0; j < rrv.size(); ++j)
		{
			ResourceRecord &rr = rrv[j];
			this->PackName(output, len, rr.name);

			short i = htons(rr.type);
			memcpy(&output[len], &i, 2);
			len += 2;

			i = htons(rr.qclass);
			memcpy(&output[len], &i, 2);
			len += 2;

			long l = htonl(rr.ttl);
			memcpy(&output[len], &l, 4);
			len += 4;

			switch (rr.type)
			{
				case DNS_QUERY_A:
				{

					try
					{
						sockaddrs addr;
						addr.pton(AF_INET, rr.rdata);

						short s = htons(4);
						memcpy(&output[len], &s, 2);
						len += 2;

						memcpy(&output[len], &addr.sa4.sin_addr, 4);
						len += 4;
					}
					catch (const Sinkhole::SocketException &) { }
					break;
				}
				case DNS_QUERY_AAAA:
				{
					try
					{
						sockaddrs addr;
						addr.pton(AF_INET6, rr.rdata);

						short s = htons(16);
						memcpy(&output[len], &s, 2);
						len += 2;

						memcpy(&output[len], &addr.sa6.sin6_addr, 16);
						len += 16;
					}
					catch (const Sinkhole::SocketException &) { }
					break;
				}
				case DNS_QUERY_NS:
				case DNS_QUERY_CNAME:
				case DNS_QUERY_PTR:
				{
					unsigned short packet_pos_save = len;
					len += 2;

					this->PackName(output, len, rr.rdata);

					short s = htons(len - packet_pos_save - 2);
					memcpy(&output[packet_pos_save], &s, 2);
					break;
				}
				case DNS_QUERY_SOA:
				{
					unsigned short packet_pos_save = len;
					len += 2;

					this->PackName(output, len, soa.mname);
					this->PackName(output, len, soa.rname);

					long l = htonl(soa.serial);
					memcpy(&output[len], &l, 4);
					len += 4;

					l = htonl(soa.refresh);
					memcpy(&output[len], &l, 4);
					len += 4;

					l = htonl(soa.retry);
					memcpy(&output[len], &l, 4);
					len += 4;

					l = htonl(soa.expire);
					memcpy(&output[len], &l, 4);
					len += 4;

					l = htonl(soa.minimum);
					memcpy(&output[len], &l, 4);
					len += 4;

					short s = htons(len - packet_pos_save - 2);
					memcpy(&output[packet_pos_save], &s, 2);
					break;
				}
			}
		}
	}
}

