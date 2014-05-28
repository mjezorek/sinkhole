#include "sinkhole.hpp"
#include "network.hpp"
#include "include/dns.hpp"
#include "include/class.hpp"

using namespace Sinkhole::Protocol::DNS;

DNSZone *DNSClass::FindZoneFor(const std::string &name)
{
	for (unsigned i = this->zones.size(); i > 0; --i)
	{
		DNSZone &z = this->zones[i - 1];

		if (z.name.length() > name.length())
			continue;

		size_t pos = name.find(z.name);
		size_t want = name.length() - z.name.length();

		if (pos == want)
			return &z;
	}

	return NULL;
}

