#include "sinkhole.hpp"
#include "network.hpp"
#include "io.hpp"
#include "include/irc.hpp"
#include "include/io.hpp"
#include "include/mode.hpp"
#include "include/user.hpp"
#include "include/command.hpp"

using namespace Sinkhole::Protocol::IRC;

ClientSocket::ClientSocket(IRCServer *s, int fd) : Sinkhole::Socket(fd), server(s)
{
	User *u = new User(this->server);
	u->sock = this;
	this->server->AllUsers[this] = u;
}

ClientSocket::~ClientSocket()
{
	delete this->server->AllUsers[this];
	this->server->AllUsers.erase(this);
}

bool ClientSocket::ProcessRead()
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

bool ClientSocket::ProcessWrite()
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

void ClientSocket::Write(const std::string &buffer)
{
	this->writebuffer += buffer + "\r\n";
	SocketEngine::ModifySocket(this, this->flags | SF_WANT_WRITE);
}

bool ClientSocket::Process(const std::string &buffer)
{
	std::vector<std::string> parameters;
	std::string token;
	Sinkhole::sepstream tokens(buffer, ' ');

	if (!tokens.GetToken(token))
		return true;
	
	User *source = this->server->AllUsers[this];
	Command *c = FindCommand(token);

	if (!source->IsRegistered() && (c == NULL || !c->AllowsUnregistered()))
	{
		source->WriteNumeric(451, token + " :You have not registered");
		return true;
	}

	if (c == NULL)
	{
		source->WriteNumeric(421, token + " :Unknown command");
		return true;
	}

	while (tokens.GetToken(token))
	{
		if (token[0] != ':')
			parameters.push_back(token);
		else
		{
			parameters.push_back(token.substr(1) + (!tokens.StreamEnd() ? (" " + tokens.GetRemaining()) : ""));
			break;
		}
	}

	if (parameters.size() < c->params)
	{
		source->WriteNumeric(461, c->GetName() + " :Not enough parameters");
		return true;
	}

	c->Execute(this->server, source, parameters);

	return true;
}

