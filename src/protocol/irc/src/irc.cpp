#include "sinkhole.hpp"
#include "conf.hpp"
#include "network.hpp"
#include "io.hpp"
#include "logger.hpp"
#include "module.hpp"
#include "timers.hpp"
#include "include/irc.hpp"
#include "include/io.hpp"
#include "include/listener.hpp"
#include "include/mode.hpp"
#include "include/user.hpp"

using namespace Sinkhole::Modules;
using namespace Sinkhole::Protocol::IRC;

const int IRCServer::maxchannels = 25;
const unsigned IRCServer::channellen = 52;
const unsigned IRCServer::nicklen = 32;
const unsigned IRCServer::topiclen = 160;
const unsigned IRCServer::kicklen = 160;

std::vector<IRCServer *> Sinkhole::Protocol::IRC::servers;

IRCServer::IRCServer(const std::string &n) : created(Sinkhole::curtime), name(n)
{
	servers.push_back(this);
}

IRCServer::~IRCServer()
{
	for (std::map<ClientSocket *, User *>::iterator it = this->AllUsers.begin(), it_end = this->AllUsers.end(); it != it_end;)
	{
		ClientSocket *s = it->first;
		++it;

		delete s;
	}

	for (unsigned j = this->listeners.size(); j > 0; --j)
		delete this->listeners[j - 1];
	this->listeners.clear();

	std::vector<IRCServer *>::iterator it = std::find(servers.begin(), servers.end(), this);
	if (it != servers.end())
		servers.erase(it);
}

IRCServer *IRCServer::Find(const std::string &name)
{
	for (unsigned i = 0, j = servers.size(); i < j; ++i)
		if (servers[i]->name == name)
			return servers[i];
	return NULL;
}
	User *FindUser(const std::string &name);

std::string IRCServer::BuildCreationTime()
{
	if (!this->creation_time_cache.empty())
		return this->creation_time_cache;

	char timebuf[64];
	tm creation_time = *localtime(&this->created);
	strftime(timebuf, sizeof(timebuf), "%b %e %Y at %T", &creation_time);
	this->creation_time_cache = timebuf;

	return BuildCreationTime();
}

User *IRCServer::FindUser(const std::string &name)
{
	std::map<std::string, User *, Sinkhole::less_ci>::iterator it = this->Users.find(name);
	if (it != this->Users.end())
		return it->second;
	
	return NULL;
}

Channel *IRCServer::FindChannel(const std::string &name)
{
	std::map<std::string, Channel *, Sinkhole::less_ci>::iterator it = this->Channels.find(name);
	if (it != this->Channels.end())
		return it->second;

	return NULL;
}

class PingTimer : public Sinkhole::Timer
{
 public:
	PingTimer() : Sinkhole::Timer(60, Sinkhole::curtime, true) { }

	void Tick()
	{
		for (unsigned i = 0, j = servers.size(); i < j; ++i)
			for (std::map<ClientSocket *, User *>::iterator it = servers[i]->AllUsers.begin(), it_end = servers[i]->AllUsers.end(); it != it_end;)
			{
				User *u = it->second;
				++it;

				if (u->got_pong == false)
				{
					if (u->IsRegistered())
					{
						Sinkhole::Log(u->GetServer()->name, u->GetIP(), "QUIT") << "Ping timeout";
						u->WriteCommonUsers(u->GetMask(), "QUIT :Ping timeout", u);
						u->Write("", "ERROR :Closing Link: " + u->GetNick() + "[" + u->GetIP() + "] (Ping timeout)");
					}
					else
						u->Write("", "ERROR :Registration timeout");
					u->Quit();
				}
				else
				{
					u->Write(u->GetServer()->servername, "PING :" + u->GetServer()->servername); 
					u->got_pong = false;
				}
			}
	}
};

class ProtocolIRC : public Module
{
	PingTimer pingTimer;

 public:
	ProtocolIRC(const std::string &modname) : Module(modname)
	{
		for (int i = 0, j = Sinkhole::Config->CountBlock("irc"); i < j; ++i)
		{
			Sinkhole::ConfigurationBlock &b = Sinkhole::Config->GetBlock("irc", i);

			try
			{
				std::string name = b.GetValue("name");
	
				std::string server_name = b.GetValue("server-name");
				std::string network_name = b.GetValue("network-name");
				std::string desc = b.GetValue("desc");

				IRCServer *s = IRCServer::Find(name);
				if (s == NULL)
				{
					s = new IRCServer(name);
					Sinkhole::Log(Sinkhole::LOG_INFORMATIONAL) << "IRC server " << name << " starting up";
				}

				s->servername = server_name;
				s->networkname = network_name;
				s->desc = desc;

				for (int k = 0, l = b.CountBlock("listen"); k < l; ++k)
				{
					Sinkhole::ConfigurationBlock &lb = b.GetBlock("listen", k);

					try
					{
						std::string listen_address = lb.GetValue("addr");
						int listen_port = atoi(lb.GetValue("port").c_str());
						if (listen_port <= 0 || listen_port > 65535)
							listen_port = 6667;
						bool ipv6 = lb.GetBool("ipv6");

						Sinkhole::sockaddrs addr;
						addr.pton(ipv6 ? AF_INET6 : AF_INET, listen_address, listen_port);
						s->listeners.push_back(new IRCListener(s, addr.addr(), listen_port, ipv6 ? AF_INET6 : AF_INET));
					}
					catch (const Sinkhole::SocketException &ex)
					{
						Sinkhole::Log(Sinkhole::LOG_ERROR) << "Error binding to address " << lb.GetValue("addr") << ":" << lb.GetValue("port") << ": " << ex.GetReason();
					}
					catch (const Sinkhole::ConfigException &ex)
					{
						Sinkhole::Log(Sinkhole::LOG_ERROR) << "Error loading listen block number " << k << " for " << name << ": " << ex.GetReason();
					}
				}
			}
			catch (const Sinkhole::ConfigException &ex)
			{
				Sinkhole::Log(Sinkhole::LOG_ERROR) << "Error loading irc block " << i << ": " << ex.GetReason();
			}
		}
	}

	~ProtocolIRC()
	{
		for (unsigned i = servers.size(); i > 0; --i)
			delete servers[i - 1];
		servers.clear();
	}
};

MODULE_INIT(ProtocolIRC)

