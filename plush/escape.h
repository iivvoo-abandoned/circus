#ifndef ESCAPE_H
#define ESCAPE_H

#include <string.h>

#include "cstring.h"

#define	esc(x)	escape(x)

inline cstring	escape(const char *src)
{
	cstring 	str = "";

	while(*src)
	{
		if(strchr("\\\"[]${}", *src))
			str += '\\';
		str += *src;
		src++;
	}
	return str;
}

#endif // ESCAPE_H
