#ifndef MENUBUTTON_H
#define MENUBUTTON_H

#include "widget.h"

class	menubutton: public widget
{
public:
	menubutton(const char * = NULL, const char * = NULL);
	menubutton(widget *, const char * = NULL, const char * = NULL);
        virtual ~menubutton();
	void	text(const char *) const;
	void	menu(class menu *);

static	int	created = 0;
static	int	deleted = 0;

private:
};

#endif // MENUBUTTON_H
