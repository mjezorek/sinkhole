#ifndef FTP_FTP_H
#define FTP_FTP_H

#define FTP_NAMESPACE_BEGIN namespace Sinkhole { namespace Protocol { namespace FTP {
#define FTP_NAMESPACE_END } } }

FTP_NAMESPACE_BEGIN

class FTPServer;

class FTPException : public Exception
{
 public:
	FTPException(const std::string &r) : Exception(r) { }
};

extern std::vector<FTPServer *> servers;

FTP_NAMESPACE_END

#endif // FTP_FTP_H

