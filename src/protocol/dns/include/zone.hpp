#ifndef DNS_ZONE_H
#define DNS_ZONE_H

DNS_NAMESPACE_BEGIN

class DNSZone
{
 public:
	std::string name;
	SOARecord soa;
	std::multimap<std::string, DNSRecord> records;

	void AddRecord(const std::string &n, DNSRecord &record);
};

DNS_NAMESPACE_END

#endif // DNS_ZONE_H

