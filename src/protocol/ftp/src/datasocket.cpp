#include "sinkhole.hpp"
#include "network.hpp"
#include "io.hpp"
#include "include/ftp.hpp"
#include "include/command.hpp"
#include "include/client.hpp"
#include "include/datasocket.hpp"
#include <errno.h>
#include <unistd.h>

using namespace Sinkhole;
using namespace Sinkhole::Protocol::FTP;

FTPDataSocket::DataBlock::DataBlock(const char *b, int l)
{
	this->buf = new char[l];
	this->len = l;
	memcpy(this->buf, b, l);
}

FTPDataSocket::DataBlock::~DataBlock()
{
	delete [] this->buf;
}

FTPDataSocket::FTPDataSocket(FTPClient *o) : Sinkhole::Socket(AF_INET, SOCK_STREAM), owner(o)
{
	o->data = this;
}

FTPDataSocket::FTPDataSocket(FTPClient *o, int fd) : Sinkhole::Socket(fd), owner(o)
{
	o->data = this;
	this->processing = NULL;
	this->processing_fd = -1;
}

FTPDataSocket::~FTPDataSocket()
{
	if (this->processing_fd >= 0)
	{
		close(this->processing_fd);
		this->processing_fd = -1;
	}
	if (this->processing != NULL)
	{
		this->processing->Finish(this->owner);
		this->processing = NULL;
	}
	if (this->owner != NULL)
		this->owner->data = NULL;
}

bool FTPDataSocket::ProcessRead()
{
	if (this->owner == NULL || this->processing == NULL)
		return false;

	std::pair<char *, size_t> net_buffer = Sinkhole::NetBuffer();

	int len = recv(this->fd, net_buffer.first, net_buffer.second - 1, 0);
	if (len <= 0)
		return false;
	net_buffer.first[len] = 0;

	this->processing->OnData(this->owner, net_buffer.first, len);
	return true;
}

bool FTPDataSocket::ProcessWrite()
{
	if (this->writebuffer.empty())
	{
		SocketEngine::ModifySocket(this, this->flags & ~SF_WANT_WRITE);
		return true;
	}

	DataBlock *b = this->writebuffer.front();
	int len = send(this->fd, b->buf, b->len, 0);
	if (len == -1)
		return false;
	else if (len == b->len)
	{
		delete b;
		this->writebuffer.pop_front();
	}
	else
	{
		b->buf += len;
		b->len -= len;
	}
	if (this->writebuffer.empty())
		SocketEngine::ModifySocket(this, this->flags & ~SF_WANT_WRITE);

	return true;
}

void FTPDataSocket::Write(const char *data, int len)
{
	if (len == 0)
		len = strlen(data);
	DataBlock *b = new DataBlock(data, len);
	this->writebuffer.push_back(b);
	SocketEngine::ModifySocket(this, this->flags | SF_WANT_WRITE);
}

