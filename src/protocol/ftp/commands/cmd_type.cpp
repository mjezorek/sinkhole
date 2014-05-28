#include "sinkhole.hpp"
#include "network.hpp"
#include "io.hpp"
#include "include/ftp.hpp"
#include "include/client.hpp"
#include "include/command.hpp"

using namespace Sinkhole::Protocol::FTP;

class CommandType : public Command
{
 public:
	CommandType() : Command("TYPE")
	{
	}

	void Execute(FTPServer *, FTPClient *c, const std::vector<std::string> &params)
	{
		if (params.empty())
			c->Write("Using binary mode to transfer files.");
		else if (params[0] == "BINARY")
			c->WriteCode(200, "Switching to Binary mode.");
		else
			c->WriteCode(500, "Unrecognised TYPE command.");
	}
} cmd_type;

