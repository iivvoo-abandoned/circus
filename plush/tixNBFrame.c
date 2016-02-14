#include "tixNBFrame.h"

int	tixNBFrame::created = 0;
int	tixNBFrame::deleted = 0;

/*
 * tixNBFrames cannot be explicitly created!
 */

tixNBFrame::tixNBFrame(const char *path, const char *options):
       widget(path, options)
{
	_type = "tixNBFrame";
	created++;
}

tixNBFrame::tixNBFrame(widget *w, const char *path, const char *options): 
       widget(w, path, options)
{
	_type = "frame";
	created++;
}

tixNBFrame::~tixNBFrame()
{
	deleted++;
}

void	tixNBFrame::setname(const char *name)
{
	_name = name;
}

void	tixNBFrame::label(const char *msg)
/*
 * Personally I think it's quite ugly to set the label through the parent
 * but that's the way tix works :(
 */
{
	_parent->eval("pageconfigure " + _name + " -label \"" 
	            + esc(msg) + "\"");
}
