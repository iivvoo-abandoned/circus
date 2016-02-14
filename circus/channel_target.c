/*
 * channel_target.c - controls channelwindows
 * Copyright (C) 1998 Ivo van der Wijk (ivo@cs.vu.nl) and others
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#include "channel_target.h"
#include "misc.h"

/*
 * This, together with channelwin.[ch] really need some restructuring...
 */

channel_target::channel_target(cstring w, int sessionid): channelwin(w)
{
	this->sessionid = sessionid;
	channelname = w;
	my_mode = 0;
	mode = 0;
	limit = 0;
	topic = "";
	key = "";
	info_mode = true;
	info_who = true;
	info_names = true;
	info_bans = true;
	user_count = 0;
	havefocus = true;		// really?
	do_autoraise = false;

	status = STATE_NOTJOIN;
	// disable entries/buttons in the window
	// except topicentry
	channelwin::set_op(false);
}

channel_target::~channel_target()
{
	garbage<KitObject *>	usertrash;
	garbage<cstring *>	bantrash;

	map<cstring, KitObject *, mapcmp>::iterator t;
	for(t = usr_list.begin(); t != usr_list.end(); t++)
		usertrash._register((*t).second);
	usr_list.erase(usr_list.begin(), usr_list.end());

	for(unsigned int i = 0; i < ban_list.size(); i++)
		bantrash._register(ban_list[i]);
	ban_list.erase(ban_list.begin(), ban_list.end());
}

user_info	*channel_target::find(cstring nick)
{
cout << "-------- Finding " << nick << endl;
	map<cstring, KitObject *, mapcmp>::iterator t;
	
	t = usr_list.find(nick);

	if(t == usr_list.end())
	{
cout << "Not found.. " << endl;
		return NULL;
	}

	return ((user_info *)(*t).second);
}

void	channel_target::disable(void)
// clear userlist and disable channelwindow
{
	garbage<KitObject *>	usertrash;
	garbage<cstring *>	bantrash;

	channelwin::disable();

	map<cstring, KitObject *, mapcmp>::iterator t;
	for(t = usr_list.begin(); t != usr_list.end(); t++)
		usertrash._register((*t).second);
	usr_list.erase(usr_list.begin(), usr_list.end());

	for(unsigned int i = 0; i < ban_list.size(); i++)
		bantrash._register(ban_list[i]);
	ban_list.erase(ban_list.begin(), ban_list.end());
}

int	channel_target::wantinfo(int, char **argv)
{
	cstring	rpl = argv[2];

	if(!(info_mode || info_who || info_names || info_bans))
		return false;

	if(rpl == "324")		// MODEISREPLY
	{
		if(!info_mode)
			return false;

        	cstring which = argv[4];
        	cstring modes = argv[5];
        	cstring targ;
	        int     targ_cnt = 6;
 
        	for(int i = 0; i < modes.length(); i++)
                	switch(modes[i])
                	{
                	case '+':
                        	break;
                	case 'i':
                        	set_cmode(CMODE_INVITE);
                        	mode_toggle(CMODE_INVITE, true);
                        	break;
                	case 'k':
                        	targ = argv[targ_cnt++];
                        	set_cmode(CMODE_KEY);
                        	set_key(targ);
                        	set_keyentry(targ);
                        	mode_toggle(CMODE_KEY, true);
                        	break;
                	case 'l':
                        	targ = argv[targ_cnt++];
                        	set_cmode(CMODE_LIMIT);
                        	set_limit(atoi(targ));
                        	mode_toggle(CMODE_LIMIT, true);
                        	break;
                	case 'm':
                        	set_cmode(CMODE_MODERATE);
                        	mode_toggle(CMODE_MODERATE, true);
                        	break;
                	case 'n':
                        	set_cmode(CMODE_NO_MSG);
                        	mode_toggle(CMODE_NO_MSG, true);
                        	break;
                	case 'p':
                        	set_cmode(CMODE_PRIVATE);
                        	mode_toggle(CMODE_PRIVATE, true);
                        	break;
                	case 's':
                        	set_cmode(CMODE_SECRET);
                        	mode_toggle(CMODE_SECRET, true);
                        	break;
                	case 't':
                       		set_cmode(CMODE_TOPIC);
                        	mode_toggle(CMODE_TOPIC, true);
                        	if(!(get_mymode() & UMODE_OP))
                                	topic_disable();
                        	break;
                	default:
                        	cstring mod = modes[i];
                        	info_insert("Unknown mode " + mod +" set\n");
                        	break;
                	}
		info_mode = false;
		return true;
	}
	if(rpl == "352")		// WHOREPLY
	{
		if(!info_who)
			return false;

		cstring		nick = argv[8];
		cstring		login = argv[5];
		cstring		host = argv[6];
		cstring		mode = argv[9];
		cstring		ircname = argv[10];
		user_info	*u;

		u = find(nick);

		u->login = login;
		u->host = host;
		if(mode[1] == '@')
			u->set_umode(UMODE_OP);
		if(mode[1] == '+')
			u->set_umode(UMODE_VOICE);
		return true;
	}
	else if(rpl == "315")			// end of who
	{
		if(!info_who)
			return false;
		info_who = false;
		return true;
	}
	else if(rpl == "353")			// namereply
	{
		if(!info_names)
			return false;

		char	*users = argv[6];
		char	*p;

		// quick and dirty parsing

		while(*users)
		{
			// skip leading spaces
			while(*users && *users == ' ')
				users++;
			// something left?
			if(*users)
			{
				p = users;
				// find next space
				while(*users && *users != ' ')
					users++;
				// Terminate and advance, if necessary
				if(*users)
				{
					*users = '\0';
					users++;
				}

				int	mode = 0;

				if(p[0] == '@')
				{
					mode |= UMODE_OP;
					p++;
				}
				else if(p[0] == '+')
				{
					mode |= UMODE_VOICE;
					p++;
				}
				add_user(p, user_count, mode);
				if(user_count == 0) // it's me!
				{
					set_mymode(mode);
					if(mode & UMODE_OP)
						set_op(true);
				}
				user_count++;
			}
		}
		return true;	
	}
	else if(rpl == "366")			// end of names
	{
		if(!info_names)
			return false;
		info_names = false;
		return true;
	}
	else if(rpl == "367")			// banreply
	{
		if(!info_bans)
			return false;
		add_ban(argv[5]);
		return true;
	}
	else if(rpl == "368")			// end of banlist
	{
		if(!info_bans)
			return false;
		info_bans = false;
		return true;
	}
	return false;
}

void	channel_target::setstate(const int s) 
{ 
	status = s; 
	switch(status)
	{
	case STATE_CLEAR:
		enable();
		my_mode = 0;
		mode = 0;
		limit = 0;
		topic = "";
		key = "";
		info_mode = true;
		info_who = true;
		info_names = true;
		info_bans = true;
		user_count = 0;
		
		break;
	case STATE_KICKED:
		disable();
		break;
	case STATE_NOTJOIN:
		disable();
		break;
	}
}

int	channel_target::getstate(void)
{
	return status;
}

void	channel_target::set_mymode(int m)
{
	my_mode |= m;
}

void	channel_target::del_mymode(int m)
{
	my_mode &= ~m;
}

int	channel_target::get_mymode(void)
{
	return my_mode;
}

void	channel_target::set_op(int on)
{
	if(on)
		set_mymode(UMODE_OP);
	else
	{
		del_mymode(UMODE_OP);
		if(mode & CMODE_TOPIC && !(my_mode & UMODE_OP))
			topic_disable();
	}
	channelwin::set_op(on);
}

void	channel_target::set_voice(const int on)
{
	if(on)
		set_mymode(UMODE_VOICE);
	else
		del_mymode(UMODE_VOICE);
}

void	channel_target::set_cmode(int m)
{
	mode |= m;
}

void	channel_target::del_cmode(int m)
{
	mode &=~m;
}

int	channel_target::get_cmode(void)
{
	return mode;
}

void	channel_target::set_key(cstring k)
{
	key = k;
	key_enable();
	set_keyentry(key);
	if(!(my_mode & UMODE_OP))		// disable?
		key_disable();
}

cstring	&channel_target::get_key(void)
{
	return key;
}

void	channel_target::set_topic(const cstring t)
{
	topic = (char *)t;
	topic_enable();
	set_topicentry(t);
	if(mode & CMODE_TOPIC && !(my_mode & UMODE_OP))
		topic_disable();
}

cstring	&channel_target::get_topic(void)
{
	return topic;
}

void	channel_target::set_limit(const int l)
{
	limit = l;

	limit_enable();
	if(l == 0)
		set_limitentry("");
	else
	{
		char	buf[256];		// long enough :)

		sprintf(buf, "%d", l);
		set_limitentry(buf);
	}
	if(!(my_mode & UMODE_OP))		// disable?
		limit_disable();
}

int	channel_target::get_limit(void)
{
	return limit;
}

vector<cstring *>	&channel_target::get_ban_list(void)
{
	return ban_list;
}

void	channel_target::add_ban(const cstring b)
{
	ban_list.insert(ban_list.end(), new cstring(b));
}

void	channel_target::del_ban(const cstring &b)
{
	vector<cstring *>::iterator t;

	for(t = ban_list.begin(); t != ban_list.end(); t++)
		if(*(*t) == b)
		{
			delete (*t);
			ban_list.erase(t);
			return;
		}
}

void	channel_target::add_user(cstring who, const int index, const int mode)
{
	cstring	nick, login, host;

	if(isnick(who))
	{
		nick = who;
		login = "unknown";
		host = "unknown";
	}
	else
		split_nuh(who, nick, login, host);

	user_info *u = new user_info();
	usr_list[nick] = u;
	u->login = login;
	u->host = host;
	u->set_umode(mode);
	channelwin::add_user(nick, index, mode);
}

void	channel_target::del_user(const cstring &user)
{
	map<cstring, KitObject *, mapcmp>::iterator t=usr_list.find(user);

if(t == usr_list.end())
{
	cout << "Could not find " << user << endl;

	for(t = usr_list.begin(); t != usr_list.end(); t++)
		cout << "- [" << (*t).first << "]" << endl;
	abort();
}

	delete (*t).second;
	usr_list.erase(t);

	channelwin::del_user(user);

	user_count--;
}

void	channel_target::change_user(const cstring &nick, const cstring &newnick)
{
	user_info *u = find(nick);
	int index, selected;

cout << " -- " << nick << " -> " << newnick << endl;
if(u == NULL)
	cout << "Could not find " << nick << endl;

	usr_list.erase(nick);
	usr_list[newnick] = u;
	selected = user_selected(nick);
	index = channelwin::del_user(nick);
	channelwin::add_user(newnick, index, u->get_umode());
	if(selected)
		user_select(index);
}

cstring	channel_target::get_userlogin(const cstring &nick)
{
	user_info	*u = find(nick);

	if(u == NULL)
		return "";

	return u->login;
}

cstring	channel_target::get_userhost(const cstring &nick)
{
	user_info	*u = find(nick);

	if(u == NULL)
		return "";

	return u->host;
}

void	channel_target::set_umode(const cstring &nick, const int mode)
{
	user_info *u = find(nick);
	int index, selected;

	u->set_umode(mode);
	selected = user_selected(nick);
	index = channelwin::del_user(nick);
	channelwin::add_user(nick, index, u->get_umode());
	if(selected)
		user_select(index);
}

int	channel_target::get_umode(const cstring &nick)
{
	user_info *u = find(nick);
	return u->get_umode();
}

void	channel_target::del_umode(const cstring &nick, const int mode)
{
	user_info *u = find(nick);
	int index, selected;

	u->del_umode(mode);
	selected = user_selected(nick);
	index = channelwin::del_user(nick);
	channelwin::add_user(nick, index, u->get_umode());
	if(selected)
		user_select(index);
}

void	channel_target::setfocus(int mode)
{
	if(mode)
	{
		if(!havefocus)
			havefocus = true;
	}
	else
		havefocus = false;
}

void	channel_target::toggle_autoraise()
{
	do_autoraise = !do_autoraise;
}

void	channel_target::toggle_lock()
{
	channelwin::toggle_lock();
}

int	channel_target::get_autoraise()
{
	return do_autoraise;
}

void    channel_target::insert(const cstring &s, const cstring &t)
{
	if(do_autoraise)
	{
		tk->eval("wm deiconify " + cstring(path()));
		tk->eval("raise " + cstring(path()));
	}
        channelwin::insert(s, t);
}

void    channel_target::insert(const cstring &p, const cstring &s,
                          const unsigned int l, const cstring &tagname)
{
	if(do_autoraise)
	{
		tk->eval("wm deiconify " + cstring(path()));
		tk->eval("raise " + cstring(path()));
	}
        channelwin::insert(p, s, l, tagname);
}

void    channel_target::info_insert(const cstring &s)
{
	if(do_autoraise)
	{
		tk->eval("wm deiconify " + cstring(path()));
		tk->eval("raise " + cstring(path()));
	}
        channelwin::info_insert(s);
}

void	channel_target::nickchange(const cstring &oldnick, 
				   const cstring &newnick)
{
	if(find(oldnick))
	{
		info_insert(oldnick + " is now known as " + newnick + "\n");
		insert(oldnick + " -> " + newnick + "\n", "notice");
	}
}

void	channel_target::quituser(const cstring &nick, const cstring &why)
{
	if(find(nick))
	{
		info_insert("Signoff: " + nick + " (" + why + ")\n");
		insert(nick + " has signed off\n", "notice");
	}
}
