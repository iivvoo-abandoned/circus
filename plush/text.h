#ifndef TEXT_H
#define TEXT_H

#include "cstring.h"
#include "widget.h"

class	text: public widget
{
public:
	text(const char * = NULL, const char * = NULL);
	text(widget *, const char * = NULL, const char * = NULL);
	
	virtual ~text();
	
	void	insert(const char *, const char *, const char * = "") const;
	void	del(const char * = "0.1", const char * = "end") const;

	const	char	*get(const char * = "0.1", 
	                     const char * = "end") const;

	void	enable(void) const;
	void	disable(void) const;

static	int	created;
static	int	deleted;

private:

};

#endif // TEXT_H
