#ifndef FTP_SERVER_H
#define FTP_SERVER_H

FTP_NAMESPACE_BEGIN

class FTPAccess;
class FTPClass;
class FTPClient;
class FTPListener;

class FTPServer
{
 public:
	std::string name;
	std::string banner;
	std::map<std::string, FTPAccess> access;
	std::vector<FTPClass> classes;
	std::vector<FTPListener *> listeners;
	std::set<FTPClient *> users;

	static FTPServer *Find(const std::string &name);

	FTPServer(const std::string &n);
	~FTPServer();

	FTPAccess *FindAccess(const std::string &n);
};

FTP_NAMESPACE_END

#endif // FTP_SERVER_H

