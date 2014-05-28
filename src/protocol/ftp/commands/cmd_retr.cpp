#include "sinkhole.hpp"
#include "network.hpp"
#include "io.hpp"
#include "logger.hpp"
#include "string.hpp"
#include "include/ftp.hpp"
#include "include/aio.hpp"
#include "include/client.hpp"
#include "include/command.hpp"
#include "include/datasocket.hpp"
#include "include/server.hpp"
#include <sys/stat.h>
#include <fcntl.h> 
#include <unistd.h>
using namespace Sinkhole::Protocol::FTP;

class CommandRetr : public Command
{
 public:
	CommandRetr() : Command("RETR")
	{
	}

	void Execute(FTPServer *server, FTPClient *c, const std::vector<std::string> &params)
	{
		if (!c->data)
		{
			c->WriteCode(425, "Use PORT or PASV first.");
			return;
		}

		if (params.empty())
		{
			c->WriteCode(550, "Failed to open file.");
			return;
		}

		std::string real_path = c->cwd.GetRelativePath(params[0]);
		if (c->data->processing_fd >= 0)
			close(c->data->processing_fd);
		c->data->processing_fd = open(real_path.c_str(), O_RDONLY | O_APPEND | O_CREAT | O_NONBLOCK, S_IRUSR | S_IWUSR);
		if (c->data->processing_fd == -1)
		{
			c->WriteCode(550, "Failed to open file.");
			return;
		}

		struct stat s;
		if (fstat(c->data->processing_fd, &s))
		{
			close(c->data->processing_fd);
			c->data->processing_fd = -1;
			c->WriteCode(550, "Failed to open file.");
			return;
		}

		c->WriteCode(150, "Opening BINARY mode data connection for " + params[0] + " (" + Sinkhole::stringify(s.st_size) + " bytes).");
		Sinkhole::Log(server->name, c->GetIP(), "RETR") << params[0];

		new AIORead(c, this, c->data->processing_fd);
	}

	void OnData(FTPClient *c, const char *data, int len)
	{
		if (c->data && c->data->processing_fd >= 0)
			c->data->Write(data, len);
	}

	void Finish(FTPClient *c)
	{
		c->WriteCode(226, "Transfer complete.");
		Sinkhole::SocketEngine::ModifySocket(c->data, c->data->flags | Sinkhole::SF_DEAD);
	}
} cmd_retr;

