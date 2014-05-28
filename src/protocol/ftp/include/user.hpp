#ifndef FTP_USER_H
#define FTP_USER_H

FTP_NAMESPACE_BEGIN

class FTPAccess;

class FTPUser
{
 public:
	std::string username;
	std::string password;
	std::string root;
	FTPAccess *access;

	FTPUser(const std::string &u, const std::string &p, const std::string &r, FTPAccess *a) :
		username(u), password(p), root(r), access(a)
	{
	}
};

FTP_NAMESPACE_END

#endif // FTP_USER_H

