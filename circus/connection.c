/*
 * connection.c - Encapsulates connection-related classes (servergui, 
 *		  serverconnection, etc)
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

#include <iostream.h>
#include "connection.h"

#include "dcc.h"

#include "misc.h"
#include "images.h"
#include "const.h"

extern	images	&img;
extern	int	dbg;

int	connection::count = 0;

connection::connection(): servergui(NULL, "ERROR", 0),
			  ircserver(),
			  handler()
{
	sessionid = 0;
}

connection::connection(tixNoteBook *parent, int sessionid): 
            servergui(parent, "Not Connected", count++), 
	    ircserver(),
	    handler()
{
	this->sessionid = sessionid;
}

connection::~connection()
{
}

void	connection::init(const char *name, const char *host, unsigned int port)
{
	char	*_nick, *_login, *_ircname, *_fingerinfo, *_userinfo,
		*_scriptdir, *_downloaddir, *_signoff;

	setlabel(name);

	_nick = tk->getvar("nick");
	_login = tk->getvar("login");
	_ircname = tk->getvar("ircname");
	_fingerinfo = tk->getvar("fingerinfo");
	_userinfo = tk->getvar("userinfo");
	_scriptdir = tk->getvar("scriptdir");
	_downloaddir = tk->getvar("dccdownloaddir");
	_signoff = tk->getvar("signoff");
	
	if(_nick) nick = _nick;
	else nick = DEFAULT_NICK;
		
	if(_login) login = _login;
	else login = DEFAULT_LOGIN;
		
	if(_ircname) ircname = _ircname;
	else ircname = DEFAULT_IRCNAME;
		
	if(_fingerinfo) fingerinfo = _fingerinfo;
	else fingerinfo = DEFAULT_FINGERINFO;
		
	if(_userinfo) userinfo = _userinfo;
	else userinfo = DEFAULT_USERINFO;
		
	if(_scriptdir) scriptdir = _scriptdir;
	else scriptdir = DEFAULT_SCRIPTDIR;

	if(_downloaddir) downloaddir = _downloaddir;
	else downloaddir = DEFAULT_DOWNLOADDIR;

	if(_signoff) signoff = _signoff;
	else signoff = CIRCUS_QUIT;

	serverhost = host;
	serverport = port;

	umode = 0;
	deathwish = false;
	//bans = NULL;
	awaymsg = "";
	current = "";
}

void	connection::start(void)
{
	setnick(nick);

	connect_server();

	dcc_handler	*dcc_hndlr = new dcc_handler();

	KitObject::_register(dcc_hndlr);

}

void	connection::reregister(void)
// After a tcp connection, register yourself
{
	reguser(nick, login, ircname);
}

void	connection::connected(void)
// called when we've connected to the server
// delete old handlers?
{
	servergui::mode_enable(MODE_WALLOP);
	servergui::mode_enable(MODE_INVISIBLE);
	servergui::mode_toggle(MODE_RESTRICTED, false);

	awaymsg = "";
	update_title();
}

int	connection::reconnect_dialog(const cstring &problem)
{
#warning "dialogs removed (combobox shit)"
#if 0
	aws_dialog	reconnect(".reconnectDialog");

	reconnect.title("Reconnect to a server");

	reconnect.message(problem);

	reconnect.image(img["disconnect.gif"]);
	aws_combobox *serv_combo = reconnect.combobox("Connect to: ");

//	serv_combo->add(serverhost);

	for(int i = 0; i < servers.size(); i++)
	{
		cstring hostport;

		hostport << servers[i]->host << ":" << servers[i]->port;
		serv_combo->add(hostport);
	}

	reconnect.button("Reconnect", "ReconnectPressed", true);
	reconnect.button("Cancel", "CancelPressed");

	cstring result = reconnect.wacht();
	if(result == "ReconnectPressed")
	{
		cstring_iter hostportiter = reconnect.getcombobox(serv_combo);
		cstring	serverhost = hostportiter(" :;,");
		cstring	serverport = hostportiter(" :;,");
		//add_server(serverhost, atoi(serverport));

		ircserver::init(serverhost, atoi(serverport));

		insert("Attempting to connect to server " << serverhost <<
                       " port " << serverport << "\n");

		ircserver::establish();

		update_title();
		refocus();
		return true;
	}
#endif
	return false;
}

void	connection::disconnected(const cstring &error, int showdialog)
// called when we're disconnected from the server
{
	cstring	why = error;

//	masterwindow::buttons_disable(BUTTON_SOME);
	servergui::mode_disable(MODE_WALLOP);
	servergui::mode_disable(MODE_INVISIBLE);
	servergui::mode_disable(MODE_SERV_NOT);

	cstring	serverhost;
	serverhost << serverhost << ':' << serverport;

	insert("Connection to " + serverhost + " lost: " + why + "\n");
	ircserver::abort();

	update_title();

	if(deathwish)
		tk->quit();
	else if(showdialog)
	{
		int	done;

		do
		{
			done = true;

//			if(reconnect_dialog(serverhost + 
//			  " has closed the connection:\n" + why))
//				done = ircserver::establish() > 0;
			reconnect_dialog(serverhost +
			                 " has closed the connection:\n" + why);
			why = ircserver::geterrorstr();
		}
		while(!done);
	}
}

void	connection::update_title()
{
	cstring	statemsg;

	if(ircserver::state(ircserver::disconnected))
		statemsg = ", not connected";
	else if(ircserver::state(ircserver::connecting))
		statemsg = ", connecting";
	else
	{
		cstring	tl_away, tl_target;

		if(awaymsg != (const char *) "")
			tl_away = " ("+awaymsg+")";
		if(current == "")
			tl_target =  ", no current target";
		else if(ischannel(current))
			tl_target = " on channel " + current;
		else
			tl_target = " querying " + current;
		statemsg = tl_away + tl_target;
	}
	tk->eval("wm title . \"" + esc(nick + statemsg) + '"');
}

void	connection::setnick(const cstring &n)
{
	nick = n;
	servergui::setnick(n);
	update_title();
}

void	connection::setaway(const cstring &msg)
{
	awaymsg = msg;
	update_title();
}

void	connection::add_target(const cstring &t)
{
	if(ischannel(t))
	{
//		servergui::buttons_enable(BUTTON_CHAN);
		//if(bans)
		//	bans->add_channel(t);
	}
	set_current(t);
	servergui::add_target(t);
}

void	connection::del_target(const cstring &t)
{
	servergui::del_target(t);
	set_current(gettargetlist(0));
//	if(!ischannel(current))
//		masterwindow::buttons_disable(BUTTON_CHAN);
}

void	connection::ren_target(const cstring &oldnick, const cstring &newnick)
// rename a nick, works only for nicks.
{
	servergui::ren_target(oldnick, newnick);
	if(current == oldnick)
	{
		current = newnick;
		servergui::settarget(current);
		update_title();
	}
}

void	connection::set_current(const cstring t)
{
	current = t;
	servergui::settarget(t);
	update_title();
}

int	connection::connect_server(void)
// Connect to the current server.
{
	// disconnect if we we're connected/connecting
	if(!ircserver::state(ircserver::disconnected))
	{
		ircserver::quit("Changing servers");
		disconnected("Changing servers", false);
		ircserver::abort();
	}

	insert("Attempting to connect to server " << serverhost << " port "
	        << serverport << "\n");
	ircserver::init(serverhost, serverport);
	if(ircserver::establish() < 0)
	{
		cstring	why = ircserver::geterrorstr();
		insert("Could not connect to server " << serverhost << " port "
		       << serverport << ": " << why << "\n");
		return false;
	}
	update_title();
	return true;
}

int	connection::get_mode(void) const
{
	return umode;
}

void	connection::set_mode(const int m)
{
	umode |= m;
}

void	connection::del_mode(const int m)
{
	umode &= ~m;
}

int	connection::operator()()
{
	int	argc = _event->argc();
	char	**argv = _event->argv();
	cstring	eventtype = argv[0];
	
cout << "connection::operator()()" << endl;
	if(dbg > 5)
	{
           cout << "Masterclass: An event happened:" << eventtype << endl;
	   for(int i = 0; i < argc; i++)
		cout << "arg: " << i << ": " << argv[i] << endl;
	}
	return false;		// can't handle events. Pass to other?
}
