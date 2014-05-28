#include "sinkhole.hpp"
#include "network.hpp"
#include "io.hpp"
#include "include/ftp.hpp"
#include "include/client.hpp"
#include "include/command.hpp"

using namespace Sinkhole::Protocol::FTP;

class CommandNoop : public Command
{
 public:
	CommandNoop() : Command("NOOP")
	{
	}

	void Execute(FTPServer *, FTPClient *c, const std::vector<std::string> &)
	{
		c->WriteCode(200, "NOOP ok.");
	}
} cmd_noop;

