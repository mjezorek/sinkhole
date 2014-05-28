#ifndef HTTP_HTTP_H
#define HTTP_HTTP_H

#define HTTP_NAMESPACE_BEGIN namespace Sinkhole { namespace Protocol { namespace HTTP {
#define HTTP_NAMESPACE_END } } }

HTTP_NAMESPACE_BEGIN

class HTTPVhost;
class HTTPAction;
class HTTPClass;
class HTTPListener;
class HTTPServer;
class HTTPHeader;
class HTTPRequest;
class ClientSocket;

extern std::vector<HTTPServer *> servers;

class HTTPException : public Exception
{
 public:
	HTTPException(const std::string &r) : Exception(r) { }
};

HTTP_NAMESPACE_END

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "vhost.hpp"
#include "action.hpp"
#include "class.hpp"
#include "listener.hpp"
#include "io.hpp"
#include "server.hpp"
#include "header.hpp"
#include "aio.hpp"
#include "request.hpp"

#ifndef O_NOATIME
# define O_NOATIME 0
#endif

#endif // HTTP_HTTP_H

