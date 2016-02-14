/*
 * qyerwin.c - implements query- and messagewindows
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

#include <plush.h>
#include "cstring.h"
#include "irctext.h"
#include "querywin.h"
#include "images.h"
#include "misc.h"
#include "internalevent.h"

extern	images	&img;
extern	int	dbg;

static 	int _serial = 0;

querywin::querywin(cstring p, cstring opt): 
	  targetif(targetif::query_target), 
          toplevel(cstring(".query") << _serial++, opt)

{
	sessionid = 0;
	serverentry = NULL;
	person = p;
	cstring dcc_status = tk->evaluate("dcc status " + p + " chat");
	if(dcc_status == "active")
		has_dcc = active;
	else
		has_dcc = none;
	use_dcc = false;
	build_querywindow(this);
	bindings();
	info = true;
}

querywin::~querywin(void)
{
}

int	querywin::wantinfo(int, char *argv[])
{
	cstring	rpl = argv[2];

	if(rpl == "301")
	{
		cstring	nick = argv[4];
		cstring	msg = argv[5];

		miscentry->configure("-state normal");
		miscentry->del();
		if(info)
			miscentry->insert(">AWAY< : " + msg);
		else
			miscentry->insert(nick + " is AWAY: " + msg);
		miscentry->configure("-state disabled");
		return true;
	}
	else if(rpl == "401")
	{
		cstring	nick = argv[4];

		nickentry->configure("-state normal");
		nickentry->del();
		if(serverentry)
			serverentry->del();
		miscentry->del();
		nickentry->insert(nick + " is not online");
		nickentry->configure("-state disabled");

		insert(nick + " is not online\n", "notice");
		info = false;
	}

	if(!info)
		return false;

	if(rpl == "311")		// nick is user@host 
	{
		cstring	nick = argv[4];
		cstring user = argv[5];
		cstring host = argv[6];
		cstring name = argv[8];

		nickentry->configure("-state normal");
		nickentry->del();
		nickentry->insert(nick + " (" + user + "@" + 
		                  host + "): "+name);
		nickentry->configure("-state disabled");
	}
	else if(rpl == "312")
	{
		cstring	server = argv[5];
		cstring 	serverinfo = argv[6];

		serverentry->configure("-state normal");
		serverentry->del();
		serverentry->insert(server + " (" + serverinfo + ")");
		serverentry->configure("-state disabled");
	}
	else if(rpl == "318")		// end of whois
		info = false;
	return true;
}

int	querywin::get_autoraise(void)
{
	return autoraise->value();
}

cstring	querywin::get_person(void) const
{
	return person;
}

void	querywin::insert(const cstring &s, const cstring &t)
/*
 * insert a string into this window.
 * s		the message
 * tagname	one or more tag attributes
 */
{
	query->insert(s, t);
}

void	querywin::insert(const cstring &p, const cstring &s, 
                         const unsigned int l,
			 const cstring &tagname)
/*
 * insert a string into this window.
 * p 		the prompt (i.e. a nickname)
 * s		the message
 * l		the alignment of the message
 * tagname	one or more tag attributes
 */
{
	query->insert(p, s, l, tagname);
}

void	querywin::current(int mode, int doraise)
{
	if(mode)
	{
		queryframe->configure("-bg yellow");
		if(doraise)
		{
			tk->eval("wm deiconify " + cstring(path()));
			tk->eval("raise " + cstring(path()));
		}
	}
	else
		queryframe->configure("-bg grey");
}

void	querywin::build_querywindow(widget *top)
{
	// Frames
	cstring	winname(path());

	nickframe = new frame(top, ".nickFrame");
	serverframe = new frame(top, ".serverFrame");
	miscframe = new frame(top, ".miscframe");

	queryframe = new frame(top, ".queryFrame", "-border 2");
	buttonframe = new frame(top, ".buttonFrame");

	nicklabel = new label(nickframe, ".nickLabel");
	serverlabel = new label(serverframe, ".serverLabel");
	misclabel = new label(miscframe, ".miscFrame");

	nicklabel->text("Nick  :");
	serverlabel->text("Server:");
	misclabel->text("Info  :");

	nickentry = new entry(nickframe, ".nickEntry", "-border 1");
	serverentry = new entry(serverframe, ".serverEntry", "-border 1");
	miscentry = new entry(miscframe, ".miscEntry", "-border 1");

	nickentry->configure("-state disabled");
	serverentry->configure("-state disabled");
	miscentry->configure("-state disabled");

	dcc_button = new checkbutton(miscframe, ".dccButton");
	dcc_button->text("dcc");
	if(has_dcc == active)
		dcc_button->select();

	chat_button = new checkbutton(miscframe, ".chatButton");
	chat_button->text("chat");

	autoraise = new checkbutton(miscframe, ".autoraiseCheck");
	autoraise->text("autoraise");

	query = new irctext(queryframe, ".queryText", "-width 80 -height 13");
	queryscroll = new scrollbar(queryframe, ".queryScroll");
	queryscroll->yview(query);
	query->yscroll(queryscroll);

	stopbutton = new plushButton(buttonframe, ".stopButton");
	stopbutton->image(img["tnt.gif"]);
	stopbutton->help_image(img["explode.gif"]);

	in = new plushHistEntry(buttonframe, ".queryIn");

	nicklabel->pack("-side left");
	serverlabel->pack("-side left");
	misclabel->pack("-side left");

	nickentry->pack("-side left -fill x -expand true");
	serverentry->pack("-side left -fill x -expand true");
	miscentry->pack("-side left -fill x -expand true");

	dcc_button->pack("-side left");
	chat_button->pack("-side left");
	autoraise->pack("-side left");

        stopbutton->pack("-side left");
	in->pack("-side left -expand true -fill x");

	query->configure("-state disabled");
	query->pack("-side left -fill both -expand true");
	queryscroll->pack("-side right -fill y -expand false");

	nickframe->pack("-fill x -expand false");
	serverframe->pack("-fill x -expand false");
	miscframe->pack("-fill x -expand false");
	queryframe->pack("-side top -fill both -expand true");
	buttonframe->pack("-side bottom -fill x -expand false");

	title("Conversation with " + person);
}

void	querywin::bindings()
{
	// are path and person different?
	cstring wpath = path();
	cstring	which = esc(person.lower());

	tk->eval("wm protocol " + wpath + " WM_DELETE_WINDOW "
	         "\" QueryWin " + esc(which) + " xx close_query \"");
	bind(this);
	toplevel::bind("<Any-FocusIn>", this, "take_focus");
	stopbutton->bind(this, "close_query");
	dcc_button->bind(this, "dcc_pressed");
	chat_button->bind(this, "chat_pressed");
//	in->bind("<Return>", this, "input_typed");
	in->bind("<Return>", (void *)this, StaticHandleInput, "input");
	queryscroll->bind("<ButtonPress>", this, "togglelock");
	queryscroll->bind("<ButtonRelease>", this, "togglelock");
}

void	querywin::nickchange(const cstring &newnick)
{
	insert(person + " is now known as " + newnick + "\n", "notice");
	person = newnick;
	title("Conversation with " + person);
	info = true;		// reset info
}

char	*querywin::getinput(void)
{
	return in->get();
}

void	querywin::delinput(void)
{
	in->del();
}

void	querywin::setdcc(int mode)
{
	if(mode != none)
		dcc_button->select();
	else
		dcc_button->deselect();
	has_dcc = mode;
}

int	querywin::getdcc(void)
{
	return has_dcc;
}

void	querywin::setchat(int mode)
{
	if(mode)
		chat_button->select();
	else
		chat_button->deselect();
	use_dcc = mode;
}

int	querywin::getchat(void)
{
	return use_dcc;
}

void	querywin::toggle_lock(void)
{
	query->toggle_lock();
}

void	querywin::clear(void)
{
	query->clear();
}

void	querywin::bind(handler *_h)
{
	tk->bind("QueryWin", _h);
}

int	querywin::operator()()
{
	int 	i;
	int 	argc = _event->argc(); char **argv = _event->argv();
	cstring	cmd;

	if(dbg > 5)
	{
	    cout << "An event happened:" << endl;
	    for(i = 0; i < argc; i++)
		cout << "Arg " << i << ": " << argv[i] << endl;
	}
	cmd = "QueryWin ";
	cmd += esc(person.lower());

	for(i = 0; i < argc; i++)
	{
	        cmd += " ";
		cmd += esc(argv[i]);
	}

	tk->eval(cmd);
	return true;
}



messagewin::messagewin(cstring p, cstring opt):
	  targetif(targetif::query_target), 
	  toplevel(p.lower(), opt)
{
	sessionid = 0;
	info = false;
	serverentry = NULL;
	build_messagewindow(this);
	bindings();
}

messagewin::~messagewin(void)
{
}

int	messagewin::wantinfo(int, char *argv[])
{
	cstring	rpl = argv[2];

	if(rpl == "301")
	{
		cstring	nick = argv[4];
		cstring	msg = argv[5];

		miscentry->configure("-state normal");
		miscentry->del();
		if(info)
			miscentry->insert(">AWAY< : " + msg);
		else
			miscentry->insert(nick + " is AWAY: " + msg);
		miscentry->configure("-state disabled");
		return true;
	}
	else if(rpl == "401")
	{
		cstring	nick = argv[4];

		nickentry->configure("-state normal");
		nickentry->del();
		if(serverentry)
			serverentry->del();
		miscentry->del();
		nickentry->insert(nick + " is not online");
		nickentry->configure("-state disabled");

		insert(nick + " is not online\n", "notice");
		info = false;
	}

	if(!info)
		return false;

	if(rpl == "311")		// nick is user@host 
	{
		cstring	nick = argv[4];
		cstring user = argv[5];
		cstring host = argv[6];
		cstring name = argv[8];

		nickentry->configure("-state normal");
		nickentry->del();
		nickentry->insert(nick + " (" + user + "@" + 
		                  host + "): "+name);
		nickentry->configure("-state disabled");
	}
	else if(rpl == "312")
	{
		cstring	server = argv[5];
		cstring 	serverinfo = argv[6];

		serverentry->configure("-state normal");
		serverentry->del();
		serverentry->insert(server + " (" + serverinfo + ")");
		serverentry->configure("-state disabled");
	}
	else if(rpl == "318")		// end of whois
		info = false;
	return true;
}

int	messagewin::get_autoraise(void)
{
	return autoraise->value();
}

void	messagewin::insert(const cstring &s, const cstring &t)
{
	message->insert(s, t);
}

void	messagewin::insert(const cstring &p, const cstring &s, 
                         const unsigned int l,
			 const cstring &tagname)
{
	message->insert(p, s, l, tagname);
}

void	messagewin::build_messagewindow(widget *top)
{
	// Frames
	messageframe = new frame(top, ".messageFrame");
	outframe = new frame(messageframe, ".outFrame");
	rightframe = new frame(top, ".rightframe");
	nickframe = new frame(messageframe, ".nickFrame");
	miscframe = new frame(messageframe, ".miscframe");
	listframe = new frame(rightframe, ".listframe");
	buttonframe = new frame(rightframe, ".buttonFrame");

	nicklabel = new label(nickframe, ".nickLabel");
	misclabel = new label(miscframe, ".miscFrame");

	nicklabel->text("Last from:");
	misclabel->text("Away info:");

	nickentry = new entry(nickframe, ".nickEntry", "-border 1");
	miscentry = new entry(miscframe, ".miscEntry", "-border 1");

	nickentry->configure("-state disabled");
	miscentry->configure("-state disabled");

	message = new irctext(outframe, ".messageText", 
			      "-width 80 -height 13");
	messagescroll = new scrollbar(outframe, ".messageScroll");
	messagescroll->yview(message);
	message->yscroll(messagescroll);

	in = new plushHistEntry(messageframe, ".messageIn");

        targetlist = new listbox(listframe, ".targetlist", 
                                 "-width 10 -height 8 -font \"6x13\" "
                                 "-exportselection false -selectmode extended");
        targetscroll = new scrollbar(listframe, ".targetscroll");
        targetscroll->yview(targetlist);
        targetlist->yscroll(targetscroll);
//        targetlist->singleselect();

	autoraise = new checkbutton(buttonframe, ".autoraiseCheck");
	autoraise->text("autoraise");
	autoraise->select();

	nicklabel->pack("-side left");
	misclabel->pack("-side left");

	nickentry->pack("-side left -fill x -expand true");
	miscentry->pack("-side left -fill x -expand true");

	autoraise->pack("-side top -fill both -expand true");

        targetlist->pack("-side left -fill y -expand true");
        targetscroll->pack("-side right -fill y -expand true");

	nickframe->pack("-fill x -expand false");
	miscframe->pack("-fill x -expand false");
	listframe->pack("-side top -fill y -expand true");
	buttonframe->pack("-fill both -expand false");
	rightframe->pack("-side right -fill y -expand false");

	messageframe->pack("-side top -fill both -expand true");
	message->configure("-state disabled");
	message->pack("-side left -fill both -expand true");
	messagescroll->pack("-side right -fill y -expand false");
	outframe->pack("-side top -fill both -expand true");

	in->pack("-side left -expand true -fill x");


	title("Messages");
}

void	messagewin::add_person(const cstring &person)
{
        int     size = targetlist->size();
 
        for(int i = 0; i < size; i++)
        {
                cstring  who = targetlist->get(i);
		if(who.compare(person))
			return;
	}
	targetlist->insert(0, person);
}

void	messagewin::del_person(const cstring &person)
{
        int     size = targetlist->size();
 
        for(int i = 0; i < size; i++)
        {
                cstring  who = targetlist->get(i);
                if(who.compare(person))
		{
                        targetlist->del(i);
			return;
		}
	}
}

cstring	messagewin::selected_person(void)
{
        int     size = targetlist->size();
 
        for(int i = 0; i < size; i++)
        {
                cstring  who = targetlist->get(i);
		if(targetlist->selected(i))
			return who;
	}
	return "";
}

char	*messagewin::getinput(void)
{
	return in->get();
}

void	messagewin::delinput(void)
{
	in->del();
}

int	messagewin::getdcc(void)
{
	return false;		// perhaps supply nickname and check?
}

int	messagewin::getchat(void)
{
	return false;
}

void	messagewin::clear(void)
{
	message->clear();
}

void	messagewin::toggle_lock(void)
{
	message->toggle_lock();
}

void	messagewin::bindings()
{
	cstring wpath = path();

	tk->eval("wm protocol " + wpath + " WM_DELETE_WINDOW "
	         "{ MessageWin messages xx close_message }");
	bind(this);
	targetlist->bind("<Double-Button-1>", this, "start_query");
	targetlist->bind("<Button-3>", this, "open_mess_menu");
	in->bind("<Return>", this, "input_typed");
	messagescroll->bind("<ButtonPress>", this, "togglelock");
	messagescroll->bind("<ButtonRelease>", this, "togglelock");
}

void	messagewin::bind(handler *_h)
{
	tk->bind("MessageWin", _h);
}

int	messagewin::operator()()
{
	int 	i;
	int 	argc = _event->argc(); char **argv = _event->argv();
	cstring	cmd;

	if(dbg > 5)
	{
	    cout << "An event happened:" << endl;
	    for(i = 0; i < argc; i++)
	  	cout << "Arg " << i << ": " << argv[i] << endl;
	}

	cmd = "MessageWin ";
	cmd += (path() + 1);

	for(i = 0; i < argc; i++)
	{
	        cmd += " ";
		cmd += argv[i];
	}

	tk->eval(esc(cmd));
	return true;
}

/*
 * Event handlers/forwarders
 */

int	querywin::handleInput(void)
{
	cstring	typed = in->get();

	if(typed.length() != 0)
	{
		// Commands are prefixed by a /. If it's not, it's a message
		if(typed[0] != '/')
			tk << CMDLINE << sessionid << " " << escape(person) 
			 	      << " \"msg " << escape(person) << " " 
				      << escape(typed) << "\"" << end;
		else	// skip / when passing to cmdhandler!
			tk << CMDLINE << sessionid << " " << escape(person) 
			   << " \"" << escape(typed(1)) << "\"" << end;
		
		in->del();
	}
	return TCL_OK;
}

int	querywin::StaticHandleInput(void *h, event *e)
{
	((querywin *)h)->setevent(e);
	return ((querywin *)h)->handleInput();
}
