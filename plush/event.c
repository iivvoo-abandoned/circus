#include "event.h"

int	event::created = 0;
int	event::deleted = 0;

#if 0
interface event  {

int argc() const;               /// no. of arguments
char** argv(int i=0) const;     /// the (Tcl) string arguments
char* arg(int i=0) const;       /// to fetch i th argument

int type() const;               /// X event type
char* name() const;             /// type as string

int x();
int y();

int button(int i = 0);          /// ButtonPress
int buttonup(int i = 0);        /// ButtonRelease
int motion();                   /// MotionNotify

int keyevent();                 /// KeyPress or KeyRelease
int buttonevent(int i = 0);     /// ButtonPress or ButtonRelease

int keycode();

void trace();                   /// prints event information

void* rawevent();                 /// returns raw X event
};
#endif

event::event(): KitObject()
{
	created++;
}

event::~event()
{
	deleted++;
}

int	event::argc(void) const
{
	return _argc;
}

char	*event::arg(const int idx) const
{
	if(idx >= _argc)
	{
		cerr << "index out of bounds in event::arg(idx)" << endl;
		exit(0);
	}
	return _argv[idx];
}

char	**event::argv(void) const
{
	return _argv;
}

char	*event::name(void) const
{
	// is this correct?

	return (_argc > 0)? _argv[0]: "";
}

int	event::mask(void) const
{
	return _mask;
}

int	event::fd(void) const
{
	return _fd;
}

int	event::x(void) const
{
	if(!kit::lastXevent())
		return 0;		// what else?

	// code stolen from hush, mostly
	
	XEvent *xev = kit::lastXevent();
	
	switch(xev->type){
		case ButtonPress:
		case ButtonRelease:
			return xev->xbutton.x;
		case MotionNotify:
			return xev->xmotion.x;
		case KeyPress:
		case KeyRelease:
			return xev->xkey.x;
		default:
			return 0;		// what else?
	}
	return 0;
}

int	event::y(void) const
{
	if(!kit::lastXevent())
		return 0;		// what else?

	// code stolen from hush, mostly
	
	XEvent *xev = kit::lastXevent();
	
	switch(xev->type){
		case ButtonPress:
		case ButtonRelease:
			return xev->xbutton.y;
		case MotionNotify:
			return xev->xmotion.y;
		case KeyPress:
		case KeyRelease:
			return xev->xkey.y;
		default:
			return 0;		// what else?
	}
	return 0;
}

int	event::x_root(void) const
{
	if(!kit::lastXevent())
		return 0;		// what else?

	// code stolen from hush, mostly
	
	XEvent *xev = kit::lastXevent();
	
	switch(xev->type){
		case ButtonPress:
		case ButtonRelease:
			return xev->xbutton.x_root;
		case MotionNotify:
			return xev->xmotion.x_root;
		case KeyPress:
		case KeyRelease:
			return xev->xkey.x_root;
		default:
			return 0;		// what else?
	}
	return 0;
}
	
int	event::y_root(void) const
{
	if(!kit::lastXevent())
		return 0;		// what else?

	// code stolen from hush, mostly
	
	XEvent *xev = kit::lastXevent();
	
	switch(xev->type){
		case ButtonPress:
		case ButtonRelease:
			return xev->xbutton.y_root;
		case MotionNotify:
			return xev->xmotion.y_root;
		case KeyPress:
		case KeyRelease:
			return xev->xkey.y_root;
		default:
			return 0;		// what else?
	}
	return 0;
}


void	event::setarg(int argc, char **argv)
{
	_argc = argc;
	_argv = argv;
}

void	event::setfd(const int fd, const int mask)
{
	_fd = fd;
	_mask = mask;
}
