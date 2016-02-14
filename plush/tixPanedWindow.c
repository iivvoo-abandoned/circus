#include "tixPanedWindow.h"

int	tixPanedWindow::created = 0;
int	tixPanedWindow::deleted = 0;

/*
 * TODO: Store created subpanes, 
 *       function for retrieving subpanes (operator[]?)
 *       fix pathname kludge in add()
 */
tixPanedWindow::tixPanedWindow(const char *path, const char *options):
               widget(path, options)
{
	_type = "tixPanedWindow";

	tk << "tixPanedWindow " << _path << " " << _options << end;
	created++;
}

tixPanedWindow::tixPanedWindow(widget *w, const char *path, const char *options):
               widget(w, path, options)
{
	_type = "tixPanedWindow";

	tk << "tixPanedWindow " << _path << " " << _options << end;
	created++;
}

tixPanedWindow::~tixPanedWindow()
{
	deleted++;
}

frame	*tixPanedWindow::add(const char *name, const char *options)
{
	tk << _path << " add " << name << " " << options << end;

	cstring pname;
	
	pname << "." << name;

	return new frame(this, pname, options, false);
}

void	tixPanedWindow::forget(const char *name)
{
	tk << _path << " forget " << name << end;
}

void	tixPanedWindow::remove(const char *name)
{
	tk << _path << " delete " << name << end;
}
