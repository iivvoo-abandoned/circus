/*
 * misc.c - implements misc routines
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
#include <string.h>		// for strchr
#include <ctype.h>		// for isalpha, isalnum, etc.
#include "cstring.h"
#include "misc.h"
#include "fnmatch.h"

int	split_nuh(const cstring &who, cstring &nick, cstring &login, 
                  cstring &host)
{
	int	nickend = who.find('!');
	int	loginend = who.find('@');

	if(nickend != -1 && loginend > nickend)
	{
		nick = who(0, nickend - 1);
		login = who(nickend + 1, loginend - 1);
		host = who(loginend+1);
		return 1;		// true;
	}
	// Not a nick!user@host but probably a server
	else
	{
		cout << "WARNING: Could not split " << who << " - assuming "
		     << "server!! " << endl;
		nick = who;
		login = who;
		host = who;
		return 1; //true;		// should be false, fix later
	}
//	return false;
}

cstring	unescape(const cstring &src)
{
	cstring	str;
	int	i = 0, j = 0;

	while(i < src.length())
	{
		if(src[i] == '\\')
			i++;		// simply skip 
		if(i < src.length())	// and copy next char
			str[j++] = src[i++];
	}
	return str;
}

cstring	filter(const cstring &src)
// filter out control characters.. hi-ascii is still fine
{
	cstring	str;
	
	for(int i = 0; i < src.length(); i++)
		if(src[i] >= ' ')
			str += src[i];
	return str;
}

int     strcompare(const cstring *a, const cstring *b)
{
        return (*a == *b);
}

int	ischannel(const cstring &chan)
// Check if 's' is a valid channelname, i.e. start with a ' ' and does
// not contain a space, a ',' or ctrl-g
{
	char	*s = chan;

	if(chan.length() == 0)
		return 0; // false;
	if(!(s[0] == '#' || s[0] == '&' || s[0] == '+'))
		return 0; // false;
	if(strchr(s, '\r') || strchr(s, '\n') || strchr(s, '\x07') ||
	   strchr(s, ' ') || strchr(s, ','))
	   	return 0; // false;
	return 1; // true;
}

int	isnick(const cstring &s)
// Check is 's' is a valid nick, i.e.
// nick ::= <letter> { <letter> | <number> | <special> }
// special ::= '-' | '[' | ']' | '\' | '`' | '^' | '{' | '}'
//
// that's what rfc1459 says. But a nick can also contain _ (and start..)
// rfc1459 sucks
{
	if(s.length() == 0)
		return 0; // false;
	if(!(isalpha(s[0]) || strchr("_^[]{}|\\`", s[0])))
		return 0; // false;
	for(int i = 1; i < s.length(); i++)
		if(!isalnum(s[i]) &&
		   !strchr("-[]\\`^{}_|", s[i]))
		   	return 0; // false;
	return 1; // true;
}

int	isserver(const cstring &s)
// Check if 's' is a server. Keep it simple, just check if it contains
// a '!' and/or a '@' (which should usually not be the case). 
{
	return 	s.find("!@") == -1;
}

int	rematch(char *a, char *re)
// return true if a matches re, false otherwise
{
	return fnmatch(re, a, FNM_CASEFOLD) == 0;
}
