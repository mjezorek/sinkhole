#ifndef DNS_REQUEST_H
#define DNS_REQUEST_H

DNS_NAMESPACE_BEGIN

class DNSRequest : public Sinkhole::Timer
{
	std::string host;
	QueryType type;
	unsigned short id;

 public:
	DNSRequest(const std::string &h, QueryType t);
	virtual ~DNSRequest();
	void Dispatch();

	void Tick();

	virtual void OnResult(const DNSPacket &p) = 0;

	static void ProcessAnswer(DNSPacket &p);
};

DNS_NAMESPACE_END

#endif // DNS_REQUEST_H

