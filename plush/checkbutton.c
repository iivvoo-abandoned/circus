#include "checkbutton.h"

int	checkbutton::created = 0;
int	checkbutton::deleted = 0;

checkbutton::checkbutton(const char *path, const char *options): 
             button(path, options, false)
{
	_type = "checkbutton";
	_seq = "<Button-1>";		// use this if no sequence specified

	tk << "checkbutton " << _path << " " << _options << end;

	// events have to be passed to class specific handlers first..

	tk << "bindtags " << _path << " { Checkbutton " 
	   << _path << " . all }" << end;

	_var << "cb" << created;
	tk << _path << " configure -variable " << _var << end;
	created++;
}

checkbutton::checkbutton(widget *w, const char *path, const char *options): 
          button(w, path, options, false)
{
	_type = "checkbutton";
	_seq = "<Button-1>";		// use this if no sequence specified

	tk << "checkbutton " << _path << " " << _options << end;
	tk << "bindtags " << _path << " { Checkbutton " 
	   << _path << " . all }" << end;

	_var << "cb" << created;
	tk << _path << " configure -variable " << _var << end;
	created++;
}

checkbutton::~checkbutton()
{
	deleted++;
}

void	checkbutton::variable(const char *var)
{
	tk << _path << " configure -variable " << esc(var) << end;
}

void	checkbutton::select(void) const
{
	tk << _path << " select" << end;
}

void	checkbutton::deselect(void) const
{
	tk << _path << " deselect" << end;
}

void	checkbutton::toggle(void) const
{
	tk << _path << " toggle" << end;
}

char	*checkbutton::value(void) const
{
	return tk.getvar(_var);
}
