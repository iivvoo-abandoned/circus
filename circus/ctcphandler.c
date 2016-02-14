/*
 * ctcphandler.c - handles ctcpevents
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

// for open()
#include <sys/stat.h>
#include <fcntl.h>

// for close()
#include <unistd.h>

#include "ctcphandler.h"

#include "const.h"

#include "if_dcc.h"

#define	pack(f)	new func_packer(this, &ctcphandler::f)

ctcphandler::ctcphandler(): targets()
{
	tk->bind("CTCP", this);
	ctcp_table["action"] = pack(ctcp_action);
	ctcp_table["version"] = pack(ctcp_version);
	ctcp_table["clientinfo"] = pack(ctcp_clientinfo);
	ctcp_table["finger"] = pack(ctcp_finger);
	ctcp_table["dcc"] = pack(ctcp_dcc);
	ctcp_table["ping"] = pack(ctcp_ping);
	ctcp_table["ping_reply"] = pack(ctcp_reply);
	ctcp_table["time_reply"] = pack(ctcp_reply);
	ctcp_table["clientinfo_reply"] = pack(ctcp_reply);
	ctcp_table["userinfo_reply"] = pack(ctcp_reply);
	ctcp_table["finger_reply"] = pack(ctcp_reply);
	ctcp_table["version_reply"] = pack(ctcp_reply);
	ctcp_table["errmsg_reply"] = pack(ctcp_reply);
}

ctcphandler::~ctcphandler()
{
}

int	ctcphandler::ctcp_action(const cstring &from, const cstring &to,
                                  const cstring &, const cstring &rest)
{
	targets::recvaction(from, to, rest);
	return true;
}

int	ctcphandler::ctcp_version(const cstring &from, const cstring &,
                                  const cstring &, const cstring &)
{
	cstring	nick, login, host;

	split_nuh(from, nick, login, host);
	ircserver::ctcpreply(nick, "VERSION", CTCP_VERSION);
	return true;
}

int	ctcphandler::ctcp_clientinfo(const cstring &from, const cstring &,
                                  const cstring &, const cstring &)
{
	cstring	nick, login, host;

	split_nuh(from, nick, login, host);
	ircserver::ctcpreply(nick, "CLIENTINFO", CTCP_CLIENTINFO);
	return true;
}

int	ctcphandler::ctcp_finger(const cstring &from, const cstring &,
                                  const cstring &, const cstring &)
{
	cstring	nick, login, host;

	split_nuh(from, nick, login, host);
	ircserver::ctcpreply(nick, "FINGER", fingerinfo);
	return true;
}

int	ctcphandler::ctcp_dcc(const cstring &from, const cstring &,
                                  const cstring &, const cstring &rest)
// check if dcc is not public!
{
	cstring	nick, login, host;

	cstring	type, fname, rhost, rport, fsize;
	cstring_iter	req_iter = rest;
	
	type = req_iter(" ");
	fname = req_iter(" ");
	rhost = req_iter(" ");
	rport = req_iter(" ");
	fsize = req_iter(" ");

	split_nuh(from, nick, login, host);
	if(type.compare("chat"))
	{
		if(dcc_acceptchat(from, rhost, rport))
			tk->eval(esc("dcc chat " + from + " " + 
		                        rhost + " " + rport));
		return true;
	}
	else if(type.compare("send"))
	{
		fname.rskip('/');
		fname = downloaddir + '/' + fname;
		if(dcc_acceptfile(from, fname, rhost, rport, atoi(fsize)))
		{
			int fd;
			
			if((fd=open(fname, O_CREAT|O_WRONLY|O_TRUNC, 0600)) < 0)
			{
				insert("Could not open " + fname +
				          " for writing\n");
				return true;
			}
			::close(fd);
			tk->eval(esc("dcc get " + from + " " + fname + " " + 
		                         rhost + " " + rport + " " + fsize));
		}
		return true;
	}

	ircserver::ctcpreply(nick, "ERRMSG", CIRCUS_VERSION 
			     " does not support dcc " + type + ", sorry...");
	return true;
}

int	ctcphandler::ctcp_ping(const cstring &from, const cstring &,
                                  const cstring &, const cstring &rest)
{
	cstring	nick, login, host;

	split_nuh(from, nick, login, host);
	ircserver::ctcpreply(nick, "PING", rest);
	return true;
}

int	ctcphandler::ctcp_reply(const cstring &from, const cstring &,
                                 const cstring &cmd, const cstring &rest)
{
	cstring	nick, login, host;
	cstring command = cmd(0, cmd.length()-7);

	split_nuh(from, nick, login, host);
	cstring	repl("CTCP " + command + " reply from " + nick +
		     "(" + login + "@" + host + "): ");
	if(command.compare("ping"))
	{
		int	diff = time(NULL) - atol(rest);
		repl << diff;
		if(diff == 1)
			repl << " second.\n";
		else
			repl << " seconds.\n";
	}
	else //if(command.compare("time"))
		repl << rest << "\n";
	insert(repl);
	return true;
}

int	ctcphandler::dispatch_ctcp(const cstring &cmd, const cstring &from, 
                                   const cstring &to, const cstring &args)
{
	func_packer  *d = (func_packer *)ctcp_table[cmd.lower()];
 
	if(d)
		d->execute(from, to, cmd, args);

	cstring	nick, login, host;

	split_nuh(from, nick, login, host);

	insert("Received a CTCP " + cmd  + " " + args + 
	          " from " + nick + " to " + to + "\n");
	return TCL_OK;
}

int	ctcphandler::operator()()
{
	cstring	ev = _event->arg(0);
	cstring command = _event->arg(1);

cout << "ctcphandler::operator()()" << endl;
	if(ev == "CTCP")
	{
		if(_event->argc() < 2)
		{
			tk->result("Insufficient parameters for ctcp");
			return true;
		}

		cstring	target = _event->arg(1);
		cstring	ctcp = _event->arg(2)? _event->arg(2):"";
		ircserver::privmsg(target, '\x01'+ctcp+'\x01');
		return true;	
	}
	// don't pass event to connection::operator()() as it might get
	// passed twice. Let ircsession handle it.
	return false;
}
