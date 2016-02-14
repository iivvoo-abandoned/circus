#include "tixLabelFrame.h"

int	tixLabelFrame::created = 0;
int	tixLabelFrame::deleted = 0;

tixLabelFrame::tixLabelFrame(const char *path, const char *options, 
  			     int create): frame(path, options, false)
{
	if(create)
	{
		_type = "tixLabelFrame";

		tk << "tixLabelFrame " << _path << " " << _options << end;
	}
	created++;
}

tixLabelFrame::tixLabelFrame(widget *w, const char *path, 
		             const char *options, int create): 
       	       frame(w, path, options, false)
{
	if(create)
	{
		_type = "tixLabelFrame";

		tk << "tixLabelFrame " << _path << " " << _options << end;
	}
	created++;
}

tixLabelFrame::~tixLabelFrame()
{
	deleted++;
}

void	tixLabelFrame::title(const char *ttl)
{
	tk << _path << " subwidget label configure -text \"" 
	   << escape(ttl) << "\"" << end;
}

void	tixLabelFrame::pack(const char *opts) const
{
	widget::pack(opts);
}

cstring	tixLabelFrame::container(void) const
{
	/*
	 * perhaps it's a bad idea to 'hard-code' the ".border.frame"
	 * string, it would be better to concatenate 
	 * tk->eval(_path + " subwidget frame");
	 * But it has the same effect (but might cause bugs with newer
	 * tix releases that have different names for subwidgets
	 */
	return _path + ".border.frame";
}

