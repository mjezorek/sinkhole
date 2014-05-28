#ifndef FTP_DATASOCKET_H
#define FTP_DATASOCKET_H

FTP_NAMESPACE_BEGIN

class FTPClient;
class Command;

class FTPDataSocket : public Sinkhole::Socket
{
	struct DataBlock
	{
		char *buf;
		int len;

		DataBlock(const char *b, int l);
		~DataBlock();
	};

	std::deque<DataBlock *> writebuffer;
 public:
	FTPClient *owner;
	Command *processing;
	int processing_fd;

 	FTPDataSocket(FTPClient *o);
 	FTPDataSocket(FTPClient *o, int fd);
	~FTPDataSocket();
	
	bool ProcessRead();
	bool ProcessWrite();

	void Write(const char *data, int len = 0);
};

FTP_NAMESPACE_END

#endif // FTP_DATASOCKET_H
