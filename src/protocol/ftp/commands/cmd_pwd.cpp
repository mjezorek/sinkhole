#include "sinkhole.hpp"
#include "network.hpp"
#include "io.hpp"
#include "string.hpp"
#include "include/ftp.hpp"
#include "include/client.hpp"
#include "include/command.hpp"

using namespace Sinkhole::Protocol::FTP;

class CommandPwd : public Command
{
 public:
	CommandPwd() : Command("PWD")
	{
	}

	void Execute(FTPServer *, FTPClient *c, const std::vector<std::string> &)
	{
		c->WriteCode(257, "\"" + Sinkhole::replace(c->cwd.GetCWD(), "\\", "\\\\") + "\"");
	}
} cmd_pwd;

