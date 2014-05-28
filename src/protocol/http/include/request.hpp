#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

HTTP_NAMESPACE_BEGIN

class HTTPRequest
{
	ClientSocket *con;
	AIO *aio;
	HTTPMethod method;
	HTTPProtocol protocol;
	std::string path;
	std::map<std::string, std::string> attributes;
 public:
	HTTPRequest(ClientSocket *c, const std::string &d);
	~HTTPRequest();

	void Read(const std::string &buffer);
	void Process();
};

HTTP_NAMESPACE_END

#endif // HTTP_REQUEST_H

