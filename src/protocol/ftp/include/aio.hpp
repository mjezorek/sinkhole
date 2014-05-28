#ifndef FTP_AIO_H
#define FTP_AIO_H

FTP_NAMESPACE_BEGIN

#include <aio.h>

class FTPClient;
class Command;

class AIO : public aiocb
{
 public:
	AIO(int fd);
	virtual ~AIO();
	void Cancel();
	virtual void OnReturn(int ret) = 0;
	virtual void OnError();
};

class AIORead : public AIO
{
	Command *command;
	char read_buffer[1024];
 public:
	FTPClient *client;
	AIORead(FTPClient *cl, Command *c, int fd);
	~AIORead();
	void OnReturn(int ret);
};

class AIOWrite : public AIO
{
 public:
	AIOWrite(int fd, const char *data, int len);
	~AIOWrite();
	void OnReturn(int ret);
};

FTP_NAMESPACE_END

#endif // FTP_AIO_H

