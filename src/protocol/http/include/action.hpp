#ifndef HTTP_ACTION_H
#define HTTP_ACTION_H

HTTP_NAMESPACE_BEGIN

class HTTPAction : public HTTPVhost
{
 public:
	std::vector<HTTPVhost> vhosts;
	std::string server_name;
};

HTTP_NAMESPACE_END

#endif // HTTP_ACTION_H
