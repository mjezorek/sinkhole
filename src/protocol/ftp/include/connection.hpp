#ifndef FTP_CONNECTION_H
#define FTP_CONNECTION_H

FTP_NAMESPACE_BEGIN

class FTPDataConnection : public FTPDataSocket
{
	bool connected;
 public:
	FTPDataConnection(FTPClient *o, const std::string &addr, int port);

	bool ProcessWrite();
};

FTP_NAMESPACE_END

#endif // FTP_CONNECTION_H
