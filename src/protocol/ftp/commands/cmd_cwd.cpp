#include "sinkhole.hpp"
#include "network.hpp"
#include "io.hpp"
#include "string.hpp"
#include "include/ftp.hpp"
#include "include/client.hpp"
#include "include/command.hpp"

using namespace Sinkhole::Protocol::FTP;

class CommandCwd : public Command
{
 public:
	CommandCwd() : Command("CWD")
	{
	}

	void Execute(FTPServer *, FTPClient *c, const std::vector<std::string> &params)
	{
		try
		{
			if (params.empty())
				throw FTPException("Invalid directory");
			c->cwd.Chdir(params[0]);
			c->WriteCode(250, "Directory successfully changed.");
		}
		catch (const FTPException &)
		{
			c->WriteCode(550, "Failed to change directory.");
		}

	}
} cmd_cwd;

