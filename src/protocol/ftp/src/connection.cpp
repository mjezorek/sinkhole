#include "sinkhole.hpp"
#include "network.hpp"
#include "io.hpp"
#include "include/ftp.hpp"
#include "include/datasocket.hpp"
#include "include/connection.hpp"
#include <errno.h>

using namespace Sinkhole::Protocol::FTP;

FTPDataConnection::FTPDataConnection(FTPClient *o, const std::string &addr, int port) : FTPDataSocket(o)
{
	sockaddrs con_addr;
	con_addr.pton(AF_INET, addr, port);
	this->connected = !connect(this->fd, &con_addr.sa, con_addr.size());
	if (!this->connected && errno != EINPROGRESS)
		throw Sinkhole::SocketException(Sinkhole::LastError());
	else if (!this->connected)
		Sinkhole::SocketEngine::ModifySocket(this, this->flags | Sinkhole::SF_WANT_WRITE);
}

bool FTPDataConnection::ProcessWrite()
{
	if (!this->connected)
	{
		int optval = 0;
		socklen_t optlen = sizeof(optval);
		if (!getsockopt(this->fd, SOL_SOCKET, SO_ERROR, &optval, &optlen) && !optval)
		{
			this->connected = true;
			return true;
		}

		this->ProcessError();
		return false;
	}

	return FTPDataSocket::ProcessWrite();
}

