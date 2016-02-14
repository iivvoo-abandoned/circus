#ifndef SCROLLBAR_H
#define SCROLLBAR_H

#include "cstring.h"
#include "widget.h"

class	scrollbar: public widget
{
public:
	scrollbar(const char * = NULL, const char * = NULL);
	scrollbar(widget *, const char * = NULL, const char * = NULL);
	
	virtual ~scrollbar();

	void	yview(const widget *) const;
	void	xview(const widget *) const;

static	int	created;
static	int	deleted;

private:

};

#endif // SCROLLBAR_H
