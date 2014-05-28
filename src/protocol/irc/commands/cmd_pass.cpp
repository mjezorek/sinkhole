#include "sinkhole.hpp"
#include "logger.hpp"
#include "include/irc.hpp"
#include "include/mode.hpp"
#include "include/user.hpp"
#include "include/command.hpp"

using namespace Sinkhole::Protocol::IRC;

class CommandPass : public Command
{
 public:
	CommandPass() : Command("PASS", 1)
	{
	}

	void Execute(IRCServer *server, User *u, const std::vector<std::string> &params)
	{
		Sinkhole::Log(server->name, u->GetIP(), "PASS") << params[0];
	}
} commandpass;
