#include "sinkhole.hpp"
#include "network.hpp"
#include "io.hpp"
#include "include/ftp.hpp"
#include "include/client.hpp"
#include "include/command.hpp"

using namespace Sinkhole::Protocol::FTP;

class CommandUser : public Command
{
 public:
	CommandUser() : Command("USER")
	{
		this->AllowUnregistered();
	}

	void Execute(FTPServer *, FTPClient *c, const std::vector<std::string> &params)
	{
		if (c->user)
			return;

		const std::string &param = params.size() > 0 ? params[0] : "";

		c->username = param;
		c->WriteCode(331, "Please specify the password.");
	}
} cmd_user;

