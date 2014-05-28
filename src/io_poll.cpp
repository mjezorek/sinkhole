#include "sinkhole.hpp"
#include "network.hpp"
#include "io.hpp"
#include "string.hpp"
#include <sys/resource.h>
#include <sys/poll.h>
#include <errno.h>

using namespace Sinkhole;

static rlimit fd_limit;
static pollfd *pollfds;
static int SocketCount;
static std::map<int, int> socket_indexes;

void SocketEngine::Init()
{ 
	if (getrlimit(RLIMIT_NOFILE, &fd_limit) == -1)
		throw SocketException(LastError());

	pollfds = new pollfd[fd_limit.rlim_cur];
	SocketCount = 0;
}

void SocketEngine::Shutdown()
{
	for (std::map<int, Socket *>::iterator it = SocketEngine::sockets.begin(), it_end = SocketEngine::sockets.end(); it != it_end;)
	{
		Socket *s = it->second;
		++it;

		delete s;
	}

	delete [] pollfds;
}

void SocketEngine::AddSocket(Socket *s)
{
	if (SocketCount == static_cast<int>(fd_limit.rlim_cur))
		throw SocketException("Unable to add fd " + stringify(s->fd) + " to poll engine: fd limit max reached");

	pollfd *fd = &pollfds[SocketCount];
	fd->fd = s->fd;
	fd->events = POLLIN;
	fd->revents = 0;

	SocketEngine::sockets[s->fd] = s;
	socket_indexes[s->fd] = SocketCount;

	++SocketCount;
}

void SocketEngine::DelSocket(Socket *s)
{
	std::map<int, int>::iterator it = socket_indexes.find(s->fd);
	if (it == socket_indexes.end())
		throw SocketException("Unable to remove unknown fd " + stringify(s->fd) + " from poll engine");

	if (it->second != SocketCount - 1)
	{
		pollfd *ev = &pollfds[it->second], *last_ev = &pollfds[SocketCount - 1];

		ev->fd = last_ev->fd;
		ev->events = last_ev->events;
		ev->revents = last_ev->revents;

		socket_indexes[ev->fd] = it->second;
	}

	SocketEngine::sockets.erase(s->fd);
	socket_indexes.erase(it);

	--SocketCount;
}

void SocketEngine::ModifySocket(Socket *s, short flags)
{
	if (flags == s->flags)
		return;

	int index = socket_indexes[s->fd];
	pollfd *fd = &pollfds[index];

	fd->events = 0;
	if (flags & SF_WANT_READ)
		fd->events |= POLLIN;
	if (flags & SF_WANT_WRITE)
		fd->events |= POLLOUT;

	s->flags = flags;
}

void SocketEngine::Process()
{
	int total = poll(pollfds, SocketCount, 10000);
	curtime = time(NULL);

	if (total == -1 && errno != EINTR)
		throw SocketException("Error from poll(): " + LastError());
	else
		for (int i = 0, j = total; i < SocketCount && j > 0; ++i)
		{
			pollfd *fd = &pollfds[i];
			if (fd->fd == -1 || fd->revents == 0)
				continue;
			--j;

			Socket *s = sockets[fd->fd];

			if (fd->revents & POLLERR)
			{
				socklen_t sz = sizeof(errno);
				getsockopt(s->fd, SOL_SOCKET, SO_ERROR, &errno, &sz);
				s->ProcessError();
				delete s;
				continue;
			}

			if (fd->revents & POLLIN && !s->ProcessRead())
			{
				ModifySocket(s, s->flags & ~SF_WANT_READ);
				s->flags |= SF_DEAD;
			}

			if (fd->revents & POLLOUT && !s->ProcessWrite())
			{
				ModifySocket(s, s->flags & ~SF_WANT_WRITE);
				s->flags |= SF_DEAD;
			}

			if (s->flags & SF_DEAD && !(s->flags & SF_WANT_WRITE))
				delete s;
		}
}

