#ifndef IRC_CHANNEL_H
#define IRC_CHANNEL_H

IRC_NAMESPACE_BEGIN

class user_status : public std::bitset<CMODE_END>
{
 public:
 	bool HasMode(ChannelModeName name) const;
	std::string BuildModePrefix() const;
};

class Channel
{
 public:
	typedef std::multimap<ChannelModeName, std::string> modes_map;

 private:
	IRCServer *server;
	std::string name;
	std::string topic;
	std::set<User *> users;
	std::map<User *, user_status> usermodes;
	modes_map modes;

 public:
 	time_t created;
	time_t topic_time;

	Channel(const std::string &chname, IRCServer *s);
	~Channel();

	const std::string &GetName() const;
	
	void SetTopic(const std::string &newtopic);
	const std::string &GetTopic() const;

	const std::set<User *> &GetUsers() const;
	bool IsOnChannel(User *u) const;
	user_status *FindUserStatus(User *u);

	void Join(User *u);
	void Part(User *u);

	void SetMode(ChannelModeName name, const std::string &param = "");
	void RemoveMode(ChannelModeName name, const std::string &param = "");
	bool HasMode(ChannelModeName name, const std::string &param = "");
	bool GetParam(ChannelModeName name, std::string &param);
	std::pair<modes_map::iterator, modes_map::iterator> GetModeList(ChannelModeName name);
	const std::string BuildModeString() const;

	void Send(const std::string &source, const std::string &buffer, User *but = NULL);
};

IRC_NAMESPACE_END

#endif // IRC_CHANNEL_H

