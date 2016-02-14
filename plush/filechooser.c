#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>

#include <stdlib.h>

#include "filechooser.h"

filesel::filesel(const cstring &where, int mode): toplevel(".fileselector")
{
	char	buf[1024];
	
	if(where[0] != '/')
	{
		wd = getcwd(buf, 1024);
		wd += '/' + where;
	}
	else
		wd = where;
	if(wd[-1] != '/')
		wd += '/';

	file = "";
	result = false;
	params = mode;
	build();
}

filesel::~filesel()
{}

void	filesel::build(void)
{
	topframe = new frame(this, ".top", "-relief raised -border 1");
	chooseframe = new frame(topframe, ".choose");
	buttonframe = new frame(this, ".button", "-relief raised -border 1");

	dirframe = new frame(chooseframe, ".dirs");
	fileframe = new frame(chooseframe, ".files");

	dirlbl = new label(dirframe, ".lbl", "-text Directories");
	directories = new listbox(dirframe, ".list");
	dirscroll = new scrollbar(dirframe, ".scroll");
	dirscroll->yview(directories);
	directories->yscroll(dirscroll);

	filelbl = new label(fileframe, ".lbl", "-text Files");
	files = new listbox(fileframe, ".list");
	filescroll = new scrollbar(fileframe, ".scroll");
	filescroll->yview(files);
	files->yscroll(filescroll);

	selframe = new frame(topframe, ".selframe");
	sellbl = new label(selframe, ".sellabel", "-text Selection: "
	                                          "-justify left");
	selection = new entry(selframe, ".selentry");

	ok = new button(buttonframe, ".ok", "-text Ok");
	cancel = new button(buttonframe, ".cancel", "-text Cancel");

	// now pack it!

	dirlbl->pack("-pady 10");
	directories->pack("-expand true -fill both -side left");
	dirscroll->pack("-fill y -expand false -side right");
	dirframe->pack("-expand true -fill both -side left -padx 5 -pady 5");

	filelbl->pack("-pady 10");
	files->pack("-expand true -fill both -side left");
	filescroll->pack("-fill y -expand false -side right");
	fileframe->pack("-expand true -fill both -side right -padx 5 -pady 5");


	ok->pack("-expand true -fill none -side left -padx 10 -pady 10");
	cancel->pack("-expand true -fill none -side right -padx 10 -pady 10");
	chooseframe->pack("-expand true -fill both");
	sellbl->pack("-side top -expand false -fill x");
	selection->pack("-side bottom -expand true -fill x");
	selframe->pack("-expand false -fill both -padx 5 -pady 5");
	topframe->pack("-expand true -fill both");
	buttonframe->pack("-expand false -fill both");

	// bind

	ok->bind(this, "OkPressed");
	cancel->bind(this, "CancelPressed");

	directories->bind("<Double-1>", this, "SelectDir");
	files->bind("<1>", this, "SelectFile");
	files->bind("<Double-1>", this, "SelectFileDone");
	selection->bind(this, "SelectionTyped");

	selection->insert(wd);
}

int	cmp(const void *a, const void *b)
{
	return strcmp(*(char **)a, *(char **)b);
}

int	filesel::dirlist(void)
{
	DIR	*dir;
	struct	dirent	*file;
	struct	stat	sbuf;
	
	char	*fils[512];
	char	*dirs[512];

	int	fcnt=0, dcnt=0, i;

	if((dir = opendir(wd)) == NULL)
	{
//		cout << "Cannot access " << wd << endl;
		return false;
	}
	
	while((file = readdir(dir)))
	{
		if(stat(wd + file->d_name, &sbuf) == -1)
		{
			;	// fine, i.e. broken symlink	
	//		cout << "Could not stat " << wd <<file->d_name << endl;
		}
		else if(S_ISDIR(sbuf.st_mode))
		{
			dirs[dcnt] = new char[strlen(file->d_name)+1];
			strcpy(dirs[dcnt++], file->d_name);
		}
		else
		{
			fils[fcnt] = new char[strlen(file->d_name)+1];
			strcpy(fils[fcnt++], file->d_name);
		}		
	}

	qsort(fils, fcnt, sizeof(char *), cmp);
	qsort(dirs, dcnt, sizeof(char *), cmp);

	for(i = 0; i < fcnt; i++)
	{
		files->insert(fils[i]);
		delete fils[i];
	}

	for(i = 0; i < dcnt; i++)
	{
		directories->insert(dirs[i]);
		delete dirs[i];
	}
	closedir(dir);

	return true;
}

void	filesel::showselection(void)
{
	selection->del();
	selection->insert(wd + file);
}

int	filesel::checkselection(void)
{
	cstring	sel = selection->get();
	struct	stat	sbuf;
	
	if(stat(sel, &sbuf) == -1)
	{
		if(params & require_exist)
		{
			tk->beep();
			return false;
		}
		else
		{
			wd = sel; file = "";
			return true;
		}
	}
	else if(S_ISDIR(sbuf.st_mode))
	{
		if(params & require_nodir)
		{
			cd(sel);	// display new dir
			return false;	// selection ok, but not yet complete
		}
		// else fall through - selection complete
	}

	wd = sel; file = "";		// rather ugly, really should split it
	return true;
}

void	filesel::selectfile(void)
{
	cstring f = files->get(files->selection());

	file = f;
	showselection();
}

void	filesel::cd(const cstring &where)
{
	cstring	dir;
	
	if(where == "")
		dir = directories->get(directories->selection());
	else
		dir = where;

	cstring old_wd = wd;

	file = "";
	files->unselect();

	if(!(dir == "."))
	{
		if(dir == "..")
		{
			if(!(wd == "/"))
			{
				--wd;			// remove last /
				wd.rchomp('/');
				wd += '/';		// add / again
			}
		}
		else if(dir[0] == '/')
			wd = dir;
		else
			wd += dir;

		// make sure directory ends in /

		if(wd[-1] != '/')
			wd += '/';

		directories->del();
		files->del();
		if(dirlist())
			showselection();
		else
		{
			tk->beep();
			wd = old_wd;
			dirlist();		// show old dir again
		}
	}
}

int	filesel::get(cstring &path)
{
	dirlist();
	wait();
	path = wd + file;
	return result;
}

int	filesel::operator()()
{
	int	argc = _event->argc();
	char	**argv = _event->argv();

	for(int i = 0; i < argc; i++)
		cout << "arg: " << i << ": " << argv[i] << endl;

	if(argc == 2)
	{
		cstring	e(argv[1]);

		if(e == "SelectDir")
			cd();
		else if(e == "SelectFile")
			selectfile();
		else if(e == "SelectionTyped")
		{
			if(checkselection())
			{
				result = true;
				destroy();
			}
		}
		else if(e == "OkPressed")
		{
			if((file == "") && (params & require_nodir))
				tk->beep();
			else
			{
				result = true;
				destroy();
			}
		}
		else if(e == "SelectFileDone")
		{
			selectfile();
			result = true;
			destroy();
		}
		else if(e == "CancelPressed")
		{
			result = false;
			destroy();
		}
	}
	return TCL_OK;
}

