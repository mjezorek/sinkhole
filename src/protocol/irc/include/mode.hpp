#ifndef IRC_MODE_H
#define IRC_MODE_H

IRC_NAMESPACE_BEGIN

enum ChannelModeName
{
	CMODE_NOEXTERNAL,
	CMODE_PROTECTEDTOPIC,
	CMODE_MODERATED,
	CMODE_PRIVATE,
	CMODE_SECRET,
	CMODE_INVITEONLY,
	CMODE_KEY,
	CMODE_LIMIT,
	CMODE_BAN,
	CMODE_VOICE,
	CMODE_OP,
	CMODE_END
};

enum ChannelModeType
{
	CMODET_REGULAR,
	CMODET_PARAM,
	CMODET_STATUS,
	CMODET_LIST
};

enum UserModeName
{
	UMODE_INVISIBLE,
	UMODE_END
};

class User;
class Channel;

class ChannelMode
{
 public:
	ChannelModeName name;
	ChannelModeType type;
	char modechar;
	bool minusnoarg;
	char modesymbol;

	ChannelMode(ChannelModeName n, ChannelModeType t, char c, bool m = false, char s = 0);
	virtual bool CanSet(User *u, Channel *c, bool set, const std::string &param);
	virtual bool ValidParam(User *u, Channel *c, bool set, const std::string &param);
};

struct UserMode
{
	UserModeName name;
	char modechar;

	UserMode(UserModeName n, char c);
	virtual bool CanSet(User *u, bool set);
};

extern ChannelMode ChannelModeNoExternal, ChannelModeProtectedTopic, ChannelModeModerated, ChannelModePrivate,
			ChannelModeSecret, ChannelModeInviteOnly, ChannelModeKey, ChannelModeLimit, ChannelModeBan,
			ChannelModeVoice, ChannelModeOp;

extern UserMode UserModeInvisible;

extern ChannelMode *FindChannelMode(ChannelModeName name);
extern ChannelMode *FindChannelMode(char mode);

extern UserMode *FindUserMode(UserModeName name);
extern UserMode *FindUserMode(char mode);

IRC_NAMESPACE_END

#endif // IRC_MODE_H

