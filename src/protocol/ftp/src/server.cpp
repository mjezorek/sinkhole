#include "sinkhole.hpp"
#include "network.hpp"
#include "io.hpp"
#include "include/ftp.hpp"
#include "include/access.hpp"
#include "include/client.hpp"
#include "include/class.hpp"
#include "include/listener.hpp"
#include "include/user.hpp"
#include "include/server.hpp"

using namespace Sinkhole::Protocol::FTP;

FTPServer *FTPServer::Find(const std::string &name)
{
	for (unsigned i = 0, j = servers.size(); i < j; ++i)
	{
		FTPServer *s = servers[i];
		if (s->name == name)
			return s;
	}

	return NULL;
}

FTPServer::FTPServer(const std::string &n) : name(n)
{
	servers.push_back(this);
}

FTPServer::~FTPServer()
{
	for (std::set<FTPClient *>::iterator it = this->users.begin(), it_end = this->users.end(); it != it_end;)
	{
		FTPClient *c = *it++;
		c->Destroy();
	}
	for (unsigned i = this->listeners.size(); i > 0; --i)
		delete this->listeners[i - 1];
	
	std::vector<FTPServer *>::iterator it = std::find(servers.begin(), servers.end(), this);
	if (it != servers.end())
		servers.erase(it);
}

FTPAccess *FTPServer::FindAccess(const std::string &n)
{
	std::map<std::string, FTPAccess>::iterator it = this->access.find(n);
	if (it != this->access.end())
		return &it->second;
	return NULL;
}

