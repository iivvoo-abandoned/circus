/*
 * serverhandler.c - handlers irc2 protocol events from class ircserver
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
#include <stdlib.h>
#include <errno.h>

#include <ctype.h>		// isdigit

#include <time.h>		// localtime

#include "serverhandler.h"

#include "cstring.h"
#include "misc.h"

#include "const.h"

extern	int	dbg;

#define	pack(f)	new func_packer(this, &serverhandler::f)

serverhandler::serverhandler(): ctcphandler()
{
	dispatch_table["ERROR"] =  pack(dispatch_error);
	dispatch_table["JOIN"] =  pack(dispatch_join);
	dispatch_table["KICK"] = pack(dispatch_kick);
	dispatch_table["MODE"] = pack(dispatch_mode);
	dispatch_table["NICK"] = pack(dispatch_nick);
	dispatch_table["PART"] = pack(dispatch_leave);
	dispatch_table["PING"] =  pack(dispatch_ping);
	dispatch_table["PRIVMSG"] = pack(dispatch_privmsg);
	dispatch_table["TOPIC"] = pack(dispatch_topic);
	dispatch_table["NOTICE"] = pack(dispatch_privmsg);
	dispatch_table["QUIT"] = pack(dispatch_quit);
	dispatch_table["INVITE"] = pack(dispatch_invite);

	// Lots of servermessage don't really have anything to
	// do with the MOTD and can just be redirected to the master
	// window. dispatch_motd is probably a bad choice for a name
	dispatch_table["001"] = pack(dispatch_welcome);
//	dispatch_table["004"] = pack(dispatch_motd);
	dispatch_table["221"] = pack(dispatch_rplumodeis);

	// ISON REPLY
	dispatch_table["303"] = pack(dispatch_isonreply);

	// AWAY REPLY
	dispatch_table["301"] = pack(dispatch_whoisreply);
	dispatch_table["305"] = pack(dispatch_awayreply);
	dispatch_table["306"] = pack(dispatch_awayreply);

	// WHOIS REPLY/WHO REPLY
	dispatch_table["311"] = pack(dispatch_whoisreply);
	dispatch_table["312"] = pack(dispatch_whoisreply);
	dispatch_table["313"] = pack(dispatch_whoisreply);
	dispatch_table["314"] = pack(dispatch_whoisreply);
	dispatch_table["315"] = pack(dispatch_whoreply);
	dispatch_table["317"] = pack(dispatch_whoisreply);
	dispatch_table["318"] = pack(dispatch_whoisreply);
	dispatch_table["319"] = pack(dispatch_whoisreply);
	dispatch_table["401"] = pack(dispatch_whoisreply);

	dispatch_table["324"] = pack(dispatch_rplchanmodeis);
	dispatch_table["329"] = pack(dispatch_chancreated);
	dispatch_table["332"] = pack(dispatch_rpltopic);
	dispatch_table["333"] = pack(dispatch_topicset);
	dispatch_table["352"] = pack(dispatch_whoreply);
	dispatch_table["353"] = pack(dispatch_namereply);
	dispatch_table["366"] = pack(dispatch_namereply);
	dispatch_table["367"] = pack(dispatch_banlistreply);
	dispatch_table["368"] = pack(dispatch_banlistreply);

	// Resource (nick/channel) unavailable
	dispatch_table["437"] = pack(dispatch_nickchanlock);

	// Channeljoin errors
	dispatch_table["471"] = pack(dispatch_nojoin);
	dispatch_table["473"] = pack(dispatch_nojoin);
	dispatch_table["474"] = pack(dispatch_nojoin);
	dispatch_table["475"] = pack(dispatch_nojoin);

	replies["WALLOPS"] = reg_string("WALLOP -$1- $3");
	replies["002"] = reg_string("$4");
	replies["003"] = reg_string("$4");
	replies["004"] = reg_string("$4");	// available usermodes etc.
	replies["005"] = reg_string("$4");	// undernet 'map' reply
	replies["006"] = reg_string("$4");
	replies["007"] = reg_string(".oO0Oo. End of MAP .oO0Oo.");

	// trace replies
	replies["200"] = reg_string("$4 $5 $6 $7");
	replies["201"] = reg_string("$4 $5 $6");
	replies["202"] = reg_string("$4 $5 $6");
	replies["203"] = reg_string("$4 $5 $6 $7");
	replies["204"] = reg_string("$4 [$3.5] => $6");
	replies["205"] = reg_string("$4 [$3.5] => $6");
	replies["206"] = reg_string("$4 [$3.5] => $6 $7 $8 $9");
	replies["208"] = reg_string("$4 $5 $6");
	replies["209"] = reg_string("$4 $3.5 Entries linked: $6");

	// stats stuff
	replies["211"] = reg_string("$4 $5 $6 $7 $8 $9 $10");

	replies["213"] = reg_string("$4:$5:$6:$7:$8:$9"); // stats c
	replies["214"] = reg_string("$4:$5:$6:$7:$8:$9"); // stats n
	replies["215"] = reg_string("$4:$5:$6:$7:$8:$9"); // stats i
	replies["216"] = reg_string("$4:$5:$6:$7:$8:$9"); // stats k
	replies["218"] = reg_string("$4:$5:$6:$7:$8:$9"); // stats y
	replies["219"] = reg_string(".oO0Oo. End of STATS $4 .oO0Oo.");
	replies["241"] = reg_string("$4 $5 $6 $7 $8 $9 $10"); // stats l
	replies["242"] = reg_string("$4");		// stats u
	replies["243"] = reg_string("$4:$5:$6:$7:$8:$9"); // stats o
	replies["244"] = reg_string("$4:$5:$6:$7:$8:$9"); // stats h

	replies["249"] = reg_string("$4");
	
	replies["250"] = reg_string("$4");	// highest connection count
	replies["251"] = reg_string("$4");
	replies["252"] = reg_string("There are $4 $5");
	replies["253"] = reg_string("There are $4 $5");
	replies["254"] = reg_string("There are $4 $5");
	replies["255"] = reg_string("$4");

	// Admin replies
	replies["256"] = reg_string("$4");
	replies["257"] = reg_string("$4");
	replies["258"] = reg_string("$4");
	replies["259"] = reg_string("$4");

	replies["261"] = reg_string("$4 $5 $6");
	replies["262"] = reg_string(".oO0Oo. End of TRACE .oO0Oo.");

	// us-efnet stuff (new numerics)
	replies["265"] = reg_string("$4");
	replies["266"] = reg_string("$4");

	replies["301"] = reg_string("");
	// userhost reply
	replies["302"] = reg_string("$4");
	replies["311"] = reg_string("$4 is $5@$6 ($8)");
	replies["312"] = reg_string("-- Using server $5 ($6)");
	replies["313"] = reg_string("-- Is an IRC-operator");
	replies["314"] = reg_string("$4 was $5@$6 ($8)");
	replies["315"] = reg_string("");		// End of WHO reply
	replies["317"] = reg_string("-- Has been idle for $5 second(s)");
	replies["318"] = reg_string("");		// end of whois
	replies["319"] = reg_string("-- Is on channels $5");
	replies["321"] = reg_string("Channel    users Topic");
	replies["322"] = reg_string("$-10.4 $-5.5 $6");
	replies["323"] = reg_string("");		// end of channel list
	replies["341"] = reg_string("Inviting $4 to $5");
	replies["352"] = reg_string("$-10.4 $9.8 $-2.9 $5@$6 ($10)");
	replies["353"] = reg_string("$-10.5 $6");
	replies["364"] = reg_string("$-20.4 - $-20.5: $6"); // RPL_LINKS
	replies["365"] = reg_string("");		// end of links list
	replies["366"] = reg_string("");		// end of names list
	replies["367"] = reg_string("$-10.4 $5");	// banlistreply
	replies["368"] = reg_string("");		// end of banlist
	replies["369"] = reg_string("");		// end of whowas
	replies["371"] = reg_string("$4");
	replies["372"] = reg_string("$4");
	replies["374"] = reg_string(".oO0Oo. Message of Info .oO0Oo.");
	replies["375"] = reg_string(".oO0Oo. Message of the day .oO0Oo.");
	replies["376"] = reg_string(".oO0Oo. End of message of the day .oO0Oo.");

	replies["377"] = reg_string("$4");		// us-efnet

	replies["381"] = reg_string("You are now IRC operator");
	replies["382"] = reg_string("Rehashing $4");
	replies["391"] = reg_string("Time/Date is: $5");

	// weird USER replies
	replies["392"] = reg_string("UserID  Terminal Host");
	replies["393"] = reg_string("$-8.4 $-9.5 $-8.6");
	replies["394"] = reg_string(".oO0Oo. End of USERS .oO0Oo.");
	replies["395"] = reg_string("Nobody logged on");

	replies["401"] = reg_string("$4 is not online");
	replies["406"] = reg_string("$4 was not online");
	replies["408"] = reg_string("No service named '$4'");
	replies["412"] = reg_string("No text to send typed");
	replies["421"] = reg_string("$5: $4");		// unknown command
	replies["432"] = reg_string("Incorrect nickname $4");
	replies["433"] = reg_string("Nickname $4 already in use");
	// undernet reply - to prevent nickfloods
	replies["438"] = reg_string("$5");		// 'Nick change ...'
	replies["441"] = reg_string("$4 is not on channel $5");
	replies["442"] = reg_string("You have not joined channel $4");
	replies["443"] = reg_string("$4 $6 $5"); 	//already on channel
	replies["461"] = reg_string("Not enough parameters for $4");
	replies["462"] = reg_string("Already connected - command not allowed");
	replies["464"] = reg_string("Incorrect password");
	replies["471"] = reg_string("Userlimit on $4 has been reached");
	replies["473"] = reg_string("Channel $4 is invite-only");
	replies["474"] = reg_string("You are banned from channel $4");
	replies["475"] = reg_string("Wrong key used for channel $4");
	replies["481"] = reg_string("You're not IRC operator");
	replies["482"] = reg_string("$5 on $4");	//not op
	replies["491"] = reg_string("There are no O-lines for your host");
	replies["502"] = reg_string("You cannot change mode for other users.");
}

serverhandler::~serverhandler()
{
}

	

cstring	*serverhandler::reg_string(const cstring &val)
{
	cstring	*s = new cstring(val);
	strings._register(s);
	return s;
}

int	serverhandler::expand_reply(cstring &result, int argc, char **argv)
{
	cstring  rpl = argv[2];
	cstring	*Bar, Foo;

	result = "";
	Bar = replies[rpl];
	if(Bar == NULL)
		return false;

	Foo = *Bar;

	if(Foo == "")			// empty string
		return true;		// ugly hack
	int i = 0;

	// this is VOC = very obscure code

	while(i < Foo.length())
	{
		if(Foo[i] == '$')
		{
			cstring	align = "";

			i++;
			if(Foo[i] == '$')
				result += (char)Foo[i++];
			else
			{
				int	j;
				cstring	fill = "";
				char	fillbuf[10];

				if(Foo[i] == '-')
				{
					align = "-";
					i++;
				}
				for(j = i; j < Foo.length() &&
				    isdigit(Foo[j]) && Foo[j] != '.';
				    j++)
					fillbuf[j-i] = Foo[j];
				fillbuf[j-i] = '\0';

				// Found a dot?

				if(Foo[j] == '.')
				{
					fill = fillbuf;
					i = j+1;
				}

				char	pbuf[10];
				for(j = i; j < Foo.length() && isdigit(Foo[j]);
				    j++)
				    	pbuf[j-i] = Foo[j];
				pbuf[j-i] = '\0';
				i = j;

				int	p = atoi(pbuf);

				if(p <= 0 || p >= argc)
				{
					cout << "INTERNAL ERROR: " <<
					   "Request for argument " << p
				     	<< " while reply " << rpl
				     	<< " only has " << argc << "Params: "
				     	<< "[" << Foo << "]" << endl;
					tk->quit();
				}

				if(fill == "")
					result += argv[p];
				else
				{
					char buf[255];

					cstring format = "%" + align + 
					                      fill +"s";

					sprintf(buf, format, argv[p]);
					result += buf;
				}
			}
		}
		else
			result += (char)Foo[i++];
	}
	return true;
}

int	serverhandler::handle_reply(int argc, char **argv)
{
	cstring	 result;

	if(expand_reply(result, argc, argv))
	{
		if(result != (const char *) "")
			insert(result + "\n");
		return true;
	}
	return false;
}

int	serverhandler::dispatch_error(int, char **argv)
{
	cstring	error = argv[3];
	cstring	type;
	int	idx;

	if((idx = error.find(':')) != -1)
	{
		type = error(0, idx-1);
		// add 2 to skip ':' and space
		error = error(idx+2);

		if(type == "Closing Link")
			disconnected(error);
		else
			disconnected(type + ": " + error);
	}
	else
		disconnected(error);
	targetlist.disable();
	return true;
}

int	serverhandler::dispatch_join(int, char **argv)
{
	targets::userjoin(argv[3], argv[1]);
	return true;
}

int	serverhandler::dispatch_kick(int, char **argv)
{

	targets::kickuser(argv[4], argv[1], argv[3], argv[5]); 
	return true;
}

int	serverhandler::dispatch_leave(int, char **argv)
{
	// undernet doesn't have argv[4] - leave comment
	// pass an empy string in that case
	char	*comment = argv[4]?argv[4]:"";

	targets::userleave(argv[3], argv[1], comment);
	return true;
}

int	serverhandler::dispatch_mode(int argc, char **argv)
{
	cstring	from = argv[1];
	cstring	which = argv[3];
	cstring	modes = argv[4];
	// targets start at argv[5]

	cstring	nick, login, host;
	cstring	targ;

	if(which == connection::nick)		// usermode reply!
		return dispatch_rplumodeis(argc, argv);

	if(isserver(from))
		nick = "server " + from;
	else
		split_nuh(from, nick, login, host);

	channel_target	*t = (channel_target *)targetlist[which];
	
	int	on = true;
	int	targ_cnt = 5;

	for(int i = 0; i < modes.length(); i++)
		switch(modes[i])
		{
		case '+':
			on = true;
			break;
		case '-':
			on = false;
			break;
		case 'o':
			targ = argv[targ_cnt++];
			if(on)
			{

				t->set_umode(targ, UMODE_OP);
				t->info_insert(targ + 
				               " has been made operator on " +
					       which + " by " + nick + "\n");
			}
			else
			{
				t->del_umode(targ, UMODE_OP);
				t->info_insert(targ +
				               " is no longer operator on " + 
					       which + " thanks to " + nick + 
					       "\n");
			}
			if(targ.compare(nick.lower()))
			{
				t->set_op(on);
				//bans->op();
			}
			break;
		case 'v':
			targ = argv[targ_cnt++];
			if(on)
			{
				t->set_umode(targ, UMODE_VOICE);
				t->info_insert(targ + 
				               " has been given a voice on " +
					       which + " by " + nick + "\n");
			}
			else
			{
				t->del_umode(targ, UMODE_VOICE);
				t->info_insert(targ + 
				               " no longer has a voice on " +
					       which + " thanks to " + nick + 
					       "\n");
			}
			if(targ.compare(nick.lower()))
				t->set_voice(on);
			break;
		case 'i':
			if(on)
			{
				t->set_cmode(CMODE_INVITE);
				t->info_insert(which + " has been made invite-"
				 	       "only by " + nick + "\n");
			}
			else
			{
				t->del_cmode(CMODE_INVITE);
				t->info_insert(which + " is no longer invite-"
					       "only thanks to " + nick + "\n");
			}
			t->mode_toggle(CMODE_INVITE, on);
			t->mode_enable(CMODE_INVITE);
			break;
		case 'k':
			targ = argv[targ_cnt++];
			if(on)
			{
				t->set_cmode(CMODE_KEY);
				t->set_key(targ);
				t->info_insert(nick + " has set the key " +
				               targ + " for channel " +
					       which + "\n");
				t->set_keyentry(targ);
			}
			else
			{
				t->del_cmode(CMODE_KEY);
				t->info_insert(nick + " has removed the key "
				               "from " + which + "\n");
				t->set_keyentry("");
				t->set_key("");
			}
			t->mode_toggle(CMODE_KEY, on);
			t->mode_enable(CMODE_KEY);
			break;
		case 'l':
			if(on)
			{
				targ = argv[targ_cnt++];
				t->set_cmode(CMODE_LIMIT);
				t->set_limit(atoi(targ));

				cstring plural;

				if(atoi(targ) == 1)
					plural = " user";
				else
					plural = " users";
				t->info_insert("Userlimit set to " + targ + 
					       plural + " on " + which + 
					       " by " + nick + "\n");
			}
			else
			{
				t->del_cmode(CMODE_LIMIT);
				t->info_insert(nick + " has removed the limit "
				               "from " + which + "\n");
				t->set_limit(0);
			}
			t->mode_toggle(CMODE_LIMIT, on);
			t->mode_enable(CMODE_LIMIT);
			break;
		case 'm':
			if(on)
			{
				t->set_cmode(CMODE_MODERATE);
				t->info_insert(which +" has been made moderated"
					       " by " + nick + "\n");
			}
			else
			{
				t->del_cmode(CMODE_MODERATE);
				t->info_insert(which + " is no longer moderated"
					       " thanks to " + nick + "\n");
			}
			t->mode_toggle(CMODE_MODERATE, on);
			t->mode_enable(CMODE_MODERATE);
			break;
		case 'n':
			if(on)
			{
			        t->set_cmode(CMODE_NO_MSG);
				t->info_insert(which + " can no longer be "
					       "messaged to from the outside "
					       "thanks to " + nick + "\n");
			}
			else
			{
				t->del_cmode(CMODE_NO_MSG);
				t->info_insert(which + " can now be messaged to"
					       " from the outside thanks to " +
					        nick + "\n");
			}
			t->mode_toggle(CMODE_NO_MSG, on);
			t->mode_enable(CMODE_NO_MSG);
			break;
		case 'p':
			if(on)
			{
			        t->set_cmode(CMODE_PRIVATE);
				t->info_insert(which + " has been made private "
					       "by "  + nick + "\n");
			}
			else
			{
				t->del_cmode(CMODE_PRIVATE);
				t->info_insert(which + " is no longer private "
				               "thanks to " + nick + "\n");
			}
			t->mode_toggle(CMODE_PRIVATE, on);
			t->mode_enable(CMODE_PRIVATE);
			break;
		case 's':
		        if(on)
			{
			        t->set_cmode(CMODE_SECRET);
				t->info_insert(which + " has been made secret "
				               "by "  + nick + "\n");
			}
			else
			{
			        t->del_cmode(CMODE_SECRET);
				t->info_insert(which + " is no longer secret "
				               "thanks to " + nick + "\n");
			}
			t->mode_toggle(CMODE_SECRET, on);
			t->mode_enable(CMODE_SECRET);
			break;
		case 't':
			if(on)
			{
				t->set_cmode(CMODE_TOPIC);
				t->info_insert("Only operators can set the "
					       "topic on "+which+" thanks to "+ 
					       nick + "\n");
				if(!(t->get_mymode() & UMODE_OP))
					t->topic_disable();
			}
			else
			{
				t->del_cmode(CMODE_TOPIC);
				t->info_insert("Everybody can set the topic on "
					       +which+" thanks to "+nick+"\n");
				t->topic_enable();
			}
			t->mode_toggle(CMODE_TOPIC, on);
			t->mode_enable(CMODE_TOPIC);
			break;
		case 'b':
			targ = argv[targ_cnt++];
			if(on)
			{
				t->add_ban(targ);
				//bans->add_ban(t->channelname, targ);
				t->info_insert(targ + " has been banned on " +
					       which + " by " + nick + "\n");
			}
			else
			{
				t->del_ban(targ);
				//bans->del_ban(t->channelname, targ);
				t->info_insert(targ + " is no longer banned on "
					       + which + " by " +nick + "\n");
			}
			break;
		default:
			cstring mod = modes[i];
			if(on)
				t->info_insert("Unknown mode " + mod + "set\n");
			else
				t->info_insert("Unknown mode " + mod +
				               " removed\n");
			break;
		}

	// Disable possible incorrect enabled modebuttons
	if(!(t->get_mymode() & UMODE_OP))
		t->mode_disable_all();
	return true;
}

int	serverhandler::dispatch_nick(int, char **argv)
{
	cstring	who = argv[1];
	cstring	nick, login, host;
	cstring	newnick = argv[3];
	
	split_nuh(who, nick, login, host);

	if(connection::nick == nick)
	{
		// I changed my nick, update it
		setnick(newnick);
	}

	
	targetlist.nickchange(nick, newnick);
	return true;
}

int	serverhandler::dispatch_ping(int, char **argv)
{
	ircserver::pong(argv[3]);
	return true;
}

int	serverhandler::dispatch_privmsg(int, char **argv)
// handles PRIVMSG and NOTICE!!
{
	cstring	from = argv[1];
	cstring	to = argv[3];
	cstring	command = argv[2];
	cstring	msg = argv[4];

	// Handle CTCP

	if(msg[0] == '\x01' && msg[msg.length()-1] == '\x01')
	{
		cstring	cmd = msg(1, msg.length() - 2);
		cstring	rest = "";
		int	idx;

		if((idx = cmd.find(' ')) != -1)
		{
			rest = cmd(idx+1);
			cmd = cmd(0,idx-1);
		}
			
		if(command == "PRIVMSG")
			tk->eval("CTCPEvent \"" + esc(cmd) + "\" \"" + 
			         esc(from) + "\" \"" + esc(to) + 
				 "\" \"" + esc(rest) + "\"");
		else
		{
			tk->eval("CTCPEvent \"" + esc(cmd) + "_REPLY" + 
				 "\" \"" + esc(from) + "\" \"" + esc(to) +
				 "\" \"" + esc(rest) + "\"");
		}
		return true;
	}
		
	// make a difference between the prompt of NOTICE and PRIVMSG

	if(command == "PRIVMSG")
		targets::recvmsg(from, to, msg);
	// notice
	else if(isserver(from))
		insert("-"+from+"- " + msg + "\n");
	else
		targets::recvnotice(from, to, msg);
	return true;
}

int	serverhandler::dispatch_topic(int, char **argv)
{
	cstring	from = argv[1];
	cstring	topic = argv[4];
	cstring	which = argv[3];
	channel_target	*t = (channel_target *)targetlist[which];
	cstring	nick, login, host;

	split_nuh(from, nick, login, host);

	if(t->get_topic() == "")
		t->info_insert(nick + " has set the topic for " + which + "\n");
	else
		t->info_insert(nick + " has changed the topic for " + 
		               which + "\n");
	t->set_topic(topic);
	return true;
}

int	serverhandler::dispatch_quit(int, char **argv)
{
	cstring	who = argv[1];
	cstring	why = argv[3];

	targetlist.quituser(who, why);
	return true;

}

int	serverhandler::dispatch_invite(int, char **argv)
{
	cstring	who = argv[1];
	cstring	nick, login, host;
	cstring	channel = argv[4];
	
	split_nuh(who, nick, login, host);
	insert(nick + " (" + login + "@" + host + ") invites you to channel "
		  + channel + ".\n");

	return true;
}

int	serverhandler::dispatch_welcome(int, char **argv)
/*
 * Handle 001 - 'Welcome to the Internet Relay Network, <nick>'
 *
 * Use this to set the current nickname, and display the welcome message
 */
{
	cstring	nick = argv[3];
	cstring	msg = argv[4];

	connection::connected();
	connection::setnick(nick);
	targets::reset();

	insert(msg + "\n");
	return true;
}

int	serverhandler::dispatch_namereply(int argc, char **argv)
{
	if(!targets::wantinfo(argc, argv))
	{
		cstring	result;

		if(expand_reply(result, argc, argv))
		{
			if(result != (const char *) "")
				insert(result + "\n");
			return true;
		}
		return false;
	}
	return true;
}

int	serverhandler::dispatch_whoreply(int argc, char **argv)
{
	if(!targets::wantinfo(argc, argv))
	{
		cstring	result;

		if(expand_reply(result, argc, argv))
		{
			if(result != (const char *) "")
				insert(result + "\n");
			return true;
		}
		return false;
	}
	return true;
}

int	serverhandler::dispatch_banlistreply(int argc, char **argv)
{
	cstring	rpl = argv[2];

	//if(rpl == "367")	// normal reply
		//bans->add_ban(argv[4], argv[5]);
	if(!targets::wantinfo(argc, argv))
	{
		cstring	result;

		if(expand_reply(result, argc, argv))
		{
			if(result != (const char *) "")
				insert(result + "\n");
			return true;
		}
		return false;
	}
	return true;
}

int	serverhandler::dispatch_nickchanlock(int, char **argv)
{
	cstring	which = argv[4];

	if(isnick(which))
		insert("Nickname " + which  + 
		          " is locked - try again later\n");
	else
	{
		channel_target *t = (channel_target *)targetlist[which];

		if(t == NULL)				// channel not joined
			t = targets::createchannel(which);
		t->setstate(STATE_NOTJOIN);
	}
	return true;
}

int	serverhandler::dispatch_nojoin(int argc, char **argv)
{
	cstring	which = argv[4];
	cstring	numeric = argv[2];
	cstring	reason;
	channel_target	*t = (channel_target *)targetlist[which];

	expand_reply(reason, argc, argv);
	if(t == NULL)				// channel not joined
		t = targets::createchannel(which);
	t->setstate(STATE_NOTJOIN);
	return true;
}

int	serverhandler::dispatch_rpltopic(int, char **argv)
{
	cstring	topic = argv[5];
	cstring	which = argv[4];
	channel_target	*t = (channel_target *)targetlist[which];

	t->set_topic(topic);
	return true;
}

int	serverhandler::dispatch_chancreated(int, char **argv)
{
	cstring	rpl = argv[2];
	cstring	channel = argv[4];
	char	*when;

	when = argv[5];

	long t = atol(when);
	cstring	d = ctime(&t);
	targets::insert_info(channel,
	                          channel + " was created " + --d + "\n");
	return true;
}

int	serverhandler::dispatch_topicset(int, char **argv)
{
	cstring	rpl = argv[2];
	cstring	channel = argv[4];
	cstring	who;
	char	*when;

	who = argv[5];
	when = argv[6];

	long t = atol(when);
	cstring	d = ctime(&t);
	targets::insert_info(channel,
	                          "Topic was set " + --d + " by " + who + "\n");
	return true;
}
int     serverhandler::dispatch_rplumodeis(int, char **argv)
{
	cstring	modes = argv[4];
	int	on = false;

	for(int i = 0; i < modes.length(); i++)
		switch(modes[i])
		{
		case '+':
			on = true;
			break;
		case '-':
			on = false;
			break;
		case 'w':
			if(on)
			{
				set_mode(MODE_WALLOP);
				insert("You can now receive wallops\n");
			}
			else
			{
				del_mode(MODE_WALLOP);
				insert("You will no longer receive "
					  "wallops\n");
			}
			mode_toggle(MODE_WALLOP, on);
			mode_enable(MODE_WALLOP);
			break;
		case 'i':
			if(on)
			{
				set_mode(MODE_INVISIBLE);
				insert("You are now invisible\n");
			}
			else
			{
				del_mode(MODE_INVISIBLE);
				insert("You are no longer invisible"
						  "\n");
			}
			mode_toggle(MODE_INVISIBLE, on);
			mode_enable(MODE_INVISIBLE);
			break;
		case 's':
			if(on)
			{
				set_mode(MODE_SERV_NOT);
				insert("You can now receive server-"
						  "notices\n");
			}
			else
			{
				del_mode(MODE_SERV_NOT);
				insert("You will no longer receive " 
						  "server-notices\n");
			}
			mode_toggle(MODE_SERV_NOT, on);
			mode_enable(MODE_SERV_NOT);
			break;
		case 'r':
			if(on)
			{
				set_mode(MODE_RESTRICTED);
				insert("Your connection is now "
				          "restricted\n");
			}
			else // this is not possible, right?
			{
				del_mode(MODE_RESTRICTED);
				insert("Your connection is no longer "
				          "restricted\n");
			}
			mode_toggle(MODE_RESTRICTED, on);
			break;
		default:
			cstring mod = modes[i];
			if(on)
				insert("Unknown mode " + mod +
				          " set\n");
			else
				insert("Unknown mode " + mod +
				          " removed\n");
			break;
		}
	return true;
}

int	serverhandler::dispatch_rplchanmodeis(int argc, char **argv)
{
	if(!targets::wantinfo(argc, argv))
	{
		cstring	channel = argv[4];
		cstring	modes = argv[5];
		cstring mode_str = "";
		int	argc_cnt = 6;
	
		for(int i = 0; i < modes.length(); i++)
		{
			if(i > 1)
				mode_str += ", ";
			switch(modes[i])
			{
			case '+':
				break;
			case 'i':
				mode_str += "invite-only";
				break;
			case 'k':
				mode_str += "key";
				if(argc > argc_cnt)
				{
					mode_str += " (";
					mode_str += argv[argc_cnt++];
					mode_str += ")";
				}
				break;
			case 'l':
				mode_str += "limit";
				if(argc > argc_cnt)
				{
					mode_str += " (";
					mode_str += argv[argc_cnt++];
					mode_str += ")";
				}
				break;
			case 'm':
				mode_str += "moderated";
				break;
			case 'n':
				mode_str += "no-messages";
				break;
			case 'p':
				mode_str += "private";
				break;
			case 's':
				mode_str += "secret";
				break;
			case 't':
				mode_str += "topic";
				break;
			default:
				mode_str += "unknown mode ";
				mode_str += modes[i];
				break;
			}
		}
		mode_str += ".\n";
		insert("Modes on " + channel + ": " + mode_str);
	}
	return true;
}

int	serverhandler::dispatch_isonreply(int, char **)
{
	return true;
}

int	serverhandler::dispatch_awayreply(int, char **argv)
{
	cstring rpl = argv[2];

	if(rpl == "305")	// no longer away
	{
		if(awaymsg != (const char *) "")
			insert("You are no longer marked as being away\n");
		setaway("");
	}
	else			// 306 - away
		insert("You have been marked as being away\n");
	return true;
}

int	serverhandler::dispatch_whoisreply(int argc, char **argv)
{
	cstring rpl = argv[2];

	if(!targets::whoisinfo(argc, argv))
	{
		cstring	result;

		if(expand_reply(result, argc, argv))
		{
			if(result != (const char *) "")
				insert(result + "\n");
			return true;
		}
		return false;
	}
	return true;
}

int     serverhandler::dispatch_msg(int argc, char *argv[])
{
        func_packer  *d = (func_packer *) dispatch_table[argv[2]];
 
        if(d)
	{
                if(!d->execute(argc, argv))
		{
			cout << "Failed to handle " << argv[2] << endl;
			tk->quit();
		}
		return true;
	}
        else 
		return handle_reply(argc, argv);
        return false;
}

int	serverhandler::handlemessage(const cstring &msg)
// nettere oplossing: werk met char[] buffer, argv[] is pointers in buffer
{
	int	_argc = 0;
	char	*_argv[20];		// 20 should be more than plenty
	char	*ptr = msg, *end;

	if(dbg > 5)
		cout << msg << endl;

	_argv[_argc++] = "ServerEvent";

	if(((char*)msg)[0] == ':')
		ptr++;			// skip ':'
	else // no 'from' - use server
		_argv[_argc++] = _host;

	while(ptr && *ptr && *ptr != ':')
	{
		_argv[_argc++] = ptr;

		end = strchr(ptr, ' ');
		ptr = end;
		if(end)
		{
			*end = '\0';
			ptr++;		// skip token-terminating zero
		}
	}

	// last parameter, can be multiple tokens but consider it as one
	// because it starts with a ':'

	if(ptr && *ptr == ':')
		_argv[_argc++] = ++ptr;

	serverhandler::dispatch_msg(_argc, _argv);
	return true;
}

void    serverhandler::error(const char *errstr)
{
        // abort() is already being called by the handler for SOCKERR
        if(errstr == NULL)
		disconnected("Unknown error");
	else
                disconnected(errstr);
       	targetlist.disable();
        setstate(ircserver::disconnected); //// FOUT!!!
}

int	serverhandler::operator()()
{
	char	*msg, *nextmsg;
	int	bytes_read;

	cstring	ev = _event->arg(0);

cout << "serverhandler::dispatch()" << endl;

	if(ev == "raw")
	{
		if(_event->argc() != 2)
			tk->result("wrong # args: should be \""
			           "raw rawmsg\"");
			// return TCL_ERROR?
		else
			raw(_event->arg(1));
		return true;
	}

	if(_event->fd() == io::fd())
	{
		if(state(ircserver::connecting))
		{
			// we weren't connected, but now we are
			setstate(ircserver::connected);
			reregister();
			tk->unbind(fd);
			bindfd();
			return true;
		}

		if(_event->mask()&kit::writable)
		{
			flush(io::io_write);
			tk->unbind(fd);
			bindfd();
			return true;
		}


		if(flush(io::io_read) < 0)
		{
			last_error = errno;
			error(strerror(errno));
			return true;
		}

		bytes_read = read(buffer + bytes, buffersize - bytes);

		if(bytes_read == 0)
		{
			last_error = errno;
			error(strerror(errno));
			return true;
		}
		// Is recursion possible?

		tk->unbind(fd);

		bytes += bytes_read;

		// Make sure buffer is terminated with a zero
		buffer[bytes] = '\0';

		msg = buffer;

// indentation removed


	// The buffer can/will contain several messages, seperated by '\r\n''s
	// watch out, the last \n might be missing if the message is not
	// complete!


	// *sigh* sometimes a server seems to send \r\n, sometimes \n
	// this means there might be a leftover \n from a previous message
	// which just came in..

	if(*msg == '\n')
	{
		msg++;
		bytes--;
	}

	while((nextmsg = strchr(msg, '\r')) ||
	      (nextmsg = strchr(msg, '\n')))
	{
		if(*nextmsg == '\r')
		{
			*nextmsg = '\0';
			nextmsg++;
			bytes--;
		}
		if(*nextmsg == '\n')
		{
			*nextmsg = '\0';
			nextmsg++;
			bytes--;
		}
		
		// very ugly fix. It seems we might end up with an empy
		// message
		if(*msg)
			handlemessage(msg);

		if(!state(ircserver::connected))// don't bother about the rest
			return true;

		bytes -= strlen(msg);
		msg = nextmsg;
	}
		
	char	*s = buffer;

	// copy remaining part, but don't bother if we've lost the connection
	// (if it's incomplete, we will never receive the remaining part 
	// anyway

	if(state(ircserver::connected))
	{
		for(unsigned int i = 0; i < bytes; i++)
			s[i] = *(msg++);
		buffer[bytes] = '\0';
		bindfd();
	}
        return true;
	
	}  // indentation incorrect
	
	return ctcphandler::operator()();
}
