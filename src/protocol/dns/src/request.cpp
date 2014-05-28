#include "sinkhole.hpp"
#include "network.hpp"
#include "io.hpp"
#include "string.hpp"
#include "timers.hpp"
#include "include/dns.hpp"
#include "include/query.hpp"
#include "include/datasocket.hpp"
#include "include/request.hpp"

using namespace Sinkhole::Protocol::DNS;

static std::map<short, DNSRequest *> requests;

DNSRequest::DNSRequest(const std::string &h, QueryType t) : Timer(resolver_timeout), host(h), type(t)
{
	do
		this->id = random();
	while (requests.count(this->id) > 0);
}

DNSRequest::~DNSRequest()
{
	requests.erase(this->id);
}

void DNSRequest::Dispatch()
{
	DNSServer *request_server = DNSServer::Find("dns/resolver");
	if (request_server == NULL)
		return;
	if (request_server->listeners.empty())
		request_server->listeners.push_back(new DNSDataSocket(request_server, "0.0.0.0", 0, false));
	DNSDataSocket *resolver_socket = request_server->listeners[0];

	DNSPacket *packet = new DNSPacket();

	packet->id = this->id;
	packet->source = resolver;
	packet->flags = DNS_QUERYFLAGS_RD;

	Question question;
	question.name = this->host;
	question.type = this->type;
	question.qclass = 1;

	packet->questions.push_back(question);

	resolver_socket->write_packets.push_back(packet);
	SocketEngine::ModifySocket(resolver_socket, resolver_socket->flags | SF_WANT_WRITE);

	requests[packet->id] = this;
}

void DNSRequest::Tick()
{
}

void DNSRequest::ProcessAnswer(DNSPacket &p)
{
	std::map<short, DNSRequest *>::iterator it = requests.find(p.id);
	if (it == requests.end())
		return;
	DNSRequest *request = it->second;

	if (p.source == resolver)
		request->OnResult(p);
	delete request;
}

