#include "message.h"

int	message::created = 0;
int	message::deleted = 0;

message::message(const char *path, const char *options): widget(path, options)
{
	_type = "message";
	_seq = "<Button-1>";		// use this if no sequence specified

	tk << "message " << _path << " " << _options << end;
	created++;
}

message::message(widget *w, const char *path, const char *options): 
         widget(w, path, options)
{
	_type = "message";
	_seq = "<Button-1>";		// use this if no sequence specified

	tk << "message " << _path << " " << _options << end;
	created++;
}

message::~message()
{
	deleted++;
}

void	message::text(const char *txt) const
{
	tk << _path << " configure -text \"" << esc(txt) << "\"" << end;
}
