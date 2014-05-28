#include "sinkhole.hpp"
#include "network.hpp"
#include "io.hpp"
#include "logger.hpp"
#include "include/ftp.hpp"
#include "include/client.hpp"
#include "include/listener.hpp"
#include "include/datasocket.hpp"

using namespace Sinkhole::Protocol::FTP;

FTPListener::FTPListener(FTPServer *s, const std::string &bindip, int port) : Listener(bindip, port, AF_INET), server(s)
{
}

FTPListener::~FTPListener()
{
}

Sinkhole::Socket *FTPListener::OnAccept(int fd)
{
	return new FTPClient(this->server, fd);
}

FTPDataListener::FTPDataListener(FTPClient *o, const std::string &bindip) : Listener(bindip, 0, AF_INET), owner(o)
{
}

FTPDataListener::~FTPDataListener()
{
	if (this->owner != NULL)
		this->owner->listener = NULL;
}

Sinkhole::Socket *FTPDataListener::OnAccept(int fd)
{
	SocketEngine::ModifySocket(this, this->flags | SF_DEAD);
	if (this->owner == NULL)
		return NULL;

	return new FTPDataSocket(this->owner, fd);
}

