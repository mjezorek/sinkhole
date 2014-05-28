#ifndef IRC_LISTENER_H
#define IRC_LISTENER_H

IRC_NAMESPACE_BEGIN

class IRCListener : public Sinkhole::Listener
{
	IRCServer *server;
 public:
	IRCListener(IRCServer *s, const std::string &bindip, int port, int type);
	~IRCListener();

	Socket *OnAccept(int fd);
};

IRC_NAMESPACE_END

#endif // IRC_LISTENER_H
