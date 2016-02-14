#ifndef MESSAGE_H
#define MESSAGE_H

#include "cstring.h"
#include "widget.h"

class	message: public widget
{
public:
	message(const char * = NULL, const char * = NULL);
	message(widget *, const char * = NULL, const char * = NULL);
	
	virtual ~message();
	
	void	text(const char *) const;


static	int	created;
static	int	deleted;

private:
};

#endif // MESSAGE_H
