#include "sinkhole.hpp"
#include "network.hpp"
#include "io.hpp"
#include "include/ftp.hpp"
#include "include/client.hpp"
#include "include/command.hpp"

using namespace Sinkhole::Protocol::FTP;

class CommandSyst : public Command
{
 public:
	CommandSyst() : Command("SYST")
	{
	}

	void Execute(FTPServer *, FTPClient *c, const std::vector<std::string> &)
	{
		c->WriteCode(215, "UNIX");
	}
} cmd_syst;

