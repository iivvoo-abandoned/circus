#ifndef SESSION_H
#define SESSION_H

#include <iostream.h>

#include "handler.h"

#include "widgets.h"		// for statistics

class	session: public handler
{
public:
	session();
	virtual ~session();
	virtual void	prelude(void);
	virtual	int	main(void);
	void	run(void);

	toplevel	*root;

static	void	statistics(void);
static	void	exit(int = 0);

private:
};

#endif // SESSION_H
