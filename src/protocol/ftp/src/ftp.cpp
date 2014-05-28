#include "sinkhole.hpp"
#include "conf.hpp"
#include "network.hpp"
#include "io.hpp"
#include "logger.hpp"
#include "module.hpp"
#include "string.hpp"
#include "include/ftp.hpp"
#include "include/access.hpp"
#include "include/class.hpp"
#include "include/server.hpp"
#include "include/listener.hpp"
#include "include/user.hpp"

using namespace Sinkhole::Modules;
using namespace Sinkhole::Protocol::FTP;

std::vector<FTPServer *> Sinkhole::Protocol::FTP::servers;

class ProtocolFTP : public Module
{
 public:
	ProtocolFTP(const std::string &modname) : Module(modname)
	{
		for (int i = 0, j = Sinkhole::Config->CountBlock("ftp"); i < j; ++i)
		{
			Sinkhole::ConfigurationBlock &b = Sinkhole::Config->GetBlock("ftp", i);

			try
			{
				std::string name = b.GetValue("name");
				std::string banner = b.GetValue("banner");

				FTPServer *s = FTPServer::Find(name);
				if (s == NULL)
				{
					s = new FTPServer(name);
					Sinkhole::Log(Sinkhole::LOG_INFORMATIONAL) << "FTP server " << name << " starting up";
				}

				s->name = name;
				s->banner = banner;

				for (int j = 0, k = b.CountBlock("listen"); j < k; ++j)
				{
					Sinkhole::ConfigurationBlock &lb = b.GetBlock("listen", j);

					try
					{
						std::string addr = lb.GetValue("addr");
						int port = atoi(lb.GetValue("port").c_str());
						if (port < 0 || port > 65535)
							throw Sinkhole::ConfigException("Invalid port");

						s->listeners.push_back(new FTPListener(s, addr, port));
					}
					catch (const Sinkhole::SocketException &ex)
					{
						Sinkhole::Log(Sinkhole::LOG_ERROR) << "Error binding to address " << lb.GetValue("addr") << ":" << lb.GetValue("port") << ": " << ex.GetReason();
					}
					catch (const Sinkhole::ConfigException &ex)
					{
						Sinkhole::Log(Sinkhole::LOG_ERROR) << "Error loading listen block number " << j << " for " << name << ": " << ex.GetReason();
					}
				}

				for (int j = 0, k = b.CountBlock("access"); j < k; ++j)
				{
					Sinkhole::ConfigurationBlock &ab = b.GetBlock("access", j);

					try
					{
						std::string name = ab.GetValue("name");
						std::string commands = ab.GetValue("commands");

						Sinkhole::sepstream sep(commands, ' ');
						std::string token;
						std::vector<std::string> commands_vector;
						while (sep.GetToken(token))
							commands_vector.push_back(token);

						s->access.insert(std::make_pair(name, FTPAccess(name, commands_vector)));
					}
					catch (const Sinkhole::ConfigException &ex)
					{
						Sinkhole::Log(Sinkhole::LOG_ERROR) << "Error loading access block number " << j << " for " << name << ": " << ex.GetReason();
					}
				}

				for (int j = 0, k = b.CountBlock("class"); j < k; ++j)
				{
					Sinkhole::ConfigurationBlock &cb = b.GetBlock("class");

					try
					{
						std::string cname = cb.GetValue("name");

						std::vector<Sinkhole::cidr> sources;
						for (int l = 0, m = cb.CountBlock("source"); l < m; ++l)
						{
							try
							{
								Sinkhole::ConfigurationBlock &cbs = cb.GetBlock("source", l);
								Sinkhole::cidr range(cbs.GetValue("addr"));
								sources.push_back(range);
							}
							catch (const Sinkhole::ConfigException &ex)
							{
								Sinkhole::Log(Sinkhole::LOG_ERROR) << "Error loading source block number " << l << " for " << name << " (class " << cname << "): " << ex.GetReason();
							}
						}

						s->classes.push_back(FTPClass(sources));
						FTPClass *ftpclass = &s->classes[s->classes.size() - 1];

						for (int l = 0, m = cb.CountBlock("user"); l < m; ++l)
						{
							Sinkhole::ConfigurationBlock &ub = cb.GetBlock("user", l);

							try
							{
								std::string name = ub.GetValue("name");
								std::string password;
								try { password = ub.GetValue("password"); }
								catch (const Sinkhole::ConfigException &) { }
								std::string root = ub.GetValue("root");
								FTPAccess *access = s->FindAccess(ub.GetValue("access"));
								if (access == NULL)
									throw Sinkhole::ConfigException("No access block named " + ub.GetValue("access"));

								ftpclass->users.insert(std::make_pair(name, FTPUser(name, password, root, access)));
							}
							catch (const Sinkhole::ConfigException &ex)
							{
								Sinkhole::Log(Sinkhole::LOG_ERROR) << "Error loading user block number " << l << " for " << name << " (class " << cname << "): " << ex.GetReason();
							}
						}
					}
					catch (const Sinkhole::ConfigException &ex)
					{
						Sinkhole::Log(Sinkhole::LOG_ERROR) << "Error loading class block number " << j << " for " << name << ": " << ex.GetReason();
					}
				}
			}
			catch (const Sinkhole::ConfigException &ex)
			{
				Sinkhole::Log(Sinkhole::LOG_ERROR) << "Error loading ftp block " << i << ": " << ex.GetReason();
			}
		}
	}

	~ProtocolFTP()
	{
		for (unsigned i = servers.size(); i > 0; --i)
			delete servers[i - 1];
	}
};

MODULE_INIT(ProtocolFTP)

