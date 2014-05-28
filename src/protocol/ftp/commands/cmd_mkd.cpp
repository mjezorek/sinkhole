#include "sinkhole.hpp"
#include "network.hpp"
#include "io.hpp"
#include "logger.hpp"
#include "string.hpp"
#include "include/ftp.hpp"
#include "include/client.hpp"
#include "include/command.hpp"
#include "include/server.hpp"

using namespace Sinkhole::Protocol::FTP;

class CommandMkd : public Command
{
 public:
	CommandMkd() : Command("MKD")
	{
	}

	void Execute(FTPServer *server, FTPClient *c, const std::vector<std::string> &params)
	{
		try
		{
			if (params.empty())
				throw FTPException("Invalid directory name");
			c->cwd.Mkdir(params[0]);
			c->WriteCode(257, "\"" + params[0] + "\" created");
			Sinkhole::Log(server->name, c->GetIP(), "MKD") << params[0];
		}
		catch (const FTPException &)
		{
			c->WriteCode(550, "Create directory operation failed.");
		}
	}
} cmd_mkd;

