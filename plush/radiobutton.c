#include "radiobutton.h"

int	radiobutton::created = 0;
int	radiobutton::deleted = 0;

/*
 * BUG?
 *
 * It's not possible to pass the options "-variable" and "-value"
 * to the constructor, because the constructor itself will overwrite
 * these values..
 */

radiobutton::radiobutton(const char *path, const char *options): 
             button(path, options, false)
{
	_type = "radiobutton";
	_seq = "<Button-1>";		// use this if no sequence specified

	tk << "radiobutton " << _path << " " << _options << end;

	// events have to be passed to class specific handlers first..

	tk << "bindtags " << _path << " { Radiobutton " 
	   << _path << " . all }" << end;

	// associate a unique variable with the button.

	_var << "rb" << created;
	tk << _path << " configure -value " << _var 
	   << " -variable " << _var << end;
	
	deselect();
	created++;
}

radiobutton::radiobutton(widget *w, const char *path, const char *options): 
             button(w, path, options, false)
{
	_type = "radiobutton";
	_seq = "<Button-1>";		// use this if no sequence specified

	tk << "radiobutton " << _path << " " << _options << end;
	tk << "bindtags " << _path << " { Radiobutton " 
	   << _path << " . all }" << end;

	// associate a unique variable with the button.

	_var << "rb" << created;
	tk << _path << " configure -value " << _var
	   << " -variable " << _var << end;

	deselect();
	created++;
}

radiobutton::~radiobutton()
{
	deleted++;
}

void	radiobutton::select(void) const
{
	tk << _path << " select" << end;
}

void	radiobutton::deselect(void) const
{
	tk << _path << " deselect" << end;
}

void	radiobutton::group(radiobutton *r)
{
	_var = r->_var;
	tk << _path << " configure -variable " << _var << end;
}

void	radiobutton::value(const char *val)
{
	tk << _path << " configure -value " << val << end;
}

char	*radiobutton::value(void) const
{
	return tk.getvar(_var);
}

