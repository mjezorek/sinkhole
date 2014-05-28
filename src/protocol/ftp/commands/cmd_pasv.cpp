#include "sinkhole.hpp"
#include "network.hpp"
#include "io.hpp"
#include "string.hpp"
#include "include/ftp.hpp"
#include "include/client.hpp"
#include "include/command.hpp"
#include "include/datasocket.hpp"
#include "include/listener.hpp"

using namespace Sinkhole::Protocol::FTP;

class CommandPasv : public Command
{
 public:
	CommandPasv() : Command("PASV")
	{
	}

	void Execute(FTPServer *, FTPClient *c, const std::vector<std::string> &)
	{
		try
		{
			if (c->listener == NULL)
				c->listener = new FTPDataListener(c, c->GetSock().addr());

			Sinkhole::sockaddrs &bind_host = c->listener->GetSock();

			if (bind_host.sa.sa_family != AF_INET)
				throw Sinkhole::SocketException("Invalid address family");

			unsigned long bind_addr = bind_host.sa4.sin_addr.s_addr;
			short bind_port = bind_host.port() & 0xFFFF;

			unsigned char i4 = bind_addr >> 24, i3 = bind_addr >> 16, i2 = bind_addr >> 8, i1 = bind_addr & 0xFF;
			unsigned char p1 = bind_port >> 8, p2 = bind_port & 0xFF;

			c->WriteCode(227, Sinkhole::printf("Entering Passive Mode (%d,%d,%d,%d,%d,%d).", i1, i2, i3, i4, p1, p2));
		}
		catch (const Sinkhole::SocketException &)
		{
			c->WriteCode(502, "PASV not implemented.");
		}
	}
} cmd_pasv;

