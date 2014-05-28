#include "sinkhole.hpp"
#include "network.hpp"
#include "io.hpp"
#include "logger.hpp"
#include "include/ftp.hpp"
#include "include/class.hpp"
#include "include/client.hpp"
#include "include/command.hpp"
#include "include/server.hpp"
#include "include/user.hpp"

using namespace Sinkhole::Protocol::FTP;

class CommandPass : public Command
{
 public:
	CommandPass() : Command("PASS")
	{
		this->AllowUnregistered();
	}

	void Execute(FTPServer *server, FTPClient *c, const std::vector<std::string> &params)
	{
		if (c->user)
		{
			c->WriteCode(230, "Login successful.");
			return;
		}

		const std::string &param = params.size() > 0 ? params[0] : "";

		FTPUser *user = c->ftpclass->FindUser(c->username);
		if (user == NULL || user->password != param)
		{
			Sinkhole::Log(server->name, c->GetIP(), "AUTH_FAIL") << c->username << " " << param;
			c->WriteCode(530, "Login incorrect.");
		}
		else
		{
			try
			{
				c->cwd.Init(user->root);
			}
			catch (const FTPException &ex)
			{
				Sinkhole::Log(server->name, c->GetIP(), "AUTH_FAIL") << c->username << ": " << ex.GetReason();
				c->WriteCode(530, "Unable to CHDIR to your directory root.");
				return;
			}

			c->user = user;
			Sinkhole::Log(server->name, c->GetIP(), "AUTH") << c->username << " " << param;
			c->WriteCode(230, "Login successful.");
		}
	}
} cmd_pass;

