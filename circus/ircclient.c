/*
 * ircclient.c - Encapsulate the GUI, the serverconnection and everything 
 *		 else into one object.
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
#include "ircclient.h"
#include "internalevent.h"

ircclient::ircclient(widget *w, char *path): masterwindow(w, path)
{
	serverserial = 0;
	serverList.initialize();

	tk->bind(SERVER, (void *)this, staticHandleServer);
}

ircclient::~ircclient()
{
	// remove connections in connectionList after properly closing them
}

void	ircclient::start()
{
	// search the serverlist for servers to connect to

	for(int i = 0; i < serverList.size(); i++)
	{
		serverlist::serverentry	*e = serverList.serverAt(i);

		if(e->autoconnect)
		{
			ircsession	*c = new ircsession(serverWidget(), 
					     serverserial++);
			c->init(e->host, e->host, e->port);
			c->start();

			((targets *)c)->bind(this);

			connectionList.insert(connectionList.end(), c);
		}
	}
}

/*
 * Masterwindow event handlers
 *
 * These handlers get called by ircclient::operator()(), which dispatches events
 * from the masterwindow
 */

int	ircclient::operator()()
{
	cout << "ircclient::operator()()" << endl;

	int	argc = _event->argc();
	int	i;
	char	**argv = _event->argv();
	cstring	eventtype = argv[0];
	
        cout << "ircclient: An event happened:" << eventtype << endl;
	for(i = 0; i < argc; i++)
		cout << "arg: " << i << ": " << argv[i] << endl;

	return true;
}

void	ircclient::changeServer(int serverid, char *host, int port)
{
	for(int i = 0; i < connectionList.size(); i++)
	{
		ircsession *c = connectionList[i];

		if(c->sessionid == serverid)
		{
			cout << "SERVER FOUND!" << endl;
		}
	}
}

/////////////////////////////////////////////////////////

int	ircclient::handleServer(void)
{
	
	int	argc = _event->argc();
	int	i;
	char	**argv = _event->argv();
	cstring	eventtype = argv[0];
	
        cout << "ircclient::handleServer An event happened:" << 
		eventtype << endl;
	for(i = 0; i < argc; i++)
		cout << "arg: " << i << ": " << argv[i] << endl;
	
	if(argc < 3)
		; 	// error, sowieso te weinig
	
	cstring	option = argv[1];

	if(option == "-change")
	{
		if(argc < 4)
			; // error, teweinig

		int	serverid;
		int	port = DEFAULT_PORT;
		char	*host = argv[3];
		
		if(sscanf(argv[2], "%d", &serverid) != 1)
			; // error not a number
		
		if(argc == 4)
			if(sscanf(argv[4], "%d", &port) != 1)
				; // error incorrect port
		if(argc > 4)
			; // error, teveel

		changeServer(serverid, host, port);
	}
	else if(option == "-close")
	{
	}
	else if(option == "-disconnect")
	{
	}
	else if(option == "-connect")
	{
	}
	else
	{
	}
	return TCL_OK;
}

int	ircclient::staticHandleServer(void *h, event *e)
{
	((ircclient *)h)->setevent(e);
	return ((ircclient *)h)->handleServer();
}
