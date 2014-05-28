#include "sinkhole.hpp"
#include "logger.hpp"
#include "include/irc.hpp"
#include "include/mode.hpp"
#include "include/user.hpp"
#include "include/command.hpp"

using namespace Sinkhole::Protocol::IRC;

class CommandQuit : public Command
{
 public:
	CommandQuit() : Command("QUIT", 0)
	{
		this->AllowUnregistered();
	}

	void Execute(IRCServer *server, User *u, const std::vector<std::string> &params)
	{
		const std::string &reason = (!params.empty() ? params[0] : "");

		if (u->IsRegistered())
		{
			Sinkhole::Log(server->name, u->GetIP(), "QUIT") << reason;
			u->WriteCommonUsers(u->GetMask(), "QUIT :" + reason, u);
		}
		u->Write("", "ERROR :Closing Link: " + u->GetNick() + "[" + u->GetIP() + "] (Quit: " + reason + ")");
		u->Quit();
	}
} commandquit;

