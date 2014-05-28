#include "sinkhole.hpp"
#include "network.hpp"
#include "io.hpp"
#include "include/ftp.hpp"
#include "include/client.hpp"
#include "include/command.hpp"

using namespace Sinkhole::Protocol::FTP;

class CommandQuit : public Command
{
 public:
	CommandQuit() : Command("QUIT")
	{
		this->AllowUnregistered();
	}

	void Execute(FTPServer *, FTPClient *c, const std::vector<std::string> &)
	{
		c->WriteCode(221, "Goodbye.");
		using namespace Sinkhole;
		SocketEngine::ModifySocket(c, (c->flags | SF_DEAD) & ~SF_WANT_READ);
	}
} cmd_quit;

