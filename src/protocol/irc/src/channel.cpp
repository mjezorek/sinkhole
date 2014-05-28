#include "sinkhole.hpp"
#include "include/irc.hpp"
#include "include/mode.hpp"
#include "include/user.hpp"
#include "include/channel.hpp"

using namespace Sinkhole::Protocol::IRC;

bool user_status::HasMode(ChannelModeName name) const
{
	return this->test(name);
}

std::string user_status::BuildModePrefix() const
{
	std::string prefixes;

	for (size_t i = CMODE_END; i > 0; --i)
		if (this->HasMode(static_cast<ChannelModeName>(i - 1)))
		{
			ChannelMode *cm = FindChannelMode(static_cast<ChannelModeName>(i - 1));
			if (cm != NULL && cm->modesymbol)
				prefixes += cm->modesymbol;
		}
	
	return prefixes;
}

Channel::Channel(const std::string &chname, IRCServer *s) : server(s), name(chname)
{
	this->created = Sinkhole::curtime;
	this->topic_time = 0;
	this->server->Channels[this->name] = this;
}

Channel::~Channel()
{
	this->server->Channels.erase(this->GetName());
}

const std::string &Channel::GetName() const
{
	return this->name;
}

void Channel::SetTopic(const std::string &newtopic)
{
	this->topic_time = Sinkhole::curtime;
	this->topic = newtopic;
}

const std::string &Channel::GetTopic() const
{
	return this->topic;
}

const std::set<User *> &Channel::GetUsers() const
{
	return this->users;
}

bool Channel::IsOnChannel(User *u) const
{
	return this->GetUsers().count(u) > 0;
}

user_status *Channel::FindUserStatus(User *u)
{
	std::map<User *, user_status>::iterator it = this->usermodes.find(u);
	if (it != this->usermodes.end())
		return &it->second;

	return NULL;
}

void Channel::Join(User *u)
{
	this->users.insert(u);
}

void Channel::Part(User *u)
{
	this->users.erase(u);
	this->usermodes.erase(u);
	if (this->users.empty())
		delete this;
}

void Channel::SetMode(ChannelModeName name, const std::string &param)
{
	ChannelMode *cm = FindChannelMode(name);
	if (cm == NULL)
		return;
	
	if (cm->type == CMODET_STATUS)
	{
		User *u = this->server->FindUser(param);
		if (u == NULL)
			return;
		user_status *status = this->FindUserStatus(u);
		if (status == NULL)
		{
			this->usermodes.insert(std::make_pair(u, user_status()));
			status = this->FindUserStatus(u);
		}
		status->set(name, true);
	}
	else
	{
		this->modes.insert(std::make_pair(name, param));
	}
}

void Channel::RemoveMode(ChannelModeName name, const std::string &param)
{
	ChannelMode *cm = FindChannelMode(name);
	if (cm == NULL)
		return;
	
	if (cm->type == CMODET_STATUS)
	{
		User *u = this->server->FindUser(param);
		if (u == NULL)
			return;
		user_status *status = this->FindUserStatus(u);
		if (status == NULL)
			return;
		status->set(name, false);
		if (status->any() == false)
			this->usermodes.erase(u);
	}
	else
	{
		std::pair<modes_map::iterator, modes_map::iterator> iterators = this->GetModeList(name);
		for (; iterators.first != iterators.second; ++iterators.first)
		{
			if (iterators.first->second == param)
			{
				this->modes.erase(iterators.first);
				break;
			}
		}
	}
}

bool Channel::HasMode(ChannelModeName name, const std::string &param)
{
	ChannelMode *cm = FindChannelMode(name);
	if (cm == NULL)
		return false;

	if (cm->type == CMODET_STATUS)
	{
		User *u = this->server->FindUser(param);
		if (u != NULL)
		{
			user_status *status = this->FindUserStatus(u);
			if (status != NULL && status->HasMode(name))
				return true;
		}
	}
	else if (cm->type == CMODET_LIST)
	{
		std::pair<modes_map::iterator, modes_map::iterator> iterators = this->GetModeList(name);
		for (; iterators.first != iterators.second; ++iterators.first)
			if (iterators.first->second == param)
				return true;
	}
	else
	{
		return this->modes.count(name) > 0;
	}

	return false;
}

bool Channel::GetParam(ChannelModeName name, std::string &param)
{
	param.clear();
	modes_map::const_iterator it = this->modes.find(name);
	if (it != this->modes.end())
	{
		param = it->second;
		return true;
	}
	return false;
}

std::pair<Channel::modes_map::iterator, Channel::modes_map::iterator> Channel::GetModeList(ChannelModeName name)
{
	modes_map::iterator it = this->modes.find(name), it_end = it;
	if (it != this->modes.end())
		it_end = this->modes.upper_bound(name);
	return std::make_pair(it, it_end);
}

const std::string Channel::BuildModeString() const
{
	std::string modes = "+", params;
	for (modes_map::const_iterator it = this->modes.begin(), it_end = this->modes.end(); it != it_end; ++it)
	{
		ChannelMode *cm = FindChannelMode(it->first);
		if (cm != NULL)
		{
			modes += cm->modechar;
			if (!it->second.empty())
				params += " " + it->second;
		}
	}

	return modes + params;
}

void Channel::Send(const std::string &source, const std::string &buffer, User *but)
{
	for (std::set<User *>::iterator it = this->users.begin(), it_end = this->users.end(); it != it_end; ++it)
	{
		User *u = *it;
		
		if (u != but)
			u->Write(source, buffer);
	}
}

