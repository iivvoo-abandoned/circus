#include "menubutton.h"
#include "menu.h"

int	menubutton::created = 0;
int	menubutton::deleted = 0;

menubutton::menubutton(const char *path, const char *options):
            widget(path, options)
{
	_type = "menubutton";
	_seq = "<Button-1>";	// use this if no sequence specified

	tk << "menubutton " << _path << " " << _options << end;

	// events have to be passed to class specific handlers first..

	tk << "bindtags " << _path << " { Menubutton " 
   	    << _path << " . all }" << end;
	created++;
}

menubutton::menubutton(widget *w, const char *path, const char *options):
            widget(w, path, options)
{
	_type = "menubutton";
	_seq = "<Button-1>";	// use this if no sequence specified

	tk << "menubutton " << _path << " " << _options << end;
	tk << "bindtags " << _path << " { Menubutton " 
	  	   << _path << " . all }" << end;
	created++;
}

menubutton::~menubutton()
{
	deleted++;
}

void	menubutton::text(const char *txt) const
{
	tk << _path << " configure -text \"" << esc(txt) << "\"" << end;
}

void	menubutton::menu(class menu *m)
{
	tk << _path << " configure -menu " << m->path() << end;
}
