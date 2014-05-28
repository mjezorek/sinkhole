#ifndef DNS_QUERY_H
#define DNS_QUERY_H

DNS_NAMESPACE_BEGIN

enum QueryType
{
	DNS_QUERY_A = 1,
	DNS_QUERY_NS = 2,
	DNS_QUERY_CNAME = 5,
	DNS_QUERY_SOA = 6,
	DNS_QUERY_PTR = 12,
	DNS_QUERY_AAAA = 28
};

enum
{
	DNS_QUERYFLAGS_QR = 0x8000,
	DNS_QUERYFLAGS_OPCODE = 0x7800,
	DNS_QUERYFLAGS_AA = 0x400,
	DBS_QUERYFLAGS_TC = 0x200,
	DNS_QUERYFLAGS_RD = 0x100,
	DNS_QUERYFLAGS_RA = 0x80,
	DNS_QUERYFLAGS_Z = 0x70,
	DNS_QUERYFLAGS_RCODE = 0xF,
	DNS_QUERYFLAGS_RCODE_FORMAT = 0x1,
	DNS_QUERYFLAGS_RCODE_SERVFAIL = 0x2,
	DNS_QUERYFLAGS_RCODE_UNKNOWN = 0x3,
	DNS_QUERYFLAGS_RCODE_NOT_IMPLEMENTED = 0x4,
	DNS_QUERYFLAGS_RCODE_REFUSED = 0x5
};

struct Question
{
	std::string name;
	QueryType type;
	unsigned short qclass;
};

struct ResourceRecord : public Question
{
	unsigned int ttl;
	std::string rdata;

	ResourceRecord();
	ResourceRecord(Question);
};

#include "record.hpp"

class DNSPacket
{
	void PackName(unsigned char *output, unsigned short &pos, const std::string &name);
	std::string UnpackName(const unsigned char *input, unsigned short &pos);
	Question UnpackQuestion(const unsigned char *input, unsigned short &pos);
 public:
	static const int HeaderLength = 12;

 	Sinkhole::sockaddrs source;
	/* 16-bit id for this header */
	unsigned short id;
	/* Flags on the query */
	unsigned short flags;

	/* SOA record for this zone */
	SOARecord soa;

	std::vector<Question> questions;
	std::vector<ResourceRecord> answers, authorities, additional;

	void Fill(const unsigned char *input);
	void Pack(unsigned char *output, unsigned short &len);
};

DNS_NAMESPACE_END

#endif

