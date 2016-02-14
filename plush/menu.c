#include "menu.h"

/*
 * TODO:
 *
 * How to handle events from cascaded menus? bind them to our operator()(),
 * creating an automatic forward if any?
 */

int	menu::created = 0;
int	menu::deleted = 0;

menu::menu(const char *path, const char *options): widget(path, options)
{
	_type	= "menu";
	_seq	= "";

	tk << "menu " << _path << " " << _options << end;

	b = tk->bind(this);
	hndlr = NULL;

	created++;
}

menu::menu(widget *w, const char *path, const char *options): widget(w, path, options)
{
	_type	= "menu";
	_seq	= "";

	tk << "menu " << _path << " " << _options << end;

	b = tk->bind(this);
	hndlr = NULL;

	created++;
}

menu::~menu()
{
#warning "Fix binding deletion"
//	if(hndlr)
//		delete hndlr;
//	delete b;
	deleted++;
}

void	menu::title(const char *title)
{
	tk << _path << " configure -title " << esc(title) << end;
}

menu	*menu::entry(const char *name, const char *command, const char *opts)
{
	const char	*cmd = command;

	if(!cmd)
		cmd = name;
	tk << _path << " add command -label \"" << esc(name) 
	   << "\" -command {" << b->cmd << " " << cmd << "} " 
	   << opts << end;
	return this;
}

menu	*menu::cascade(const char *name, menu *m)
{
	tk << _path << " add cascade -menu " << m->path() 
	   << " -label \"" << esc(name) << "\"" <<  end;
	return this;
}

menu	*menu::separator(const char *options)
{
	tk << _path << " add separator " << options << end;
	return this;
}

// checkbuttons/radiobuttons are not really supported..

menu	*menu::checkbutton(const char *options)
{
	tk << _path << " add checkbutton " << options << end;
	return this;
}

menu	*menu::radiobutton(const char *options)
{
	tk << _path << " add radiobutton " << options << end;
	return this;
}

void	menu::post(int x, int y)
{
	tk << _path << " post " << x << " " << y << end;
}

void	menu::unpost(void)
{
	tk << _path << " unpost" << end;
}

int	menu::operator()()
{
	int	argc = _event->argc();
	cstring	params = "";
	
	cout << "Menu event" << endl;

	for(int i = 0; i < argc; i++)
		cout << "argv[" << i << "] = " << _event->arg(i) << endl;

	// create new message, to be passed to handler
	for(int i = 1; i < argc; i++)
	{
		params << _event->arg(i);
		if(i < argc - 1)
			params << " ";
	}
	// forward events to real handler, if any

	if(hndlr)
		tk << hndlr->cmd << " {" << params << "}" << end;
	return TCL_OK;
}

// override the default widget::bind, which does not do what I want...
// (each option in the menu generates a unique event, which can only
// be bound to a handler *after* bind. Until then we'll have to catch
// these events ourselves, and forward them if a handler has been installed

binding	*menu::bind(handler *h, const char *command)
{
	if(command)
		hndlr = tk->bind(command, h);
	else
		hndlr = tk->bind(h);

	return hndlr;
}

binding	*menu::bind(void *h, int (*func)(void *, event *e))
{
//	if(command)
//		hndlr = tk->bind(command, func, h);
//	else
		hndlr = tk->bind(h, func);

	return hndlr;
}
