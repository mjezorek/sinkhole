#ifndef MQUEUE_MQUEUE_H
#define MQUEUE_MQUEUE_H

#define MQUEUE_NAMESPACE_BEGIN namespace Sinkhole { namespace Logging { namespace MQueue {
#define MQUEUE_NAMESPACE_END } } }

#include <mqueue.h>

MQUEUE_NAMESPACE_BEGIN

class MQueueException : public Exception
{
 public:
	MQueueException(const std::string &r) : Exception(r) { }
};

class MQueueConfiguration
{
	mqd_t fd;
	std::string name;
	std::string path;
 public:
 	MQueueConfiguration(const std::string &n, const std::string &p);
	~MQueueConfiguration();
	const std::string &GetName() const;
	void Dispatch(const std::string &message);
};

MQUEUE_NAMESPACE_END

#endif
