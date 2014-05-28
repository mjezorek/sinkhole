#ifndef HTTP_IO_H
#define HTTP_IO_H

HTTP_NAMESPACE_BEGIN

class ClientSocket : public Sinkhole::Socket
{
	struct DataBlock
	{
		char *buf;
		int len;

		DataBlock(const char *b, int l);
		~DataBlock();
	};

	std::string readbuffer;
	std::deque<DataBlock *> writebuffer;

	std::string ip_cache;

 public:
	HTTPServer *server;
	HTTPClass *connectclass;
	time_t connected;

	ClientSocket(HTTPServer *s, int fd);
	~ClientSocket();

	const std::string &GetIP();

	bool ProcessRead();
	bool ProcessWrite();

	void Write(const char *data, int len = 0);
};

HTTP_NAMESPACE_END

#endif // HTTP_IO_H

