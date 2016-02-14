#ifndef ENTRY_H
#define ENTRY_H

#include "cstring.h"
#include "widget.h"

#define	ENTRY_BEGIN	0
#define	ENTRY_END	-1

class	entry: public widget
{
public:
	entry(const char * = NULL, const char * = NULL);
	entry(widget *, const char * = NULL, const char * = NULL);
	
	virtual ~entry();
	
	void	insert(const char *, const int = ENTRY_BEGIN) const;
	void	del(const int = ENTRY_BEGIN, const int = ENTRY_END) const;

	const	char	*get(void) const;

static	int	created;
static	int	deleted;

private:

};

#endif // ENTRY_H
