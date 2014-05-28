#include "sinkhole.hpp"
#include "network.hpp"
#include "io.hpp"
#include "string.hpp"
#include "include/http.hpp"
#include "include/header.hpp"

using namespace Sinkhole::Protocol::HTTP;

ClientSocket::DataBlock::DataBlock(const char *b, int l)
{
	this->buf = new char[l];
	this->len = l;
	memcpy(this->buf, b, l);
}

ClientSocket::DataBlock::~DataBlock()
{
	delete [] this->buf;
}

ClientSocket::ClientSocket(HTTPServer *s, int fd) : Sinkhole::Socket(fd), server(s), connected(Sinkhole::curtime)
{
	for (unsigned i = 0; i < s->classes.size(); ++i)
		for (unsigned j = 0; j < s->classes[i]->sources.size(); ++j)
			if (s->classes[i]->sources[j].match(this->GetPeer()))
			{
				this->connectclass = s->classes[i];
				s->clients.insert(this);
				return;
			}
	
	throw SocketException("Dropping connection from " + this->GetPeer().addr() + ": No matching class");
}

ClientSocket::~ClientSocket()
{
	this->server->clients.erase(this);
}

const std::string &ClientSocket::GetIP()
{
	if (!this->ip_cache.empty())
		return this->ip_cache;
	
	this->ip_cache = this->GetPeer().addr();
	return this->GetIP();
}

bool ClientSocket::ProcessRead()
{
	std::pair<char *, size_t> net_buffer = Sinkhole::NetBuffer();

	int len = recv(this->fd, net_buffer.first, net_buffer.second - 1, 0);
	if (len <= 0)
		return false;

	net_buffer.first[len] = 0;
	
	this->readbuffer += net_buffer.first;

	size_t header_end = this->readbuffer.find("\r\n\r\n");
	if (header_end != std::string::npos)
	{
		try
		{
			HTTPRequest request(this, this->readbuffer.substr(0, header_end + 2));
			this->readbuffer.erase(0, header_end + 4);
			request.Process();
		}
		catch (const HTTPException &)
		{
			HTTPAction *a = this->connectclass->action;
			HTTPHeader reply;
			reply.SetAttribute("Server", a->server_name);
			reply.SetAttribute("Connection", "close");

			std::string header_data = reply.ToString();
			this->Write(header_data.c_str(), header_data.length() + 1);

			return false;
		}
	}

	return true;
}

bool ClientSocket::ProcessWrite()
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

void ClientSocket::Write(const char *data, int len)
{
	if (len == 0)
		len = strlen(data);
	DataBlock *b = new DataBlock(data, len);
	this->writebuffer.push_back(b);
	SocketEngine::ModifySocket(this, this->flags | SF_WANT_WRITE);
}

