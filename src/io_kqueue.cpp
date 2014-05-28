#include "sinkhole.hpp"
#include "network.hpp"
#include "io.hpp"
#include "string.hpp"
#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include <errno.h>

using namespace Sinkhole;

static int kq_fd, max_fds;
static struct kevent *change_events, *event_events;
static int change_count;

static struct kevent *GetChangeEvent()
{
	if (change_count == max_fds)
	{
		timespec zero_timespec = { 0, 0 };
		for (int i = 0; i < change_count; ++i)
			kevent(kq_fd, change_events + i, 1, NULL, 0, &zero_timespec);
		change_count = 0;
	}

	struct kevent *event = &change_events[change_count++];
	return event;
}

void SocketEngine::Init()
{ 
	kq_fd = kqueue();
	max_fds = getdtablesize();

	if (kq_fd < 0)
		throw SocketException("Unable to create kqueue engine: " + LastError());
	else if (max_fds < 0)
		throw SocketException("Unable to determine max file descriptors");

	change_events = new struct kevent[max_fds];
	event_events = new struct kevent[max_fds];

	change_count = 0;
}

void SocketEngine::Shutdown()
{
	for (std::map<int, Socket *>::iterator it = SocketEngine::sockets.begin(), it_end = SocketEngine::sockets.end(); it != it_end;)
	{
		Socket *s = it->second;
		++it;

		delete s;
	}

	delete [] change_events;
	delete [] event_events;
}

void SocketEngine::AddSocket(Socket *s)
{
	struct kevent *event = GetChangeEvent();
	EV_SET(event, s->fd, EVFILT_READ, EV_ADD, 0, 0, NULL);

	SocketEngine::sockets[s->fd] = s;
}

void SocketEngine::DelSocket(Socket *s)
{
	struct kevent *event = GetChangeEvent();
	EV_SET(event, s->fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);

	event = GetChangeEvent();
	EV_SET(event, s->fd, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);

	SocketEngine::sockets.erase(s->fd);
}

void SocketEngine::ModifySocket(Socket *s, short flags)
{
	int change = flags ^ s->flags;
	if (change == 0)
		return;

	if (change & SF_WANT_READ)
	{
		struct kevent *event = GetChangeEvent();
		EV_SET(event, s->fd, EVFILT_READ, flags & SF_WANT_READ ? EV_ADD : EV_DELETE, 0, 0, NULL);
	}
	if (change & SF_WANT_WRITE)
	{
		struct kevent *event = GetChangeEvent();
		EV_SET(event, s->fd, EVFILT_WRITE, flags & SF_WANT_WRITE ? EV_ADD : EV_DELETE, 0, 0, NULL);
	}
	
	s->flags = flags;
}

void SocketEngine::Process()
{
	static timespec kq_timespec = { 10, 0 };
	int total = kevent(kq_fd, change_events, change_count, event_events, max_fds, &kq_timespec);
	change_count = 0;
	curtime = time(NULL);

	if (total == -1 && errno != EINTR)
		throw SocketException("Error from kevent(): " + LastError());
	else
		for (int i = 0; i < total; ++i)
		{
			struct kevent *event = &event_events[i];
			if (event->flags & EV_ERROR)
				continue;

			std::map<int, Socket *>::iterator it = sockets.find(event->ident);
			if (it == sockets.end())
				continue;
			Socket *s = it->second;

			if (event->flags & EV_EOF)
			{
				socklen_t sz = sizeof(errno);
				getsockopt(s->fd, SOL_SOCKET, SO_ERROR, &errno, &sz);
				s->ProcessError();
				delete s;
				continue;
			}

			if (event->filter == EVFILT_READ && !s->ProcessRead())
			{
				ModifySocket(s, s->flags & ~SF_WANT_READ);
				s->flags |= SF_DEAD;
			}
			else if (event->filter == EVFILT_WRITE && !s->ProcessWrite())
			{
				ModifySocket(s, s->flags & ~SF_WANT_WRITE);
				s->flags |= SF_DEAD;
			}

			if (s->flags & SF_DEAD && !(s->flags & SF_WANT_WRITE))
				delete s;
		}
}

