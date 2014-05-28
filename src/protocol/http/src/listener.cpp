#include "sinkhole.hpp"
#include "network.hpp"
#include "io.hpp"
#include "logger.hpp"
#include "include/http.hpp"
#include "include/io.hpp"
#include "include/listener.hpp"

using namespace Sinkhole::Protocol::HTTP;

HTTPListener::HTTPListener(HTTPServer *s, const std::string &bindip, int port, int type) : Listener(bindip, port, type), server(s)
{
}

HTTPListener::~HTTPListener()
{
}

Sinkhole::Socket *HTTPListener::OnAccept(int fd)
{
	return new ClientSocket(this->server, fd);
}

