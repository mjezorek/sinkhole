#include "sinkhole.hpp"
#include "module.hpp"
#include "string.hpp"
#include "timers.hpp"
#include "include/flatfile.hpp"
#include <sys/stat.h>
#include <sys/types.h>

#include <fcntl.h>
#include <unistd.h>

using namespace Sinkhole::Modules;
using namespace Sinkhole::Logging::Flatfile;

FlatfileConfiguration::FlatfileConfiguration(const std::string &n, const std::string &d, int k) : name(n), directory(d), keeplogs(k)
{
	this->directory = "../logs/" + this->directory;
	char strf_buffer[256];
	std::string token, dir_buf;
	Sinkhole::sepstream dirs(this->directory, '/');
	while (dirs.GetToken(token) && !dirs.StreamEnd())
	{
		strftime(strf_buffer, sizeof(strf_buffer), token.c_str(), localtime(&Sinkhole::curtime));
		if (dir_buf.empty())
			dir_buf = strf_buffer;
		else
			dir_buf += std::string("/") + strf_buffer;
		mkdir(dir_buf.c_str(), S_IRWXU);
	}

	strftime(strf_buffer, sizeof(strf_buffer), this->directory.c_str(), localtime(&Sinkhole::curtime));
	this->fd = open(strf_buffer, O_CREAT | O_APPEND | O_WRONLY, S_IRUSR | S_IWUSR);
	if (this->fd < 0)
		throw Sinkhole::Exception("Unable to open " + std::string(strf_buffer) + ": " + Sinkhole::LastError());
}

FlatfileConfiguration::~FlatfileConfiguration()
{
	close(this->fd);
	for (unsigned i = this->requests.size(); i > 0; --i)
		delete this->requests[i - 1];
	this->requests.clear();
}

