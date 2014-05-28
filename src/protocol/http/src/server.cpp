#include "sinkhole.hpp"
#include "network.hpp"
#include "io.hpp"
#include "include/http.hpp"

using namespace Sinkhole::Protocol::HTTP;

HTTPServer *HTTPServer::Find(const std::string &name)
{
	for (unsigned i = servers.size(); i > 0; --i)
	{
		HTTPServer *s = servers[i - 1];
		if (s->name == name)
			return s;
	}

	return NULL;
}

HTTPServer::HTTPServer(const std::string &n) : name(n)
{
	servers.push_back(this);
}

HTTPServer::~HTTPServer()
{
	for (std::set<ClientSocket *>::iterator it = this->clients.begin(), it_end = this->clients.end(); it != it_end;)
	{
		ClientSocket *s = *it;
		++it;
		delete s;
	}

	for (unsigned i = this->listeners.size(); i > 0; --i)
		delete this->listeners[i - 1];
	this->listeners.clear();

	for (unsigned i = this->classes.size(); i > 0; --i)
		delete this->classes[i - 1];
	this->classes.clear();

	for (std::map<std::string, HTTPAction *>::iterator it = this->actions.begin(), it_end = this->actions.end(); it != it_end; ++it)
		delete it->second;
	this->actions.clear();

	std::vector<HTTPServer *>::iterator it = std::find(servers.begin(), servers.end(), this);
	if (it != servers.end())
		servers.erase(it);
}

HTTPAction *HTTPServer::FindAction(const std::string &name) const
{
	std::map<std::string, HTTPAction *>::const_iterator it = this->actions.find(name);
	if (it != this->actions.end())
		return it->second;
	return NULL;
}

