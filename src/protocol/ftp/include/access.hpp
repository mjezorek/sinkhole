#ifndef FTP_ACCESS_H
#define FTP_ACCESS_H

FTP_NAMESPACE_BEGIN

class FTPAccess
{
	std::string name;
	std::vector<std::string> commands;
 public:

	FTPAccess(const std::string &n, const std::vector<std::string> &c);

	bool Allows(const std::string &name);
};

FTP_NAMESPACE_END

#endif

