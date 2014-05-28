#include "sinkhole.hpp"
#include "network.hpp"
#include "io.hpp"
#include "pipe.hpp"
#include "string.hpp"
#include <fcntl.h>
#include <unistd.h>

using namespace Sinkhole;

PipeData::PipeData()
{
	int fds[2];
	if (pipe(fds))
		throw SocketException("Unable to create new pipe: " + LastError());
	int sockflags = fcntl(fds[1], F_GETFL, 0);
	if (fcntl(fds[1], F_SETFL, sockflags & ~O_NONBLOCK))
		throw SocketException("Unable to mark fd " + stringify(fds[1]) + " as non blocking");
	
	this->rfd = fds[0];
	this->wfd = fds[1];
}

Pipe::Pipe(PipeData d) : Socket(d.rfd), WriteFD(d.wfd)
{
}

Pipe::~Pipe()
{
	close(this->WriteFD);
}

bool Pipe::ProcessRead()
{
	static char dummy[32];
	while (read(this->fd, &dummy, sizeof(dummy)) == sizeof(dummy));
	this->OnNotify();
	return true;
}

bool Pipe::ProcessWrite()
{
	static const char dummy = '*';
	write(this->WriteFD, &dummy, 1);
	SocketEngine::ModifySocket(this, this->flags & ~SF_WANT_WRITE);
	return true;
}

void Pipe::Notify()
{
	SocketEngine::ModifySocket(this, this->flags | SF_WANT_WRITE);
}

