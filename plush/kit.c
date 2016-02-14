#include <iostream.h>

#include "kit.h"
#include "toplevel.h"

map<int, binding *, less<int> >	kit::fd_table;
Tcl_Interp	*kit::interp = NULL;
XEvent		*kit::_lastX = NULL;
cstring		kit::evalbuf = "";
int		kit::_quit = false;
int		kit::eventcount = 0;
int		kit::debug = KIT_NOTRACE;
int		kit::created = 0;
int		kit::deleted = 0;

/*
 * TODO:
 *
 * 	Proper unbind
 *	Bindings aren't stored anymore (except for fd bindings). Make
 * 	sure their garbage is collected
 *	fd_table is 'NFDBITS' big, which is incorrect! it should be
 *	2^NFDBITS big, which is way too big for an array. We need a different
 *	way to store fd's :\
 */

/*
 * Don't use USE_OBJ, it doesn't work. I don't know why, I'll have a look
 * at it later..
 */

kit::kit()
{
	created++;

	if(interp != NULL)	// already initialized?
		return;

	interp = Tcl_CreateInterp();
	if (Tcl_Init(interp) == TCL_ERROR)
	{
		cerr << "Tcl_Init(interp) failed: " << interp->result << endl;
		exit(1);
	}
	if (Tk_Init(interp) == TCL_ERROR)
	{
		cerr << "Tk_Init(interp) failed: " << interp->result << endl;
		exit(1);
	}
	if (Tix_Init(interp) == TCL_ERROR) 
	{
		cerr << "Tix_Init(interp) failed: " << interp->result << endl;
		exit(1);
	}
	Tcl_StaticPackage(interp, "Tk", Tk_Init, Tk_SafeInit);

	/*
	 * We need more X event information that tk can provide, so install
	 * a handler for *each* event, to store a pointer to the Xevent
	 * structure, which has the information we need 
	 */
	Tk_CreateGenericHandler(dispatchX, NULL);
}

kit::~kit()
{
	deleted++;
	if(deleted == created)	// last interpreter?
	{
		for(map<int, binding *, less<int> >::iterator i = fd_table.begin(); i != fd_table.end(); i++)
			unbind((*i).first);
		Tcl_DeleteInterp(interp);
	}
}

int	kit::eval(char *cmd) const
// eval a tcl-command
{
	int	result;

	if(trace() >= KIT_TRACE)
		cout << "kit::eval \"" << cmd << "\"" << endl;
	result = Tcl_Eval(interp, cmd);
	if(result == TCL_ERROR && trace() >= KIT_ERRTRACE)
	{
		cout << "TCL_ERROR: " << interp->result << endl;
	}
	return result;
}

char	*kit::result(void) const
{
	return interp->result;
}

// static
void	kit::freeProc(char *blockPtr)
{
	free(blockPtr);
}

void	kit::result(char *res)
{
	char	*result_cpy = (char *) malloc(sizeof(char) * strlen(res) + 1);
	strcpy(result_cpy, res);

	Tcl_SetResult(interp, res, kit::freeProc);
}

char 	*kit::evaluate(char *cmd) const
// eval a tcl-command
{
	int	result;

	if(trace() >= KIT_TRACE)
		cout << "kit::eval \"" << cmd << "\"" << endl;
	result = Tcl_Eval(interp, cmd);
	if(result == TCL_ERROR && trace() >= KIT_ERRTRACE)
	{
		cout << "TCL_ERROR: " << interp->result << endl;
	}
	return interp->result;
}

void	kit::source(char *file)
{
	cstring	cmd;

	cmd << "source " << file;
	eval(cmd);
}

char	*kit::getvar(char *name) const
{
	return Tcl_GetVar(interp, name, 0);
}

void	kit::quit(void)
{
	_quit = true;
}


/*
 * Static member function, used for all callback. Use argv[0] (the command 
 * invoked) to determine which handler belongs to this event, and then
 * call it.
 */

int	kit::dispatch_event(ClientData clientData, Tcl_Interp *, int argc, 
                            char *argv[])
{
	event e;

	for(int i = 0; i < argc; i++)
		cout << "argv[" << i <<"] = " << argv[i] << endl;
	binding	*b = (binding *) clientData;

	e.setarg(argc, argv);
	if(b->type == binding::context)
	{
		b->f(b->c, &e);
	}
	else
	{

		b->h->dispatch(&e);
	}
	return TCL_OK;
}

void	kit::dispatch_fdevent(ClientData clientData, int mask)
{
	binding	*b;
	event	e;
	

	b = fd_table[(int) clientData];
	
	// build a 'pseudo' argv list

	char	*argv[2];
	argv[0] = (char *) b->cmd;
	argv[1] = NULL;

	e.setarg(1, argv);
	e.setfd(b->fd, mask);

	b->h->dispatch(&e);
}

binding	*kit::bind(handler *h)
// create anonymous binding
{
	binding	*b = new binding();

	b->h = h;
	b->cmd << "anonevent" << eventcount++;

	Tcl_CreateCommand(interp, b->cmd, dispatch_event, 
	                  (ClientData) b, NULL);
	return b;
}

binding	*kit::bind(void *h, int (*func)(void *, event *))
// create anonymous binding
{
	binding	*b = new binding();

	b->type = binding::context;
	b->h = NULL;
	b->c = h;
	b->f = func;
	b->cmd << "anonevent" << eventcount++;

	Tcl_CreateCommand(interp, b->cmd, dispatch_event, 
	                  (ClientData) b, NULL);
	return b;
}

binding	*kit::bind(const char *command, handler *h)
{
	binding	*b = new binding();

	b->h = h;
	b->cmd = command;

	Tcl_CreateCommand(interp, b->cmd, dispatch_event, 
	                  (ClientData) b, NULL);
	return b;
}

binding	*kit::bind(const char *command, void *h, int (*func)(void *, event *))
{
	binding	*b = new binding();

	b->type = binding::context;
	b->h = NULL;
	b->c = h;
	b->f = func;
	b->cmd = command;

	Tcl_CreateCommand(interp, b->cmd, dispatch_event, (ClientData) b, NULL);
	return b;
}

binding	*kit::bind(int fd, handler *h, int mask)
{
	binding	*b;

	if(fd_table.find(fd) != fd_table.end())
	// change binding (i.e. change handler or mask)
	{
		Tcl_DeleteFileHandler(fd);
		b = fd_table[fd];
		b->h = h;
		b->mask = mask;
		Tcl_CreateFileHandler(fd, mask, dispatch_fdevent, 
		                      (ClientData) fd);
		return b;

	}

	// New binding...

	b = new binding();

	b->h = h;
	b->fd = fd;
	b->mask = mask;
	b->cmd << "fdevent" << fd;

	Tcl_CreateFileHandler(fd, mask, dispatch_fdevent, (ClientData) fd);
	fd_table[fd] = b;

	return b;
}

void	kit::unbind(int fd)
/*
 * unbind fd, i.e. detach handler from fd (do not close fd!)
 */
{

	if(fd_table.find(fd) == fd_table.end())
		return;	

	binding *b = fd_table[fd];
	fd_table.erase(fd);
	Tcl_DeleteCommand(interp, b->cmd);
	Tcl_DeleteFileHandler(fd);
	delete b;
}

// friend operators

const	kit	&operator<<(const kit &k, const cstring &cmd)
{
	k.evalbuf += cmd;
	return k;
}

const	kit	&operator<<(const kit &k, const char *cmd)
{
	k.evalbuf += cmd;
	return k;
}

const	kit	&operator<<(const kit &k, const int cmd)
{
	k.evalbuf << cmd;
	return k;
}

const	kit	&operator<<(const kit &k, const evalop &cmd)
//
// Eval might block (in case of wait), in which case we didn't get the
// chance to empty evalbuf. So copy evalbuf, clear it and eval the copy
{
	cstring	evalcopy = k.evalbuf;

	k.evalbuf = "";

	if(cmd == KIT_ENDCMD)
		k.eval(evalcopy);
	return k;
}

void	kit::beep(void) const
{
	eval("bell");
}

void	kit::loop() const
// Adjusted version of Tk_MainLoop()
{
	while((Tk_GetNumMainWindows() > 0) && !_quit)
		Tcl_DoOneEvent(0);
}

int	kit::trace(void) const
{
	return debug;
}

int	kit::trace(int level)
// if level == -1, simply return current level. Else set new tracelevel
{
	if(level >= 0)
		debug = level;
	return debug;
}

void	kit::update(int which) const
{
	if(which == KIT_UPDATEALL)
		eval("update");
	else // KIT_UPDATEIDLE, usefull as display update
		eval("update idletasks");
}

// static stuff
void	kit::exit(int code)
{
	Tcl_Exit(code);
}

int 	kit::dispatchX(ClientData, XEvent *eventPtr)
{
	_lastX = eventPtr;
	return TCL_OK;
}

// misc tcl interp-related stuff

int	kit::splitlist(char *str, int *argc, char ***argv)
{
	return Tcl_SplitList(interp, str, argc, argv);
}

char	*kit::alloc(int size)
{
	return Tcl_Alloc(size);
}

void	kit::free(char *ptr)
{
	Tcl_Free(ptr);
}

char	*kit::realloc(char *ptr, int size)
{
	return Tcl_Realloc(ptr, size);
}
