#ifndef DNS_RECORD_H
#define DNS_RECORD_H

struct SOARecord : public ResourceRecord
{
	std::string mname;
	std::string rname;
	unsigned int serial;
	unsigned int refresh;
	unsigned int retry;
	unsigned int expire;
	unsigned int minimum;

	SOARecord();
	SOARecord(ResourceRecord &rr);
};

struct DNSRecord
{
	/* Name of the initial lookup */
	std::string name;
	/* Result of the lookup */
	std::string result;
	/* Type of query this was */
	QueryType type;
	/* Time to live */
	time_t ttl;
};

#endif // DNS_RECORD_H

