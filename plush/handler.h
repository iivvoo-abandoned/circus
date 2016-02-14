#ifndef HANDLER_H
#define HANDLER_H

#include <tcl.h>

#define MAX_HANDLERS	1000			// very, very bad

#include "KitObject.h"
#include "event.h"

class	handler: public KitObject
{
public:
	handler();
	virtual ~handler();

	virtual	int	operator()();

	// should I add a bunch of widget-specific handlers here such as
	// keypress, etc?

	virtual void	setevent(event *);
	virtual	event	*dispatch(event *);

static	int	created;
static	int	deleted;

protected:
event	*_event;

private:
};

#endif // HANDLER_H
