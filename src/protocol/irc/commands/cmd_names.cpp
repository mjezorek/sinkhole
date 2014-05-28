#include "sinkhole.hpp"
#include "include/irc.hpp"
#include "include/mode.hpp"
#include "include/user.hpp"
#include "include/channel.hpp"
#include "include/command.hpp"

using namespace Sinkhole::Protocol::IRC;

class CommandNames : public Command
{
 public:
	CommandNames() : Command("NAMES", 1)
	{
	}

	void Execute(IRCServer *server, User *u, const std::vector<std::string> &params)
	{
		const std::string &channel = params[0];

		if (channel[0] != '#' && channel[0] != '&')
		{
			u->WriteNumeric(403, channel + " :No such channel");
			return;
		}

		Channel *c = server->FindChannel(channel);
		bool isonchan = c->IsOnChannel(u);
		if (c != NULL)
		{
			std::string namebuf;
			for (std::set<User *>::iterator it = c->GetUsers().begin(), it_end = c->GetUsers().end(); it != it_end; ++it)
			{
				User *user = *it;

				if (!isonchan && user->HasMode(UMODE_INVISIBLE))
					continue;

				user_status *us = c->FindUserStatus(user);
				char userprefix = us != NULL ? us->BuildModePrefix()[0] : 0;

				if (userprefix)
					namebuf += userprefix;
				namebuf += user->GetNick() + " ";
				if (namebuf.length() > 400)
				{
					u->WriteNumeric(353, "= " + c->GetName() + " :" + namebuf);
					namebuf.clear();
				}
			}

			if (!namebuf.empty())
				u->WriteNumeric(353, "= " + c->GetName() + " :" + namebuf);
		}

		u->WriteNumeric(366, c->GetName() + " :End of /NAMES list.");
	}
} commandnames;

