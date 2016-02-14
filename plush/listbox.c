#include <stdlib.h>		// for atoi()

#include "listbox.h"

int	listbox::created = 0;
int	listbox::deleted = 0;

listbox::listbox(const char *path, const char *options): widget(path, options)
{
	_type = "listbox";
	_seq = "<Return>";		// use this if no sequence specified

	tk << "listbox " << _path << " " << _options << end;

	// events have to behandled in a different way..

	tk << "bindtags " << _path << " { Listbox " 
	   << _path << " . all }" << end;
	created++;
}

listbox::listbox(widget *w, const char *path, 
                 const char *options): widget(w, path, options)
{
	_type = "listbox";
	_seq = "<Return>";		// use this if no sequence specified

	tk << "listbox " << _path << " " << _options << end;
	tk << "bindtags " << _path << " { Listbox " 
	   << _path << " . all }" << end;
	created++;
}

listbox::~listbox()
{
	deleted++;
}

void	listbox::insert(const char *txt) const
// insert txt at begin
{
	tk << _path << " insert end " << " \"" << esc(txt) << "\"" << end;
}

void	listbox::insert(const int at, const char *txt) const
// insert txt at 'at'
{
	tk << _path << " insert " << at << " \"" << esc(txt) << "\"" << end;
}

void	listbox::del(void) const
// delete all entries
{
	tk << _path << " delete 0 end" << end;
}

void	listbox::del(const int which) const
// delete 1 entry
{
	tk << _path << " delete " << which << end;
}

void	listbox::del(const int from, const int to) const
// delete a range of entries
{
	tk << _path << " delete " << from << " " << to << end;
}

void	listbox::select(void) const
// select all entries
{
	tk << _path << " selection set 0 end" << end;
}

void	listbox::select(const int which) const
// select 1 entry
{
	tk << _path << " selection set " << which << end;
}

void	listbox::select(const int from, const int to) const
// select a range of entries
{
	tk << _path << " selection set " << from << " " << to << end;
}

const	int	listbox::selection(void) const
// HUSH returns only the first selected entry, but tk may give a list
// of selected entries as returnvalue. Check if this works, and if it
// can be improved..
{
	tk << _path << " curselection" << end;
	return atoi(tk.result());
}
void	listbox::unselect(void) const
// unselect all entries
{
	tk << _path << " selection clear 0 end" << end;
}

void	listbox::unselect(const int which) const
// unselect 1 entry
{
	tk << _path << " selection clear " << which << end;
}

void	listbox::unselect(const int from, const int to) const
// unselect a range of entries
{
	tk << _path << " selection clear " << from << " " << to << end;
}

const	int	listbox::selected(const int idx) const
{
	tk << _path << " selection includes " << idx << end;
	return atoi(tk.result());
}

const	int	listbox::size(void) const
{
	tk << _path << " size" << end;
	return atoi(tk.result());
}

const	char	*listbox::get(const int idx) const
{
	tk << _path << " get " << idx << end;
	return tk.result();
}

// HUSH also implements void singleselect() - I don't know what it does.
