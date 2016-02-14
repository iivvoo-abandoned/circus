/*
 * What happens when an event is bound to a handler?
 *
 * It should look something like:
 *
 * button *b; b->bind(b); (button will handle it's own events)
 *
 * - where should the bind methods be implemented?
 *   + in class button? NO
 *   + in class widget? MAYBE
 *   + in class handler? MAYBE
 *   + in class KitObject? MAYBE
 *
 * What exactly happens? A new command is created (handlerXX), and it's
 * bound to the specific event, widget, etc. Whenever the event occurs,
 * handlerXX is called, which is handled by widget/handler/KitObject
 * (see above). It then finds the handler associated with the event,
 * builds an 'event' object, passes this to the handler, and calls
 * the (possibly overloaded) operator()().
 *
 * One problem: Tcl procedures such as Tcl_CreateCommand and 
 * Tcl_CreateObjCommand take a functionpointer as parameter - we cannot
 * pass methods! This means these events have to be handled by a static
 * member which determines to which handler the event should be dispatched.
 *
 * I will also need a hashtable to efficiently map commandnames (such as
 * handlerXX) to their handlers.
 *
 * Actually, an array may be sufficient (though fixed-size is a bad idea)
 * - the word 'handler' is constant, and XX is a number 0..MAXHANDLER
 * (as I said, fixed size is a bad idea). But it will give a perfect
 * hash! :)
 *
 * At this moment, I think class handler is a good place to implement all
 * this. Hope I'm not wrong...
 */

/*
 * dispatch()
 * unbind
 */

#include <iostream.h>
#include <stdlib.h>		// for atoi()
 
#include "handler.h"

int	handler::created = 0; 
int	handler::deleted = 0; 

handler::handler(): KitObject()
{
	created++;
}

handler::~handler()
{
	deleted++;
}

int	handler::operator()()
{
	return TCL_OK;
}

void	handler::setevent(event *_e)
{
	_event = _e;
}

event	*handler::dispatch(event *_e)
{
	_event = _e;
	operator()();
	return _event;
}
