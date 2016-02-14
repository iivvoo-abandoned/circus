#ifndef _MASTERWINDOW_H
#define _MASTERWINDOW_H

#include <plush.h>

#include "cstring.h"

#include "irctext.h"

#define BUTTON_ALL	0x0001
#define BUTTON_CHAN	0x0002
#define BUTTON_SOME	0x0004


class	masterwindow: public frame
{
public:
	masterwindow(widget *, char  *, char * = "");
	virtual ~masterwindow();
	void	bind(handler *);

	void	buttons_enable_disable(const int, const int);
	void	buttons_enable(const int);
	void	buttons_disable(const int);

	tixNoteBook	*serverWidget() const
	{
		return servers;
	}

protected:
	plushButton	*leavebutton, *joinbutton, *whoisbutton, *querybutton,
			*kickbutton, *banbutton, *bankickbutton, 
			*banlistbutton, *opbutton, *voicebutton, 
			*invitebutton, *awaybutton, *helpbutton, *quitbutton;
	menu		*filemenu, *toolsmenu, *dccmenu, *helpmenu;
private:
	tixNoteBook	*servers;

	void	buildwindow(widget *);
	void	bindings();

	// menu related stuff
	
	frame		*createmenubar(widget *parent);
	frame		*menuframe;
	menubutton	*file, *tools, *dcc, *help;

	// toolbar related stuff

	frame		*buttonframe;
};

#endif // _MASTERWINDOW_H
