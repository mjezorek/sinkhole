#ifndef DNS_DATASOCKET_H
#define DNS_DATASOCKET_H

DNS_NAMESPACE_BEGIN

class DNSDataSocket : public Sinkhole::Listener
{
	DNSServer *server;

 public:
	std::deque<DNSPacket *> write_packets;

	DNSDataSocket(DNSServer *s, const std::string &bindip, int port, bool ipv6);

	bool ProcessRead();

	bool ProcessWrite();
};

DNS_NAMESPACE_END

#endif // DNS_DATASOCKET_H

