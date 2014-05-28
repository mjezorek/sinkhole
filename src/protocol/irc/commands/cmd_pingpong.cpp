#include "sinkhole.hpp"
#include "include/irc.hpp"
#include "include/mode.hpp"
#include "include/user.hpp"
#include "include/command.hpp"

using namespace Sinkhole::Protocol::IRC;

class CommandPing : public Command
{
 public:
	CommandPing() : Command("PING", 1)
	{
	}

	void Execute(IRCServer *server, User *u, const std::vector<std::string> &params)
	{
		u->Write(server->servername, "PONG " + server->servername + ":" + params[0]);
	}
} commandping;

class CommandPong : public Command
{
 public:
	CommandPong() : Command("PONG", 0)
	{
	}

	void Execute(IRCServer *, User *u, const std::vector<std::string> &)
	{
		u->got_pong = true;
	}
} commandpong;

