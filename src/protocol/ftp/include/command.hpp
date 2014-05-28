#ifndef FTP_COMMANDR_H
#define FTP_COMMAND_H

FTP_NAMESPACE_BEGIN

class FTPServer;
class FTPClient;

class Command
{
	std::string name;
	bool allow_unregistered;
 public:
	Command(const std::string &cmdname);

	const std::string &GetName() const;

	void AllowUnregistered();
	bool AllowsUnregistered() const;

	virtual void Execute(FTPServer *server, FTPClient *c, const std::vector<std::string> &params) = 0;
	virtual void OnData(FTPClient *, const char *, int) { }
	virtual void Finish(FTPClient *) { }
};

Command *FindCommand(const std::string &name);

FTP_NAMESPACE_END

#endif // FTP_COMMAND_H

