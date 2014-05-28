#include "sinkhole.hpp"
#include "include/irc.hpp"
#include "include/mode.hpp"
#include "include/user.hpp"
#include "include/command.hpp"

using namespace Sinkhole::Protocol::IRC;

class CommandNick : public Command
{
 public:
	CommandNick() : Command("NICK", 1)
	{
		this->AllowUnregistered();
	}

	void Execute(IRCServer *server, User *u, const std::vector<std::string> &params)
	{
		std::string nick = params[0];

		if (nick.length() > IRCServer::nicklen)
			nick = nick.substr(0, IRCServer::nicklen);

		User *user = server->FindUser(nick);
		if (user != NULL)
		{
			u->WriteNumeric(433, user->GetNick() + " :Nickname is already in use.");
			return;
		}

		if (!u->IsRegistered())
		{
			u->SetNick(nick);
			if (!u->GetUsername().empty())
				u->Register();
		}
		else
		{
			u->WriteCommonUsers(u->GetMask(), "NICK :" + nick);
			u->Write(u->GetMask(), "NICK :" + nick);
			u->SetNick(nick);
		}
	}
} commandnick;

