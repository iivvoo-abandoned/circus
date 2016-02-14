/*
 * dcc.c - implements Direct Client Connection facilities
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
// for htonl
#include <netinet/in.h>
// file stuff
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
// for inet_ntoa
#include <sys/socket.h>
#include <arpa/inet.h>

#include <iostream.h>

#include "dcc.h"

extern	int	dbg;

//
// TODO:
//       - use inheritance?
//       - check for multiple chats with 1 person
//       - check parameters passed through DCC

// session class

dcc_session::dcc_session(const cstring &nick): handler(), link()
{
	_type = none;
	_read = _sent = 0;
	_sock = -1;
	_state = 0;
	_nick = nick;
	_uhost = "*@*";
}

dcc_session::~dcc_session()
{
}

int	dcc_session::type(void)
{
	return _type;
}

// implementation of the dcc protocol

int	dcc_session::accept_request(char *host, int port, char *)
{
	_sock = link.connect(io::make_ip(host), port);

	if(_sock >= 0)
		_state |= dcc_wait;
	return _sock;
}

int	dcc_session::start_connection(const cstring &nick, const cstring &file,
                                      int size)
// start a session with nick
{
	int	port = 0;
	cstring	siaddr, sport, type, ssize;
	cstring	trimfile = file;

	if((_sock = link.listen(port)) < 0)
		return -1;
	sport << port;
	siaddr << io::my_ip();

	if(size)
		ssize << ' ' << size;
	if(_type == chat)
		type = "CHAT";
	else				// can't be get..
		type = "SEND";

	trimfile.rskip('/');
	tk->eval("CTCP " + esc(nick) + " {DCC " + type + " " + 
	         esc(trimfile) + " " + siaddr + " " + sport + ssize + "}");
	_state |= dcc_offered;
	return _sock;
}

char	*dcc_session::nick(void)
{
	return _nick;
}

void	dcc_session::uhost(char *uh)
{
	_uhost = uh;
}

char	*dcc_session::uhost(void)
{
	return _uhost;
}

int	dcc_session::state(void)
{
	return _state;
}

void	dcc_session::state(int newstate)
{
	_state |= newstate;
}

int	dcc_session::operator()()
// this method should always be overloaded and thus never be called
{
	return true;
}

// Implementation of DCC CHAT

dcc_chat::dcc_chat(const cstring &nick): dcc_session(nick)
{
	_type = chat;
	_state |= dcc_wait;
}

dcc_chat::~dcc_chat()
{
	close();
}

int	dcc_chat::accept_request(char *host, int port, char *)
{
	if(dcc_session::accept_request(host, port) >= 0)
	{
		tk->bind(_sock, this, kit::readable);
		_state |= dcc_active;
		_state &= ~dcc_wait;
		tk->eval("query dcc " + esc(_nick+'!'+_uhost) + " on");
	}
	return _sock;
}

int	dcc_chat::start_connection(const cstring &nick, const cstring &, int)
{
	if(dcc_session::start_connection(nick) >= 0)
		tk->bind(_sock, this, kit::readable);
	return _sock;	
}

void	dcc_chat::close(void)
{
	if(_sock != -1)
	{
		link.close();
		tk->unbind(_sock);
		_sock = -1;
		tk->eval("query dcc " + esc(_nick+'!'+_uhost) + " off");
	}
}

int	dcc_chat::operator()()
{
	cout << "CHAT EVENT:" << _event->name() 
	     << ":" << _event->mask() << endl;

	if(_event->mask()&kit::readable)
		cout << "Readable event" << endl;
	if(_event->mask()&kit::writable)
		cout << "Writable event" << endl;
	
	if(_state & dcc_wait)
	{
		if(_state & dcc_offered)
		{
			tk->unbind(_sock);
			_sock = link.accept(true);
			tk->bind(_sock, this, kit::readable);
			_state &= ~dcc_offered;
		}
		_state |= dcc_active;
		_state &= ~dcc_wait;
		tk->eval("query dcc " + esc(_nick+'!'+_uhost) + " on");
		return TCL_OK;
	}

	link.flush(io::io_read);

	char	buf[1024];
	int	bytes;
	
	bytes = link.read(buf, 1024);
	buf[bytes] = '\0';

	_read += bytes;

	if(bytes == 0) // <= 0?
	{
		// if there's anything in _buf, print it?
		tk->unbind(_sock);
		_state |= dcc_delete;
		return TCL_OK;
	}

	// _buf is a private variable that contains old, incomplete messages
	// from previous reads.
	
	cstring		message = _buf + buf;
	cstring		msg;
	
	int		idx;

	while((idx = message.find('\n')) != -1)
	{
		msg = message(0, idx-1);
		message = message(idx+1);
		tk->eval("query recvchat " + esc(_nick+'!'+_uhost) + " \"" +
		         esc(msg) + "\"");
	}
	if((int)message)
		_buf = message;
	else
		_buf = "";
	return TCL_OK;
}

int	dcc_chat::msg(const char *s)
{
	// if niet connected tk->result, false
	link.write(s);
	link.write("\n");
	// if -1 ...
	_sent += link.flush(io::io_write);
	return true;
}

dcc_send::dcc_send(const cstring &nick): dcc_session(nick)
{
	_type = send;
	_state |= dcc_wait;
	_file = -1;
	size = 0;
	meter = new dcc_meter();
}

dcc_send::~dcc_send()
{
	close();
}

int	dcc_send::start_connection(const cstring &nick, const cstring &file,
                                   int)
{
	struct	stat	sbuf;

	if(stat(file, &sbuf) == -1)
		return -1;		// report error somewhere

	size = sbuf.st_size;
	_local = file;
	if(dcc_session::start_connection(nick, file, size) >= 0)
	{
		tk->bind(_sock, this, kit::readable);
		meter->info(nick, file);
		meter->message("Waiting for " + nick + " to accept \n"
		               "\"" + file + "\"");
		meter->title("Sending " + file + " to " + nick);
		meter->build();
		meter->bind(this);
	}
	return _sock;	
}

char	*dcc_send::file(void)
{
	return _local;
}

void	dcc_send::close(void)
// close everything
{
	if(meter)
	{
		delete meter;
		meter = NULL;
	}
	if(_sock != -1)
		tk->unbind(_sock);
	link.close();
	_sock = -1;
	if(_file != -1)
		::close(_file);
	_file = -1;
}

int	dcc_send::transferblock(void)
// read a block from the file and write it to the socket
// on error, close everything
{
	int	fread = -1;
	char	buf[1024];

	if(_file != -1 && _sock != -1)
	{
		cout << "SEND TRANSFERBLOCK" << endl;
		fread = read(_file, buf, 1024);
		cout << "FREAD: " << fread << endl;
		if(fread <= 0)
		{
			close();
			_state |= dcc_delete;
			return false;
		}
		link.write(buf, fread);
		link.flush(io::io_write);
		_sent += fread; 
		meter->update((float)_sent/(float)size, _sent, size);
	}
	return fread;
}

int	dcc_send::operator()()
{
	cout << "SEND event" << endl;
	if(_event->argc() == 2) 
	{
		cstring cmd = _event->arg(1);

		if(cmd == "abort")
		{
			cout << "Aborting!" << endl;
			close();
			_state |= dcc_delete;
			return TCL_OK;
		}
	}

	if(_state & dcc_wait)
	{
		cout << "SEND WAIT" << endl;
		tk->unbind(_sock);
		_sock = link.accept(true);
		tk->bind(_sock, this, kit::readable);
		_file = open(_local, O_RDONLY);
		if(_file == -1)
		{
			 perror("Open file:");
			 close();
			 _state |= dcc_delete;
			 return TCL_OK;
		}
		cout << "FILE: " << _file << endl;
		_state &= ~dcc_offered;
		_state |= dcc_active;
		_state &= ~dcc_wait;
		if(!transferblock())
			return TCL_OK;
		meter->message("Sending file \"" + _local + "\"\n"
		           "to " + _nick);
		meter->info(_nick, _local);
		return TCL_OK;
	}

	cout << "DCC SEND NOWAOT" << endl;
	unsigned	int	ack;
	link.flush(io::io_read);

	link.read((char *)&ack, sizeof(unsigned int));

	if(ntohl(ack) < (unsigned)_sent)
		return TCL_OK;		// just wait..

	cout << "DCC SEND NEXT BLOCK" << endl;
	transferblock();
	return TCL_OK;
}

dcc_get::dcc_get(const cstring &nick): dcc_session(nick)
{
	_type = get;
	_state = dcc_wait;
	_file = -1;
	size = 0;
	meter = new dcc_meter();
}

dcc_get::~dcc_get()
{
	close();
}

int	dcc_get::accept_request(char *host, int port, char *file)
{
	_remote = file;
	if(dcc_session::accept_request(host, port, file) >= 0)
	{
		tk->bind(_sock, this, kit::readable);
		_state |= dcc_active;
		_state &= ~dcc_wait;
		//////////////////////
		meter->message("Downloading \"" + _remote + 
		               "\"\nfrom " + _nick + " [" +_uhost+']');
		meter->title("Receiving " + _remote + " from " + 
		             _nick);
		meter->build();
		meter->bind(this);
	}
	_file = open(file, O_CREAT|O_WRONLY|O_TRUNC, 0600);
	if(_file < 0)
		cout << "open failed!!" << endl;
	else
		cout << "Openend " << file << ", fd " << _file << endl;
	return _sock;
}

char	*dcc_get::file(void)
{
	return _remote;
}

void	dcc_get::close(void)
// close everything
{
	if(meter)
	{
		delete meter;
		meter = NULL;
	}
	
	if(_sock != -1)
		tk->unbind(_sock);
	link.close();
	_sock = -1;
	if(_file != -1)
		::close(_file);
	_file = -1;
}

int	dcc_get::transferblock(void)
{

	char	buf[1024];
	int	bread;
	unsigned	int	ack;
	
	link.flush(io::io_read);
	bread = link.read(buf, 1024);

	if(bread <= 0)
	{
		// check if file is complete
		_state |= dcc_delete;
		close();
		return false;
	}
	write(_file, buf, bread);
	_read += bread;

	//////////////////////

	meter->update((float)_read/(float)size, _read, size);
	ack = (unsigned int) htonl(_read);

	link.write((char *) &ack, sizeof(unsigned int));
//	tk->bind(_sock, this, kit::readable||kit::writable);
	link.flush(io::io_write);
	return bread;
}

int	dcc_get::operator()()
{
	if(_event->argc() == 2) 
	{
		cstring cmd = _event->arg(1);

		if(cmd == "abort")
		{
			cout << "Aborting!" << endl;
			close();
			_state |= dcc_delete;
			return TCL_OK;
		}
	}
	transferblock();
	return TCL_OK;
}

#define pack(f)	new func_packer(this, &dcc_handler::f)

// handles all sorts of DCC

dcc_handler::dcc_handler(): handler()
{
	tk->bind("dcc", this);
	init();
}

dcc_handler::~dcc_handler()
{
}

// implementation of the dcc commands

dcc_session	*dcc_handler::find(char *nick, char *file)
{
	cstring	n = nick;

	for(unsigned int i = 0; i < sessions.size(); i++)
	{
		dcc_session *s = (dcc_session *) sessions[i];
		if(n.compare(s->nick()))
		{
			if(!file)
				return s;
			if(s->type() == dcc_session::send && 
			   ((dcc_send*)s)->file() == file)
			   	return s;
			if(s->type() == dcc_session::get && 
			   ((dcc_get*)s)->file() == file)
			   	return s;
		}
	}
	return NULL;
}

int	dcc_handler::dcc_handle_chat(int argc, char **argv)
//
// This handler has two forms: 
// - the one that takes one argument, the nickname of a person to start
//   a chat with
// - the one that takes 3 parameters, a nickname, an ip-address and a 
//   portnumber to connect to
{
	// argv[0] = "DCC" argv[1] = "CHAT" !!

	if(argc == 3)
	{
		cstring	nick = argv[2];

		if(find(nick))
		{
			tk->result("Already session with " + nick);
			return false;
		}

		dcc_chat	*s = new dcc_chat(nick);
		if(s->start_connection(nick) < 0)
			{};		/////// failed!
		sessions.insert(sessions.end(), s);
		return true;
	}
	else if(argc == 5)
	{
		cstring	from = argv[2];
		cstring nick, user, host;

		split_nuh(from, nick, user, host);

		if(find(nick))
		{
			tk->result("Already session with " + nick);
		}

		dcc_chat	*s = new dcc_chat(nick);
		s->accept_request(argv[3], atoi(argv[4]));
		sessions.insert(sessions.end(), s);
		return true;
	}
	tk->result("Wrong number of parameters");
	return false;
}

int	dcc_handler::dcc_handle_close(int argc, char **argv)
// dcc close type nick ?file?
{
	if(argc < 4 || argc > 5)
	{
		tk->result("wrong # args: should be \""
		           "dcc close type nick ?file?\"");
		return false;
	}
	
	cstring	nick = argv[3];
	cstring	type = argv[2];
	dcc_session *s;
	
	if(argc == 4)
		s = find(nick);
	else
		s = find(nick, argv[4]);

	if(s)
	{
		s->close();		// force close
		s->state(dcc_session::dcc_delete);
	}
	else // include filename!!
	{
		tk->result("no " + type + " with " + nick);
		return false;
	}
	return true;
}

int	dcc_handler::dcc_handle_get(int argc, char **argv)
// Syntax: DCC GET nuh file ip port
{
	if(argc < 6 || argc > 7)
	{
		tk->result("wrong # args: should be \""
		           "dcc get nuh file ip port ?size?\"");
		return false;
	}

	cstring size;

	if(argc == 7)
		size = argv[6];
	else
		size = "0";

	cstring	nick, user, host;

	split_nuh(argv[2], nick, user, host);

	dcc_get	*s = new dcc_get(nick);
	s->uhost(user+'@'+host);
	s->size = atoi(size);
	s->accept_request(argv[4], atoi(argv[5]), argv[3]);
	sessions.insert(sessions.end(), s);
	return true;
}

int	dcc_handler::dcc_handle_send(int argc, char **argv)
// Syntax: DCC SEND nick file
{
	if(argc != 4)
	{
		tk->result("wrong # args: should be \""
		           "dcc send nick file\"");
		return false;
	}

	cstring	nick(argv[2]);
	cstring file(argv[3]);

	dcc_send	*s = new dcc_send(nick);
	if(s->start_connection(nick, file) < 0)
		{};				/////// handle failed
	sessions.insert(sessions.end(), s);
	return true;
}

int	dcc_handler::dcc_handle_status(int argc, char **argv)
// Return certain dcc info
//
// dcc status chat vladdrac
//  returns none, waiting, active
{
	if(argc != 4)
	{
		tk->result("wrong # args: should be \""
		           "dcc status nick type\"");
		return false;
	}
	
	cstring	nick = argv[2];
	cstring	type = argv[3];
	dcc_session *s = find(nick);
	
	if(!s)
	{
		tk->result("none");
		return true;
	}

	if(type == "chat" && s->type() == dcc_session::chat)
	{
		cstring result;

		if(s->state() & dcc_session::dcc_wait)
			result += " wait";
		if(s->state() & dcc_session::dcc_offered)
			result += " offered";
		if(s->state() & dcc_session::dcc_active)
			result += " active";
		if(s->state() & dcc_session::dcc_delete)
			result += " delete";
		// string now starts ' ' (unless it's empty). Remove it.
		if((int)result)
			tk->result(result(1));
		else
			tk->result("none");
	}
	else
		tk->result("none");
	return true;
}

int	dcc_handler::dcc_handle_msg(int argc, char **argv)
// sends a message to a uses through dcc (session must exist!)
// requires 2 parameters: nickname and message
{
	if(argc != 4)
	{
		tk->result("Wrong number of parameters");
		return false;
	}

	cstring	nick = argv[2];

	dcc_session *s = find(nick);

	if(!s || s->type() != dcc_session::chat)
	{
		tk->result("No dcc chat with " + nick);
		return false;
	}

	((dcc_chat *)s)->msg(argv[3]);
	return true;
}

void	dcc_handler::init(void)
{
	dcc_table["CHAT"] = pack(dcc_handle_chat);
	dcc_table["CLOSE"] = pack(dcc_handle_close);
	dcc_table["GET"] = pack(dcc_handle_get);
	dcc_table["MSG"] = pack(dcc_handle_msg);
	dcc_table["SEND"] = pack(dcc_handle_send);
	dcc_table["STATUS"] = pack(dcc_handle_status);
}

int	dcc_handler::operator()()
{
#warning "not working listcode - fix this!"
#if 0
	int	argc = _event->argc();
	char	**argv = _event->argv();
	garbage<dcc_session>	olddcc;
	list<dcc_session>	deletedcc;
	int			i;

	if(dbg > 5)
	{
	    cout << "DCC event!" << endl;

	    for(i = 0; i < argc; i++)
		cout << "argv[" << i << "] = " << argv[i] << endl;
	}

	if(argc == 1)
	{
		tk->result("wrong # args: should be \""
		           "dcc subcommand ?params?\"");
		return TCL_OK;
	}

	cstring		type = argv[1];

	func_packer	*h = (func_packer *)dcc_table[type.upper()];

	if(h)
	{
		// check if any dcc has to be closed, before and after
		for(i = 0; i < sessions.size(); i++)
		{
			dcc_session *s = sessions[i];

			if(s->state() & dcc_session::dcc_delete)
			{
				olddcc._register(s);
				deletedcc.insert(s);
			}
		}
		sessions.remove(deletedcc);
		h->execute(argc, argv);
		for(i = 0; i < sessions.size(); i++)
		{
			dcc_session *s = sessions[i];
			if(s->state() & dcc_session::dcc_delete)
			{
				olddcc._register(s);
				deletedcc.insert(s);
			}
		sessions.remove(deletedcc);
	}
	else
		tk->result("unknown or ambiguous option \"" + type +
		           "\": must be: chat, get, msg, send or status");
#endif
	return TCL_OK;
}
