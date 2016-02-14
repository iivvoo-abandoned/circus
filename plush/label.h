#ifndef LABEL_H
#define LABEL_H

#include "cstring.h"
#include "widget.h"

class	label: public widget
{
public:
	label(const char * = NULL, const char * = NULL, int = true);
	label(widget *, const char * = NULL, const char * = NULL, int = true);
	
	virtual ~label();
	
	void	text(const char *) const;

static	int	created;
static	int	deleted;

private:

};

#endif // LABEL_H
