#include "sinkhole.hpp"
#include "network.hpp"
#include "io.hpp"
#include "logger.hpp"
#include "module.hpp"
#include "string.hpp"
#include <errno.h>

using namespace Sinkhole;

Listener::Listener(const std::string &bindip, int port, int type, int stype) : Socket(type, stype)
{
	sockaddrs bind_host;

	bind_host.pton(type, bindip, port);

	if (bind(this->fd, &bind_host.sa, bind_host.size()) == -1)
		throw SocketException(LastError());
	
	if (stype != SOCK_DGRAM && listen(this->fd, SOMAXCONN) == -1)
		throw SocketException(LastError());
}

bool Listener::ProcessRead()
{
	int newfd = accept(this->fd, NULL, NULL);
	if (newfd < 0)
	{
		if (errno != EAGAIN && errno != EWOULDBLOCK)
			Log(LOG_ERROR) << "Unable to accept connection: " << LastError();
		return true;
	}
	
	try
	{
		Socket *s = this->OnAccept(newfd);
		FOREACH_MOD(OnClientAccept(s));
	}
	catch (const SocketException &ex)
	{
		Log(LOG_INFORMATIONAL) << ex.GetReason();
	}

	return true;
}

Socket *Listener::OnAccept(int fd)
{
	return new Socket(fd);
}

