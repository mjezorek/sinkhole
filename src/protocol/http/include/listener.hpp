#ifndef HTTP_LISTENER_H
#define HTTP_LISTENER_H

HTTP_NAMESPACE_BEGIN

class HTTPListener : public Sinkhole::Listener
{
	HTTPServer *server;
 public:
	HTTPListener(HTTPServer *s, const std::string &bindip, int port, int type);
	~HTTPListener();

	Socket *OnAccept(int fd);
};

HTTP_NAMESPACE_END

#endif // HTTP_LISTENER_H

