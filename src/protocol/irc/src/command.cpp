#include "sinkhole.hpp"
#include "include/irc.hpp"
#include "include/mode.hpp"
#include "include/user.hpp"
#include "include/command.hpp"

using namespace Sinkhole::Protocol::IRC;

static std::map<std::string, Command *, Sinkhole::less_ci> *commands;

Command::Command(const std::string &cmdname, size_t minparams) : name(cmdname), allow_unregistered(false), params(minparams)
{
	if (commands == NULL)
		commands = new std::map<std::string, Command *, Sinkhole::less_ci>();
	commands->insert(std::make_pair(this->name, this));
}

const std::string &Command::GetName() const
{
	return this->name;
}

void Command::AllowUnregistered()
{
	this->allow_unregistered = true;
}

bool Command::AllowsUnregistered() const
{
	return this->allow_unregistered;
}

Command *Sinkhole::Protocol::IRC::FindCommand(const std::string &command)
{
	if (commands == NULL)
		return NULL;

	std::map<std::string, Command *, Sinkhole::less_ci>::iterator it = commands->find(command);
	if (it != commands->end())
		return it->second;
	return NULL;
}

