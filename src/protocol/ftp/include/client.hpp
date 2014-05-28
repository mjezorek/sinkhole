#ifndef FTP_IO_H
#define FTP_IO_H

#include "include/directory.hpp"

FTP_NAMESPACE_BEGIN

class FTPServer;
class FTPAccess;
class FTPUser;
class FTPClass;
class FTPDataSocket;
class FTPDataListener;
class AIORead;

class FTPClient : public Sinkhole::Socket
{
	FTPServer *server;
	std::string extra;
	std::string writebuffer;
	std::string ip_cache;

 public:
	FTPDataSocket *data;
	FTPDataListener *listener;
	AIORead *read_request;
	std::string rename_store;

	FTPClass *ftpclass;
	FTPUser *user;
	std::string username;
	FTPDirectory cwd;

	FTPClient(FTPServer *s, int fd);
	~FTPClient();
	void Destroy();

	bool ProcessRead();
	bool ProcessWrite();
	void Write(const std::string &buffer);
	bool Process(const std::string &buffer);
	
	const std::string &GetIP();

	void WriteCode(int code, const std::string &buffer);
};

FTP_NAMESPACE_END

#endif // FTP_IO_H

