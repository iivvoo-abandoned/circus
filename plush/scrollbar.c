#include "scrollbar.h"

int	scrollbar::created = 0;
int	scrollbar::deleted = 0;

scrollbar::scrollbar(const char *path, const char *options): widget(path, options)
{
	_type = "scrollbar";

	tk << "scrollbar " << _path << " " << _options << end;
	created++;
}

scrollbar::scrollbar(widget *w, const char *path, const char *options): 
           widget(w, path, options)
{
	_type = "scrollbar";

	tk << "scrollbar " << _path << " " << _options << end;
	created++;
}

scrollbar::~scrollbar()
{
	deleted++;
}

void	scrollbar::yview(const widget *w) const
{
	tk << _path << " configure -command {" << w->path() 
	   << " yview }" << end;
}

void	scrollbar::xview(const widget *w) const
{
	tk << _path << " configure -command {" << w->path() 
	   << " xview }" << end;
}
