/*
 * ircserver.c - parses networkdata and events 
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
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include <fcntl.h>		// linux

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <iostream.h>

#include "ircserver.h"
#include "io.h"
#include "misc.h"

extern	int	dbg;

#define io_endl "\n"

//
// TESTEN NA IO OMBOUW:
// - errors (foute hostname tikken)

#warning "reset stuff in ircparsergedeelte na abort()"

static	const	char	*herrors[] = 
{
	"Error 0",
	"Host not found",
	"Try again",
	"Non recoverable error",
	"No datarecord for host",
	NULL
};

/*
 * Constructors for connections with ircservers. These need some more
 * work and finetuning
 */

ircserver::ircserver(): handler(), io()
{
	init("", 0);
	tk->bind("raw", this);
}

ircserver::ircserver(const cstring &host, unsigned int port): handler()
{
	init(host, port);
	tk->bind("raw", this);
}

ircserver::~ircserver()
{
}

/*
 * Method to actually establish the connection
 */

void	ircserver::init(const cstring &host, unsigned int port)
{
	_host = host;
	_port = port;
	fd = -1;
	_state = ircserver::disconnected;
}

void	ircserver::abort(void)
{
	if(fd != -1)
	{
		flush(io::io_write);
		tk->unbind(fd);
	}
	close();
	fd = -1;
	_state = ircserver::disconnected;
	last_error = 0;
}

int	ircserver::establish(void)
{
	char	*h;

// cout << "ircserver::establish() called" << endl;

	if(fd != -1)
	{
// close??
		tk->unbind(fd);
	}

	h = io::resolvehost(_host);

	if(h == NULL)
	{
// cout << "ircserver::establish: Resolving failed... but I'll connect anyway" 
//      << endl;
		last_error = -h_errno;
//		return -1;
	}
	fd = connect(_host, _port);

	if(fd < 0)
	{
// cout << "ircserver::establish: io::connect(" << _host << ", " << _port 
//      << ") failed... ginving up" << endl;
		last_error = errno;
		return -1;
	}
	tk->bind(fd, this, kit::readable|kit::writable);

	_state = ircserver::connecting;
	return fd;
}

void	ircserver::bindfd(void)
{
	if(haswrite())
		tk->bind(fd, this, kit::readable|kit::writable);
	else
		tk->bind(fd, this, kit::readable);
}

int	ircserver::geterror(void) const
{
	return last_error;
}

const char	*ircserver::geterrorstr(void) const
{
	const char	*result;

	if(last_error >= 0)
		result = strerror(last_error);
	else
		result = herrors[-last_error];
	return result? result: "Unknown error";
}

int	ircserver::state(int state)
{
	if(state == -1)
		return _state;
	return (_state == state);
}

void	ircserver::setstate(int state)
{
	_state = state;
}

/*
 * These methods are for communicating with the server
 */

void	ircserver::away(const cstring &msg)
// Send the AWAY command to the server
{
	if(!state(ircserver::connected))
		return;
	cstring _msg;
	(*this) << "AWAY :" << msg << io_endl;
	bindfd();
}

void	ircserver::invite(const cstring &nick, const cstring &channel)
// Send the INVITE command to the server
{
	if(!state(ircserver::connected))
		return;
	(*this) << "INVITE " << nick << " " << channel << io_endl;
	bindfd();
}

void	ircserver::ison(const cstring &nicks)
// Send the JOIN command to the server
{
	if(!state(ircserver::connected))
		return;
	(*this) << "ISON " << nicks << io_endl;
	bindfd();
}

void	ircserver::join(const cstring &channel, const cstring &key)
// Send the JOIN command to the server
{
	if(!state(ircserver::connected))
		return;
	(*this) << "JOIN " << channel << " " << key << io_endl;
	bindfd();
}

void	ircserver::kick(const cstring &channel, const cstring &user, 
                        const cstring &comment)
// Send the KICK command to the server
{
	if(!state(ircserver::connected))
		return;
	(*this) << "KICK " << channel << " " << user 
    	        << " :" << comment << io_endl;
	bindfd();
}

void	ircserver::mode(const cstring &target, const cstring &mode)
// Send the MODE command to the server
{
	if(!state(ircserver::connected))
		return;
	(*this) << "MODE " << target << " " << mode << io_endl;
	bindfd();
}

void	ircserver::nick(const cstring &nickname)
// Send the NICK command to the server
{
	if(!state(ircserver::connected))
		return;
	(*this) << "NICK " << nickname << io_endl;
	bindfd();
}

void	ircserver::notice(const cstring &target, const cstring &msg)
// Send the NOTICE command to the server
{
	if(!state(ircserver::connected))
		return;
	(*this) << "NOTICE " << target << " :" << msg << io_endl;
	bindfd();
}

void	ircserver::oper(const cstring &nick, const cstring &passwd)
{
	if(!state(ircserver::connected))
		return;
	(*this) <<  "OPER " << nick << " " << passwd << io_endl;
	bindfd();
}

void	ircserver::part(const cstring &target, const cstring &comment)
// Send the PRIVMSG command to the server
{
	if(!state(ircserver::connected))
		return;
	if(!(comment == (const char *)""))
		(*this) << "PART " << target << " :" << comment << io_endl;
	else
		(*this) << "PART " << target << io_endl;
	bindfd();
}

void	ircserver::pong(const cstring &from)
// send a PONG from to the server
{
	if(!state(ircserver::connected))
		return;
	(*this) << "PONG " << from << io_endl;
	bindfd();
}

void	ircserver::privmsg(const cstring &target, const cstring &msg)
// Send the PRIVMSG command to the server
{
	if(!state(ircserver::connected))
		return;
	(*this) << "PRIVMSG " << target << " :" << msg << io_endl;
	bindfd();
}

void	ircserver::raw(const cstring &msg)
// Send a raw message to the server
{
	if(!state(ircserver::connected))
		return;
	(*this) << msg << io_endl;
	bindfd();
}

void	ircserver::topic(const cstring &target, const cstring &topic)
// Send the TOPIC command to the server
{
	if(!state(ircserver::connected))
		return;
	if((int)topic)
		(*this) << "TOPIC " << target 
		        << " :" << topic << io_endl;
	else
		(*this) << "TOPIC " << target << io_endl;
	bindfd();
}

void	ircserver::quit(const cstring &msg)
{
	if(!state(ircserver::connected))
		return;
	(*this) << "QUIT :" << msg << io_endl;
	bindfd();
}

void	ircserver::whois(const cstring &target)
// use WHOIS target target to query target's server
{
	if(!state(ircserver::connected))
		return;
	(*this) << "WHOIS " << target << io_endl;
	bindfd();
}

void	ircserver::who(const cstring &target)
// use WHOIS target target to query target's server
{
	if(!state(ircserver::connected))
		return;
	(*this) << "WHO " << target << io_endl;
	bindfd();
}

/*
 * These aren't servermessages, but a simple interface to them, mostly
 * for CTCP and registration
 */

void	ircserver::reguser(const cstring &nickname, const cstring &login, 
                           const cstring &comment)
// Register the user at the server by sending the 'USER' and 'NICK'
// commands.
{
	if(!state(ircserver::connected))
		return;
	(*this) << "USER " << login << " " << io::gethostname() << " "
	        << _host << " :" << comment << io_endl;
	nick(nickname);
	bindfd();
}

void	ircserver::ctcpreply(const cstring &target, const cstring &type, 
                             const cstring &reply)
// Send a ctcp reply
{
	if(!state(ircserver::connected))
		return;
	(*this) << "NOTICE " << target << " :\x01" << type << " " 
	        << reply << "\x01" << io_endl;
	bindfd();
}

void	ircserver::ctcp(const cstring &target, const cstring &type, 
                             const cstring &msg)
// Send a ctcp
{
	if(!state(ircserver::connected))
		return;
	(*this) << "PRIVMSG " << target << " :\x01" << type << " " 
	        << msg << "\x01" << io_endl;
	bindfd();
}

int	ircserver::operator()()
{
	return TCL_OK;
}
