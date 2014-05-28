#include "sinkhole.hpp"
#include "network.hpp"
#include "io.hpp"
#include "string.hpp"
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

using namespace Sinkhole;

std::map<int, Socket *> SocketEngine::sockets;

Socket::Socket(int sfd)
{
	this->fd = sfd;
	this->flags = SF_WANT_READ;

	int sockflags = fcntl(this->fd, F_GETFL, 0);
	if (fcntl(this->fd, F_SETFL, sockflags & ~O_NONBLOCK))
		throw SocketException("Unable to mark fd " + stringify(this->fd) + " as non blocking");
	
	SocketEngine::AddSocket(this);
}

Socket::Socket(int sdomain, int stype)
{
	this->fd = socket(sdomain, stype, 0);
	this->flags = SF_WANT_READ;

	if (this->fd == -1)
		throw Exception("Unable to create socket: " + LastError());

	int sockflags = fcntl(this->fd, F_GETFL, 0);
	if (fcntl(this->fd, F_SETFL, sockflags & ~O_NONBLOCK))
		throw SocketException("Unable to mark fd " + stringify(this->fd) + " as non blocking");

	SocketEngine::AddSocket(this);
}

Socket::~Socket()
{
	SocketEngine::DelSocket(this);

	close(this->fd);
}

sockaddrs &Socket::GetSock()
{
	if (this->source_info.sa.sa_family != AF_UNSPEC)
		return this->source_info;
	
	socklen_t addrlen = sizeof(this->source_info);
	if (getsockname(this->fd, &this->source_info.sa, &addrlen) == -1)
		throw SocketException(Sinkhole::LastError());
	
	return this->GetSock();
}

sockaddrs &Socket::GetPeer()
{
	if (this->peer_info.sa.sa_family != AF_UNSPEC)
		return this->peer_info;
	
	socklen_t addrlen = sizeof(this->peer_info);
	if (getpeername(this->fd, &this->peer_info.sa, &addrlen) == -1)
		throw SocketException(Sinkhole::LastError());
	
	return this->GetPeer();
}

bool Socket::ProcessRead()
{
	return true;
}

bool Socket::ProcessWrite()
{
	return true;
}

void Socket::ProcessError()
{
}

