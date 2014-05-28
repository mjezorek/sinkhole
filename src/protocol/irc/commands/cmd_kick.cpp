#include "sinkhole.hpp"
#include "include/irc.hpp"
#include "include/mode.hpp"
#include "include/user.hpp"
#include "include/channel.hpp"
#include "include/command.hpp"

using namespace Sinkhole::Protocol::IRC;

class CommandKick : public Command
{
 public:
	CommandKick() : Command("KICK", 2)
	{
	}

	void Execute(IRCServer *server, User *u, const std::vector<std::string> &params)
	{
		const std::string &channel = params[0];
		const std::string &targnick = params[1];
		std::string reason = params.size() > 2 ? params[2] : "";

		Channel *c = server->FindChannel(channel);
		User *target = server->FindUser(targnick);
		if (c == NULL)
			u->WriteNumeric(403, channel + " :No such channel");
		else if (target == NULL)
			u->WriteNumeric(401, targnick + " :No such nick/channel");
		else if (!target->IsOnChannel(c))
			u->WriteNumeric(441, target->GetNick() + " " + c->GetName() + " :They aren't on that channel");
		else
		{
			user_status *status = c->FindUserStatus(u);
			if (status == NULL || !status->HasMode(CMODE_OP))
				u->WriteNumeric(482, c->GetName() + " :You're not a channel operator");
			else
			{
				if (reason.length() > IRCServer::kicklen)
					reason = reason.substr(0, IRCServer::kicklen);
				c->Send(u->GetMask(), "KICK " + c->GetName() + " " + target->GetNick() + " :" + reason);

				u->Part(c);
				c->Part(u);
			}
		}
	}
} commandkick;

