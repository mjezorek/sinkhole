#include "sinkhole.hpp"
#include "network.hpp"
#include "io.hpp"
#include "string.hpp"
#include "include/ftp.hpp"
#include "include/client.hpp"
#include "include/command.hpp"
#include "include/datasocket.hpp"

using namespace Sinkhole::Protocol::FTP;

class CommandList : public Command
{
 public:
	CommandList() : Command("LIST")
	{
	}

	void Execute(FTPServer *, FTPClient *c, const std::vector<std::string> &params)
	{
		if (!c->data)
		{
			c->WriteCode(425, "Use PORT or PASV first.");
			return;
		}


		c->WriteCode(150, "Here comes the directory listing.");

		try
		{
			std::vector<FTPFile> files = c->cwd.FileList(!params.empty() ? params[0] : "");
			for (unsigned i = 0, j = files.size(); i < j; ++i)
			{
				FTPFile &f = files[i];
				c->data->Write(f.GetListInfo().c_str());
			}
		}
		catch (const FTPException &) { }

		c->WriteCode(226, "Directory send OK.");

		Sinkhole::SocketEngine::ModifySocket(c->data, c->data->flags | Sinkhole::SF_DEAD);
	}
} cmd_list;

