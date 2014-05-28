#include "sinkhole.hpp"
#include "network.hpp"
#include "io.hpp"
#include "logger.hpp"
#include "string.hpp"
#include "include/ftp.hpp"
#include "include/aio.hpp"
#include "include/datasocket.hpp"
#include "include/client.hpp"
#include "include/command.hpp"
#include "include/server.hpp"
#include <fcntl.h> 
#include <unistd.h>
using namespace Sinkhole::Protocol::FTP;

class CommandAppe : public Command
{
 public:
	CommandAppe() : Command("APPE")
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
			c->WriteCode(553, "Could not create file.");
			return;
		}

		std::string real_path = c->cwd.GetRelativePath(params[0]);
		if (c->data->processing_fd >= 0)
			close(c->data->processing_fd);
		c->data->processing_fd = open(real_path.c_str(), O_WRONLY | O_APPEND | O_CREAT | O_NONBLOCK, S_IRUSR | S_IWUSR);
		if (c->data->processing_fd == -1)
		{
			c->WriteCode(553, "Could not create file.");
			return;
		}

		c->data->processing = this;
		c->WriteCode(150, "Ok to send data.");

		Sinkhole::Log(server->name, c->GetIP(), "APPE") << real_path;
	}

	void OnData(FTPClient *c, const char *data, int len)
	{
		if (c->data && c->data->processing_fd >= 0)
			new AIOWrite(c->data->processing_fd, data, len);
	}

	void Finish(FTPClient *c)
	{
		c->WriteCode(226, "Transfer complete.");
	}
} cmd_appe;

