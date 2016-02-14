#ifndef LISTBOX_H
#define LISTBOX_H

#include "cstring.h"
#include "widget.h"

#define LISTBOX_BEGIN	1
#define LISTBOX_END	-1

class	listbox: public widget
{
public:
	listbox(const char * = NULL, const char * = NULL);
	listbox(widget *, const char * = NULL, const char * = NULL);
	
	virtual ~listbox();
	
	void	insert(const char *) const;
	void	insert(const int, const char *) const;
	void	del(void) const;
	void	del(const int) const;
	void	del(const int, const int) const;
	void	select(void) const;
	void	select(const int) const;
	void	select(const int, const int) const;

	const	int	selection(void) const;

	void	unselect(void) const;
	void	unselect(const int) const;
	void	unselect(const int, const int) const;

	const	int	selected(const int) const;
	const	int	size(void) const;
	const	char	*get(const int) const;

static	int	created;
static	int	deleted;

private:

};

#endif // LISTBOX_H
