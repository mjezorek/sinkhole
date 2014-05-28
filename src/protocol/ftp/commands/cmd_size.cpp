#include "sinkhole.hpp"
#include "network.hpp"
#include "io.hpp"
#include "string.hpp"
#include "include/ftp.hpp"
#include "include/client.hpp"
#include "include/command.hpp"

using namespace Sinkhole::Protocol::FTP;

class CommandSize : public Command
{
 public:
	CommandSize() : Command("SIZE")
	{
	}

	void Execute(FTPServer *, FTPClient *c, const std::vector<std::string> &params)
	{
		std::string path = !params.empty() ? c->cwd.GetRelativePath(params[0]) : "";
		struct stat s;

		if (!params.empty() && stat(path.c_str(), &s) == 0)
			c->WriteCode(213, Sinkhole::stringify(s.st_size));
		else
			c->WriteCode(550, "Could not get file size.");
	}
} cmd_size;

