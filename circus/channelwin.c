/*
 * channelwin.c - Graphical representation of an IRC channel
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

#include <stdlib.h>			// for atoi()

#include "cstring.h"
#include "user.h"
#include "irctext.h"

#include "internalevent.h"

#include "channel_target.h"
#include "channelwin.h"

#include "images.h"
#include "misc.h"

extern	images	&img;
extern 	int	dbg;


static int _serial = 0;

channelwin::channelwin(char *p, char *opt): 
            toplevel(cstring(".channel") << _serial++, opt)
{
	sessionid = 0;
	cname = p;
	buildwindow(this);
}

channelwin::~channelwin()
{
}

void	channelwin::buildwindow(widget *top)
{
	// Frames
	leftframe = new frame(top, ".leftframe", "-border 2");
	  split = new tixPanedWindow(leftframe, ".pane");

	    upper = split->add("upper");
	      // infoframe packs the info textwidget and scrollbar

	      infoframe = new frame(upper, ".infoframe");
	      info = new irctext(infoframe, ".infoText", "-width 80 -height 5");
	      infoscroll = new scrollbar(infoframe, ".infoscroll");
	      infoscroll->yview(info);
	      info->yscroll(infoscroll);
	
	      info->pack("-side left -fill both -expand true");
	      infoscroll->pack("-side right -fill y -expand false");
	      infoframe->pack("-side top -fill both -expand true");
	    
	    lower = split->add("lower");
	      topicframe = new frame(lower, ".topicframe");
	        topiclabel = new label(topicframe, ".topicLabel");
	        topiclabel->text("Topic: ");
	        topicentry = new entry(topicframe, ".topicEntry");
	        topiclabel->pack("-side left -fill none -expand false");
	        topicentry->pack("-side left -fill x -expand true");
	        topicframe->pack("-side top -fill x -expand false");

	      outputframe = new frame(lower, ".outputframe");
	         output = new irctext(outputframe, ".outputText", 
	                                           "-width 80 -height 15");
	         outputscroll = new scrollbar(outputframe, ".outputscroll");
	         outputscroll->yview(output);
	         output->yscroll(outputscroll);
	         output->pack("-side left -fill both -expand true");
	         outputscroll->pack("-side right -fill y -expand false");
	         outputframe->pack("-side top -fill both -expand true");

	    split->pack("-side top -expand true -fill both");

	  statusframe = new frame(leftframe, ".statusframe");
	    modelabel = new label(statusframe, ".l1", "-text {Mode: }");
	    chanmode = new menubutton(statusframe, ".chanmode", 
	                              "-border 1 -relief raised -pady 0");
	    chanmode->text("+tiklm");
	    keylabel = new label(statusframe, ".l2", "-text {Key: }");
	    keyentry = new entry(statusframe, ".keyEntry", "-width 10");
	    
	    limitlabel = new label(statusframe, ".l3", "-text {Limit: }");
	    limitentry = new entry(statusframe, ".limitEntry", "-width 5");

	    modelabel->pack("-side left -expand false");
	    chanmode->pack("-side left -expand false");
	    keylabel->pack("-side left -expand false");
	    keyentry->pack("-side left -expand false");
	    limitlabel->pack("-side left -expand false");
	    limitentry->pack("-side left -expand false");
	    
	    statusframe->pack("-side top -fill x -expand false");
	  
	  statframe = new frame(leftframe, ".statframe");
	    in = new plushHistEntry(statframe, ".inEntry", "-width 80");
	    in->pack("-expand true -fill x");
	    statframe->pack("-side bottom -expand false -fill both");

	  leftframe->pack("-side left -fill both -expand true");

	userframe = new frame(top, ".userframe");
          userlist = new listbox(userframe,
                       ".userlist", "-width 10 -font \"6x13\""
	  	        " -exportselection false -selectmode extended");
          userscroll = new scrollbar(userframe, ".userscroll");
          userscroll->yview(userlist);
          userlist->yscroll(userscroll);
	  userlist->pack("-side left -fill both -expand true");
	  userscroll->pack("-side right -fill y -expand false");
	  userframe->pack("-side top -fill y -expand true");

	title(cname);
}

void	channelwin::current(int mode, int doraise)
{
	if(mode)
	{
		leftframe->configure("-bg yellow");
		if(doraise)
		{
			toplevel::deiconify();
			toplevel::raise();
		}
	}
	else
		leftframe->configure("-bg grey");
}

void	channelwin::disable(void)
// Disable all widgets, i.e. after a kick
{
	topic_disable();
	key_disable();
	limit_disable();
	mode_disable_all();

	userlist->del();
	output->configure("-foreground white");
	info->configure("-foreground white");
}

void	channelwin::toggle_lock(void)
{
	output->toggle_lock();
}

void	channelwin::enable(void)
{
	set_op(false);
	topic_enable();
	output->configure("-foreground black");
	info->configure("-foreground black");
	// reset inputentries and checkbuttons
	topicentry->del();
	keyentry->del();
	limitentry->del();
}

void	channelwin::popup_menu(cstring, int /*modes*/, int /*x*/, 
			       int /*y*/, int /*enable*/)
{
}

void	channelwin::bind(void *h, int (*func)(void *, event *))
{
	topicentry->bind(h, func, "topic_changed");
	keyentry->bind(h, func, "key_changed");
	limitentry->bind(h, func, "limit_changed");
	userlist->bind("<Double-Button-1>", h, func, "start_query");

	leftframe->bind("<Enter>", h, func, "take_focus");
	leftframe->bind("<Leave>", h, func, "lose_focus");
	
	outputscroll->bind("<ButtonPress>", h, func, "togglelock");
	outputscroll->bind("<ButtonRelease>", h, func, "togglelock");
	in->bind("<Return>", (void*)this, StaticHandleInput, "input");

}
char	*channelwin::getinput(void)
{
	return in->get();
}

void	channelwin::delinput(void)
{
	in->del();
}

int	channelwin::operator()()
{
	int 	i;
	int 	argc = _event->argc(); char **argv = _event->argv();
	cstring	cmd;

	if(dbg > 5)
	{
	    cout << "Channelwin event happened:" << endl;
	    for(i = 0; i < argc; i++)
		cout << "Arg " << i << ": " << argv[i] << endl;
	}

	cmd = eventname + " " + esc(cname);	// .lower()?
	
	for(i = 0; i < argc; i++)
	{
		cmd += " ";
		cmd += esc(argv[i]);
	}

	cmd << " " <<  _event->x_root() << " " << _event->y_root();

	tk->eval(cmd);
	return true;
}

void	channelwin::add_user(const cstring &user, const int index, 
                                                const int mode)
{
	cstring 	new_user;

	if(mode & UMODE_OP)
		new_user = "@" + user;
	else if(mode & UMODE_VOICE)
		new_user = "+" + user;
	else
		new_user = " " + user;

	// new users are inserted at the top
	userlist->insert(index, new_user);

}

int	channelwin::del_user(const cstring &user)
{
	int	size = userlist->size();

	for(int i = 0; i < size; i++)
	{
		cstring	who = userlist->get(i);
		cstring	stripped = who(1);
		if(stripped == user)
		{
			userlist->del(i);
			return i;
		}
	}
	return -1;
}

int	channelwin::user_selected(const cstring &user) const
{
	int	size = userlist->size();

	for(int i = 0; i < size; i++)
	{
		cstring	who(userlist->get(i) + 1);
		if(who == user && userlist->selected(i))
			return true;
	}
	return false;
}

void	channelwin::user_select(const int i) 
{
	userlist->select(i,i);
}

void	channelwin::set_op(int on)
{
	if(on)
	{
		mode_enable_all();
		topic_enable();
		key_enable();
		limit_enable();
	}
	else
	{
		mode_disable_all();
		key_disable();
		limit_disable();
	}
}

void	channelwin::mode_toggle(int, int)
{
}

void	channelwin::mode_enable_disable(int, int)
{
}

void	channelwin::mode_enable_all(void)
{
	mode_enable(CMODE_INVITE);
	mode_enable(CMODE_KEY);
	mode_enable(CMODE_LIMIT);
	mode_enable(CMODE_MODERATE);
	mode_enable(CMODE_NO_MSG);
	mode_enable(CMODE_PRIVATE);
	mode_enable(CMODE_SECRET);
	mode_enable(CMODE_TOPIC);
};
void	channelwin::mode_disable_all(void)
{
	mode_disable(CMODE_INVITE);
	mode_disable(CMODE_KEY);
	mode_disable(CMODE_LIMIT);
	mode_disable(CMODE_MODERATE);
	mode_disable(CMODE_NO_MSG);
	mode_disable(CMODE_PRIVATE);
	mode_disable(CMODE_SECRET);
	mode_disable(CMODE_TOPIC);
};

void	channelwin::insert(const cstring &s, const cstring &t) 
{
	output->insert(s, t);
}

void	channelwin::insert(const cstring &p, const cstring &s, 
                          const unsigned int l, const cstring &tagname)
{
	output->insert(p, s, l, tagname);
}

void	channelwin::info_insert(const cstring &s)
{
	info->insert(s);
}

void	channelwin::set_topicentry(const cstring &topic)
{
	topicentry->del();
	topicentry->insert(topic);

	title(cname + ": " + topic);
}

cstring	channelwin::get_topicentry(void) const
{
	return topicentry->get();
}

void	channelwin::set_keyentry(const cstring &key)
{
	keyentry->del();
	keyentry->insert(key);
}

cstring	channelwin::get_keyentry(void) const
{
	return keyentry->get();
}

void	channelwin::set_limitentry(const cstring &limit)
{
	limitentry->del();
	limitentry->insert(limit);
}

cstring	channelwin::get_limitentry(void) const
{
	return limitentry->get();
}

cstring	channelwin::get_user(void) const
{
	return userlist->get(userlist->selection());
}

void	channelwin::mode_enable(int mode) 
{ 
	mode_enable_disable(mode, true); 
}
void	channelwin::mode_disable(int mode) 
{ 
	mode_enable_disable(mode, false); 
}
void	channelwin::topic_enable(void) 
{
	topicentry->configure("-state normal");
}
void	channelwin::topic_disable(void) 
{
	topicentry->configure("-state disabled");
}
void	channelwin::limit_enable(void) 
{
	limitentry->configure("-state normal");
}
void	channelwin::limit_disable(void) 
{
	limitentry->configure("-state disabled");
}
void	channelwin::key_enable(void) 
{
	keyentry->configure("-state normal");
}
void	channelwin::key_disable(void) 
{
	keyentry->configure("-state disabled");
}

void	channelwin::clear(void)
{
	output->clear();
	info->clear();
}


/*
 * Event handlers/forwarders
 */

int	channelwin::handleInput(void)
{
	cstring	typed = in->get();

	if(typed.length() != 0)
	{
		// Commands are prefixed by a /. If it's not, it's a message
		if(typed[0] != '/')
			tk << CMDLINE << sessionid << " " << escape(cname) 
			 	      << " \"msg " << escape(cname) << " " 
				      << escape(typed) << "\"" << end;
		else	// skip / when passing to cmdhandler!
			tk << CMDLINE << sessionid << " " << escape(cname) 
			   << " \"" << escape(typed(1)) << "\"" << end;
		
		in->del();
	}
	return TCL_OK;
}

int	channelwin::StaticHandleInput(void *h, event *e)
{
	((channelwin *)h)->setevent(e);
	return ((channelwin *)h)->handleInput();
}
