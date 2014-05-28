#include "sinkhole.hpp"
#include "conf.hpp"
#include "logger.hpp"
#include "module.hpp"
#include "string.hpp"
#include "timers.hpp"
#include "include/mqueue.hpp"

using namespace Sinkhole::Logging::MQueue;

MQueueConfiguration::MQueueConfiguration(const std::string &n, const std::string &p) : name(n), path(p)
{
	this->fd = mq_open(this->path.c_str(), O_WRONLY | O_CREAT | O_NONBLOCK, S_IRUSR | S_IWUSR, NULL);
	if (this->fd == -1)
		throw MQueueException(Sinkhole::LastError());
}

MQueueConfiguration::~MQueueConfiguration()
{
	mq_close(this->fd);
}

const std::string &MQueueConfiguration::GetName() const
{
	return this->name;
}

void MQueueConfiguration::Dispatch(const std::string &message)
{
	int q = mq_send(this->fd, message.c_str(), message.length(), 0);
	if (q == -1)
		throw MQueueException(Sinkhole::LastError());
}

