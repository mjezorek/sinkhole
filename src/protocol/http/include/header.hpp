#ifndef HTTP_HEADER_H
#define HTTP_HEADER_H

HTTP_NAMESPACE_BEGIN

enum HTTPCode
{
	HTTP_CODE_OK = 200,
	HTTP_CODE_BAD_REQUEST = 400,
	HTTP_CODE_UNAUTHORIZED = 401,
	HTTP_CODE_FORBIDDEN = 403,
	HTTP_CODE_NOT_FOUND = 404,
	HTTP_CODE_INTERNAL_ERROR = 500
};

enum HTTPMethod
{
	HTTP_METHOD_GET,
	HTTP_METHOD_POST
};

enum HTTPProtocol
{
	HTTP_PROTO_10,
	HTTP_PROTO_11
};

class HTTPHeader
{
	HTTPCode code;
	HTTPProtocol protocol;
	std::map<std::string, std::string> attributes;
 public:
	HTTPHeader();

	void SetCode(HTTPCode c);
	void SetProtocol(HTTPProtocol p);
	void SetAttribute(const std::string &attr, const std::string &value);

	std::string ToString();
};

HTTP_NAMESPACE_END

#endif // HTTP_HEADER_H

