#ifndef IRC_COMMANDR_H
#define IRC_COMMAND_H

IRC_NAMESPACE_BEGIN

class Command
{
	std::string name;
	bool allow_unregistered;
 public:
	size_t params;

	Command(const std::string &cmdname, size_t minparams = 0);

	const std::string &GetName() const;

	void AllowUnregistered();
	bool AllowsUnregistered() const;

	virtual void Execute(IRCServer *server, User *u, const std::vector<std::string> &params) = 0;
};

Command *FindCommand(const std::string &name);

IRC_NAMESPACE_END

#endif // IRC_COMMAND_H

