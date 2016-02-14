/*
 * cmdhandler.c - Handles commands typed at text input entries
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
#include <sys/types.h>
#include <time.h>
#include <sys/stat.h>
#include <unistd.h>


// strerror, errno
#include <string.h>
#include <errno.h>

#include "cmdhandler.h"

#include "cstring.h"
#include "misc.h"
#include "const.h"

#include "dialog.h"
#include "filechooser.h"

#include "internalevent.h"

extern	int	dbg;

#define	pack(f)	new func_packer(this, &cmdhandler::f)

cmdhandler::cmdhandler(): targets()
{
	cstring cmdline;
	
	cmdline << CMDLINE << connection::sessionid;
	init_command_table();
	tk->bind("bindcmd", this, statHandleCmdbind);
	tk->bind((char*)cmdline, this, statHandleCmdline);
}

cmdhandler::~cmdhandler()
{
}

void	cmdhandler::init_command_table(void)
{
	command_table["away"] = pack(do_away);
	command_table["clear"] = pack(do_clear);
	command_table["dcc"] = pack(do_dcc);
	command_table["ctcp"] = pack(do_ctcp);
	command_table["disconnect"] = pack(do_disconnect);
	command_table["join"] = pack(do_join);
	command_table["kick"] = pack(do_kick);
	command_table["leave"] = pack(do_leave);
	command_table["load"] = pack(do_load);
	command_table["me"] = pack(do_me);
	command_table["mode"] = pack(do_mode);
	command_table["msg"] = pack(do_msg);
	command_table["notice"] = pack(do_notice);
	command_table["nick"] = pack(do_nick);
	command_table["oper"] = pack(do_oper);
	command_table["ping"] = pack(do_ping);
	command_table["query"] = pack(do_query);
	command_table["quit"] = pack(do_quit);
	command_table["server"] = pack(do_server);
	command_table["topic"] = pack(do_topic);
	command_table["umode"] = pack(do_umode);
	command_table["whois"] = pack(do_whois);
	command_table["invite"] = pack(do_invite);
}

int	cmdhandler::do_away(const cstring &, const cstring &rest)
{
	ircserver::away(rest);
	setaway(rest);
	return true;
}

int	cmdhandler::do_clear(const cstring &, const cstring &)
{
	targetlist.clear(current);
	return true;
}

int	cmdhandler::do_ctcp(const cstring &, const cstring &rest)
{
	int	idx;
	cstring	line = rest;

	if((idx = line.find(' ')) != -1)
	{
		cstring msg, cmd, target;

		msg = line(idx+1);
		target = line(0, idx-1);
		
		if((idx = msg.find(' ')) != -1)
		{
			cmd = msg(0, idx-1);
			msg = msg(idx+1);
		}
		else
		{
			cmd = msg;
			msg = "";
		}
		if(cmd.lower() == "ping")
		{
			msg = "";
			msg << time(NULL);
		}
		
		ircserver::privmsg(target, '\x01'+cmd.upper()+" "+msg+'\x01');

		insert(">["+target+"]", cmd.upper()+" "+msg + "\n", 
		          12, "nick_me");
	}
	else
		insert("Send what CTCP to whom?\n");

	return true;
}

int	cmdhandler::do_dcc(const cstring &, const cstring &rest)
//
// dcc has lots of subcommands.
{
	int	idx;
	cstring	cmd;
	cstring	line = rest;

	if(!(int)line)
	{	
		insert("dcc requires a subcommand\n");
		return true;
	}
	else if((idx = line.find(' ')) != -1)
	{
		cmd = line(0, idx-1);
		line = line(idx+1);
	}
	else
	{
		cmd = line;
		line = "";
	}
		
	if(cmd.compare("msg"))
		if((idx = line.find(' ')) != -1)
		{
			cstring msg, target;
	
			msg = line(idx+1);
			target = line(0, idx-1);
			
			tk->eval("dcc msg " + esc(target) + 
		         	" \"" + esc(msg) + "\"");
	
			targets::sendchat(target, msg);
		}
		else
			insert("Send what message to whom?\n");
	else if(cmd.compare("chat"))
		if(!(int)line)
			insert("No target specified for dcc chat\n");
		else if(!isnick(line))
			insert(line + " is not a valid nickname\n");
		else
		{
			insert("Initiating dcc chat with " + line + "\n");
			tk->eval("dcc chat " + esc(line));
		}
	else if(cmd.compare("send"))
	{
		cstring nick, file;
		cstring_iter it = line;
		struct	stat	sbuf;

		nick = it(" ");
		file = it(" ");

		if(!(int)nick && !(int)file)
			insert("No nickname and filename specified\n");
		else if(!(int)file)
			insert("No filename specified\n");
		else if(stat(file, &sbuf) == -1)
			insert(file + ": not found\n");
		else if(!S_ISREG(sbuf.st_mode))
			insert(file + ": not a regular file\n");
		else
		{
	//		file.rskip('/');
			cout << "dcc send " + nick + " " + file << endl;
			tk->eval(esc("dcc send " + nick + " " + file));
		}
	}
	else
		insert("Unknown or unimplemented dcc command " + cmd + '\n');
	return true;
}

int	cmdhandler::do_oper(const cstring &, const cstring &rest)
{
	cstring_iter	chop = rest;
	cstring		nick = chop(" ");
	cstring		passwd = chop(" ");

	if((int)nick && (int)passwd)
		ircserver::oper(nick, passwd);
	else
	{
		if(!(int)nick)
			nick = nick;

		aws_dialog oper(".operDialog");
		oper.title("Become irc operator");
		oper.message("Please type your nickname\n"
		             "and password below.");
		entry *n = oper.entry("Nick: ", nick);
		entry *p = oper.entry("Password: ", "", "-show *");

		oper.button("Oper", "OperPressed", true);
		oper.button("Cancel", "CancelPressed");

		cstring	result = oper.wacht();

		if(result == "OperPressed")
			ircserver::oper(oper.getentry(n), oper.getentry(p));
	}
	return true;
}

int	cmdhandler::do_ping(const cstring &, const cstring &rest)
{
	int	cnt = 0;
	cstring	target;
	cstring_iter	it = rest;

	while((int)(target = it(' ')))
	{
		cstring msg;

		msg = "";
		msg << time(NULL);
		
		ircserver::privmsg(target, "\x01PING "+msg+'\x01');
		insert(">["+target+"]", "PING "+msg + "\n", 
		          12, "nick_me");
		cnt++;
	}
	if(cnt == 0)
		insert("Who do you want to ping?\n");
	return true;
}

int	cmdhandler::do_disconnect(const cstring &, const cstring &rest)
{
	if(!(rest == ""))
		ircserver::quit(rest);
	else
		ircserver::quit(signoff);
	ircserver::abort();
	disconnected("Disconnected", false);
	return true;
}

int	cmdhandler::do_join(const cstring &, const cstring &rest)
{
	cstring_iter it = rest;
	cstring		sp;
	int		count = 0;

	while((int)(sp = it(',')))
	{
		cstring	chan_key = sp;
		cstring	channel, key;

		cstring_iter	ck = sp;

		channel = ck(' ');
		key = ck(' ');

		if(!(int)channel)
			continue;
		if(ischannel(channel))
		{
			if(!(int)key)
				addchannel(channel);
			else
				addchannel(channel, key);
		}
		else
			insert("Invalid channelname " + channel + "\n");
		count++;
	}
	if(count == 0)
		insert("Join which channels?\n");
	return true;
}

int	cmdhandler::do_kick(const cstring &, const cstring &rest)
/* 
 * Parse strings of the following syntax:
 *
 * "kick" (command) "[channel] nick comment"
 */
{
	cstring	comment = rest;
	cstring	target = current;

	// Does it start with a channel?

	if(comment[0] == '#' || comment[0] == '&' || comment[0] == '+')
	{
		int	idx;

		if((idx = comment.find(" ")) != -1)
		{
			target = comment(0, idx-1);
			comment = comment(idx + 1);
		}
		else
		{
			insert("Please specify a nick\n");
			return true;
		}
	}

	if(target == "")
		insert("You have not joined any channels\n");
	else if(!ischannel(target))
		insert(target + " is not a channel, please specify one\n");
	else
	{
		channel_target	*t = (channel_target *)targetlist[target];

		if(t == NULL)
			insert("You are not on channel " + target + "\n");
		else if(!(t->get_mymode() & UMODE_OP))
			insert("You are not channel operator on " +
			          target + "\n");
		else
		{
			cstring	nick;
			int	idx;

			if((idx = comment.find(' ')) != -1)
			{
				nick = comment(0, idx-1);
				comment = comment(idx + 1);
			}
			else
			{
				insert("Please specify a nick\n");
				return true;
			}

			ircserver::kick(target, nick, comment);
		}
	}
	return true;
}

int	cmdhandler::do_leave(const cstring &, const cstring &rest)
{

	cstring_iter 	it = rest;
	cstring		channel;
	int		count = 0;
	
	while((int)(channel = it(',')))
	{
		cstring comment = "";
		cstring	chan;
		int	idx;

		if((idx = channel.find(' ')) != -1)
		{
			chan = channel(0, idx-1);
			comment = channel(idx + 1);
		}
		else
			chan = channel;
		if(ischannel(chan))
			removechannel(chan, comment);
		else
			insert("Invalid channelname " + channel + "\n");
		count++;
	}
	if(count == 0)
		insert("Leave which channel?\n");
	return true;
}

int	cmdhandler::do_load(const cstring &, const cstring &file)
// loads a tcl script
{
	struct	stat	sbuf;
	cstring	script = file;

	if(!(int)script)
	{
		filesel	load(scriptdir, filesel::require_nodir|
		                           filesel::require_exist);
		if(!load.get(script))
			return true;
	}

	if(stat(script, &sbuf) == -1)
	{
		if(stat(scriptdir + '/' + script, &sbuf) == -1)
		{
			cstring	error = strerror(errno);
			insert(script + ": " + error + '\n');
			return true;
		}
		script = scriptdir + '/' + script;
	}
	insert("Loading " + script + "...\n");
	tk->source(script);
	return true;
}

int	cmdhandler::do_me(const cstring &, const cstring &rest)
// Send a CTCP ACTION to the current target
{
	cstring	line = rest;

	if(!(int)current)
		insert("No channels/queries active!\n");
	else if((int)rest)
	{
		targets::sendaction(current, rest);
		ircserver::ctcp(current, "ACTION", rest);
	}
	else
		insert("You must specify an action!\n");
	return true;
}

int	cmdhandler::do_mode(const cstring &, const cstring &rest)
{
	cstring	modechange = rest;
	cstring	target;
	int	idx;

	// read in target
	if((idx = modechange.find(' ')) != -1)
	{
		target = modechange(0, idx-1);
		modechange = modechange(idx + 1);
	}
	else
	{
		target = rest;
		modechange = "";
	}
	if(target == "")
	{
		insert("Specify a target and a mode for the mode command\n");
		return true;
	}
	if((target[0] == '#') || (target[0] == '&') || (target[0] == '+'))
	{
		channel_target	*t = (channel_target *)targetlist[target];

		if(!(int)modechange || (modechange[0] != '+'&&modechange[0] != '-'))
		{
			// mode query
			ircserver::mode(target, modechange);
			return true;
		}
		else if(t == NULL)
		{
			insert("You are not on channel " + target + "\n");
			return true;
		}
		else if(!(t->get_mymode() & UMODE_OP))
		{
			insert("You are not channel operator on " +
			          target + "\n");
			return true;
		}
	}
	ircserver::mode(target, modechange);
	return true;
}


int	cmdhandler::do_msg(const cstring &, const cstring &rest)
{
	int	idx;
	cstring	line = rest;

	if((idx = line.find(' ')) != -1)
	{
		cstring msg, target;

		msg = line(idx+1);
		target = line(0, idx-1);
		
		ircserver::privmsg(target, msg);

		targets::sendmsg(target, msg);
	}
	else
		insert("Send what message to whom?\n");
	return true;
}

int	cmdhandler::do_nick(const cstring &, const cstring &rest)
{
	if(!isnick(rest))
		insert(rest + " is not a valid nickname\n");
	else if(ircserver::state(ircserver::connected))
		ircserver::nick(rest);
	else
		setnick(rest);
	return true;
}

int	cmdhandler::do_notice(const cstring &, const cstring &rest)
{
	int	idx;
	cstring	line = rest;

	if((idx = line.find(' ')) != -1)
	{
		cstring msg, target;

		msg = line(idx+1);
		target = line(0, idx-1);
		
		ircserver::notice(target, msg);

		targets::sendnotice(target, msg);
	}
	else
		insert("Notice what message to whom?\n");
	return true;
}

int	cmdhandler::do_query(const cstring &, const cstring &rest)
{
	if(!(int)rest)
	{
		insert("Please specify one or more nicknames\n");
		return true;
	}

	cstring_iter queryiter = rest;
	cstring	which;

	while((int)(which = queryiter(", ")))
	{
		if(which == "")
			continue;
		if(isnick(which))
			targets::addquery(which);
		else
			insert("Invalid nickname " + which + "\n");
	}
	return true;
}

int	cmdhandler::do_quit(const cstring &, const cstring &rest)
{
	if(!ircserver::state(ircserver::connected))
		tk->quit();
	else if((int)rest)
		ircserver::quit(rest);
	else
		ircserver::quit(signoff);
	deathwish = true;
	ircserver::abort();
	tk->quit();
	return true;
}

int	cmdhandler::do_server(const cstring &, const cstring &rest)
{
	cstring_iter	server_iter;
	cstring		serverhost, port;

	server_iter = rest;

	serverhost = server_iter(" ,:;");
	port = server_iter(" ,:;");
	
	if(!(int)serverhost)
		insert("No server specified - not reconnecting\n");
	else
	{
		int	_port = DEFAULT_PORT;

		if((int)port)
			_port = atoi(port);
		if(_port == 0)
		{
			char	msg[255];

			sprintf(msg, "Invalid port specified - "
			          "defaulting to %d\n", DEFAULT_PORT);
			insert(msg);
			_port = DEFAULT_PORT;
		}
		tk << SERVER << " -change " << sessionid << " " 
		   << serverhost << " " << _port << end;
	}
	return true;
}

int	cmdhandler::do_topic(const cstring &, const cstring &rest)
{
	cstring	newtopic = rest;
	cstring	target = current;

	// Does it start with a channel?

	if(newtopic[0] == '#' || newtopic[0] == '&' || newtopic[0] == '+')
	{
		int	idx;

		if((idx = newtopic.find(" ")) != -1)
		{
			target = newtopic(0, idx-1);
			newtopic = newtopic(idx + 1);
		}
		else
		{
			// simply request the topic
			//insert("Please specify a topic\n");
			ircserver::topic(target);
			return true;
		}
	}

	if(target == "")
		insert("You have not joined any channels\n");
	else if(!ischannel(target))
		insert(target + " is not a channel, please specify one\n");
	else
	{
		channel_target	*t = (channel_target *)targetlist[target];

		if(t == NULL)
			insert("You are not on channel " + target + "\n");
		else if((t->get_cmode() & CMODE_TOPIC) && 
		        !(t->get_mymode() & UMODE_OP))
			insert("You are not channel operator on " +
			          target + "\n");
		else
			ircserver::topic(target, newtopic);
	}
	return true;
}

int	cmdhandler::do_umode(const cstring &, const cstring &rest)
{
	ircserver::mode(nick, rest);
	return true;
}

int	cmdhandler::do_whois(const cstring &, const cstring &rest)
{
	if(!(int)rest)
	{
		insert("Please specify one or more nicknames\n");
		return true;
	}

	cstring_iter nickiter = rest;
	cstring	nick;

	while((int)(nick = nickiter(", ")))
		ircserver::whois(nick);
	return true;
}

int	cmdhandler::do_invite(const cstring &, const cstring &rest)
/* 
 * Parse strings of the following syntax:
 *
 * "invite" (user) "nick channel"
 */
{
	cstring_iter	it = rest;
	cstring		sp;
	int		count = 0;
	
	while((int)(sp = it(',')))
	{
		cstring	nick, channel;

		cstring_iter ck = sp;

		nick = ck(' ');
		channel = ck(' ');

		if(!(int)channel)
			channel = current;
		else if(!targetlist[channel])
			insert("You are not on channel " + channel + "\n");
		if(ischannel(channel))
			ircserver::invite(nick, channel);
		count++;
	}
	if(count == 0)
		insert("Invite who?\n");
	return true;
}

int	cmdhandler::dispatch_command(const cstring &cmd, const cstring &args)
{
	func_packer  *d = (func_packer *) command_table[cmd.lower()];
 
	if(d)
		d->execute(cmd, args);
	else if(tcl_table.find(cmd) != tcl_table.end())
	{	
		cstring	cmd = *tcl_table[cmd];
		tk->eval(cmd + " " + args);
	}
	else
	{
	 	if(dbg > 5)	
			cout << "No handler for " << cmd << ", " << args 
		             << ", passing to server" << endl;
		ircserver::raw(cmd + " " + args);
	}
	return true;
}

int	cmdhandler::operator()()
{
	// don't pass event to connection::operator()() as it might get
	// passed twice. Let ircsession handle it.
	return false;
}

int	cmdhandler::handleCmdline(void)
/*
 * The syntax of the CMDLINE commandevent is 
 *
 * CMDLINE default_target cmdline
 */
{
	if(_event->argc() != 3)
	{
		cstring cmd = _event->arg(0);

		tk->result("wrong # args: should be \"" +
			   cmd + " default_target command\"");
		return true;
	}

	cstring		def_target = _event->arg(1);
	cstring		cmdline = _event->arg(2);
	cstring 	cmd, rest = ""; 
	int		idx;

	if((idx = cmdline.find(' ')) != -1)
	{
		cmd = cmdline(0, idx-1);
		rest = cmdline(idx + 1);
	}
	else
		cmd = cmdline;

	cout << "PARSING " << cmd << ":" << rest << endl;

	dispatch_command(cmd, rest);
	
	return TCL_OK;
}

int	cmdhandler::handleCmdbind(void)
{
	// map a circuscommand to a tcl command
	cstring	*tclcmd, circmd;

	if(_event->argc() == 1 || _event->argc() > 3)
	{
		tk->result("wrong # args: should be \""
		           "bindcmd tclcmd ?circuscmd?\"");
		return true;
	}
	else if(_event->argc() == 3)
	{
		tclcmd = new cstring(_event->arg(1));
		circmd = _event->arg(2);
	}
	else // 2
	{
		tclcmd = new cstring(_event->arg(1));
		circmd = _event->arg(1);
	}
	// check if name is already bound?
	tclgarb._register(tclcmd);
	tcl_table[circmd] = tclcmd;
	return true;
}

int	cmdhandler::statHandleCmdline(void *h, event *e)
{
	((cmdhandler *)h)->setevent(e);
	return ((cmdhandler *)h)->handleCmdline();
}

int	cmdhandler::statHandleCmdbind(void *h, event *e)
{
	((cmdhandler *)h)->setevent(e);
	return ((cmdhandler *)h)->handleCmdbind();
}
