#include "toplevel.h"

int	toplevel::created = 0;
int	toplevel::deleted = 0;

toplevel::toplevel(const char *path, const char *options, int create): 
          frame(path, options, false)
{
	_type = "toplevel";

	if(create)
		tk << "toplevel " << _path << " " << _options << end;
	toplevel::created++;
}

toplevel::toplevel(widget *w, const char *path, const char *options, 
                  int create): frame(w, path, options, false)
{
	_type = "toplevel";

	if(create)
		tk << "toplevel " << _path << " " << _options << end;
	toplevel::created++;
}

toplevel::~toplevel()
{
	toplevel::deleted++;
}

void	toplevel::title(const char *ttl) const
{
	tk << "wm title " << _path << " \"" << esc(ttl) << "\"" << end;
}

void	toplevel::position(const int x, const int y) const
{
	tk << "wm geometry " << _path << " +" << x << "+" << y << end;
}

void	toplevel::transient(const char *master) const
{
	tk << "wm transient " << _path << " " << master << end;
}

void	toplevel::resize(const int w, const int h) const
{
	tk << "wm geometry " << _path << " " << w << "x" << h << end;
}

void	toplevel::iconify(void)
{
	tk << "wm iconify " << _path << end;
}

void	toplevel::deiconify(void)
{
	tk << "wm deiconify " << _path << end;
}

void	toplevel::raise(void)
{
	tk << "wm raise " << _path << end;
}

