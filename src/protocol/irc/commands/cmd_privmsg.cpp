#include "sinkhole.hpp"
#include "logger.hpp"
#include "include/irc.hpp"
#include "include/mode.hpp"
#include "include/user.hpp"
#include "include/channel.hpp"
#include "include/command.hpp"

using namespace Sinkhole::Protocol::IRC;

class CommandMessage : public Command
{
 public:
	CommandMessage(const std::string &cname) : Command(cname, 2)
	{
	}

	void Execute(IRCServer *server, User *u, const std::vector<std::string> &params)
	{
		const std::string &target = params[0];
		const std::string &message = params[1];

		if (target[0] == '#' || target[0] == '&')
		{
			Channel *c = server->FindChannel(target);
			if (c == NULL)
				u->WriteNumeric(401, target + " :No such nick/channel");
			else if (c->HasMode(CMODE_NOEXTERNAL) && !c->IsOnChannel(u))
				u->WriteNumeric(404, c->GetName() + " :No external channel messages (" + c->GetName() + ")");
			else if (c->HasMode(CMODE_MODERATED) && c->FindUserStatus(u) == NULL)
				u->WriteNumeric(404, c->GetName() + " :You need voice (+v) (" + c->GetName() + ")");
			else
			{
				Sinkhole::Log(server->name, u->GetIP(), this->GetName()) << c->GetName() << " :" << message;
				c->Send(u->GetMask(), this->GetName() + " " + c->GetName() + " :" + message, u);
			}
		}
		else
		{
			User *user = server->FindUser(target);
			if (user == NULL)
				u->WriteNumeric(401, target + " :No such nick/channel");
			else
				u->Write(u->GetMask(), this->GetName() + " " + u->GetNick() + " :" + message);
		}
	}
};

class CommandPrivmsg : public CommandMessage
{
 public:
	CommandPrivmsg() : CommandMessage("PRIVMSG") { }
} commandprivmsg;

class CommandNotice : public CommandMessage
{
 public:
	CommandNotice() : CommandMessage("NOTICE") { }
} commandnotice;

