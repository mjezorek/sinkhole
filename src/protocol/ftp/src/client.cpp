#include "sinkhole.hpp"
#include "network.hpp"
#include "io.hpp"
#include "string.hpp"
#include "include/ftp.hpp"
#include "include/access.hpp"
#include "include/aio.hpp"
#include "include/class.hpp"
#include "include/client.hpp"
#include "include/server.hpp"
#include "include/command.hpp"
#include "include/user.hpp"
#include "include/datasocket.hpp"
#include "include/listener.hpp"

using namespace Sinkhole::Protocol::FTP;

FTPClient::FTPClient(FTPServer *s, int fd) : Sinkhole::Socket(fd), server(s)
{
	this->data = NULL;
	this->listener = NULL;
	this->read_request = NULL;
	this->user = NULL;

	for (unsigned i = s->classes.size(); i > 0; --i)
	{
		FTPClass &c = s->classes[i - 1];
		for (unsigned j = c.sources.size(); j > 0; --j)
		{
			Sinkhole::cidr &range = c.sources[j - 1];
			if (range.match(this->GetPeer()))
			{
				this->ftpclass = &c;
				this->server->users.insert(this);
				return;
			}
		}
	}

	throw SocketException("Dropping connection from " + this->GetPeer().addr() + ": No matching class");
}

FTPClient::~FTPClient()
{
	if (this->read_request != NULL)
	{
		this->read_request->Cancel();
		this->read_request->client = NULL;
	}
	if (this->data != NULL)
	{
		Sinkhole::SocketEngine::ModifySocket(this->data, this->data->flags | Sinkhole::SF_DEAD);
		this->data->owner = NULL;
	}
	if (this->listener != NULL)
	{
		Sinkhole::SocketEngine::ModifySocket(this->listener, this->listener->flags | Sinkhole::SF_DEAD);
		this->listener->owner = NULL;
	}
	this->server->users.erase(this);
}

void FTPClient::Destroy()
{
	if (this->read_request != NULL)
	{
		this->read_request->Cancel();
		this->read_request->client = NULL;
	}

	delete this->data;
	this->data = NULL;

	delete this->listener;
	this->listener = NULL;

	delete this;
}

bool FTPClient::ProcessRead()
{
	std::pair<char *, size_t> net_buffer = Sinkhole::NetBuffer();

	int len = recv(this->fd, net_buffer.first, net_buffer.second - 1, 0);
	if (len <= 0)
		return false;

	net_buffer.first[len] = 0;
	
	std::string buffer = this->extra;
	buffer += net_buffer.first;
	this->extra.clear();

	size_t lnl = buffer.rfind('\n');
	if (lnl == std::string::npos)
	{
		this->extra = buffer;
		return true;
	}
	else if (lnl < buffer.size() - 1)
	{
		this->extra = buffer.substr(lnl);
		buffer = buffer.substr(0, lnl);
	}

	std::string token;
	Sinkhole::sepstream tokens(buffer, '\n');
	while (tokens.GetToken(token))
	{
		Sinkhole::strip(token);
		if (!token.empty() && !this->Process(token))
			return false;
	}

	return true;
}

bool FTPClient::ProcessWrite()
{
	if (this->writebuffer.empty())
	{
		SocketEngine::ModifySocket(this, this->flags & ~SF_WANT_WRITE);
		return true;
	}
	
	int len = send(this->fd, this->writebuffer.c_str(), this->writebuffer.length(), 0);
	if (len == -1)
		return false;
	this->writebuffer = this->writebuffer.substr(len);
	if (this->writebuffer.empty())
		SocketEngine::ModifySocket(this, this->flags & ~SF_WANT_WRITE);

	return true;
}

void FTPClient::Write(const std::string &buffer)
{
	this->writebuffer += buffer + "\r\n";
	SocketEngine::ModifySocket(this, this->flags | SF_WANT_WRITE);
}

bool FTPClient::Process(const std::string &buffer)
{
	std::vector<std::string> parameters;
	std::string token;
	Sinkhole::sepstream tokens(buffer, ' ');

	if (!tokens.GetToken(token))
		return true;
	
	Command *c = FindCommand(token);

	if (!this->user && (c == NULL || !c->AllowsUnregistered()))
	{
		this->WriteCode(530, "Please login with USER and PASS.");
		return true;
	}

	if (c == NULL)
	{
		this->WriteCode(500, "Unknown command.");
		return true;
	}

	if (this->user != NULL)
	{
		FTPAccess *access = this->user->access;
		if (!access->Allows(c->GetName()))
		{
			this->WriteCode(550, "Access denied.");
			return true;
		}
	}

	while (tokens.GetToken(token))
		parameters.push_back(token);

	c->Execute(this->server, this, parameters);

	return true;
}

const std::string &FTPClient::GetIP()
{
	if (!this->ip_cache.empty())
		return this->ip_cache;
	
	this->ip_cache = this->GetPeer().addr();
	return this->GetIP();
}

void FTPClient::WriteCode(int code, const std::string &buffer)
{
	this->Write(stringify(code) + " " + buffer);
}

