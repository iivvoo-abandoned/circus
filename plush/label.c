#include "label.h"

int	label::created = 0;
int	label::deleted = 0;

label::label(const char *path, const char *options, int create): 
       widget(path, options)
{
	if(create)
	{
		_type = "label";
		_seq = "<Button-1>";	// use this if no sequence specified

		tk << "label " << _path << " " << _options << end;
	}
	created++;
}

label::label(widget *w, const char *path, const char *options, int create): 
       widget(w, path, options)
{
	if(create)
	{
		_type = "label";
		_seq = "<Button-1>";	// use this if no sequence specified

		tk << "label " << _path << " " << _options << end;
	}
	created++;
}

label::~label()
{
	deleted++;
}

void	label::text(const char *txt) const
{
	tk << _path << " configure -text \"" << esc(txt) << "\"" << end;
}
