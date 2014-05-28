#include "sinkhole.hpp"
#include "network.hpp"
#include "io.hpp"
#include "string.hpp"
#include "include/ftp.hpp"
#include "include/client.hpp"
#include "include/command.hpp"

using namespace Sinkhole::Protocol::FTP;

class CommandCdup : public Command
{
 public:
	CommandCdup() : Command("CDUP")
	{
	}

	void Execute(FTPServer *, FTPClient *c, const std::vector<std::string> &)
	{
		try
		{
			c->cwd.Chdir("..");
			c->WriteCode(250, "Directory successfully changed.");
		}
		catch (const FTPException &)
		{
			c->WriteCode(550, "Failed to change directory.");
		}
	}
} cmd_cdup;

