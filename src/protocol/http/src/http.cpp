#include "sinkhole.hpp"
#include "conf.hpp"
#include "network.hpp"
#include "io.hpp"
#include "logger.hpp"
#include "module.hpp"
#include "timers.hpp"
#include "include/http.hpp"

using namespace Sinkhole::Modules;
using namespace Sinkhole::Protocol::HTTP;

std::vector<HTTPServer *> Sinkhole::Protocol::HTTP::servers;

class ConnectionCheck : public Sinkhole::Timer
{
 public:
	ConnectionCheck() : Sinkhole::Timer(10, Sinkhole::curtime, true) { }

	void Tick()
	{
		for (unsigned i = servers.size(); i > 0; --i)
		{
			HTTPServer *s = servers[i - 1];
			for (std::set<ClientSocket *>::iterator it = s->clients.begin(), it_end = s->clients.end(); it != it_end;)
			{
				ClientSocket *sock = *it;
				++it;

				if (Sinkhole::curtime > sock->connected + s->timeout)
				{
					Sinkhole::Log(s->name, sock->GetIP(), "TIMEOUT");
					delete sock;
				}
			}
		}
	}
};

class ProtocolHTTP : public Module
{
	ConnectionCheck connectionCheck;

 public:
	ProtocolHTTP(const std::string &modname) : Module(modname)
	{
		for (int i = 0, j = Sinkhole::Config->CountBlock("http"); i < j; ++i)
		{
			Sinkhole::ConfigurationBlock &b = Sinkhole::Config->GetBlock("http", i);

			try
			{
				std::string name = b.GetValue("name");
				int timeout = atoi(b.GetValue("timeout").c_str());
				if (timeout < 0)
					timeout = 60;
	
				HTTPServer *s = HTTPServer::Find(name);
				if (s == NULL)
				{
					s = new HTTPServer(name);
					Sinkhole::Log(Sinkhole::LOG_INFORMATIONAL) << "HTTP server " << name << " starting up";
				}

				s->timeout = timeout;

				for (int k = 0, l = b.CountBlock("listen"); k < l; ++k)
				{
					Sinkhole::ConfigurationBlock &lb = b.GetBlock("listen", k);

					try
					{
						std::string listen_address = lb.GetValue("addr");
						int listen_port = atoi(lb.GetValue("port").c_str());
						if (listen_port <= 0 || listen_port > 65535)
							listen_port = 80;
						bool ipv6 = lb.GetBool("ipv6");

						Sinkhole::sockaddrs addr;
						addr.pton(ipv6 ? AF_INET6 : AF_INET, listen_address, listen_port);
						s->listeners.push_back(new HTTPListener(s, addr.addr(), listen_port, ipv6 ? AF_INET6 : AF_INET));
					}
					catch (const Sinkhole::SocketException &ex)
					{
						Sinkhole::Log(Sinkhole::LOG_ERROR) << "Error binding to address " << lb.GetValue("addr") << ":" << lb.GetValue("port") << ": " << ex.GetReason();
					}
					catch (const Sinkhole::ConfigException &ex)
					{
						Sinkhole::Log(Sinkhole::LOG_ERROR) << "Error loading listen block number " << k << " for HTTP: " << ex.GetReason();
					}
				}

				for (int k = 0, l = b.CountBlock("action"); k < l; ++k)
				{
					Sinkhole::ConfigurationBlock &ab = b.GetBlock("action", k);

					try
					{
						std::string aname = ab.GetValue("name");
						std::string sname = ab.GetValue("server-name");

						std::vector<HTTPVhost> vhosts;
						for (int m = 0, n = ab.CountBlock("vhost"); m < n; ++m)
						{
							Sinkhole::ConfigurationBlock &vb = ab.GetBlock("vhost", m);

							std::string vname = vb.GetValue("name");
							HTTPVhost::Type t = HTTPVhost::TYPE_SERVE;
							std::string data;
							try { data = vb.GetValue("serve"); }
							catch (const Sinkhole::ConfigException &)
							{
								t = HTTPVhost::TYPE_ROOT;
								try { data = vb.GetValue("root"); }
								catch (const Sinkhole::ConfigException &)
								{
									throw Sinkhole::ConfigException("You must define either serve or root");
								}
							}

							HTTPVhost vhost;
							vhost.name = vname;
							vhost.data = data;
							vhost.type = t;
							vhosts.push_back(vhost);
						}

						HTTPVhost::Type t = HTTPVhost::TYPE_SERVE;
						std::string data;
						try { data = ab.GetValue("serve"); }
						catch (const Sinkhole::ConfigException &)
						{
							t = HTTPVhost::TYPE_ROOT;
							try { data = ab.GetValue("root"); }
							catch (const Sinkhole::ConfigException &)
							{
								throw Sinkhole::ConfigException("You must define either serve or root");
							}
						}

						HTTPAction *a = new HTTPAction();
						a->vhosts = vhosts;
						a->name = aname;
						a->server_name = sname;
						a->data = data;
						a->type = t;
						s->actions.insert(std::make_pair(a->name, a));
					}
					catch (const Sinkhole::ConfigException &ex)
					{
						Sinkhole::Log(Sinkhole::LOG_ERROR) << "Error loading http:action for " << name << ": " << ex.GetReason();
					}
				}

				for (int k = 0, l = b.CountBlock("class"); k < l; ++k)
				{
					Sinkhole::ConfigurationBlock &cb = b.GetBlock("class", k);

					try
					{
						std::vector<Sinkhole::cidr> sources;
						for (int m = 0, n = cb.CountBlock("source"); m < n; ++m)
						{
							Sinkhole::ConfigurationBlock &cbs = cb.GetBlock("source", m);
							Sinkhole::cidr range(cbs.GetValue("addr"));
							sources.push_back(range);
						}

						std::string action = cb.GetValue("action");

						HTTPAction *a = s->FindAction(action);
						if (a == NULL)
							throw Sinkhole::ConfigException("Unknown action \"" + action + "\"");

						HTTPClass *c = new HTTPClass(a);
						c->sources = sources;
						s->classes.push_back(c);
					}
					catch (const Sinkhole::SocketException &ex)
					{
						Sinkhole::Log(Sinkhole::LOG_ERROR) << "Error loading http:class:source for " << name << ": " << ex.GetReason();
					}
					catch (const Sinkhole::ConfigException &ex)
					{
						Sinkhole::Log(Sinkhole::LOG_ERROR) << "Error loading http:class:source for " << name << ": " << ex.GetReason();
					}
				}
			}
			catch (const Sinkhole::ConfigException &ex)
			{
				Sinkhole::Log(Sinkhole::LOG_ERROR) << "Error loading http block " << i << ": " << ex.GetReason();
			}
		}
	}

	~ProtocolHTTP()
	{
		for (unsigned i = servers.size(); i > 0; --i)
			delete servers[i - 1];
		servers.clear();
	}

};

MODULE_INIT(ProtocolHTTP)

