#ifndef WIDGET_H
#define WIDGET_H

#include "cstring.h"

#include "handler.h"

/*
 * class widget provides a uniform interface to widgets such as buttons,
 * listboxes, etc. It doesn't implement any actuall widgets
 *
 * It's derived from handler, so any widget can act as it's own handler
 */

/*
 * To implement:
 *
 * operator<<
 *
 */

class 	widget: public handler
{
public:
	widget(const char * = NULL, const char * = NULL);
	widget(widget *, const char * = NULL, const char * = NULL);
	virtual ~widget();

	// virtual?
	const	char	*cget(const char *) const;
	void	configure(const char *) const;
	void	destroy(void) const;
	void	pack(const char * = "") const;
	// hush also supplies a pack(widget *, char *)...

	const	char	*path(void) const;
	const	char	*type(void) const;

	// evaluate tcl in the context of this widget
	int	eval(char *) const;
	char	*evaluate(char *) const;

	// the hush-version of image takes an optional 2nd parameter.
	void	image(const char *) const;

	void	bind(handler *, char * = NULL);
	void	bind(void *, int (*func)(void *, event *), char * = NULL);
	void	bind(char *, handler *h, char * = NULL);
	void	bind(char *, void *, int (*func)(void *, event *), char * = NULL);
	void	bind(const char *);
	void	bind(const char *, const char *);

	void	xscroll(const widget *) const;
	void	yscroll(const widget *) const;

	void	focus(void) const;

	void	wait(void) const;
	void	grab(void) const;

	// configuration options, might not apply to any widget.. should
	// these be defined here or in each derived widget?
	void	enable(void) const;
	void	disable(void) const;
	void	state(char *) const;

	virtual	cstring	container(void) const;

static	int	created;
static	int	deleted;

protected:
	// protected, in case anyone doesn't like the generated names...
	virtual	cstring	uniq(void);
	// use underscores to prevent conflicts with methodnames
	cstring	_path;			// path of widget
	cstring	_options;		// options passed to constructor
	cstring	_type;			// type of widget
	cstring	_seq;			// default sequence for bind
	widget *_parent;		
private:
};


#endif // WIDGET_H
