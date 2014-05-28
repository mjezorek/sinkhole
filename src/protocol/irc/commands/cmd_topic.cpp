#include "sinkhole.hpp"
#include "include/irc.hpp"
#include "include/mode.hpp"
#include "include/user.hpp"
#include "include/channel.hpp"
#include "include/command.hpp"

using namespace Sinkhole::Protocol::IRC;

class CommandTopic : public Command
{
 public:
	CommandTopic() : Command("TOPIC", 1)
	{
	}

	void Execute(IRCServer *server, User *u, const std::vector<std::string> &params)
	{
		const std::string &target = params[0];

		Channel *c = server->FindChannel(target);
		if (c == NULL)
			u->WriteNumeric(403, target + " :No such channel");
		else if (params.size() == 1)
		{
			u->WriteNumeric(332, c->GetName() + " :" + c->GetTopic());
			u->WriteNumeric(333, c->GetName() + " " + u->GetNick() + " " + Sinkhole::stringify(c->topic_time));
		}
		else
		{
			user_status *status = c->FindUserStatus(u);
			if (c->HasMode(CMODE_PROTECTEDTOPIC) && (status == NULL || !status->HasMode(CMODE_OP)))
				u->WriteNumeric(482, c->GetName() + " :You're not a channel operator");
			else
			{
				std::string topic = params[1];
				if (topic.length() > IRCServer::topiclen)
					topic = topic.substr(0, IRCServer::topiclen);
				c->SetTopic(topic);
			
				c->Send(u->GetMask(), "TOPIC " + c->GetName() + " :" + topic);
			}
		}
	}
} commandtopic;

