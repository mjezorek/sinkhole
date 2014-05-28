#include "sinkhole.hpp"
#include "network.hpp"
#include "io.hpp"
#include "include/irc.hpp"
#include "include/io.hpp"
#include "include/listener.hpp"

using namespace Sinkhole::Protocol::IRC;

IRCListener::IRCListener(IRCServer *s, const std::string &bindip, int port, int type) : Listener(bindip, port, type), server(s)
{
}

IRCListener::~IRCListener()
{
}

Sinkhole::Socket *IRCListener::OnAccept(int fd)
{
	return new ClientSocket(this->server, fd);
}

