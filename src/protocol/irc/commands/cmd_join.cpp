#include "sinkhole.hpp"
#include "logger.hpp"
#include "include/irc.hpp"
#include "include/mode.hpp"
#include "include/user.hpp"
#include "include/channel.hpp"
#include "include/command.hpp"

using namespace Sinkhole::Protocol::IRC;

class CommandJoin : public Command
{
 public:
	CommandJoin() : Command("JOIN", 1)
	{
	}

	void Execute(IRCServer *server, User *u, const std::vector<std::string> &params)
	{
		std::string channel = params[0];
		const std::string &key = params.size() > 1 ? params[1] : "";

		if (channel[0] != '#' && channel[0] != '&')
		{
			u->WriteNumeric(403, channel + " :No such channel");
			return;
		}

		if (u->GetChannels().size() > 25)
		{
			u->WriteNumeric(405, channel + " :You may not join any more channels");
			return;
		}

		if (channel.length() > IRCServer::channellen)
			channel = channel.substr(0, IRCServer::channellen);

		Channel *c = server->FindChannel(channel);
		bool created = false;
		if (c == NULL)
		{
			c = new Channel(channel, server);
			created = true;
		}
		else
		{
			std::string mode_param;

			if (c->HasMode(CMODE_INVITEONLY))
			{
				u->WriteNumeric(473, channel + " :Cannot join channel (+i)");
				return;
			}
			else if (c->GetParam(CMODE_KEY, mode_param) && mode_param != key)
			{
				u->WriteNumeric(475, channel + " :Cannot join channel (+k)");
				return;
			}
			else if (c->GetParam(CMODE_LIMIT, mode_param) && c->GetUsers().size() >= static_cast<unsigned>(atoi(mode_param.c_str())))
			{
				u->WriteNumeric(471, channel + " :Cannot join channel (+l)");
				return;
			}
		}

		u->Join(c);
		c->Join(u);

		Sinkhole::Log(server->name, u->GetIP(), "JOIN") << c->GetName();
		c->Send(u->GetMask(), "JOIN :" + c->GetName());

		std::vector<std::string> command_params;
		command_params.push_back(c->GetName());

		if (created)
		{
			c->SetMode(CMODE_OP, u->GetNick());
			c->SetMode(CMODE_NOEXTERNAL);
			c->SetMode(CMODE_PROTECTEDTOPIC);

			Command *cmd = FindCommand("MODE");
			if (cmd != NULL)
				cmd->Execute(server, u, command_params);
		}

		if (!c->GetTopic().empty())
		{
			u->WriteNumeric(332, c->GetName() + " :" + c->GetTopic());
			u->WriteNumeric(333, c->GetName() + " " + u->GetNick() + " " + Sinkhole::stringify(c->topic_time));
		}

		Command *cmd = FindCommand("NAMES");
		if (cmd != NULL)
			cmd->Execute(server, u, command_params);
	}
} commandjoin;

