#include "frame.h"

int	frame::created = 0;
int	frame::deleted = 0;

frame::frame(const char *path, const char *options, int create): 
       widget(path, options)
{
	if(create)
	{
		_type = "frame";

		tk << "frame " << _path << " " << _options << end;
	}
	created++;
}

frame::frame(widget *w, const char *path, const char *options, int create): 
       widget(w, path, options)
{
	if(create)
	{
		_type = "frame";

		tk << "frame " << _path << " " << _options << end;
	}
	created++;
}

frame::~frame()
{
	deleted++;
}

void	frame::pack(const char *opts) const
{
	widget::pack(opts);
}
