#include "sinkhole.hpp"
#include "network.hpp"
#include "io.hpp"
#include "logger.hpp"
#include "string.hpp"
#include "include/ftp.hpp"
#include "include/client.hpp"
#include "include/command.hpp"
#include "include/server.hpp"
#include <fcntl.h> 
#include <unistd.h>
using namespace Sinkhole::Protocol::FTP;

class CommandDele : public Command
{
 public:
	CommandDele() : Command("DELE")
	{
	}

	void Execute(FTPServer *server, FTPClient *c, const std::vector<std::string> &params)
	{
		std::string file_path = !params.empty() ? c->cwd.GetRelativePath(params[0]) : "";
		struct stat s;

		if (!params.empty() && stat(file_path.c_str(), &s) == 0 && !S_ISDIR(s.st_mode) && !unlink(file_path.c_str()))
			c->WriteCode(250, "Delete operation successful.");
		else
			c->WriteCode(550, "Delete operation failed.");

		Sinkhole::Log(server->name, c->GetIP(), "DELE") << file_path;
	}
} cmd_dele;

