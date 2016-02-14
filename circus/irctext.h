#ifndef _IRCTEXT_H
#define _IRCTEXT_H

#include <plush.h>

#include "cstring.h"

#define	MODE_BOLD	0x0001
#define	MODE_INVERSE	0x0002
#define MODE_UNDERLINE	0x0004
#define MODE_COLOR	0x0008

class	irctext: public text
{
public:
	irctext(widget *w, char *path, char *opt = "");
	~irctext() {};
	void	insert(const cstring &, const cstring &, const int, 
	               const cstring & ="");
	void	insert(const cstring &, const cstring & = "");

	void	toggle_lock(void);
	void	clear(void);
private:
	void	attrwrite(const int, const cstring &, const cstring & = "");
	void	tinsert(int &, const cstring &, const cstring & = "");

static	char	*colors[];
	int	fg, bg;
	int	scroll;
};
#endif // _IRCTEXT_H
