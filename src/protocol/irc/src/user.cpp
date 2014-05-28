#include "sinkhole.hpp"
#include "network.hpp"
#include "io.hpp"
#include "logger.hpp"
#include "include/irc.hpp"
#include "include/io.hpp"
#include "include/mode.hpp"
#include "include/user.hpp"
#include "include/channel.hpp"
#include "include/command.hpp"

using namespace Sinkhole::Protocol::IRC;

User::User(IRCServer *s) : server(s)
{
	this->registered = false;
	this->got_pong = true;
}

User::~User()
{
	if (this->registered)
		this->server->Users.erase(this->nick);
}

void User::SetNick(const std::string &newnick)
{
	if (this->registered)
		this->server->Users.erase(this->nick);
	this->nick = newnick;
	if (this->registered)
		this->server->Users[this->nick] = this;
}

const std::string &User::GetNick() const
{
	return this->nick;
}

void User::SetUsername(const std::string &newusername)
{
	this->username = newusername;
}

const std::string &User::GetUsername() const
{
	return this->username;
}

void User::SetRealname(const std::string &newrealname)
{
	this->realname = newrealname;
}

const std::string &User::GetRealname() const
{
	return this->realname;
}

const std::string &User::GetIP() const
{
	if (!this->ip_cache.empty())
		return this->ip_cache;
	
	this->ip_cache = this->sock != NULL ? this->sock->GetPeer().addr() : "255.255.255.255";
	return this->GetIP();
}

const std::string User::GetMask() const
{
	return this->GetNick() + "!" + this->GetUsername() + "@" + this->GetIP();
}

void User::Register()
{
	if (this->server->FindUser(this->nick) != NULL)
	{
		std::string wantnick = this->nick;
		this->nick.clear();
		this->WriteNumeric(433, wantnick + " :Nickname is already in use.");
		return;
	}

	this->registered = true;
	this->server->Users[this->nick] = this;

	this->WriteNumeric(1, ":Welcome to Internet Relay Chat " + this->GetMask());
	this->WriteNumeric(2, ":Your host is " + this->server->servername);
	this->WriteNumeric(3, ":This server was created " + this->server->BuildCreationTime());
	this->WriteNumeric(4, this->server->servername + " irc i mntpsiklb");
	this->WriteNumeric(5, "CASEMAPPING=rfc1459 KICKLEN=" + stringify(IRCServer::kicklen) + " MODES=4 NICKLEN=" + stringify(IRCServer::nicklen) + " PREFIX=(ov)@+ TOPICLEN=" + stringify(IRCServer::topiclen) + " NETWORK=" + this->server->networkname + " MAXLIST=b:100 MAXTARGETS=1 CHANTYPES=#& :are supported by this server");
	this->WriteNumeric(5, "CHANLIMIT=#&:" + stringify(IRCServer::maxchannels) + " CHANNELLEN=" + stringify(IRCServer::channellen) + " CHANMODES=b,k,l,imnpst AWAYLEN=160 :are supported by this server");
	Command *cmd = FindCommand("LUSERS");
	if (cmd != NULL)
		cmd->Execute(this->server, this, std::vector<std::string>());
	cmd = FindCommand("MOTD");
	if (cmd != NULL)
		cmd->Execute(this->server, this, std::vector<std::string>());
	
	Sinkhole::Log(server->name, this->GetIP(), "CONNECT") << this->nick;
}

bool User::IsRegistered() const
{
	return this->registered;
}

IRCServer *User::GetServer() const
{
	return this->server;
}

void User::SetMode(UserModeName name)
{
	this->modes[name] = true;
}

void User::RemoveMode(UserModeName name)
{
	this->modes[name] = false;
}

bool User::HasMode(UserModeName name) const
{
	return this->modes.test(name);
}

const std::string User::BuildModeString() const
{
	std::string modes = "+";
	for (size_t i = UMODE_END; i > 0; --i)
		if (this->HasMode(static_cast<UserModeName>(i - 1)))
		{
			UserMode *um = FindUserMode(static_cast<UserModeName>(i - 1));
			if (um != NULL)
				modes += um->modechar;
		}

	return modes;
}

void User::Join(Channel *c)
{
	this->channels.insert(c);
}

void User::Part(Channel *c)
{
	this->channels.erase(c);
}

bool User::IsOnChannel(Channel *c) const
{
	return this->channels.count(c) > 0;
}

const std::set<Channel *> &User::GetChannels() const
{
	return this->channels;
}

void User::Quit()
{
	for (std::set<Channel *>::iterator it = this->GetChannels().begin(), it_end = this->GetChannels().end(); it != it_end;)
	{
		Channel *c = *it;
		++it;

		this->Part(c);
		c->Part(this);
	}

	if (this->sock)
		SocketEngine::ModifySocket(this->sock, (this->sock->flags | SF_DEAD) & ~SF_WANT_READ);
	else
		delete this;
}

void User::Write(const std::string &source, const std::string &buffer)
{
	if (!this->sock)
		return;
	
	if (source.empty())
		this->sock->Write(buffer);
	else
		this->sock->Write(":" + source + " " + buffer);
}

void User::WriteNumeric(int numeric, const std::string &buffer)
{
	std::string numeric_string = stringify(numeric);
	while (numeric_string.length() < 3)
		numeric_string = "0" + numeric_string;
	std::string send_nick = this->GetNick();
	if (send_nick.empty())
		send_nick = "*";
	this->Write(this->server->servername, numeric_string + " " + send_nick + " " + buffer);

}

void User::WriteCommonUsers(const std::string &source, const std::string &buffer, User *but)
{
	for (std::map<std::string, User *, Sinkhole::less_ci>::iterator it = this->server->Users.begin(), it_end = this->server->Users.end(); it != it_end; ++it)
	{
		User *u = it->second;

		if (u == but)
			continue;

		for (std::set<Channel *>::iterator it2 = this->GetChannels().begin(), it2_end = this->GetChannels().end(); it2 != it2_end; ++it2)
			if ((*it2)->IsOnChannel(u))
			{
				u->Write(source, buffer);
				break;
			}
	}
}

