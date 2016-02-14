/*
 * irctext.c - extended version of textwidget, suitable for displaying 
 *	       conversations
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
#include <stdlib.h>
#include <string.h>

#include <ctype.h>	// for isdigit()

#include <iostream.h>

#include "cstring.h"
#include "irctext.h"
#include "misc.h"

/*
 * 11-12-96:
 *           Added colortags. mIRC (and other clients) use ^Cx,y
 *           to set foreground/background color. Seems to work
 *
 */

char *irctext::colors[] = 
{ 	"white", 
	"black", 
//	"DarkBlue", 
	"RoyalBlue",
	"DarkGreen",
//	"red", 
	"red3",
	"brown", 
	"MediumPurple", 
	"Orange", 
	"yellow", 
	"LightGreen", 
	"cyan", 
	"LightCyan",
	"LightBlue", 
	"pink", 
//	"grey", 
	"DarkSlateGray",		// better visible
	"LightGrey"
};

irctext::irctext(widget *w, char *path, char *opt): text(w, path, opt)
{
	configure("-tabs { 80 right 85 left } -wrap word");

	scroll = true;

	fg = -1;
	bg = -1;

	disable();

	// configure tags
	// it's not good to hardcode this. esp. not the fonts!

	cstring	cfgbold = "-font 6x13bold";
	cstring	cfginv = "-foreground grey -background black";
	cstring	cfgundl = "-underline true";
	evaluate("tag configure attr_b " + cfgbold);
	evaluate("tag configure attr_i " + cfginv);
	evaluate("tag configure attr_u " + cfgundl);

	evaluate("tag configure nick_me -foreground red");
	evaluate("tag configure nick_other -foreground blue");

	evaluate("tag configure notice " + cfgbold);


	evaluate("tag configure uitlijn -lmargin2 85");

	cstring m = text::path();

	for(int i = 0; i < 16; i++)
	{
		char tagfg[255], tagbg[255];

		sprintf(tagfg, "tag configure fg%d -foreground %s",i,colors[i]);
		sprintf(tagbg, "tag configure bg%d -background %s",i,colors[i]);
		evaluate(tagfg);
		evaluate(tagbg);
	}
}

void	irctext::attrwrite(const int mode, const cstring &msg, 
                           const cstring &extratags)
{
	cstring	tags = extratags;
	// colors to use (possibly?)
	int	use_fg = fg, use_bg = bg;

	if(mode == 0 || msg.length() == 0)
	{
		if(msg.length() != 0)
			text::insert("end", msg, tags);
		return;
	}
	if(mode & MODE_BOLD)
		tags += " attr_b";
	if(mode & MODE_INVERSE)
		// swap colors
		if(mode & MODE_COLOR && use_fg != -1 && use_bg != -1)
		{
			int	t = use_fg;
			use_fg = use_bg;
			use_bg = t;
		}
		else
			tags += " attr_i";	// standard tag
	if(mode & MODE_UNDERLINE)
		tags += " attr_u";
	if(mode & MODE_COLOR)
	{
		if(use_fg != -1)
			tags << " fg" << use_fg;
		if(use_bg != -1)
			tags << " bg" << use_bg;
	}
	text::insert("end", msg, tags);
}

void	irctext::tinsert(int &mode, const cstring &msg, 
                         const cstring &extratags)
{
	int	idx = 0;
	int	tag_start = 0;
	int	newmode = 0;
	int	newfg=-1, newbg=-1;
	int	notbeeped = true;

	for(idx = 0; idx < msg.length(); idx++)
	{
		int	tag_end = idx - 1;	// is a tag ends, it ends here
		if(msg[idx] == '\x02')			// bold
			newmode = MODE_BOLD;
		else if(msg[idx] == '\x03')		// color
		// filter out colors
		{
			cstring	nfg, nbg;
			newfg=-1,newbg=-1;

			newmode = MODE_COLOR;

			int	idx1=0, idx2=0;

			for(idx1=idx+1; msg[idx1] && isdigit(msg[idx1]) 
			                && idx1 <= idx+2; idx1++)
				nfg += msg[idx1];

			if(msg[idx1] == ',') // read background color??
			{
				for(idx2=idx1+1; msg[idx2] && isdigit(msg[idx2])
			                        && idx2 <= idx1+2; idx2++)
					nbg += msg[idx2];
				idx = idx2-1;
			}
			else
				idx = idx1-1;

			// no colors specified? Turn them off...

			if(!((int)nfg||(int)nbg))
			{
				newfg = -1;
				newbg = -1;
			}
			else
			{
				if((int)nfg)
					newfg = atoi(nfg)%16;	// set new fg
				else
					newfg = fg;
				if((int)nbg)
					newbg = atoi(nbg)%16;	// set new bg
				else
					newbg = bg;
			}
		}
		else if(msg[idx] == '\x07')
		{
			if(notbeeped)
			{
				tk->beep();
				notbeeped = false;
			}
			continue;
		}
		else if(msg[idx] == '\x16')		// inverse
			newmode = MODE_INVERSE;
		else if(msg[idx] == '\x1f')		// underline
			newmode = MODE_UNDERLINE;
		else if(msg[idx] == '\x0f')		// everything off
			newmode = 0;
		else if(msg[idx] == '\n')
		{
			notbeeped = true;
			newmode = 0;
			newfg=-1;
			newbg=-1;
		}
		else
			continue;

		// If we got this far, attributes must have changed, so
		// write the previous text
		if(tag_end >= tag_start)	// is there anything?
			attrwrite(mode, msg(tag_start, tag_end), extratags);

		if(newmode == MODE_COLOR)
		{
			if(newfg == -1 && newbg == -1)
				mode = 0;
			else
				mode |= newmode;
		}
		else if(mode & newmode)
			// remove, ircII behaviour is to turn off all attrs!
			mode = 0;
		else
			// tag starts
			mode |= newmode;
		// mark beginning of next tag, skip control char, except '\n'
		if(msg[idx] == '\n')
			text::insert("end", "\n");
		// skip trailling controlchars. ircII ignores them!
		// but don't skip color-attribute
		else if(mode == 0)
			while(msg[idx+1] == '\x02' || msg[idx+1] == '\x0f' ||
			      msg[idx+1] == '\x16' || msg[idx+1] == '\x1f')
			    	idx++;
		// tag starts at next controlchar
		tag_start = idx + 1;
		fg = newfg;
		bg = newbg;
	}
	attrwrite(mode, msg(tag_start, idx-1), extratags);
}

void	irctext::insert(const cstring &msg, const cstring &tagname)
{
	int	mode = 0;

	enable();
	tinsert(mode, msg, tagname);
	disable();
	if(scroll)
        	evaluate("see end");
        tk->update();
}

void	irctext::insert(const cstring &prompt, const cstring &msg, 
		        const int promptlen, const cstring &tagname)
/*
 * Insert the text nicely formatted
 */
{
	int	mode = 0;			// attribute mode

	enable();

	tinsert(mode, "\t" + prompt + "\t", tagname);
	tinsert(mode, msg, tagname + " " + "uitlijn");
	disable();
	if(scroll)
		evaluate("see end");
	tk->update();
}

void	irctext::toggle_lock(void)
{
	scroll = !scroll;
}

void	irctext::clear(void)
{
	enable();
	del();
	disable();
}
