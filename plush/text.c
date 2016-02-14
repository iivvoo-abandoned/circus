#include "text.h"

int	text::created = 0;
int	text::deleted = 0;

text::text(const char *path, const char *options): widget(path, options)
{
	_type = "text";
	_seq = "<Return>";		// use this if no sequence specified

	tk << "text " << _path << " " << _options << end;
	created++;
}

text::text(widget *w, const char *path, const char *options): 
      widget(w, path, options)
{
	_type = "text";
	_seq = "<Return>";		// use this if no sequence specified

	tk << "text " << _path << " " << _options << end;
	created++;
}

text::~text()
{
	deleted++;
}

void	text::insert(const char *at, const char *text, const char *tags) const
{
	tk << _path << " insert " << at << " \"" << esc(text) 
	   << "\" {" << tags << "}" << end;
}

void	text::del(const char *from, const char *to) const
{
	tk << _path << " delete " << from << " " << to << end;
}

const	char	*text::get(const char *from, const char *to) const
{
	tk << _path << " get " << from << " " << to << end;
	return tk.result();
}

void	text::enable(void) const
{
	tk << _path << " configure -state normal" << end;
}

void	text::disable(void) const
{
	tk << _path << " configure -state disabled" << end;
}
