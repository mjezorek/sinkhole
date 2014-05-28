#ifndef IRC_USER_H
#define IRC_USER_H

IRC_NAMESPACE_BEGIN

class ClientSocket;

class User
{
 public:
	typedef std::bitset<UMODE_END> modes_map;

 private:
	IRCServer *server;
	friend class ClientSocket;
	friend class IRCServer;
	ClientSocket *sock;
	bool registered;

	std::string nick;
	std::string username;
	std::string realname;

	mutable std::string ip_cache;

	std::set<Channel *> channels;
	modes_map modes;

 public:
	bool got_pong;

	User(IRCServer *s);
	~User();

	void SetNick(const std::string &newnick);
	const std::string &GetNick() const;

	void SetUsername(const std::string &newusername);
	const std::string &GetUsername() const;

	void SetRealname(const std::string &newrealname);
	const std::string &GetRealname() const;

	const std::string &GetIP() const;
	const std::string GetMask() const;

	void Register();
	bool IsRegistered() const;
	IRCServer *GetServer() const;

	void SetMode(UserModeName name);
	void RemoveMode(UserModeName name);
	bool HasMode(UserModeName name) const;
	const std::string BuildModeString() const;

	void Join(Channel *c);
	void Part(Channel *c);
	bool IsOnChannel(Channel *c) const;
	const std::set<Channel *> &GetChannels() const;

	void Quit();

	void Write(const std::string &source, const std::string &buffer);
	void WriteNumeric(int numeric, const std::string &buffer);
	void WriteCommonUsers(const std::string &source, const std::string &buffer, User *but = NULL);
};

IRC_NAMESPACE_END

#endif // IRC_USER_H

