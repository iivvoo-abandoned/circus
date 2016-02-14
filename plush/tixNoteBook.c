#include "tixNoteBook.h"

int	tixNoteBook::created = 0;
int	tixNoteBook::deleted = 0;

/*
 * TODO: Store created subpanes, 
 *       function for retrieving subpanes (operator[]?)
 *       fix pathname kludge in add()
 */
tixNoteBook::tixNoteBook(const char *path, const char *options):
               widget(path, options)
{
	_type = "tixNoteBook";

	tk << "tixNoteBook " << _path << " " << _options << end;
	created++;
}

tixNoteBook::tixNoteBook(widget *w, const char *path, const char *options):
               widget(w, path, options)
{
	_type = "tixNoteBook";

	tk << "tixNoteBook " << _path << " " << _options << end;
	created++;
}

tixNoteBook::~tixNoteBook()
{
	deleted++;
}

/*
 * Perhaps implement a special tixNBFrame, as implemented by tix.
 *
 * For now, this will do just fine.
 */

tixNBFrame	*tixNoteBook::add(const char *name, const char *options)
{
	tixNBFrame 	*f;

	tk << _path << " add " << name << " " << options << end;

	cstring pname;
	
	pname << ".nbframe." << name;

	f = new tixNBFrame(this, pname, options);
	f->setname(name);
	return f;
}

void	tixNoteBook::forget(const char *name)
{
	tk << _path << " forget " << name << end;
}

void	tixNoteBook::remove(const char *name)
{
	tk << _path << " delete " << name << end;
}

void	tixNoteBook::raise(const char *name)
{
	tk << _path << " raise " << name << end;
}

void	tixNoteBook::pageconfigure(const char *name, const char *opts)
{
	tk << _path << " pageconfigure " << name << " " << opts << end;
}

