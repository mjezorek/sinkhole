#include "sinkhole.hpp"
#include "string.hpp"
#include "include/ftp.hpp"
#include "include/access.hpp"

using namespace Sinkhole::Protocol::FTP;

FTPAccess::FTPAccess(const std::string &n, const std::vector<std::string> &c) : name(n), commands(c)
{
}

bool FTPAccess::Allows(const std::string &name)
{
	for (unsigned i = 0, j = this->commands.size(); i < j; ++i)
	{
		std::string command = this->commands[i];
		bool inverse = false;
		if (!command.empty() && command[0] == '~')
		{
			inverse = true;
			command.erase(command.begin());
		}
		if (Sinkhole::match(name, command))
			return !inverse;
	}

	return false;
}

