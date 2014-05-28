#ifndef FTP_LISTENER_H
#define FTP_LISTENER_H

FTP_NAMESPACE_BEGIN

class FTPListener : public Sinkhole::Listener
{
	FTPServer *server;
 public:
	FTPListener(FTPServer *s, const std::string &bindip, int port);
	~FTPListener();

	Socket *OnAccept(int fd);
};

class FTPDataListener : public Sinkhole::Listener
{
 public:
	FTPClient *owner;

	FTPDataListener(FTPClient *o, const std::string &bindip);
	~FTPDataListener();

	Socket *OnAccept(int fd);
};

FTP_NAMESPACE_END

#endif // FTP_LISTENER_H
