#ifndef HTTP_AIO_H
#define HTTP_AIO_H

HTTP_NAMESPACE_BEGIN

#include <aio.h>

class AIO : public aiocb
{
	ClientSocket *con;
 public:
	char read_buffer[1024];
	AIO(ClientSocket *c, int fd);
	~AIO();
	void Cancel();
	virtual void OnRead(const char *data, int len);
	virtual void OnError();
};

HTTP_NAMESPACE_END

#endif // HTTP_AIO_H

