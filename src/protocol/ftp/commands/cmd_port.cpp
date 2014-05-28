#include "sinkhole.hpp"
#include "network.hpp"
#include "io.hpp"
#include "string.hpp"
#include "include/ftp.hpp"
#include "include/client.hpp"
#include "include/command.hpp"
#include "include/datasocket.hpp"
#include "include/connection.hpp"
#include "include/listener.hpp"

using namespace Sinkhole::Protocol::FTP;

class CommandPort : public Command
{
 public:
	CommandPort() : Command("PORT")
	{
	}

	void Execute(FTPServer *, FTPClient *c, const std::vector<std::string> &params)
	{
		if (params.empty())
		{
			c->WriteCode(504, "Invalid parameters.");
			return;
		}

		Sinkhole::sepstream sep(params[0], ',');
		std::string token, ip;
		unsigned short port;

		for (int i = 0; i < 4; ++i)
		{
			if (!sep.GetToken(token))
				return;
			ip = token + ".";
		}
		ip.erase(ip.length() - 1);

		if (!sep.GetToken(token))
		{
			c->WriteCode(504, "Invalid parameters.");
			return;
		}

		port = (atoi(token.c_str()) << 8);

		if (!sep.GetToken(token))
		{
			c->WriteCode(504, "Invalid parameters.");
			return;
		}

		port += atoi(token.c_str());

		try
		{
			FTPDataSocket *old = c->data;
			new FTPDataConnection(c, ip, port);
			if (old != NULL)
			{
				Sinkhole::SocketEngine::ModifySocket(old, old->flags | Sinkhole::SF_DEAD);
				old->owner = NULL;
			}
			if (c->listener != NULL)
			{
				Sinkhole::SocketEngine::ModifySocket(c->listener, c->listener->flags | Sinkhole::SF_DEAD);
				c->listener = NULL;
			}

			c->WriteCode(200, "PORT command successful.");
		}
		catch (const Sinkhole::SocketException &ex)
		{
			c->WriteCode(504, "Invalid parameters.");
		}
	}
} cmd_port;

