/*
 * if_dcc.c - dcc interface stuff
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
#warning "canvas, item not implemented"
/*
 * Some interfaces for DCC
 */

#include <iostream.h>

#include "dialog.h"
#include "io.h"
#include "cstring.h"
#include "misc.h"

#include "dialog.h"

#include "if_dcc.h"

#warning "lots of canvas/itemcode deleted!"
int	dcc_meter::_serial=0;

int	dcc_acceptchat(const cstring &from, const cstring &rhost,
		       const cstring &rport)
// small dialog that asks the user if he/she wants to accept the incoming
// chat
{
	aws_dialog	chat(".chat");
	cstring	nick, user, host;

	// convert host and portnumber

	cstring _host = io::make_ip(rhost);
	split_nuh(from, nick, user, host);

	chat.title("Accept dcc chat");
	chat.message(nick + " (" + user + '@' + host +
	                    ") sends a dcc chat:\n"
	                    "["+_host+':'+rport+"]\n"
		            "Do you want to accept?");
	chat.button("Accept chat", "Accept", true);
	chat.button("Reject chat", "Reject");
	if(chat.wacht() == "Accept")
		return true;
	return false;
}

int	dcc_acceptfile(const cstring &from, cstring &file, 
                       const cstring &rhost, const cstring &rport, 
		       const int size)
// small dialog that asks the user if he/she wants to accept the incoming
// file
{
	// either we have to queue the files, or generate uniqe pathnames
	aws_dialog	upload(".file");
	cstring	nick, user, host;
	cstring	filesize;

	if(size)
		filesize << ", " << size << " bytes";

	// convert host and portnumber

	cstring _host = io::make_ip(rhost);
	split_nuh(from, nick, user, host);

	upload.title("Accept offered file");
	upload.message(nick + " (" + user + '@' + host +
	                    ") offers a file:\n" + file + filesize +
	                    "["+_host+':'+rport+"]\n");

	entry  *f = upload.entry("Save as: ", file);
	upload.button("Accept file", "Accept", true);
	upload.button("Reject file", "Reject");
	if(upload.wacht() == "Accept")
	{
		file = upload.getentry(f);
		return true;
	}
	return false;
}

// unique pathname
dcc_meter::dcc_meter(): toplevel(cstring(".download") << _serial++)
{
	topframe = new frame(this, ".topFrame", "-relief raised -border 1");
	buttonframe = new frame(this, ".buttonFrame", 
	                        "-relief raised -border 1");
	msg = new label(topframe, ".msgLabel");
	state = new label(topframe, ".stateLabel");
//	meter = new canvas(topframe, ".meterCanvas", 
//	                   "-relief sunken -border 2 -height 15");

//	progression = meter->rectangle(0,0,1,1,"-fill blue");
	cancelbutton = new aws_button(buttonframe, ".cancelButton");
	cancelbutton->text("Abort");

	cstring p = path();
	tk->eval("wm resizable " + p + " false false");

	_prevperc = -1;
}

dcc_meter::~dcc_meter()
{
	destroy();
}

void	dcc_meter::message(char *text)
{
	msg->text(text);
	state->text("0 bytes");
}

void	dcc_meter::build()
{
	msg->pack("-expand true -padx 10 -pady 10");
	state->pack("-expand true -padx 10 -pady 10");
//	meter->pack("-expand true -fill both -padx 10 -pady 10");
	cancelbutton->pack("-padx 10 -pady 10");
	topframe->pack();
	buttonframe->pack();

	tk->update();
//	cstring	c_path = meter->path();
//	_cwidth = atoi(tk->evaluate("winfo width " + c_path));
//	_cheight = atoi(tk->evaluate("winfo height " + c_path));
//	percent = meter->text(_cwidth/2, _cheight/2, "0%");
//	meter->tag(*percent, "perc");
//	meter->raise("perc");
}

// actually, update should only get the percentage, and nothing more.
// let dcc do the bookkeeping
//
// another idea: not 10 separate windows but all meters in 1 window
void	dcc_meter::update(float percentage, int cur, int total)
// store old value, only adjust if changed
{
	if(_prevperc == percentage)
		return;

	cstring m;
	
	m << cur << " bytes of " << total;
	state->text((char *)m);
	_prevperc = percentage;
//	meter->del(*progression);
//	progression = meter->rectangle(0,0,
//	   (int)(percentage*_cwidth),_cheight,"-fill blue");

	cstring perc;
	perc << '"' <<(int)(percentage*100) << " %\"";
//	meter->itemconfigure(*percent, "-text " + perc);
//	meter->raise("perc");
	tk->update();
}

void	dcc_meter::info(const cstring &_nick, const cstring &_file)
{
	nick = _nick;
	file = _file;
}

int	dcc_meter::operator()()
{
	int	argc = _event->argc();
	char	**argv = _event->argv();

	for(int i = 0; i < argc; i++)
		cout << "DCC " << i << " " << argv[i] << endl;

	if(argc == 2)
	{
		cstring	cmd = argv[1];

		if(cmd == "abort")
		{
			tk->eval("dcc close send " + nick + " " + file);
			cout << "dcc close send " + nick + " " + file << endl;
		}
	}
	return TCL_OK;
}

void	dcc_meter::bind(handler *h)
{
	cancelbutton->bind(h, "abort");
}
	
