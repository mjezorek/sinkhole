#ifndef HTTP_VHOST_H
#define HTTP_VHOST_H

HTTP_NAMESPACE_BEGIN

class HTTPVhost
{
 public:
	enum Type
	{
		TYPE_SERVE,
		TYPE_ROOT
	};

	std::string name;
	std::string data;
	Type type;
};

HTTP_NAMESPACE_END

#endif // HTTP_VHOST_H
