/*
 * targets.c - handles and maintains targets (channels, queries)
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
#include "targets.h"

#include "const.h"

extern 	int	dbg;

#warning "'desync' between current window - targetlist in connection.c"

/*
 * Collection of operations on 'targets' (channels and queries)
 */

#define	pack(f)	new func_packer(this, &targets::f)

targets::targets(): connection(), targetlist()
{
}

targets::targets(tixNoteBook *parent, int sessionid): 
				connection(parent, sessionid), targetlist()
{
	dispatch_table["topic_changed"] = pack(chan_topic_changed);
	dispatch_table["key_changed"] = pack(chan_key_changed);
	dispatch_table["limit_changed"] = pack(chan_limit_changed);
	dispatch_table["start_query"] = pack(start_query);
	dispatch_table["cmode_i_changed"] = pack(chan_cmode_i_changed);
	dispatch_table["cmode_k_changed"] = pack(chan_cmode_k_changed);
	dispatch_table["cmode_l_changed"] = pack(chan_cmode_l_changed);
	dispatch_table["cmode_m_changed"] = pack(chan_cmode_m_changed);
	dispatch_table["cmode_n_changed"] = pack(chan_cmode_n_changed);
	dispatch_table["cmode_p_changed"] = pack(chan_cmode_p_changed);
	dispatch_table["cmode_s_changed"] = pack(chan_cmode_s_changed);
	dispatch_table["cmode_t_changed"] = pack(chan_cmode_t_changed);
	dispatch_table["leavebutton_pressed"] = pack(leave_channel);
	dispatch_table["take_focus"] = pack(take_focus);
	dispatch_table["lose_focus"] = pack(lose_focus);
	dispatch_table["open_menu"] = pack(openmenu);
	dispatch_table["toggleraise"] = pack(toggleraise);
	dispatch_table["togglelock"] = pack(togglelock);
	dispatch_table["Ban"] = pack(banselected);
	dispatch_table["Query"] = pack(queryselected);
	dispatch_table["Whois"] = pack(whoisselected);
	dispatch_table["Op"] = pack(opselected);
	dispatch_table["Deop"] = pack(deopselected);
	dispatch_table["Voice"] = pack(voiceselected);
	dispatch_table["Mute"] = pack(muteselected);
	dispatch_table["Kick"] = pack(kickselected);
	dispatch_table["Kick-Ban"] = pack(kickbanselected);
	dispatch_table["Version"] = pack(ctcpselected);
	dispatch_table["Clientinfo"] = pack(ctcpselected);
	dispatch_table["Userinfo"] = pack(ctcpselected);
	dispatch_table["Finger"] = pack(ctcpselected);
	dispatch_table["Time"] = pack(ctcpselected);
	dispatch_table["Ping"] = pack(ctcpselected);
	dispatch_table["input_typed"] = pack(handle_input);
}

targets::~targets()
{
	if(messages)
		delete messages;
}

int	targets::chan_topic_changed(int /* argc */, char **argv)
{
	cstring	which = argv[1];
	channel_target *t = (channel_target *) targetlist.findtarget(which);
	cstring topic = t->get_topicentry();

	if(topic != t->get_topic())
		topic(which, topic);
	return true;
}

int	targets::chan_key_changed(int /* argc */, char **argv)
{
	cstring	which = argv[1];
	channel_target *t = (channel_target *) targetlist.findtarget(which);
	cstring 	key = t->get_keyentry();
	
	if(t->get_cmode() & CMODE_KEY)
		mode(which, "-k " + t->get_key());
	if(!(key == "") && key != t->get_key())
		mode(which, "+k " + key);
	return true;
}

int	targets::start_query(int /* argc */, char **argv)
{
	cstring	which = argv[1];
	
	return selection_operate(which, QUERY);

}

int	targets::chan_limit_changed(int /* argc */, char **argv)
{
	cstring	which = argv[1];
	channel_target *t = (channel_target *) targetlist.findtarget(which);
	cstring limit = t->get_limitentry();

	if(limit.length() == 0)
		mode(which, "-l");
	else if(atoi(limit) != t->get_limit())
		mode(which, "+l " + limit);
	return true;
}

int	targets::handlemode(int /* argc */, char **argv, int cmode)
{
	cstring	which = argv[1];
	channel_target *t = (channel_target *) targetlist.findtarget(which);
	cstring 	key, cmd;

	if(t->get_cmode() & cmode)
		cmd = "-";
	else
		cmd = "+";

	switch(cmode)
	{
	case CMODE_INVITE:
		cmd += "i";
		break;
	case CMODE_KEY:
		if(cmd == "-")
		{
			key = t->get_key();
			mode(which, "-k " + key);
			t->mode_toggle(CMODE_KEY);
			t->mode_disable(CMODE_KEY);
			return true;
		}
		t->mode_toggle(CMODE_KEY);	// restore button
		return true;
	case CMODE_LIMIT:
		if(cmd == "-")
		{
			mode(which, "-l");
			t->mode_toggle(CMODE_LIMIT);
			t->mode_disable(CMODE_LIMIT);
			return true;
		}
		t->mode_toggle(CMODE_LIMIT);
		return true;
	case CMODE_MODERATE:
		cmd += "m";
		break;
	case CMODE_NO_MSG:
		cmd += "n";
		break;
	case CMODE_PRIVATE:
		cmd += "p";
		break;
	case CMODE_SECRET:
		cmd += "s";
		break;
	case CMODE_TOPIC:
		cmd += "t";
		break;
	}
	mode(which, cmd);
	t->mode_toggle(cmode);
	t->mode_disable(cmode);
	return true;
}

int	targets::chan_cmode_i_changed(int argc, char **argv)
{
	return handlemode(argc, argv, CMODE_INVITE);
}

int	targets::chan_cmode_k_changed(int argc, char **argv)
{
	return handlemode(argc, argv, CMODE_KEY);
}

int	targets::chan_cmode_l_changed(int argc, char **argv)
{
	return handlemode(argc, argv, CMODE_LIMIT);
}

int	targets::chan_cmode_m_changed(int argc, char **argv)
{
	return handlemode(argc, argv, CMODE_MODERATE);
}

int	targets::chan_cmode_n_changed(int argc, char **argv)
{
	return handlemode(argc, argv, CMODE_NO_MSG);
}

int	targets::chan_cmode_p_changed(int argc, char **argv)
{
	return handlemode(argc, argv, CMODE_PRIVATE);
}

int	targets::chan_cmode_s_changed(int argc, char **argv)
{
	return handlemode(argc, argv, CMODE_SECRET);
}

int	targets::chan_cmode_t_changed(int argc, char **argv)
{
	return handlemode(argc, argv, CMODE_TOPIC);
}

int	targets::leave_channel(int /* argc */, char **argv)
{
	remove(argv[1]);
	return true;
}

int	targets::take_focus(int /* argc */, char **argv)
{
	cstring	channel = argv[1];
	channel_target	*t = (channel_target *) targetlist.findtarget(channel);

	//t->show_entry();
	t->setfocus(true);
	set_current(channel, false);
	return true;
}

int	targets::toggleraise(int /* argc */, char **argv)
{
	cstring	channel = argv[1];
	channel_target	*t = (channel_target *) targetlist.findtarget(channel);

	t->toggle_autoraise();
	return true;
}

int	targets::togglelock(int /* argc */, char **argv)
{
	cstring	channel = argv[1];
	channel_target	*t = (channel_target *) targetlist.findtarget(channel);

	t->toggle_lock();
	return true;
}

int	targets::lose_focus(int /* argc */, char **argv)
{
	cstring	channel = argv[1];
	channel_target	*t = (channel_target *) targetlist.findtarget(channel);

//	t->hide_entry();
	t->setfocus(false);
	return true;
}

int	targets::openmenu(int /* argc */, char **argv)
{
	cstring		which = argv[1];
	channel_target	*t = (channel_target *) targetlist.findtarget(which);
	int		size = t->userlist->size();
	int		x_pos = atoi(argv[4]);
	int		y_pos = atoi(argv[5]);
	int		modes = 0;
	int		cnt = 0;

	for(int i = 0; i < size; i++)
	{
		cstring 	who = t->userlist->get(i)+1;
		int	hasmode = t->get_umode(who);

		if(t->userlist->selected(i))
		{
				if(hasmode & UMODE_OP)
					modes |= DEOP;
				else
					modes |= OP;
				if(hasmode & UMODE_VOICE)
					modes |= MUTE;
				else
					modes |= VOICE;
				cnt++;
		}
	}
	if(cnt == 0)		// no user selected, so no menu
		return false;
	if(t->get_mymode() & UMODE_OP)
		t->popup_menu(which, modes, x_pos, y_pos, true);
	else
		t->popup_menu(which, modes, x_pos, y_pos, false);
	return true;
}

int	targets::whoisselected(int, char **argv)
{
        cstring  which = argv[1];
 
        return selection_operate(which, WHOIS);
}
 
int	targets::banselected(int, char **argv)
{
        cstring  which = argv[1];

	return selection_operate(which, BAN);
}

int	targets::kickbanselected(int, char **argv)
{	
        cstring  which = argv[1];

	return selection_operate(which, KICK|BAN);
}

int	targets::queryselected(int, char **argv)
{
        cstring  which = argv[1];

	return selection_operate(which, QUERY);
}
 
int	targets::opselected(int, char **argv)
{
	cstring	which = argv[1];

	return selection_setmode(which, UMODE_OP, true);
}

int	targets::deopselected(int, char **argv)
{
        cstring  which = argv[1];
 
        return selection_setmode(which, UMODE_OP, false);
}
 
int	targets::kickselected(int, char **argv)
{
        cstring  which = argv[1];
 
        return selection_operate(which, KICK);
}
 
int	targets::voiceselected(int, char **argv)
{
	cstring	which = argv[1];

	return selection_setmode(which, UMODE_VOICE, true);
}

int	targets::muteselected(int, char **argv)
{
	cstring	which = argv[1];

	return selection_setmode(which, UMODE_VOICE, false);
}

int	targets::ctcpselected(int, char **argv) 
{
	cstring	channel = argv[1];
	cstring command = argv[3];
	int	size;
	channel_target	*t = (channel_target *) targetlist.findtarget(channel);

	if(t == NULL)
		return false;

	size = t->userlist->size();

	for(int i = 0; i < size; i++)
	{
		cstring	who = t->userlist->get(i)+1;
		
		if(t->userlist->selected(i))
		{
                        cstring msg = "";
                        if(command.compare("ping"))
                                msg << time(NULL);
                        ctcp(who, command.upper(), msg);
		}
	}
	
	return true;
}

int	targets::handle_input(int, char **argv) 
{
	cstring	channel = argv[1];
	channel_target	*t = (channel_target *) targetlist.findtarget(channel);

        cstring msgs = t->getinput();
        cstring_iter    msg_list = msgs;
        cstring msg;

        if(msgs.length() == 0)
                return false;

        while((int)(msg = msg_list('\n')))
        {
                if(msg[0] == '/')
                {
                        cstring cmd = msg(1);   // skip frist char
                        cstring rest = "";
                        int     idx;

                        if((idx = cmd.find(' ')) != -1)
                        {
                                rest = cmd(idx+1);
                                cmd = cmd(0, idx-1);
                        }

//                        tk->eval(chandler + " \"" + esc(cmd) +
//                                "\" \"" + esc(rest) + "\"");
                }
//                else
//                        tk->eval(chandler + " {msg} \"" + esc(channel) +
//                                " " + esc(msg) + "\"");
                t->delinput();
        }
	return true;
}

int	targets::addchannel(const cstring &which, const cstring &key)
// join a new channel if not joined, or reset the channel
{
	channel_target	*chan = (channel_target *) targetlist.findtarget(which);

	if(chan)
	{
		set_current(which, true);
		if(chan->getstate() != STATE_CLEAR)
			ircserver::join(which, key);
		return false;
	}
	ircserver::join(which, key);
	return true;
}

void	targets::userjoin(const cstring &which, const cstring &who)
/*
 * A user (who) join a channel (which). It can be me (succesfull join), or
 * some other IRC user.
 */
{
	cstring		nick, login, host;

	cstring_iter 	elt = which;
	cstring		channel, modes;
	int		usermode = 0;

	channel = elt("\x07 ");
	modes = elt("\x07 ");

	if((int)modes)
		for(int i = 0; i < modes.length(); i++)
			if(modes[i] == 'o')
				usermode |= UMODE_OP;
			else if(modes[i] == 'v')
				usermode |= UMODE_VOICE;

	channel_target	*chan = (channel_target *) targetlist.findtarget(channel);

	split_nuh(who, nick, login, host);

	if(connection::nick == nick)
	{
		if(chan == NULL)
		{
			chan = new channel_target(channel, connection::sessionid);

			targetlist.add(channel, chan);
			chan->bind(this, targets::chanwinevent);

			connection::add_target(channel);
			set_current(channel, true);
			refocus();
		}
		chan->setstate(STATE_CLEAR);
		mode(channel);
		mode(channel, "b");
		who(channel);
	}
	else
		chan->add_user(who, 0, usermode);

	// Notify the user of the join, even if it was the user
	// itself
	// say something about possible serverop?
	chan->info_insert(nick + " (" + login + "@" + host + 
                          ") has joined channel " + channel + "\n");
	if(usermode & UMODE_OP)
		chan->info_insert(nick + " has been made operator "
		                  "by the server\n");
	if(usermode & UMODE_VOICE)
		chan->info_insert(nick + " has been given a voice "
		                  "by the server\n");
	chan->insert(nick + " has joined\n", "notice");
}

void	targets::userleave(const cstring &which, const cstring &who, 
                            const cstring &reason)
/* 
 * A user left (who) the channel (which). It can be me (delete window),
 * or some other IRC user (update userlist)
 */
{
	cstring	nick, user, host;

	split_nuh(who, nick, user, host);

	if(nick == connection::nick)
	{
		targetlist.del(which);
		connection::del_target(which);
	}
	else
	{
		channel_target	*chan = (channel_target *) targetlist.findtarget(which);
		
		chan->del_user(nick);
		if((int)reason && !(reason == nick))
			chan->info_insert(nick + " has left channel " + 
			                  which + " (" + reason + ")\n");
		else
			chan->info_insert(nick + " has left channel " + 
			                  which + "\n");
		chan->insert(nick + " has left\n", "notice");
	}
}

channel_target 	*targets::createchannel(const cstring &which)
// Create a channelwindow, i.e. when we try to join a channel, but failed.
// once we actually rejoin, modes etc should be requested
{
	channel_target	*chan = (channel_target *) targetlist.findtarget(which);

	if(chan == NULL)
	{
		chan = new channel_target(which, connection::sessionid);

		targetlist.add(which, chan);
		chan->bind(this, targets::chanwinevent);
		add_target(which);
		set_current(which, true);
		refocus();
	}
	return chan;
}

void	targets::removechannel(const cstring &which, const cstring &comment)
// PART a channel if we're still on it, else delete the window
{
	channel_target	*t = (channel_target *) targetlist.findtarget(which);

	if(t->getstate() != STATE_CLEAR)
		targetlist.del(which);
	else
		ircserver::part(which, comment);
}

int	targets::delchannel(const cstring &which)
{
	if(!targetlist.del(which))
		return false;
	del_target(which);
	return true;
}

int	targets::whoisinfo(int argc, char **argv)
{
	cstring	nick(argv[4]);

	querywin	*q = (querywin*)targetlist.findtarget(nick);
	
	if(q)
		return q->wantinfo(argc, argv);
	else if(messages) 
		return messages->wantinfo(argc, argv);
	return false;
}

int	targets::wantinfo(int argc, char **argv)
{
	cstring rpl = argv[2];
	cstring channel;

	if(rpl == "353")
		channel = argv[5];
	else
		channel = argv[4];
	channel_target	*chan;

	chan = (channel_target *) targetlist.findtarget(channel);

	if(chan == NULL)
		return false;

	return chan->wantinfo(argc, argv);
}

void	targets::kickuser(const cstring &who, const cstring &from,
                           const cstring &where, const cstring &why)
{
	channel_target	*t = (channel_target *) targetlist.findtarget(where);

	if(t == NULL)		// might be NULL 
		return;

	cstring	nick, user, host;

	split_nuh(from, nick, user, host);

	cstring reason;

	if(why == nick)
		reason = "";
	else
		reason = '(' + why + ')';

	if(who == connection::nick)	// is it me?
	{
		t->info_insert("You have been kicked off channel " +
		               where + " by " + nick + " " + reason + "\n");
		t->setstate(STATE_KICKED);
		// try an immediate rejoin
		connection::join(t->channelname, t->get_key());
	}
	else
	{
		t->info_insert(who + " has been kicked off channel " +
		               where + " by " + nick +" " + reason + "\n");
		t->insert(who + " has been kicked\n", "notice");
		t->del_user(who);
	}
}

int	targets::selection_togglemode(const cstring &, const int )
// Toggle the op-status of the selected users
{
	return true;
}

int	targets::selection_setmode(const cstring &, const int, 
                                    const int)
// Give or take voice/operator from selected users
{
	return true;
}

int	targets::selection_operate(const cstring &chan, const int &operation)
{
	int	size;
	cstring	comment;

	channel_target	*t = (channel_target *) targetlist.findtarget(chan);

	if(t == NULL)
		return false;

	size = t->userlist->size();

	if(operation & KICK)
		comment = getinput();

	for(int i = 0; i < size; i++)
	{
		cstring	who = t->userlist->get(i)+1;
		
		if(t->userlist->selected(i))
		{
			if(operation & WHOIS)
				whois(who);
			if(operation & QUERY)
				targets::addquery(who);
			if(operation & BAN)
				mode(chan, "+b *!*"+
				  t->get_userlogin(who) + "@" + 
				  t->get_userhost(who));
			if(operation & KICK)
				kick(chan,  who, comment);
		}
	}

	if(operation & KICK)
		// erase possible comment
		delinput();
	return true;
}

void	targets::reset(void)
// Rejoin all channels (i.e. after disconnect)
// They have to be disabled first somewhere else
{
	target_iterator iter;

	for(iter = targetlist.begin(); iter != targetlist.end(); iter++)
	{
		channel_target *t = (channel_target *) (*iter).second;

		join(t->channelname, t->get_key());
	}
}

/*
 *
 * QUERIES
 *
 */


void	targets::toggle_message_window(void)
{
	if(messages == NULL)
	{
		messages = targets::create_message_window();
	}
	else
	{
		cstring wpath(esc(messages->path()));
		cstring state(tk->evaluate("wm state " + wpath));
		
		if(state == "normal")
			tk->eval("wm iconify " + wpath); // is escaped
		else if(state == "iconic")
		{
			tk->eval("wm deiconify " + wpath); // is escaped
		}
	}
}

void	targets::close_message(void)
{
	messages->destroy();
	messages = NULL;
	insert("Messagewindow closed\n");
}

void	targets::close_query(const cstring which)
{
	querywin	*q = (querywin *)targetlist.findtarget(which);
	cstring		person(q->get_person());

	insert("Conversation with " + person + " ended\n");
	// remove from masterwindow
	connection::del_target(which);
	// remove from query list
	targetlist.del(which);
}

void	targets::toggle_dcc(const cstring &which)
{
	querywin	*q = (querywin *)targetlist.findtarget(which);

	if(q->getdcc() == querywin::none)	// no dcc
	{
		tk->eval(esc("dcc chat " + which));
		q->insert("Starting dcc chat\n", "notice");
		q->setdcc(querywin::started);
	}
	else
	{
		tk->eval(esc("dcc close chat " + which));
//		q->insert("Aborting dcc chat\n", "notice");
		q->setdcc(querywin::none);
	}
}

void	targets::toggle_lock(const cstring &which)
{
	if(which == "")
		messages->toggle_lock();
	else
	{
		querywin *q = (querywin *)targetlist.findtarget(which);
		q->toggle_lock();
	}
}

void	targets::toggle_chat(const cstring &which)
{
	querywin	*q = (querywin *)targetlist.findtarget(which);

	q->setchat(!q->getchat());

	if(q->getchat())
		if(q->getdcc() == querywin::none)
		{
			q->insert("Starting dcc chat\n", "notice");
			tk->eval(esc("dcc chat " + which));
		q->setdcc(querywin::started);
		}
		else if(q->getdcc() == querywin::started)
			q->insert("Waiting for remote party to accept chat\n", 
			          "notice");
		else
			q->insert("Using dcc chat\n", "notice");
	else
		q->insert("Using normal messages\n", "notice");
}

void	targets::handle_input(const cstring &which)
// which can be an empty string, in which case the messagewindow will be used.
{
	targetif		*q;

	if(which == "")
		q = messages;
	else
	{
		q = targetlist.findtarget(which);
	}
	cstring		msgs = q->getinput();

        cstring_iter    msg_list = msgs;
        cstring msg;

        if(msgs.length() == 0)
                return;

        while((int)(msg = msg_list('\n')))
        {
                if(msg[0] == '/')
                {
                        cstring cmd = msg(1);   // skip first char
                        cstring rest = "";
                        int     idx;

                        if((idx = cmd.find(' ')) != -1)
                        {
                                rest = cmd(idx+1);
                                cmd = cmd(0, idx-1);
                        }

//                        tk->eval(c->chandler + " \"" + esc(cmd) +
//                               "\" \"" + esc(rest) + "\"");
                }
//		else if(q->getchat() && q->getdcc() == querywin::active)
//               		tk->eval(c->chandler + " dcc \"msg " + esc(which) +
//                                 " " + esc(msg) + "\"");
		else if(which == "")
		// combine nicknames in nick1,nick2,nick3
		{
			int     size = messages->targetlist->size();
			int	count = 0;

			for(int i = 0; i < size; i++)
				if(messages->targetlist->selected(i))
				{
					cstring who = 
					        messages->targetlist->get(i);
//               				tk->eval(c->chandler + " {msg} \"" + 
//					         esc(who) +
//                                                 " " + esc(msg) + "\"");
					count++;
				}
			if(count == 0)
				messages->insert("No target selected!\n", 
				                 "notice");
		}
//		else
//               		tk->eval(c->chandler + " {msg} \"" + esc(which) +
//                                 " " + esc(msg) + "\"");
        }
        q->delinput();
}

void	targets::selection_operate(const int &operation, 
				   const cstring &inv_ch)
{
	int	size = messages->targetlist->size();

	for(int i = 0; i < size; i++)
	{
		if(messages->targetlist->selected(i))
		{
			cstring	who = messages->targetlist->get(i);

			if(operation & QUERY)
				addquery(who);
			else if(operation & WHOIS)
				ircserver::whois(who);
			else if(operation & INVITE)
				ircserver::invite(who, inv_ch);
		}
	}
}

void	targets::selection_ctcp(const cstring &command)
{
	int	size = messages->targetlist->size();

	for(int i = 0; i < size; i++)
	{
		if(messages->targetlist->selected(i))
		{
			cstring	who = messages->targetlist->get(i);

                        cstring msg = "";
                        if(command.compare("ping"))
                                msg << time(NULL);
                        ircserver::ctcp(who, command.upper(), msg);
		}
	}
}

int	targets::handle_command(int argc, char **argv)
// QUERY recvchat VladDrac!user@host "Hello world"
// QUERY dcc VladDrac on/off/toggle
{
	cstring	qcommand = "";

	if(argc >= 2)
		qcommand = argv[1];
	if(argc == 4 && (qcommand == "dcc"))
	{
		cstring	mode = argv[3];
		cstring	nick, login, host;

		split_nuh(argv[2], nick, login, host);
		querywin	*q = (querywin *)targetlist.findtarget(nick);

		if(!q)
			return false;

		if(mode == "on")
		{
			q->insert("DCC connection accepted\n", "notice");
			q->setdcc(querywin::active);
		}
		else if(mode == "off")
		{
			q->insert("DCC connection closed\n", "notice");
			q->setdcc(querywin::none);
		}
		return true;
	}
	if(argc != 4)
	{
		tk->result("Incorrect param count");
		return false;
	}
	
	if(!(qcommand == "recvchat"))
	{
		tk->result("unknown command");
		return false;
	}
	recvchat(argv[2], argv[3]);
	return true;
}

int     targets::operator()()
{
	int	argc = _event->argc();
	char	**argv = _event->argv();

	cstring  command = _event->arg(3);
	func_packer  *d = (func_packer *) dispatch_table[command];

	if(d)
		d->execute(_event->argc(), _event->argv());
	else if(dbg > 5)
	{
		cout << "ChanWinEvent: Could not handle \"" 
                     << command << "\"" << endl;
		for(int i = 0; i < argc; i++)
			cout << "arg: " << i << ": " << argv[i] << endl;
	}

/*
 * query operator()
 */

	cstring	which = argv[1];

	if(dbg > 5)
	{
            cout << "Handling QueryEvent" << endl;
            for(int i = 0; i < argc; i++)
                cout << "arg: " << i << ": " << argv[i] << endl;
	}

 	// menu events require special treatment
	cstring	arg0 = argv[0];
	// Is it the tcl QUERY command?
	if(arg0 == "query")
		return handle_command(argc, argv);
	if((arg0 == "QueryWin") || (arg0 == "MessageWin"))
		command = argv[3];
	else
		command = argv[2];
	if(command == "MessMenu")
		command = argv[1];

	// time for a dictionary?

	if(command == "close_query")
		close_query(which);
	else if(command == "togglelock")
		if(arg0 == "MessageWin")
			toggle_lock("");
		else
			toggle_lock(which);
	else if(command == "dcc_pressed")
		toggle_dcc(which);
	else if(command == "chat_pressed")
		toggle_chat(which);
	else if(command == "input_typed")
		if(arg0 == "MessageWin")
			handle_input("");	// "" means messagewin
		else
			handle_input(which);
	else if(command == "close_message")
		close_message();
	else if(command == "open_mess_menu")
		popup_menu(_event->x_root(), _event->y_root());
	else if((command == "start_query") || (command == "Query"))
		selection_operate(QUERY);
	else if(command == "Whois")
		selection_operate(WHOIS);
	else if(command == "Invite")
		selection_operate(INVITE, which);
	else if(command == "CTCP")
		selection_ctcp(which);
	else if(command == "take_focus")
	{
		querywin	*q = (querywin *)targetlist.findtarget(which);

		connection::set_current(q->get_person());
		set_current(q->get_person(), false);
	}
 	else if(dbg > 5)
        {
                cout << "Queryevent: Could not handle \"" 
                     << command << "\"" << endl;
                for(int i = 0; i < argc; i++)
                        cout << "arg: " << i << ": " << argv[i] << endl;
        }
        return true;
}

/*
 * Try to merge addquery/delquery into addchannel/delchannel
 */

int	targets::addquery(const cstring &which)
{
	querywin	*q = (querywin *)targetlist.findtarget(which);

	if(q)
	{
		set_current(which, true);
		return false;
	}

	q = new querywin(which);
	q->sessionid = sessionid;

	targetlist.add(which, (targetif *)q);
	ircserver::whois(which);
	q->bind(this);
	connection::add_target(which);
	set_current(which, true);
	insert("Conversation with " + which + " started\n");
	if(messages)
		messages->del_person(which);
	refocus();
	return true;
}

int	targets::delquery(const cstring &which)
{
	if(targetlist.del(which))
	 	connection::del_target(which);
	else
		return false;
	return true;
}

messagewin	*targets::create_message_window(void)
{
	if(messages)
		return messages;

	messages = new messagewin(".messages");

	messages->sessionid = sessionid;
	messages->bind(this);
	refocus();
	return messages;
}

void	targets::set_current(const cstring &t, int doraise)
{
	targetlist.active(current, false, doraise);
	targetlist.active(t, true, doraise);
	
	connection::set_current(t);
}
void	targets::recv(const cstring &prompt, const cstring &from, 
                      const cstring &msg)
{
	targetif *q;
	cstring	nick, login, host;

	split_nuh(from, nick, login, host);

 	q = targetlist.findtarget(nick);

	if(q == NULL)
	{
		q = create_message_window();

		cstring wpath(esc(messages->path()));
		cstring state(tk->evaluate("wm state " + wpath));

		if(messages->get_autoraise())
			tk->eval("raise " + wpath);
		messages->add_person(nick);
		messages->nickentry->configure("-state normal");
		messages->nickentry->del();
		messages->nickentry->insert(nick+" ("+login+"@"+host+")");
		messages->nickentry->configure("-state disabled");
		// find if there is a selection

		int     size = messages->targetlist->size();
		int	count = 0;

		for(int i = 0; i < size; i++)
			if(messages->targetlist->selected(i))
			{
				count++;
				break;  // there's a selection, no need to cont.
			}
		if(count == 0)
			messages->targetlist->select(0,0);

	}
#warning "implement targetif::get_autoraise??"

//	else if(q->get_autoraise())
//	{
//		tk->eval("wm deiconify " + cstring(q->path()));
//		tk->eval("raise " + cstring(q->path()));
//	}
	q->insert(prompt, msg + "\n", 13, "nick_other");
}

void	targets::send(const char pchar, const cstring &to, const cstring &msg)
{
	targetif 	*q = targetlist.findtarget(to);
	cstring 	prompt;

	if(q == NULL)
	{
		q = create_message_window();

		prompt << "=>" << pchar << to << pchar;
		messages->add_person(to);

		int     size = messages->targetlist->size();
		int	count = 0;

		for(int i = 0; i < size; i++)
			if(messages->targetlist->selected(i))
			{
				count++;
				break;  // there's a selection, no need to cont.
			}
		if(count == 0)
			messages->targetlist->select(0,0);
	}
	else
		prompt << pchar << connection::nick << pchar;

	// to delete the awaymessage
//	q->miscentry->configure("-state normal");
//	q->miscentry->del();
//	q->miscentry->configure("-state disabled");

	q->insert(prompt, msg + "\n", 13, "nick_me");
}

void	targets::recvchat(const cstring &from, const cstring &msg)
{
	cstring	nick, login, host;

	split_nuh(from, nick, login, host);
	recv('='+nick+'=', from, msg);
}

void	targets::sendchat(const cstring &to, const cstring &msg)
{
	send('=', to, msg);
}

void	targets::recvmsg(const cstring &from, const cstring &to,
                         const cstring &msg)
{
	cstring	nick, login, host;

	split_nuh(from, nick, login, host);

	if(to == connection::nick)	// message 
		recv('*'+nick+'*', from, msg);
	else
	{
		channel_target	*chan = (channel_target *) 
		                        targetlist.findtarget(to);
		chan->insert("<"+nick+">", msg + "\n", 12, "nick_other");
	}
}

void	targets::sendmsg(const cstring &to, const cstring &msg)
{
	if(ischannel(to))
	{
		channel_target	*chan = (channel_target *) 
		                        targetlist.findtarget(to);

		if(chan == NULL)
		// message sent to non-joined channel
			insert("><"+to+">", msg + "\n", 12, "nick_me");
		else
			chan->insert("<"+nick+">", msg + "\n", 12, "nick_me");
	}
	else
		send('*', to, msg);
}

void	targets::recvnotice(const cstring &from, const cstring &to, 
                             const cstring &msg)
{
	cstring		nick, login, host;

	split_nuh(from, nick, login, host);

	if(to == connection::nick)
		recv('-'+nick+'-', from, msg);
	else
	{
		channel_target	*chan = (channel_target *) 
					targetlist.findtarget(to);
		chan->insert("-"+nick+"-", msg + "\n", 12, "nick_other");
	}
}

void	targets::sendnotice(const cstring &to, const cstring &msg)
{
	if(ischannel(to))
	{
		channel_target	*chan = (channel_target *) 
		                        targetlist.findtarget(to);

		if(chan == NULL)
		// sent to non-joined channel
			insert(">-"+to+"-", msg + "\n", 12, "nick_me");
		else
			chan->insert("-"+nick+"-", msg + "\n", 12, "nick_me");
	}
	else
		send('-', to, msg);
}

void	targets::recvaction(const cstring &from, const cstring &to,
                            const cstring &msg)
{
	cstring	nick, login, host;

	split_nuh(from, nick, login, host);

	if(to == connection::nick)
	{
	
		targetif *q;

 		q = targetlist.findtarget(nick);

		if(q == NULL)
		{
			q = create_message_window();

			cstring wpath(esc(messages->path()));
			cstring state(tk->evaluate("wm state " + wpath));

			if(messages->get_autoraise())
				tk->eval("raise " + wpath);
			messages->add_person(nick);
			messages->nickentry->configure("-state normal");
			messages->nickentry->del();
			messages->nickentry->insert(nick+
			                     " ("+login+"@"+host+")");
			messages->nickentry->configure("-state disabled");
		}
		q->insert("->"+nick, msg + "\n", 13, "nick_other");
	}
	else
	{
		channel_target	*chan = (channel_target *) 
		                         targetlist.findtarget(to);

		chan->insert("->"+nick, msg + "\n", nick.length() + 2, 
		             "nick_other");
	}
}

void	targets::sendaction(const cstring &to, const cstring &msg)
{
	if(ischannel(to))
	{
		channel_target	*chan = (channel_target *) 
		                        targetlist.findtarget(to);

		if(chan == NULL)
			insert("->"+to+":"+nick, msg + "\n", 12, "nick_me");
		else
			chan->insert("->"+nick, msg + "\n", nick.length() + 2, 
		             "nick_me");
	}
	else
	{
		targetif *q = targetlist.findtarget(to);

		if(q == NULL)
		{
			q = create_message_window();

			cstring wpath(esc(messages->path()));
			cstring state(tk->evaluate("wm state " + wpath));
			messages->add_person(to);
		}

//		// to delete the awaymessage
//		q->miscentry->configure("-state normal");
//		q->miscentry->del();
//		q->miscentry->configure("-state disabled");

		q->insert("->"+connection::nick, msg + "\n", 13, "nick_me");
	}
}

int	targets::insert_info(cstring which, const cstring &msg)
{
	channel_target	*chan = (channel_target *) targetlist.findtarget(which);
	
	if(chan)
	{
		chan->info_insert(msg);
		return true;
	}
	insert("-> " + which + ": " + msg);
	return false;
}

int	targets::insert_msg(cstring which, const cstring &msg)
{
	channel_target	*chan = (channel_target *) targetlist.findtarget(which);
	
	if(chan)
	{
		chan->insert(msg);
		return true;
	}
	insert("-> " + which + ": " + msg);
	return false;
}

void	targets::bind(handler *h)
{
	targetcallback = h;
}

void    targets::popup_menu(int, int)
{
}
 
int	targets::handlechanwinevent(void)
{
	cout << "handlerchanwinevent" << endl;

//handler *h = this;

//((targets*)h)->popup_menu(1,1);
	int	argc = _event->argc();
	char	**argv = _event->argv();

	for(int i = 0; i < argc; i++)
		cout << "arg: " << i << ": " << argv[i] << endl;

	targetcallback->dispatch(_event);
	return 0;
}

int	targets::chanwinevent(void *h, event *e)
{
	cout << "chanwinevent" << endl;

	((targets *)h)->setevent(e);
	/* 
	 * first cast handler *h to void, then to target*
	 * It's ugly I know but it's the only solution gcc accepts
	 * (it causes a problem because handler is a public virtual base
	 * class for connection)
	 */
	return ((targets *)h)->handlechanwinevent();
}
