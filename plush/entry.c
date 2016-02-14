#include "entry.h"

int	entry::created = 0;
int	entry::deleted = 0;

entry::entry(const char *path, const char *options): widget(path, options)
{
	_type = "entry";
	_seq = "<Return>";		// use this if no sequence specified

	tk << "entry " << _path << " " << _options << end;
	created++;
}

entry::entry(widget *w, const char *path, 
             const char *options): widget(w, path, options)
{
	_type = "entry";
	_seq = "<Return>";		// use this if no sequence specified

	tk << "entry " << _path << " " << _options << end;
	created++;
}

entry::~entry()
{
	deleted++;
}

void	entry::insert(const char *txt, const int at) const
{
	tk << _path << " insert " << at << " \"" << esc(txt) << "\"" << end;
}

void	entry::del(const int from, const int to) const
{
	if(to == ENTRY_END)
		tk << _path << " delete " << from << " end" << end;
	else
		tk << _path << " delete " << from << " " << to << end;
}

const	char	*entry::get(void) const
{
	tk << _path << " get" << end;
	return tk.result();
}

