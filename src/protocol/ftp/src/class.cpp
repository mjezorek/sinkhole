#include "sinkhole.hpp"
#include "network.hpp"
#include "include/ftp.hpp"
#include "include/class.hpp"
#include "include/user.hpp"

using namespace Sinkhole::Protocol::FTP;

FTPClass::FTPClass(const std::vector<Sinkhole::cidr> &s) : sources(s)
{
}

FTPUser *FTPClass::FindUser(const std::string &name)
{
	std::map<std::string, FTPUser>::iterator it = this->users.find(name);
	if (it != this->users.end())
		return &it->second;
	return NULL;
}

