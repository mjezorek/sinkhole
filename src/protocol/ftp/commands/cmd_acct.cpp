#include "sinkhole.hpp"
#include "network.hpp"
#include "io.hpp"
#include "include/ftp.hpp"
#include "include/client.hpp"
#include "include/command.hpp"

using namespace Sinkhole::Protocol::FTP;

class CommandAcct : public Command
{
 public:
	CommandAcct() : Command("ACCT")
	{
	}

	void Execute(FTPServer *, FTPClient *c, const std::vector<std::string> &)
	{
		c->WriteCode(502, "ACCT not implemented.");
	}
} cmd_acct;

