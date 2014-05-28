#include "sinkhole.hpp"
#include "conf.hpp"
#include "logger.hpp"
#include "module.hpp"
#include "string.hpp"
#include "timers.hpp"
#include "include/mqueue.hpp"

using namespace Sinkhole::Modules;
using namespace Sinkhole::Logging::MQueue;

class LoggingMQueue : public Module
{
	std::vector<MQueueConfiguration *> configurations;

 public:
	LoggingMQueue(const std::string &modname) : Module(modname)
	{
		for (int i = 0, j = Sinkhole::Config->CountBlock("mqueue"); i < j; ++i)
		{
			Sinkhole::ConfigurationBlock &b = Sinkhole::Config->GetBlock("mqueue", i);

			try
			{
				std::string name = b.GetValue("name");
				std::string path = b.GetValue("path");

				MQueueConfiguration *mq = new MQueueConfiguration(name, path);
				this->configurations.push_back(mq);
			}
			catch (const MQueueException &ex)
			{
				Sinkhole::Log(Sinkhole::LOG_ERROR) << "Error loading mqueue block: " << ex.GetReason();
			}
			catch (const Sinkhole::ConfigException &ex)
			{
				Sinkhole::Log(Sinkhole::LOG_ERROR) << "Error loading mqueue block: " << ex.GetReason();
			}
		}
	}

	~LoggingMQueue()
	{
		for (unsigned j = this->configurations.size(); j > 0; --j)
			delete this->configurations[j - 1];
		this->configurations.clear();
	}

	void OnLog(const std::string &target, const std::string &module, const std::string &source, const std::string &action, const std::string &data)
	{
		for (unsigned j = this->configurations.size(); j > 0; --j)
		{
			MQueueConfiguration *q = this->configurations[j - 1];

			if (q->GetName() == target)
			{
				std::string message = "[" + module + "] " + source + ": " + action + " " + data; 
				try
				{
					q->Dispatch(message);
				}
				catch (const MQueueException &ex)
				{
					Sinkhole::Log(Sinkhole::LOG_ERROR) << "Error logging message to mqueue: " << ex.GetReason();
				}
				break;
			}
		}
	}
};

MODULE_INIT(LoggingMQueue)

