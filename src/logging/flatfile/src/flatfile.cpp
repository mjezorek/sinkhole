#include "sinkhole.hpp"
#include "conf.hpp"
#include "logger.hpp"
#include "module.hpp"
#include "string.hpp"
#include "timers.hpp"
#include "include/flatfile.hpp"
#include <errno.h>

#include <fcntl.h>
#include <unistd.h>

using namespace Sinkhole::Modules;
using namespace Sinkhole::Logging::Flatfile;

class LoggingFlatfile : public Module, public Sinkhole::Timer
{
	std::vector<FlatfileConfiguration *> configurations;

	void Clean()
	{
		for (unsigned i = configurations.size(); i > 0; --i)
		{
			FlatfileConfiguration *f = this->configurations[i - 1];

			while (!f->requests.empty())
			{
				WriteRequest *req = f->requests[0];
				int err = aio_error(req);
				if (err == EINPROGRESS)
					break;

				if (err != 0)
				{
					errno = err;
					Sinkhole::Log(Sinkhole::LOG_ERROR) << "Unable to write to logfile: " << Sinkhole::LastError();
				}

				delete req;
				f->requests.erase(f->requests.begin());
			}
		}
	}

 public:
	LoggingFlatfile(const std::string &modname) : Module(modname), Sinkhole::Timer(86400, Sinkhole::curtime, true)
	{
		for (int i = 0, j = Sinkhole::Config->CountBlock("flatfile"); i < j; ++i)
		{
			Sinkhole::ConfigurationBlock &b = Sinkhole::Config->GetBlock("flatfile", i);

			try
			{
				std::string name = b.GetValue("name");
				std::string dir = b.GetValue("directory");
				int keep = atoi(b.GetValue("keeplogs").c_str());
				if (keep < 0)
					keep = 0;

				try
				{
					FlatfileConfiguration *f = new FlatfileConfiguration(name, dir, keep);
					this->configurations.push_back(f);
				}
				catch (const Sinkhole::Exception &ex)
				{
					Sinkhole::Log(Sinkhole::LOG_ERROR) << ex.GetReason();
				}
			}
			catch (const Sinkhole::ConfigException &ex)
			{
				Sinkhole::Log(Sinkhole::LOG_ERROR) << "Error loading flatfile block: " << ex.GetReason();
			}
		}
	}

	~LoggingFlatfile()
	{
		for (unsigned j = this->configurations.size(); j > 0; --j)
			delete this->configurations[j - 1];
		this->configurations.clear();
	}

	void Tick()
	{
		for (unsigned j = this->configurations.size(); j > 0; --j)
		{
			FlatfileConfiguration *c = this->configurations[j - 1];

			if (!c->keeplogs)
				continue;

			time_t t = time(NULL) - (86400 * (c->keeplogs + 1));
			char strf_buffer[256];
			
			strftime(strf_buffer, sizeof(strf_buffer), c->directory.c_str(), localtime(&t));
			if (unlink(strf_buffer) == 0)
				Sinkhole::Log(Sinkhole::LOG_INFORMATIONAL) << "Deleting old logfile " << strf_buffer;
		}
	}

	void OnLog(const std::string &target, const std::string &module, const std::string &source, const std::string &action, const std::string &data)
	{
		for (unsigned j = this->configurations.size(); j > 0; --j)
		{
			FlatfileConfiguration *c = this->configurations[j - 1];

			if (c->name == target)
			{
				this->Clean();

				std::string message = ctime(&Sinkhole::curtime);
				Sinkhole::strip(message);

				message += " [" + module + "] " + source + ": " + action + " " + data; 
				WriteRequest *req = new WriteRequest(c, message);
				try
				{
					req->Dispatch();
					c->requests.push_back(req);
				}
				catch (const Sinkhole::Exception &ex)
				{
					Sinkhole::Log(Sinkhole::LOG_ERROR) << "Error queing log message: " << ex.GetReason();
				}
				break;
			}
		}
	}
};

MODULE_INIT(LoggingFlatfile)

