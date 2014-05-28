#include "sinkhole.hpp"
#include "logger.hpp"
#include "include/irc.hpp"
#include "include/mode.hpp"
#include "include/user.hpp"
#include "include/channel.hpp"
#include "include/command.hpp"

using namespace Sinkhole::Protocol::IRC;

class CommandPart : public Command
{
 public:
	CommandPart() : Command("PART", 1)
	{
	}

	void Execute(IRCServer *server, User *u, const std::vector<std::string> &params)
	{
		const std::string &channel = params[0];
		const std::string &reason = params.size() > 1 ? params[1] : "";

		if (channel[0] != '#' && channel[0] != '&')
		{
			u->WriteNumeric(403, channel + " :No such channel");
			return;
		}

		Channel *c = server->FindChannel(channel);
		if (c == NULL)
		{
			u->WriteNumeric(403, channel + " :No such channel");
			return;
		}

		if (!c->IsOnChannel(u))
		{
			u->WriteNumeric(442, c->GetName() + " :You're not on that channel");
		}

		Sinkhole::Log(server->name, u->GetIP(), "PART") << c->GetName();

		if (!reason.empty())
			c->Send(u->GetMask(), "PART " + c->GetName() + " :" + reason);
		else
			c->Send(u->GetMask(), "PART " + c->GetName());

		u->Part(c);
		c->Part(u);
	}
} commandpart;

