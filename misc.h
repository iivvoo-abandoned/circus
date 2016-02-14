#ifndef _MISC_H
#define _MISC_H

#include "cstring.h"

/*
 * comparefunction for dictionaries
 */
class mapcmp
{
public:
  	bool operator()(const char* s1, const char* s2) const
    	{
        	return strcmp(s1, s2) < 0;
    	}
};
int	split_nuh(const cstring &, cstring &, cstring &, cstring &);
cstring	escape(const cstring &);
cstring	unescape(const cstring &);
cstring	filter(const cstring &);
int     strcompare(const cstring *, const cstring *);
int	ischannel(const cstring &);
int	isnick(const cstring &);
int	isserver(const cstring &);

int	rematch(char *, char *);

#endif // _MISC_H
