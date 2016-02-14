#ifndef MENU_H
#define MENU_H

#include "widget.h"

class	menu: public widget
{
public:
	menu(const char * = NULL, const char * = NULL);
	menu(widget *, const char * = NULL, const char * = NULL);
	virtual	~menu();

// add defaults
	void	title(const char *);
	menu	*entry(const char *, const char * = NULL, const char * = "");
	menu	*cascade(const char *, menu *);
	menu	*separator(const char * = "");

	// not supported - they don't work
	menu	*checkbutton(const char * = "");
	menu	*radiobutton(const char * = "");

	void	post(int = -1, int = -1);
	void	unpost(void);
	
	int	operator()();

	binding	*bind(handler *, const char *cmd = NULL);
	binding	*bind(void *, int (*func)(void *, event *e));

static int	created = 0;
static int	deleted = 0;

private:
	binding	*b, *hndlr;
};

#endif	// MENU_H
