#include "sinkhole.hpp"
#include "include/irc.hpp"
#include "include/mode.hpp"
#include "include/user.hpp"
#include "include/command.hpp"

using namespace Sinkhole::Protocol::IRC;

class CommandUser : public Command
{
 public:
	CommandUser() : Command("USER", 4)
	{
		this->AllowUnregistered();
	}

	void Execute(IRCServer *, User *u, const std::vector<std::string> &params)
	{
		if (u->IsRegistered())
			return;

		u->SetUsername(params[0]);
		u->SetRealname(params[3]);

		if (!u->GetNick().empty())
			u->Register();
	}
} commanduser;

