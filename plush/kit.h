#ifndef KIT_H
#define KIT_H

#include <tcl.h>
#include <tk.h>
#include <tix.h>

#include <map.h>	// STL hashtable
#include <vector.h>	// STL vector

#include <sys/time.h>

#include "cstring.h"
#include "escape.h"	// some strings passed to eval need to be escaped
#include "binding.h"

#include "kitcmp.h"	// compare for map<>

class	handler;	// for bind
class	event;

/*
 * implement a usable tcl/tk kit
 * basically, it's a stripped-down version of tk's Tk_Main
 *
 * make stuff static - we need several instances but only one interpreter!
 *
 * values for kit::debug
 *
 * 1, 3, 5.. are reserved for future use
 */
 
#define	KIT_NOTRACE	0
#define	KIT_ERRTRACE	2
#define	KIT_TRACE	4

//
// Optional parameters for update() - perform update on idletasks (default)
// unless user really requests else (KIT_UPDATEALL may proces new events ->
// problems!)

#define	KIT_UPDATEIDLE	0
#define KIT_UPDATEALL	1

// We need a special class 'eval' to pass a iniquely-typed 'end of command'
// operator to the << 'eval' operator
class	evalop
{
public:
	evalop(int i) {_i = i;}
	~evalop() {}
	operator int() const
	{
		return _i;
	}
private:
	int	_i;
};

#define KIT_ENDCMD	1

const evalop	end(KIT_ENDCMD);

// kit::loop() should only be called ONCE by class session. Perhaps it 
// should be a protected member, and session should become a friend?

class	kit
{
public:
	enum	{	
			readable = TCL_READABLE, 
			writable = TCL_WRITABLE, 
			exception = TCL_EXCEPTION
		};

// operators, to make 'eval' more easy and abstract
	friend	const	kit	 &operator<<(const kit &, const cstring &);
	friend	const	kit	 &operator<<(const kit &, const char *);
	friend	const	kit	 &operator<<(const kit &, const int);
	friend	const	kit	 &operator<<(const kit &, const evalop &);

	kit();
	virtual ~kit();
	int	eval(char *) const;
static	void	freeProc(char *);
	char	*result(void) const;	// result of eval
	void	result(char *);		// set result
	char	*evaluate(char *) const;
	void	source(char *);
	char	*getvar(char *) const;
	void	quit(void);

	// debug
	int	trace(void) const;
	int	trace(int);

	void	update(int = KIT_UPDATEIDLE) const;

	void	beep(void) const;

// we instantiate kit as a class, not pointer to class. However to be compatible
// with hush, a -> pointer needs to be defined to make kit look like kit *

	kit	*operator->() { return this; }

	// bind - to install callbacks

	binding	*bind(handler *);
	binding	*bind(void *, int (*func)(void *, event *));
	binding	*bind(const char *, handler *);
	binding	*bind(const char *, void *, int (*func)(void *, event *));
	binding	*bind(int, handler *, int = kit::readable);

	// remove bindings - not implemented yet, except for fd's
	void	unbind(int);

	// pro's only
	void	loop() const;		// start main loop - don't touch
static	XEvent	*lastXevent(void) { return _lastX; }

// static stuff
	static	void	exit(int = 0);

static	int		created;
static	int		deleted;

// misc interp-related functions
	int		splitlist(char *, int *, char ***);
static	char		*alloc(int);
static	void		free(char *);
static	char		*realloc(char *, int);

private:
	// callback handlers
	
inline static	Tcl_CmdProc	dispatch_event;
inline static	Tcl_FileProc	dispatch_fdevent;
	
static	XEvent		*_lastX;
inline	static	Tk_GenericProc dispatchX;


static	map<int, binding *, less<int> >	fd_table;
static	cstring		evalbuf;
static	Tcl_Interp	*interp;
static	int		_quit;
static	int		debug;
static	int		eventcount;
};

#endif // KIT_H
