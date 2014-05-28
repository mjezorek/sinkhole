#ifndef DNS_DNS_H
#define DNS_DNS_H

#define DNS_NAMESPACE_BEGIN namespace Sinkhole { namespace Protocol { namespace DNS {
#define DNS_NAMESPACE_END } } }

DNS_NAMESPACE_BEGIN

class DNSException : public Exception
{
 public:
	DNSException(const std::string &r) : Exception(r) { }
};

class DNSDataSocket;
class DNSClass;

class DNSServer
{
 public:
	std::string name;
	std::vector<DNSDataSocket *> listeners;
	std::vector<DNSClass> classes;

	static DNSServer *Find(const std::string &name);
	DNSServer(const std::string &n);
	~DNSServer();
};

extern std::vector<DNSServer *> servers;

extern Sinkhole::sockaddrs resolver;
extern int resolver_timeout;

extern bool resolve_clients;

DNS_NAMESPACE_END

#endif // DNS_DNS_H

