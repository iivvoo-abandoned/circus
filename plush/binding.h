#ifndef BINDING_H
#define BINDING_H

#include "cstring.h"

/*
 * The class binding is used to store binding-specific data, such
 * as handler, fd, mask, etc
 */

class handler;
class event;

class binding
{
public:
	binding();
	virtual ~binding();

	enum	{ context, other } type;
	handler	*h;
	void	*c;
	int	fd;
	int	mask;
	cstring	cmd;

	int	(*f)(void *, event *);	// func
static	int	created;
static	int	deleted;

private:

};

#endif // BINDING_H

