#ifndef FLATFILE_FLATFILE_H
#define FLATFILE_FLATFILE_H

#include <aio.h>
#include <string.h>

#define FLATFILE_NAMESPACE_BEGIN namespace Sinkhole { namespace Logging { namespace Flatfile {
#define FLATFILE_NAMESPACE_END } } }

FLATFILE_NAMESPACE_BEGIN

class WriteRequest;

class FlatfileConfiguration
{
 public:
	int fd;
	std::string name;
	std::string directory;
	int keeplogs;
	std::vector<WriteRequest *> requests;

	FlatfileConfiguration(const std::string &n, const std::string &d, int k);
	~FlatfileConfiguration();
};

class WriteRequest : public aiocb
{
	FlatfileConfiguration *ffconf;
	std::string buffer;
 public:
	WriteRequest(FlatfileConfiguration *f, const std::string &b);
	void Dispatch();
};

class CleanTimer : public Sinkhole::Timer
{
 public:
	CleanTimer();
	void Tick();
};

FLATFILE_NAMESPACE_END

#endif // FLATFILE_FLATFILE_H

