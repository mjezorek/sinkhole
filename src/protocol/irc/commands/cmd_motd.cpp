#include "sinkhole.hpp"
#include "include/irc.hpp"
#include "include/mode.hpp"
#include "include/user.hpp"
#include "include/command.hpp"

using namespace Sinkhole::Protocol::IRC;

class CommandMOTD : public Command
{
 public:
	CommandMOTD() : Command("MOTD", 0)
	{
	}

	void Execute(IRCServer *, User *u, const std::vector<std::string> &)
	{
		u->WriteNumeric(376, "End of /MOTD command.");
	}
} commandmotd;

