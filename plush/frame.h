#ifndef FRAME_H
#define FRAME_H

#include "cstring.h"
#include "widget.h"

class	frame: public widget
{
public:
	frame(const char * = NULL, const char * = NULL, int = true);
	frame(widget *, const char * = NULL, const char * = NULL, int = true);
	
	virtual ~frame();

	void	pack(const char * = "") const;
static	int	created;
static	int	deleted;

private:

};

#endif // FRAME_H
