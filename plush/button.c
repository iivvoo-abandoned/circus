#include "button.h"

/*
 * Problem: setting _type to, somewhat, influence generated pathname
 * doesn't work. widget's constructor is called before _type is set.
 *
 * Possible (and used) solution: overload uniq()
 */

int	button::created = 0;
int	button::deleted = 0;

button::button(const char *path, const char *options, int create): 
        widget(path, options)
{
	if(create)
	{
		_type = "button";
		_seq = "<Button-1>";	// use this if no sequence specified

		tk << "button " << _path << " " << _options << end;

		// events have to be passed to class specific handlers first..

		tk << "bindtags " << _path << " { Button " 
	   	    << _path << " . all }" << end;
	}
	created++;
}

button::button(widget *w, const char *path, const char *options, int create): 
        widget(w, path, options)
{
	if(create)
	{
		_type = "button";
		_seq = "<Button-1>";	// use this if no sequence specified

		tk << "button " << _path << " " << _options << end;
		tk << "bindtags " << _path << " { Button " 
	   	   << _path << " . all }" << end;
	}
	created++;
}

button::~button()
{
	deleted++;
}

void	button::text(const char *txt) const
{
	tk << _path << " configure -text \"" << esc(txt) << "\"" << end;
}

void	button::flash(void) const
{
	tk << _path << " flash" << end;
}

void	button::invoke(void) const
{
	tk << _path << " invoke" << end;
}
