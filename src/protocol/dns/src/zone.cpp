#include "sinkhole.hpp"
#include "network.hpp"
#include "include/dns.hpp"
#include "include/query.hpp"
#include "include/zone.hpp"

using namespace Sinkhole::Protocol::DNS;

void DNSZone::AddRecord(const std::string &n, DNSRecord &record)
{
	std::string hostname = n;
	if (hostname == "@")
		hostname = this->name;
	else if (hostname[hostname.length() - 1] != '.')
		hostname += "." + this->name;
	record.name = hostname;
	this->records.insert(std::make_pair(hostname, record));
}

