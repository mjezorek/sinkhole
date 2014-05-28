#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

HTTP_NAMESPACE_BEGIN

class HTTPServer
{
 public:
 	std::string name;
	int timeout;
	std::vector<HTTPListener *> listeners;
	std::vector<HTTPClass *> classes;
	std::map<std::string, HTTPAction *> actions;
	std::set<ClientSocket *> clients;

	static HTTPServer *Find(const std::string &name);

	HTTPServer(const std::string &n);
	~HTTPServer();

	HTTPAction *FindAction(const std::string &name) const;
};

HTTP_NAMESPACE_END

#endif // HTTP_SERVER_H

