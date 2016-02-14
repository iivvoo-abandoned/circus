/*
 * masterwindow.c - implements the masterwindow
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

#include "cstring.h"

#include "irctext.h"
#include "masterwindow.h"
#include "connection.h"

#include "misc.h"
#include "images.h"

#include "const.h"

extern	images  &img;

masterwindow::masterwindow(widget *w, char *p, char *opt): frame(w, p, opt)
{
	buildwindow(w);
	bindings();
}

masterwindow::~masterwindow(void)
{
}

frame	*masterwindow::createmenubar(widget *parent)
/*
 * This is the menu structure I'm currently thinking of:
 *
 * File			- Basic file/server/client management + exit
 *  - New connection	- Create a new (extra) connection
 *  - Edit connection	- change current connection
 *  - Close connection  - Close but not remove connection
 *  - Delete connection	- Remove connection
 *  - ----------
 *  - Load script	- Load a script
 *  - ----------
 *  - Exit		- Exits cIRCus
 *
 * Tools		- Tools such as Ignore, Log, Alias
 *  - Ignore		- pops up the 'ignore editor'
 *  - Log		- pops up the 'log editor'
 *  - Alias		- pops up the 'alias editor'
 *
 * DCC			- DCC stuff
 *  - Send		- Send a file to someone
 *  - Chat		- Start a chat session
 *
 * ..
 *
 * Help			- cIRCus help
 *  - Index		- Index of available topics
 *  (release note? etc?)
 *  - --------
 *  - About		- About cIRCus
 */

{
	menuframe = new frame(parent, ".menuFrame", 
	                              "-relief raised -border 1");

	file = new menubutton(menuframe, ".file");
	file->text("File");

	filemenu = new menu(file, ".menu", "-tearoff false");
	filemenu->entry("New connection", "new_connect");
	filemenu->entry("Change connection", "change_connect");
	filemenu->entry("Close connection", "close_connect");
	filemenu->entry("Delete connection", "delete_connect");
	filemenu->separator();
	filemenu->entry("Load script", "do_load");
	filemenu->separator();
	filemenu->entry("Exit", "quit");
	file->menu(filemenu);
	   
	tools = new menubutton(menuframe, ".tools");
	tools->text("Tools");

	toolsmenu = new menu(tools, ".menu", "-tearoff false");
	toolsmenu->entry("Ignore", "do_ignore");
	toolsmenu->entry("Log", "do_log");
	toolsmenu->entry("Alias", "do_alias");
	tools->menu(toolsmenu);

	dcc = new menubutton(menuframe, ".dcc");
	dcc->text("DCC");

	dccmenu = new menu(dcc, ".menu", "-tearoff false");
	dccmenu->entry("Send", "do_dccsend");
	dccmenu->entry("Chat", "do_dccchat");
	dcc->menu(dccmenu);

	help = new menubutton(menuframe, ".help");
	help->text("Help");

	helpmenu = new menu(help, ".menu", "-tearoff false");
	helpmenu->entry("Index", "do_helpindex");
	helpmenu->separator();
	helpmenu->entry("About cIRCus", "do_helpabout");
	help->menu(helpmenu);

	file->pack("-side left");
	tools->pack("-side left");
	dcc->pack("-side left");
	help->pack("-side right");

	menuframe->pack("-side top -fill x -expand false");
	return menuframe;
}

void	masterwindow::buildwindow(widget *top)
{
	createmenubar(top);		// build the menubar

	buttonframe = new frame(top, ".buttonframe");
	servers = new tixNoteBook(top, ".servers");

// UGLY HACK

	servers->eval("subwidget nbframe config -font 6x9");

	tixBalloon	*help = new tixBalloon(top, ".balloon");

	leavebutton = new plushButton(buttonframe, ".leaveButton");
	leavebutton->image(img["leave.gif"]);
	leavebutton->help_image(img["tleave.gif"]);
	help->bind(leavebutton, "Leave");
	
	joinbutton = new plushButton(buttonframe, ".joinButton");
	joinbutton->image(img["join.gif"]);
	joinbutton->help_image(img["tjoin.gif"]);
	help->bind(joinbutton, "Join");
	
	whoisbutton = new plushButton(buttonframe, ".whoisButton");
	whoisbutton->image(img["whois.gif"]);
	whoisbutton->help_image(img["twhois.gif"]);
	help->bind(whoisbutton, "Whois");
	
	querybutton = new plushButton(buttonframe, ".msgButton");
	querybutton->image(img["query.gif"]);
	querybutton->help_image(img["tquery.gif"]);
	help->bind(querybutton, "Query");
	
	kickbutton = new plushButton(buttonframe, ".kickButton");
	kickbutton->image(img["kick.gif"]);
	kickbutton->help_image(img["tkick.gif"]);
	help->bind(kickbutton, "Kick");
	
	banbutton = new plushButton(buttonframe, ".banButton");
	banbutton->image(img["ban.gif"]);
	banbutton->help_image(img["tban.gif"]);
	help->bind(banbutton, "Ban");
	
	bankickbutton = new plushButton(buttonframe, ".bankickButton");
	bankickbutton->image(img["kickban.gif"]);
	bankickbutton->help_image(img["tkickban.gif"]);
	help->bind(bankickbutton, "Bankick");
	
	banlistbutton = new plushButton(buttonframe, ".banlistButton");
	banlistbutton->image(img["banlist.gif"]);
	banlistbutton->help_image(img["tbanlist.gif"]);
	help->bind(banlistbutton, "Banlist");
	
	opbutton = new plushButton(buttonframe, ".opButton");
	opbutton->image(img["op.gif"]);
	opbutton->help_image(img["top.gif"]);
	help->bind(opbutton, "Op");
	
	voicebutton = new plushButton(buttonframe, ".voiceButton");
	voicebutton->image(img["voice.gif"]);
	voicebutton->help_image(img["tvoice.gif"]);
	help->bind(voicebutton, "Voice");
	
	invitebutton = new plushButton(buttonframe, ".inviteButton");
	invitebutton->image(img["invite.gif"]);
	invitebutton->help_image(img["tinvite.gif"]);
	help->bind(invitebutton, "Invite");
	
	awaybutton = new plushButton(buttonframe, ".awayButton");
	awaybutton->image(img["away.gif"]);
	awaybutton->help_image(img["taway.gif"]);
	help->bind(awaybutton, "Away");
	
	helpbutton = new plushButton(buttonframe, ".helpButton");
	helpbutton->image(img["help.gif"]);
	helpbutton->help_image(img["thelp.gif"]);
	help->bind(helpbutton, "Help");

	quitbutton = new plushButton(buttonframe, ".quitButton");
	quitbutton->image(img["quit.gif"]);
	quitbutton->help_image(img["tquit.gif"]);
	help->bind(quitbutton, "Quit");

        leavebutton->pack("-side left");
        joinbutton->pack("-side left");
        whoisbutton->pack("-side left");
        querybutton->pack("-side left");
        kickbutton->pack("-side left");
        banbutton->pack("-side left");
        bankickbutton->pack("-side left");
        banlistbutton->pack("-side left");
        opbutton->pack("-side left");
        voicebutton->pack("-side left");
        invitebutton->pack("-side left");
        awaybutton->pack("-side left");

        quitbutton->pack("-side right -padx 2");
        helpbutton->pack("-side right -padx 2");
	buttonframe->pack("-side top -fill x -expand false");

	servers->pack("-side top -expand true -fill both");

	// Disable buttons
	buttons_disable(BUTTON_ALL);
}

void	masterwindow::bindings()
{
	tk->eval("wm protocol . WM_DELETE_WINDOW { Master xxx quitbutton_pressed }");
	leavebutton->bind(this, "LeaveRequested");
//	leavebutton->bind("<Button-3>", this, "LeaveSelected");
//	joinbutton->bind(this, "JoinRequested");
//	joinbutton->bind("<Button-3>", this, "JoinSelected");
	whoisbutton->bind(this, "WhoisRequested");
	whoisbutton->bind("<Button-3>", this, "WhoisSelected");
	querybutton->bind(this, "QueryRequested");
	querybutton->bind("<Button-3>", this, "QuerySelected");
	kickbutton->bind(this, "KickRequested");
	kickbutton->bind("<Button-3>", this, "KickSelected");
	opbutton->bind(this, "OpRequested");
	opbutton->bind("<Button-3>", this, "OpSelected");
	voicebutton->bind(this, "VoiceRequested");
	voicebutton->bind("<Button-3>", this, "VoiceSelected");
	banbutton->bind(this, "BanRequested");
	banbutton->bind("<Button-3>", this, "BanSelected");
	bankickbutton->bind(this, "BankickRequested");
	bankickbutton->bind("<Button-3>", this, "BankickSelected");
	banlistbutton->bind(this, "banlistbutton_pressed");
	invitebutton->bind(this, "invitebutton_pressed");
	awaybutton->bind(this, "awaybutton_pressed");
	helpbutton->bind(this, "helpbutton_pressed");
	quitbutton->bind(this, "quitbutton_pressed");
}

void	masterwindow::buttons_enable_disable(const int which, const int mode)
{
	cstring newstate;

	if(mode)
		newstate = "active";
	else
		newstate = "disabled";
	
	if(which & BUTTON_ALL)			// disable all but help and quit
	{
		leavebutton->state(newstate);
		joinbutton->state(newstate);
		whoisbutton->state(newstate);
		querybutton->state(newstate);
		kickbutton->state(newstate);
		banbutton->state(newstate);
		bankickbutton->state(newstate);
		banlistbutton->state(newstate);
		opbutton->state(newstate);
		voicebutton->state(newstate);
		invitebutton->state(newstate);		
		awaybutton->state(newstate);
	}
	if(which & BUTTON_CHAN)			// channel related
	{
		leavebutton->state(newstate);
		kickbutton->state(newstate);
		banbutton->state(newstate);
		bankickbutton->state(newstate);
		banlistbutton->state(newstate);
		opbutton->state(newstate);
		voicebutton->state(newstate);
		invitebutton->state(newstate);	// invite still might work..
	}
	if(which & BUTTON_SOME)			// not channel related
	{
		joinbutton->state(newstate);
		whoisbutton->state(newstate);
		querybutton->state(newstate);
		awaybutton->state(newstate);
	}
}

void	masterwindow::buttons_enable(const int which)
{
	buttons_enable_disable(which, true);
}

void	masterwindow::buttons_disable(const int which)
{
	buttons_enable_disable(which, false);
}

