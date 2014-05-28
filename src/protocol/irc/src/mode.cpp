#include "sinkhole.hpp"
#include "include/irc.hpp"
#include "include/mode.hpp"
#include "include/user.hpp"
#include "include/channel.hpp"

using namespace Sinkhole::Protocol::IRC;

ChannelMode ChannelModeNoExternal(CMODE_NOEXTERNAL, CMODET_REGULAR, 'n'),
		ChannelModeProtectedTopic(CMODE_PROTECTEDTOPIC, CMODET_REGULAR, 't'),
		ChannelModeModerated(CMODE_MODERATED, CMODET_REGULAR, 'm'),
		ChannelModePrivate(CMODE_PRIVATE, CMODET_REGULAR, 'p'),
		ChannelModeSecret(CMODE_SECRET, CMODET_REGULAR, 's'),
		ChannelModeInviteOnly(CMODE_INVITEONLY, CMODET_REGULAR, 'i'),
		ChannelModeKey(CMODE_KEY, CMODET_PARAM, 'k'),
		ChannelModeLimit(CMODE_LIMIT, CMODET_PARAM, 'l', true),
		ChannelModeBan(CMODE_BAN, CMODET_LIST, 'b'),
		ChannelModeVoice(CMODE_VOICE, CMODET_STATUS, 'v', false, '+'),
		ChannelModeOp(CMODE_OP, CMODET_STATUS, 'o', false, '@');

UserMode UserModeInvisible(UMODE_INVISIBLE, 'i');

static std::vector<ChannelMode *> *ChannelModes;
static std::vector<UserMode *> *UserModes;

ChannelMode::ChannelMode(ChannelModeName n, ChannelModeType t, char c, bool m, char s) : name(n), type(t), modechar(c), minusnoarg(m), modesymbol(s)
{
	if (ChannelModes == NULL)
		ChannelModes = new std::vector<ChannelMode *>();
	ChannelModes->push_back(this);
}

bool ChannelMode::CanSet(User *u, Channel *c, bool, const std::string &)
{
	user_status *status = c->FindUserStatus(u);
	if (status != NULL && status->HasMode(CMODE_OP))
		return true;

	return false;
}

bool ChannelMode::ValidParam(User *, Channel *, bool, const std::string &)
{
	return true;
}

UserMode::UserMode(UserModeName n, char c) : name(n), modechar(c)
{
	if (UserModes == NULL)
		UserModes = new std::vector<UserMode *>();
	UserModes->push_back(this);
}

bool UserMode::CanSet(User *, bool)
{
	return true;
}

ChannelMode *Sinkhole::Protocol::IRC::FindChannelMode(ChannelModeName name)
{
	for (unsigned i = ChannelModes ? ChannelModes->size() : 0; i > 0; --i)
	{
		ChannelMode *cm = ChannelModes->at(i - 1);

		if (cm->name == name)
			return cm;
	}

	return NULL;
}

ChannelMode *Sinkhole::Protocol::IRC::FindChannelMode(char mode)
{
	for (unsigned i = ChannelModes ? ChannelModes->size() : 0; i > 0; --i)
	{
		ChannelMode *cm = ChannelModes->at(i - 1);

		if (cm->modechar == mode)
			return cm;
	}

	return NULL;
}

UserMode *Sinkhole::Protocol::IRC::FindUserMode(UserModeName name)
{
	for (unsigned i = UserModes ? UserModes->size() : 0; i > 0; --i)
	{
		UserMode *um = UserModes->at(i - 1);

		if (um->name == name)
			return um;
	}

	return NULL;
}

UserMode *Sinkhole::Protocol::IRC::FindUserMode(char mode)
{
	for (unsigned i = UserModes ? UserModes->size() : 0; i > 0; --i)
	{
		UserMode *um = UserModes->at(i - 1);

		if (um->modechar == mode)
			return um;
	}

	return NULL;
}

