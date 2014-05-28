#ifndef DNS_CLASS_H
#define DNS_CLASS_H

#include "query.hpp"
#include "zone.hpp"

DNS_NAMESPACE_BEGIN

class DNSClass
{
 public:
	std::vector<Sinkhole::cidr> sources;
	std::vector<DNSZone> zones;

	DNSZone *FindZoneFor(const std::string &name);
};

DNS_NAMESPACE_END

#endif // DNS_CLASS_H

