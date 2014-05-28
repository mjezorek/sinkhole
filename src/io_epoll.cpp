#include "sinkhole.hpp"
#include "network.hpp"
#include "io.hpp"
#include "logger.hpp"
#include "string.hpp"
#include <sys/resource.h>
#include <sys/epoll.h>
#include <errno.h>

using namespace Sinkhole;

static int handle;
static rlimit fd_limit;
static epoll_event *events, event;

void SocketEngine::Init()
{ 
	if (getrlimit(RLIMIT_NOFILE, &fd_limit) == -1)
		throw SocketException(LastError());

	handle = epoll_create(fd_limit.rlim_cur);
	if (handle == -1)
		throw SocketException("Unable to create epoll engine: " + LastError());

	events = new epoll_event[fd_limit.rlim_cur];
}

void SocketEngine::Shutdown()
{
	for (std::map<int, Socket *>::iterator it = SocketEngine::sockets.begin(), it_end = SocketEngine::sockets.end(); it != it_end;)
	{
		Socket *s = it->second;
		++it;

		delete s;
	}

	delete [] events;
}

void SocketEngine::AddSocket(Socket *s)
{
	memset(&event, 0, sizeof(event));

	event.events = EPOLLIN;
	event.data.fd = s->fd;

	if (epoll_ctl(handle, EPOLL_CTL_ADD, s->fd, &event) == -1)
		throw SocketException("Unable to add fd " + stringify(s->fd) + " to epoll engine: " + LastError());
	
	SocketEngine::sockets[s->fd] = s;
}

void SocketEngine::DelSocket(Socket *s)
{
	memset(&event, 0, sizeof(event));

	event.data.fd = s->fd;

	if (epoll_ctl(handle, EPOLL_CTL_DEL, s->fd, &event) == -1)
		throw SocketException("Unable to remove fd " + stringify(s->fd) + " from epoll engine: " + LastError());
	
	SocketEngine::sockets.erase(s->fd);
}

void SocketEngine::ModifySocket(Socket *s, short flags)
{
	if (flags == s->flags)
		return;

	memset(&event, 0, sizeof(event));
	event.data.fd = s->fd;

	if (flags & SF_WANT_READ)
		event.events |= EPOLLIN;
	if (flags & SF_WANT_WRITE)
		event.events |= EPOLLOUT;
	
	if (epoll_ctl(handle, EPOLL_CTL_MOD, s->fd, &event) == -1)
		throw SocketException("Unable to modify fd " + stringify(s->fd) + " in epoll engine: " + LastError());

	s->flags = flags;
}

void SocketEngine::Process()
{
	int total = epoll_wait(handle, events, fd_limit.rlim_cur, 10000);
	curtime = time(NULL);

	if (total == -1 && errno != EINTR)
		throw SocketException("Error from epoll_wait(): " + LastError());
	else
		for (int i = 0; i < total; ++i)
		{
			epoll_event *ev = &events[i];
			Socket *s = sockets[ev->data.fd];

			if (ev->events & EPOLLERR)
			{
				socklen_t sz = sizeof(errno);
				getsockopt(s->fd, SOL_SOCKET, SO_ERROR, &errno, &sz);
				s->ProcessError();
				delete s;
				continue;
			}

			if (ev->events & EPOLLIN && !s->ProcessRead())
			{
				ModifySocket(s, s->flags & ~SF_WANT_READ);
				s->flags |= SF_DEAD;
			}

			if (ev->events & EPOLLOUT && !s->ProcessWrite())
			{
				ModifySocket(s, s->flags & ~SF_WANT_WRITE);
				s->flags |= SF_DEAD;
			}

			if (s->flags & SF_DEAD && !(s->flags & SF_WANT_WRITE))
				delete s;
		}
}

