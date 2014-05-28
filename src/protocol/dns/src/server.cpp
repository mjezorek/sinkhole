#include "sinkhole.hpp"
#include "network.hpp"
#include "io.hpp"
#include "include/dns.hpp"
#include "include/class.hpp"
#include "include/datasocket.hpp"

using namespace Sinkhole::Protocol::DNS;

DNSServer *DNSServer::Find(const std::string &name)
{
	for (unsigned i = servers.size(); i > 0; --i)
		if (servers[i - 1]->name == name)
			return servers[i - 1];
	return NULL;
}

DNSServer::DNSServer(const std::string &n) : name(n)
{
	servers.push_back(this);
}

DNSServer::~DNSServer()
{
	for (unsigned i = this->listeners.size(); i > 0; --i)
		delete this->listeners[i - 1];

	std::vector<DNSServer *>::iterator it = std::find(servers.begin(), servers.end(), this);
	if (it != servers.end())
		servers.erase(it);
}

