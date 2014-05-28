#ifndef FTP_CLASS_H
#define FTP_CLASS_H

FTP_NAMESPACE_BEGIN

class FTPUser;

class FTPClass
{
 public:
	std::vector<Sinkhole::cidr> sources;
	std::map<std::string, FTPUser> users;

	FTPClass(const std::vector<Sinkhole::cidr> &s);
	FTPUser *FindUser(const std::string &name);
};

FTP_NAMESPACE_END

#endif // FTP_CLASS_H
 
