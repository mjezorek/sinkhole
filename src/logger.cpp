#include "sinkhole.hpp"
#include "conf.hpp"
#include "logger.hpp"
#include "module.hpp"
#include "string.hpp"

#include <syslog.h>

using namespace Sinkhole;

Sinkhole::Log::Log(Sinkhole::LogLevel l) : level(l)
{
}

Sinkhole::Log::Log(const std::string &m, const std::string &s, const std::string &a) : level(LOG_NORMAL), module(m), source(s), action(a)
{
}

Sinkhole::Log::~Log()
{
	if (this->level == LOG_DEBUG && debug == false)
		return;
	if (nofork)
	{
		if (!this->module.empty())
			std::cout << "[" << this->module << "] ";
		if (!this->source.empty())
			std::cout << this->source << ": ";
		if (!this->action.empty())
			std::cout << this->action << " ";
		if (!this->buf.str().empty())
			std::cout << this->buf.str();
		std::cout << std::endl;
	}
	if (this->level == LOG_NORMAL)
	{
		try
		{
			size_t sl = this->module.find('/');
			std::string transport = this->module.substr(0, sl != std::string::npos ? sl : this->module.length());
			std::string log_block_name;

			for (int i = 0, j = Config->CountBlock(transport); i < j; ++i)
			{
				ConfigurationBlock &b = Config->GetBlock(transport, i);
	
				if (this->module == b.GetValue("name"))
				{
					log_block_name = b.GetValue("log");
					break;
				}
			}
			if (log_block_name.empty())
				return;

			for (int i = 0, j = Config->CountBlock("log"); i < j; ++i)
			{
				ConfigurationBlock &b = Config->GetBlock("log", i);

				if (log_block_name == b.GetValue("name"))
				{
					const std::string &filters = b.GetValue(transport);
					std::string token;
					sepstream sep(filters, ' ');

					bool fmatch = false;
					while (sep.GetToken(token))
						if (match(this->action, token))
						{
							fmatch = true;
							break;
						}

					if (fmatch)
					{
						const std::string &transports = b.GetValue("targets");
						sepstream sep2(transports, ' ');

						while (sep2.GetToken(token))
						{
							FOREACH_MOD(OnLog(token, this->module, this->source, this->action, this->buf.str()));
						}
					}
	
					break;
				}
			}
		}
		catch (const ConfigException &ex)
		{
			Log(LOG_ERROR) << "Error processing log message: " << ex.GetReason();
		}
	}
	else if (this->level != LOG_DEBUG)
	{
		syslog(static_cast<int>(this->level), "%s", this->buf.str().c_str());
	}
}

