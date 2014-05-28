#include "sinkhole.hpp"
#include "include/irc.hpp"
#include "include/mode.hpp"
#include "include/user.hpp"
#include "include/channel.hpp"
#include "include/command.hpp"

using namespace Sinkhole::Protocol::IRC;

class CommandWhois : public Command
{
 public:
	CommandWhois() : Command("WHOIS", 1)
	{
	}

	void Execute(IRCServer *server, User *u, const std::vector<std::string> &params)
	{
		const std::string &target = params[0];

		User *user = server->FindUser(target);
		if (user == NULL)
			u->WriteNumeric(401, target + " :No suck nick/channel");
		else
		{
			u->WriteNumeric(311, user->GetNick() + " " + user->GetUsername() + " " + user->GetIP() + " * :" + user->GetRealname());
			u->WriteNumeric(379, user->GetNick() + " :is using modes " + user->BuildModeString());
			u->WriteNumeric(378, user->GetNick() + " :is connecting from " + user->GetUsername() + "@" + user->GetIP() + " " + user->GetIP());

			std::string channelbuf;
			for (std::set<Channel *>::iterator it = user->GetChannels().begin(), it_end = user->GetChannels().end(); it != it_end; ++it)
			{
				Channel *c = *it;

				bool can_show = false;
				if (!user->HasMode(UMODE_INVISIBLE))
					can_show = true;
				if (c->HasMode(CMODE_PRIVATE) || c->HasMode(CMODE_SECRET))
					can_show = false;
				if (c->IsOnChannel(u))
					can_show = true;
				if (can_show)
				{
					user_status *us = c->FindUserStatus(user);
					std::string userprefix = us != NULL ? us->BuildModePrefix().substr(0, 1) : "";

					channelbuf += userprefix + c->GetName() + " ";

					if (channelbuf.length() > 400)
					{
						u->WriteNumeric(319, u->GetNick() + " :" + channelbuf);
						channelbuf.clear();
					}
				}
			}
			if (!channelbuf.empty())
				u->WriteNumeric(319, u->GetNick() + " :" + channelbuf);

			u->WriteNumeric(312, user->GetNick() + " " + server->servername + " :" + server->desc);
		}

		u->WriteNumeric(318, (user != NULL ? user->GetNick() : "*") + " :End of /WHOIS list.");
	}
} commandwhois;

