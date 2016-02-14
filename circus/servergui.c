/*
 * servergui.c - implements server-specific gui part in masterwindow
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
#include "servergui.h"

#include "misc.h"
#include "images.h"

#include "const.h"

extern	images  &img;

servergui::servergui(tixNoteBook *parent, const cstring &nm, int idx): 
           handler()
{
	this->name << "tab" << idx;
	this->parent = parent;

	addTab(parent);
	setlabel(nm);

	bindings();

	eventloop = false;
}

servergui::~servergui(void)
{
}

tixNBFrame	*servergui::addTab(tixNoteBook *parent)
{

	tabframe = parent->add(name);

	// Frames
	leftframe = new frame(tabframe, ".leftframe");
	outframe = new frame(leftframe, ".outframe");
	statusframe = new frame(leftframe, ".statusframe");
	targetframe = new frame(tabframe, ".targetframe");
	listframe = new frame(targetframe, ".listframe");

	out = new irctext(outframe, ".masterText", "-width 80 -height 12");
	outscroll = new scrollbar(outframe, ".outscroll");
	outscroll->yview(out);
	out->yscroll(outscroll);

	in = new plushHistEntry(leftframe, ".inEntry");

	targetlabel = new label(statusframe, ".targetLabel", "-width 8");
	targetlabel->text("Channel:");
	targetentry = new entry(statusframe, ".targetEntry", "-width 10");
	
	nicklabel = new label(statusframe, ".nickLabel");
	nicklabel->text("Nick:");
	nickentry = new entry(statusframe, ".nickEntry", "-width 10");
	
	usermodelabel = new label(statusframe, ".usermodeLabel");
	usermodelabel->text("mode: ");
	mode_w = new checkbutton(statusframe, ".mode_wCheck", "-padx 0 -pady 0 -highlightthickness 0 -borderwidth 1");
	mode_w->text("w");
	mode_i = new checkbutton(statusframe, ".mode_iCheck", "-padx 0 -pady 0 -highlightthickness 0 -borderwidth 1" );
	mode_i->text("i");
	mode_r = new checkbutton(statusframe, ".mode_rCheck", "-padx 0 -pady 0 -highlightthickness 0 -borderwidth 1 -state disabled");
	mode_r->text("r");
	targetslist = new listbox(listframe, ".targetslist", 
	                         "-width 10 -height 8 -font \"6x13\" "
				 "-exportselection false -selectmode extended");
	targetscroll = new scrollbar(listframe, ".targetscroll");
	targetscroll->yview(targetslist);
	targetslist->yscroll(targetscroll);
//	targetslist->singleselect();

	out->pack("-side left -fill both -expand true");
	outscroll->pack("-side right -fill y -expand false");
	outframe->pack("-fill both -expand true");

	targetlabel->pack("-side left");
	targetentry->pack("-side left");
	nicklabel->pack("-side left");
	nickentry->pack("-side left");
	mode_r->pack("-side right");
	mode_i->pack("-side right");
	mode_w->pack("-side right");

	usermodelabel->pack("-side right");

	statusframe->pack("-fill x -expand false");
	in->pack("-side bottom -fill x -expand false");


	targetslist->pack("-side left -fill y -expand true");
	targetscroll->pack("-side right -fill y -expand false");
	listframe->pack("-side top -fill y -expand true");

	targetframe->pack("-side right -fill both -expand false");
	leftframe->pack("-side left -fill both -expand true");

	return tabframe;
}

void	servergui::setlabel(const cstring &msg)
{
	tabframe->label(msg);
}

void	servergui::insert(const cstring &m, const cstring &t)
{
	out->insert(m, t);
}

void	servergui::insert(const cstring &prompt, const cstring &msg,
                          const int len, const cstring &tagname)
{
	out->insert(prompt, msg, len, tagname);
}

void	servergui::setnick(const cstring &n)
{
	cstring	target = targetentry->get();

	nickentry->del();
	nickentry->insert(n);
}

cstring	servergui::getnick(void) const
{
	return nickentry->get();
}

cstring servergui::gettarget(void) const
{
	return targetentry->get();
}

cstring	servergui::gettargetlist(const int index) const
{
	if(index == -1)
		return targetslist->get(targetslist->selection());
	else
		return targetslist->get(index);
}

cstring	servergui::getinput(void) const
{
	return in->get();
}

void	servergui::delinput(void)
{
	in->del();
}

void	servergui::refocus(void)
{
	in->focus();
}

void	servergui::bindings()
{
	in->bind("<Return>", this, "input_typed");

	targetslist->bind("<Button-1>", this, "targetlist_changed");
	targetslist->bind("<Button-3>", this, "target_menu");
	targetentry->bind(this, "target_changed");
	nickentry->bind(this, "nick_changed");
//	lbl_w->bind(this, "w_mode_changed");
//	lbl_i->bind(this, "i_mode_changed");
}

void	servergui::settarget(const cstring &target)
{
	cstring	nick = nickentry->get();

	targetentry->del();
	targetentry->insert(target);
	if(target == "")
	{
		targetlabel->text("Channel: ");
		return;
	}	
	if(ischannel(target))
		targetlabel->text("Channel: ");
	else
		targetlabel->text("Query: ");

	// Select the target in the listbox
	targetslist->unselect();
	int	size = targetslist->size();

	for(int i = 0; i < size; i++)
	{
		cstring	elt = targetslist->get(i);

		if(elt.compare(target))
		{
			targetslist->select(i, i);
			return;
		}
	}
}

void	servergui::add_target(const cstring &target)
{
	int size = targetslist->size(), ch_total = 0;

	if(ischannel(target))
	{
		targetslist->insert(0, target);
	}
	else
	{
        	for(int i = 0; i < size; i++)
        	{
               		cstring  elt = targetslist->get(i);
               		if(ischannel(elt))
				ch_total++;
			else
				break;
       		}
		targetslist->insert(ch_total, target);
	}
}

void	servergui::ren_target(const cstring &oldnick, const cstring &newnick)
{
	int size = targetslist->size();

	for(int i = 0; i < size; i++)
	{
       		cstring  elt = targetslist->get(i);

		if(elt.compare(oldnick))
		{
			targetslist->del(i);
			targetslist->insert(i, newnick);
			return;
		}
	}
	// if we get here something must have gone wrong. But my code
	// is perfect so we will never reach this point >:)
}

void	servergui::del_target(const cstring &target)
{
        int size = targetslist->size();
 
        for(int i = 0; i < size; i++)
        {
               	cstring  elt = targetslist->get(i);
               	if(elt.compare(target))
                       	targetslist->del(i);
       	}
}

void	servergui::mode_toggle(const int mode, const int on)
{
#if 0
	switch(mode)
	{
	case MODE_WALLOP:
                if(on == -1)
                        mode_w->toggle();
                else if(on)
                        mode_w->select();
                else
                        mode_w->deselect();
                break;
        case MODE_INVISIBLE:
                if(on == -1)
                        mode_i->toggle();
                else if(on)
                        mode_i->select();
                else
                        mode_i->deselect();
                break;
#if 0 // removed, not present in server > 2.9.1
        case MODE_SERV_NOT:
                if(on == -1)
                        mode_s->toggle();
                else if(on)
                        mode_s->select();
                else
                        mode_s->deselect();
                break;
	}
#endif
        case MODE_RESTRICTED:
                if(on == -1)
                        mode_r->toggle();
                else if(on)
                        mode_r->select();
                else
                        mode_r->deselect();
                break;
	}
#endif
}

void	servergui::mode_enable_disable(const int mode, const int on)
{
#if 0
	switch(mode)
	{
	case MODE_WALLOP:
		if(on)
			mode_w->configure("-state normal");
		else
			mode_w->configure("-state disabled");
		break;
	case MODE_INVISIBLE:
		if(on)
                        mode_i->configure("-state normal");
                else
                        mode_i->configure("-state disabled");
		break;
#if 0 // removed, not present in server > 2.9.1
	case MODE_SERV_NOT:
		if(on)
                        mode_s->configure("-state normal");
                else
                        mode_s->configure("-state disabled");
		break;
#endif
	case MODE_RESTRICTED:
		if(on)
                        mode_r->configure("-state normal");
                else
                        mode_r->configure("-state disabled");
		break;
	}
#endif
}

void	servergui::mode_enable(const int mode)
{
	mode_enable_disable(mode, true);
}

void	servergui::mode_disable(const int mode)
{
	mode_enable_disable(mode, false);
}

void	servergui::popup_menu(int x, int y, int channels, int nicks)
/*
 * These menu's need alot of improvement. How about checkbuttons for modes?
 */
{
	int	size = targetslist->size();

	// check this code... /////////////////////
        tk->eval("if {[winfo exists .main]} {destroy .main}");
        if (mainmenu != NULL)
                delete mainmenu;
 
        mainmenu = new menu(".main", "-tearoff false");
	ctcpmenu = new menu(mainmenu, ".ctcp", "-tearoff false");
	mainmenu->bind(this, "MainManu");
        ctcpmenu->bind(this, "CTCPMenu");


	mainmenu->separator();
	mainmenu->entry("General Options", "", "-state disabled");
	mainmenu->separator();
	mainmenu->entry("Leave/Close");
	mainmenu->entry("Who/Whois");

	mainmenu->cascade("CTCP", ctcpmenu);
	ctcpmenu->separator();
	ctcpmenu->entry("CTCP", "", "-state disabled");
	ctcpmenu->separator();
       	ctcpmenu->entry("Version");
       	ctcpmenu->entry("Clientinfo");
       	ctcpmenu->entry("Userinfo");
        ctcpmenu->entry("Finger");
        ctcpmenu->entry("Time");
        ctcpmenu->entry("Ping");

	if(channels)
	{
		setmodemenu = new menu(mainmenu, ".setmode", "-tearoff false");
		delmodemenu = new menu(mainmenu, ".delmode", "-tearoff false");
		setmodemenu->bind(this, "SetModeMenu");
		delmodemenu->bind(this, "DelModeMenu");

		mainmenu->separator();
		mainmenu->entry("Channel Options", "", "-state disabled");
		mainmenu->separator();

		mainmenu->entry("Bans");
		mainmenu->cascade("Set Mode", setmodemenu);
		setmodemenu->separator();
		setmodemenu->entry("Set Mode", "", "-state disabled");
		setmodemenu->separator();
		setmodemenu->entry("Invite-only"); 
		setmodemenu->entry("Moderated");
		setmodemenu->entry("No-messages");
		setmodemenu->entry("Private");
		setmodemenu->entry("Secret");
		setmodemenu->entry("Topic");
		mainmenu->cascade("Remove Mode", delmodemenu);
		delmodemenu->separator();
		delmodemenu->entry("Remove Mode", "", "-state disabled");
		delmodemenu->separator();
		delmodemenu->entry("Invite-only");
		delmodemenu->entry("Key");
		delmodemenu->entry("Limit");
		delmodemenu->entry("Moderated");
		delmodemenu->entry("No-messages");
		delmodemenu->entry("Private");
		delmodemenu->entry("Secret");
		delmodemenu->entry("Topic");
	}

	if(nicks)
	{
		invitemenu = new menu(mainmenu, ".invite", "-tearoff false");
		invitemenu->bind(this, "InviteMenu");

		mainmenu->separator();
		mainmenu->entry("Nick Options", "", "-state disabled");
		mainmenu->separator();

		mainmenu->cascade("Invite", invitemenu);
		for(int i = 0; i < size; i++)
		{
			cstring	which = targetslist->get(i);
			if(ischannel(which))
				invitemenu->entry(which);
		}
		
	}
        mainmenu->post(x,y);
        mainmenu->grab();
}
