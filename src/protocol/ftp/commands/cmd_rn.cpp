#include "sinkhole.hpp"
#include "network.hpp"
#include "io.hpp"
#include "logger.hpp"
#include "include/ftp.hpp"
#include "include/client.hpp"
#include "include/command.hpp"
#include "include/server.hpp"

using namespace Sinkhole::Protocol::FTP;

class CommandRnFr : public Command
{
 public:
	CommandRnFr() : Command("RNFR")
	{
	}

	void Execute(FTPServer *, FTPClient *c, const std::vector<std::string> &params)
	{
		if (params.empty())
		{
			c->WriteCode(550, "RNFR command failed.");
			return;
		}

		std::string real_path = c->cwd.GetRelativePath(params[0]);

		struct stat s;
		if (stat(real_path.c_str(), &s))
		{
			c->WriteCode(550, "RNFR command failed.");
			return;
		}

		c->WriteCode(350, "Ready for RNTO.");
		c->rename_store = real_path;
	}
} cmd_rnfr;

class CommandRnTo : public Command
{
 public:
	CommandRnTo() : Command("RNTO")
	{
	}

	void Execute(FTPServer *server, FTPClient *c, const std::vector<std::string> &params)
	{
		std::string from_store = c->rename_store;
		c->rename_store.clear();

		if (params.empty())
		{
			c->WriteCode(550, "RNTO command failed.");
			return;
		}

		if (from_store.empty())
		{
			c->WriteCode(503, "RNFR required first.");
			return;
		}

		std::string real_path = c->cwd.GetRelativePath(params[0]);

		if (rename(from_store.c_str(), real_path.c_str()))
		{
			c->WriteCode(550, "RNTO command failed.");
			return;
		}

		c->WriteCode(250, "Rename successful.");
		Sinkhole::Log(server->name, c->GetIP(), "RN") << from_store << " " << real_path;
	}
} cmd_rnto;

