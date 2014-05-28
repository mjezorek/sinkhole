#ifndef IRC_IRC_H
#define IRC_IRC_H

#define IRC_NAMESPACE_BEGIN namespace Sinkhole { namespace Protocol { namespace IRC {
#define IRC_NAMESPACE_END } } }

#include "string.hpp"
#include <bitset>

IRC_NAMESPACE_BEGIN

class ClientSocket;
class User;
class Channel;
class IRCListener;

class IRCServer
{
 public:
	static const int maxchannels;
	static const unsigned channellen;
	static const unsigned nicklen;
	static const unsigned topiclen;
	static const unsigned kicklen;

	time_t created;
	std::string creation_time_cache;

	std::string name;
	std::string servername;
	std::string networkname;
	std::string desc;

	std::vector<IRCListener *> listeners;

	IRCServer(const std::string &n);
	~IRCServer();
	static IRCServer *Find(const std::string &name);

	std::string BuildCreationTime();

	std::map<ClientSocket *, User *> AllUsers;
	std::map<std::string, User *, Sinkhole::less_ci> Users;
	User *FindUser(const std::string &name);

	std::map<std::string, Channel *, Sinkhole::less_ci> Channels;
	Channel *FindChannel(const std::string &name);
};

extern std::vector<IRCServer *> servers;

IRC_NAMESPACE_END

#endif // IRC_IRC_H
