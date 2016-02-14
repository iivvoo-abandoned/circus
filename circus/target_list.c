/*
 * target_list.c - maintains a list of targets (queries, channels)
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
#include "target_list.h"

/*
 * target_list
 */

target_list::target_list(): target_map()
{
}

target_list::~target_list()
{
	for(target_iterator iter = begin(); iter != end(); iter++)
		delete iter_value(iter);
}

targetif	*target_list::add(const cstring &which, targetif *newtarget)
{
	target_map::operator[](which) = newtarget;
	return newtarget;
}

int	target_list::del(const cstring &which)
{
	target_iterator i = find(which);

	if(i == end())
		return false;

	targetif	*t = iter_value(i);

	t->destroy();
	delete t;

	erase(i);
	return true;
}

targetif	*target_list::findtarget(const cstring &targ)
{
	target_iterator i = find(targ);

	if(i == end())
		return NULL;

	return iter_value(i);
}

targetif	*target_list::operator[](const cstring &which)
{
	return findtarget(which);
}

targetif	*target_list::operator[](const int idx)
{
	target_iterator iter;
	int	i = 0;
	
	for(iter = begin(); iter != end() && i < idx; 
	    iter++, i++)
		;	/* search .. */

	if(iter == end())
		return NULL;

	return iter_value(iter);
}

void	target_list::disable(void)
// disable all channels (and queries?)
{
	for(target_iterator iter = begin(); iter != end(); iter++)
		iter_value(iter)->disable();
}

void	target_list::clear(const cstring &targ)
{
	findtarget(targ)->clear();
}

void	target_list::active(const cstring &which, int mode, int doraise)
// make a channel (and it's window) 'active'
{
	findtarget(which)->current(mode, doraise);
}

void	target_list::nickchange(const cstring &oldnick, const cstring &newnick)
// check all channels, change oldnick to newnick if present
{
	for(target_iterator iter = begin(); iter != end(); iter++)
	{
		targetif	*t = iter_value(iter);

		t->nickchange(oldnick, newnick);
		t->change_user(oldnick, newnick);
	}
}

void	target_list::quituser(const cstring &who, const cstring &why)
// check all channels and remove 'who' if present
{
	cstring nick, user, host;

	split_nuh(who, nick, user, host);

	for(target_iterator iter = begin(); iter != end(); iter++)
	{
		targetif	*t = iter_value(iter);

		t->quituser(nick, why);
		t->del_user(nick);
	}
}

