#include "sinkhole.hpp"
#include "include/irc.hpp"
#include "include/mode.hpp"
#include "include/user.hpp"
#include "include/channel.hpp"
#include "include/command.hpp"

using namespace Sinkhole::Protocol::IRC;

class CommandWho : public Command
{
	bool CanShow(User *u, User *u2)
	{
		bool show = false;
		if (!u2->HasMode(UMODE_INVISIBLE))
			show = true;
		else
			for (std::set<Channel *>::iterator it = u->GetChannels().begin(), it_end = u->GetChannels().end(); it != it_end; ++it)
			{
				Channel *c = *it;

				if (c->IsOnChannel(u) && c->IsOnChannel(u2))
				{
					show = true;
					break;
				}
			}
		return show;
	}

	void WriteWho(IRCServer *server, User *target, User *user, Channel *c)
	{
		user_status *us = c != NULL ? c->FindUserStatus(user) : NULL;
		std::string userprefix = us != NULL ? us->BuildModePrefix().substr(0, 1) : "";
		target->WriteNumeric(352, (c != NULL ? c->GetName() : "*") + " " + user->GetUsername() + " " + user->GetIP() + " " + server->servername + " " + user->GetNick() + " H" + userprefix + " :0 " + user->GetRealname());
	}

 public:
	CommandWho() : Command("WHO", 0)
	{
	}

	void Execute(IRCServer *server, User *u, const std::vector<std::string> &params)
	{
		const std::string &target = params.size() > 0 ? params[0] : "";
		bool operonly = params.size() > 1 && params[1] == "o";

		if (operonly)
			;
		else if (target.empty())
		{
			for (std::map<std::string, User *, Sinkhole::less_ci>::iterator it = server->Users.begin(), it_end = server->Users.end(); it != it_end; ++it)
			{
				User *user = it->second;
				
				if (this->CanShow(u, user))
					this->WriteWho(server, u, user, NULL);
			}
		}
		else if (target[0] == '#' || target[0] == '&')
		{
			Channel *c = server->FindChannel(target);
			if (c != NULL)
			{
				const std::set<User *> &users = c->GetUsers();
				for (std::set<User *>::const_iterator it = users.begin(), it_end = users.end(); it != it_end; ++it)
				{
					User *user = *it;

					if (this->CanShow(u, user))
						this->WriteWho(server, u, user, c);
				}
			}
		}
		else
		{
			User *user = server->FindUser(target);
			if (user != NULL)
			{
				if (this->CanShow(u, user))
					this->WriteWho(server, u, user, NULL);
			}
			else
				for (std::map<std::string, User *, Sinkhole::less_ci>::iterator it = server->Users.begin(), it_end = server->Users.end(); it != it_end; ++it)
				{
					user = it->second;

					if (Sinkhole::match(user->GetNick(), target))
						if (this->CanShow(u, user))
							this->WriteWho(server, u, user, NULL);
				}
		}

		u->WriteNumeric(315, (!target.empty() ? target : "*") + " :End of /WHO list.");
	}
} commandwho;

