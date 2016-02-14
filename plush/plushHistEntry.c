#include "plushHistEntry.h"

/*
 * plushHistEntry
 */

plushHistEntry::plushHistEntry(): entry()
{
	init();
}
plushHistEntry::plushHistEntry(char* p, char* options): 
               entry(p, options)
{	
	init();
}
plushHistEntry::plushHistEntry(widget* w,char* p, char* options): 
	       entry(w, p, options)
{
	init();
}

plushHistEntry::~plushHistEntry()
{
}

event	*plushHistEntry::dispatch(event *_e)
{
	_event = _e;
	
	if(_event->argc() > 1)
	{
		cstring	ev = _event->arg(1);

		if(ev == "hist_up")
		{
			if(idx > 0)
				idx--;
		}
		else if(ev == "hist_down")
		{
			if(idx < history.size())
				idx++;
		}
		del();
		if(idx < history.size())
			insert(*history[idx]);
		else
			insert("");
	}
	operator()();
	entry::dispatch(_e);
	return _event;
}

void	plushHistEntry::init(void)
{
	idx = 0;
	bind("<Control-KeyPress-p>", this, "hist_up");
	bind("<Control-KeyPress-n>", this, "hist_down");
	bind("<KeyPress-Up>", this, "hist_up");
	bind("<KeyPress-Down>", this, "hist_down");
}

char	*plushHistEntry::get()
{
	const	char	*result = entry::get();

	cstring	*lcopy = new cstring(result);

	histrash._register(lcopy);
	history.insert(history.end(), lcopy);
	idx = history.size();
	return *lcopy;
}
