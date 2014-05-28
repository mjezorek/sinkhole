#ifndef IRC_IO_H
#define IRC_IO_H

IRC_NAMESPACE_BEGIN

class ClientSocket : public Sinkhole::Socket
{
	IRCServer *server;
	std::string extra;
	std::string writebuffer;

 public:
	ClientSocket(IRCServer *s, int fd);
	~ClientSocket();

	bool ProcessRead();
	bool ProcessWrite();

	void Write(const std::string &buffer);
	bool Process(const std::string &buffer);
};

IRC_NAMESPACE_END

#endif // IRC_IO_H

