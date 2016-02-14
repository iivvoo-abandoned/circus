#ifndef _FILECHOOSER_H
#define _FILECHOOSER_H

#include "widgets.h"
#include "cstring.h"

class	filesel: public toplevel
{
public:
	enum	{ require_nothing = 0, require_nodir = 1, require_exist = 2 };
	filesel(const cstring & = "", int = require_nothing);
	~filesel();

	void	build(void);
	int	dirlist(void);

	
	void	showselection(void);
	int	checkselection(void);
	void	selectfile(void);
	void	cd(const cstring & = "");
	int	get(cstring &);
	int	operator()();

private:
	cstring	wd, file;
	int	result;
	int	params;

	frame	*topframe;
	frame	*chooseframe, *buttonframe, *selframe;
	frame	*dirframe, *fileframe;

	listbox	*directories, *files;
	label	*filelbl, *dirlbl, *sellbl;

	scrollbar	*dirscroll, *filescroll;
	entry		*selection;
	button	*ok, *cancel;
};

#endif
