#ifndef TIXNOTEBOOX_H
#define TIXNOTEBOOX_H

#include "cstring.h"
#include "widget.h"
#include "tixNBFrame.h"

class	tixNoteBook: public widget
{
public:
	tixNoteBook(const char * = NULL, const char * = NULL);
	tixNoteBook(widget *, const char * = NULL, const char * = NULL);
	
	virtual ~tixNoteBook();

	tixNBFrame	*add(const char *, const char * = "");
	void	forget(const char *);
	void	remove(const char *);
	void	raise(const char *);

	void	pageconfigure(const char *, const char *);

static	int	created;
static	int	deleted;

private:
};

#endif // TIXNOTEBOOX_H
