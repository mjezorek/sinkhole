#include "sinkhole.hpp"
#include "include/irc.hpp"
#include "include/mode.hpp"
#include "include/user.hpp"
#include "include/channel.hpp"
#include "include/command.hpp"

using namespace Sinkhole::Protocol::IRC;

class CommandMode : public Command
{
 public:
	CommandMode() : Command("MODE", 1)
	{
	}

	void Execute(IRCServer *server, User *u, const std::vector<std::string> &params)
	{
		const std::string &target = params[0];

		if (target[0] == '#' || target[0] == '&')
		{
			Channel *c = server->FindChannel(target);

			if (c == NULL)
			{
				u->WriteNumeric(401, target + " :No such nick/channel");
				return;
			}

			if (params.size() == 1)
			{
				u->WriteNumeric(324, c->GetName() + " " + c->BuildModeString());
				u->WriteNumeric(329, c->GetName() + " " + Sinkhole::stringify(c->created));
			}
			else
			{
				const std::string &modes = params[1];
				unsigned cur_param = 2;
				int add = -1;
				
				std::string addmodes = "+", remmodes = "-", addparams, remparams;
				for (unsigned i = 0, j = modes.size(); i < j; ++i)
				{
					if (modes[i] == '+')
						add = 1;
					else if (modes[i] == '-')
						add = 0;
					else if (add > -1)
					{
						ChannelMode *cm = FindChannelMode(modes[i]);
						if (cm == NULL)
							u->WriteNumeric(472, modes[i] + " :is unknown mode char to me");
						else
						{
							bool want_arg = cm->type != CMODET_REGULAR && (add || !cm->minusnoarg);

							if (want_arg && cur_param >= params.size())
								continue;
							else if (add == c->HasMode(cm->name, want_arg ? params[cur_param] : ""))
							{
								if (want_arg)
									++cur_param;
								continue;
							}
							else if (!cm->ValidParam(u, c, add == 1, want_arg ? params[cur_param] : ""))
							{
								if (want_arg)
									++cur_param;
								continue;
							}
							else if (!cm->CanSet(u, c, add == 1, want_arg ? params[cur_param] : ""))
							{
								if (want_arg)
									++cur_param;
								continue;
							}
							else
							{
								if (add)
								{
									c->SetMode(cm->name, want_arg ? params[cur_param] : "");
									addmodes += cm->modechar;
								}
								else
								{
									std::string param;
									if (cm->minusnoarg)
										c->GetParam(cm->name, param);
									c->RemoveMode(cm->name, want_arg ? params[cur_param] : param);
									remmodes += cm->modechar;
								}
								if (want_arg)
								{
									if (add)
										addparams += " " + params[cur_param];
									else
										remparams += " " + params[cur_param];

									++cur_param;
								}
							}
						}
					}
				}

				if (addmodes.length() == 1)
					addmodes.clear();
				if (remmodes.length() == 1)
					remmodes.clear();
				std::string finalmodes = addmodes + remmodes + addparams + remparams;
				if (!finalmodes.empty())
					c->Send(u->GetMask(), "MODE " + c->GetName() + " " + finalmodes);
			}
		}
		else
		{
			User *targ = server->FindUser(target);

			if (targ == NULL)
			{
				u->WriteNumeric(401, target + " :No such nick/channel");
				return;
			}

			if (targ != u)
				return;
			
			if (params.size() == 1)
			{
				u->WriteNumeric(221, u->BuildModeString());
			}
			else
			{
				const std::string &modes = params[1];
				int add = -1;

				std::string addmodes = "+", remmodes = "-";
				for (unsigned i = 0, j = modes.size(); i < j; ++i)
				{
					if (modes[i] == '+')
						add = 1;
					else if (modes[i] == '-')
						add = 0;
					else if (add > -1)
					{
						UserMode *um = FindUserMode(modes[i]);
						if (um == NULL)
							u->WriteNumeric(501, ":Unknown MODE flag");
						else if (!um->CanSet(u, add == 1))
							continue;
						else if (add == u->HasMode(um->name))
							continue;
						else
						{
							if (add)
							{
								u->SetMode(um->name);
								addmodes += modes[i];
							}
							else
							{
								u->RemoveMode(um->name);
								remmodes += modes[i];
							}
						}
					}
				}

				if (addmodes.length() == 1)
					addmodes.clear();
				if (remmodes.length() == 1)
					remmodes.clear();
				std::string finalmodes = addmodes + remmodes;
				if (!finalmodes.empty())
					u->Write(u->GetNick(), "MODE " + u->GetNick() + " :" + finalmodes);
			}
		}
	}
} commandmode;

