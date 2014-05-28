#include "sinkhole.hpp"
#include "include/irc.hpp"
#include "include/mode.hpp"
#include "include/user.hpp"
#include "include/channel.hpp"
#include "include/command.hpp"

using namespace Sinkhole::Protocol::IRC;

class CommandLUsers : public Command
{
	unsigned MaxUsers;
 public:
	CommandLUsers() : Command("LUSERS", 0)
	{
		this->MaxUsers = 0;
	}

	void Execute(IRCServer *server, User *u, const std::vector<std::string> &)
	{
		unsigned UserCount = server->Users.size();
		if (UserCount > this->MaxUsers)
			this->MaxUsers = UserCount;
		std::string UserCountS = Sinkhole::stringify(UserCount),
				MaxUserCountS = Sinkhole::stringify(this->MaxUsers);

		u->WriteNumeric(251, ":There are " + UserCountS + " users on 1 servers");
		u->WriteNumeric(254, server->Channels.size() + ":channels formed");
		u->WriteNumeric(255, ":I have " + UserCountS + " clients and 0 servers");
		u->WriteNumeric(265, ":Current Local Users: " + UserCountS + "  Max: " + MaxUserCountS);
		u->WriteNumeric(266, ":Current Global Users: " + UserCountS + "  Max: " + MaxUserCountS);
	}
} commandlusers;

