#ifndef TIXNBFRAME_H
#define TIXNBFRAME_H

#include "cstring.h"
#include "widget.h"

class	tixNBFrame: public widget
{
public:
	tixNBFrame(const char * = NULL, const char * = NULL);
	tixNBFrame(widget *, const char * = NULL, const char * = NULL);
	
	virtual ~tixNBFrame();

	void	setname(const char * = "");
	void	label(const char * = "");
static	int	created;
static	int	deleted;

private:
	cstring	_name;
};

#endif // TIXNBFRAME_H
