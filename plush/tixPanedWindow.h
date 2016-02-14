#ifndef TIXPANEDWINDOW_H
#define TIXPANEDWINDOW_H

#include "cstring.h"
#include "widget.h"
#include "frame.h"

class	tixPanedWindow: public widget
{
public:
	tixPanedWindow(const char * = NULL, const char * = NULL);
	tixPanedWindow(widget *, const char * = NULL, const char * = NULL);
	
	virtual ~tixPanedWindow();

	frame	*add(const char *, const char * = "");
	void	forget(const char *);
	void	remove(const char *);

static	int	created;
static	int	deleted;

private:
};

#endif // TIXPANEDWINDOW_H
