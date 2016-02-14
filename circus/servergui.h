#ifndef _SERVERGUI_H
#define _SERVERGUI_H

#include <plush.h>

#include "cstring.h"

#include "irctext.h"

class	servergui: public virtual handler
{
public:
	servergui(tixNoteBook *, const cstring &, int);
	virtual ~servergui();

	void	setlabel(const cstring &msg);
	void	insert(const cstring &, const cstring & = "");
	void	insert(const cstring &, const cstring &, const int, 
	               const cstring & = "");
	void	setnick(const cstring &);
	cstring	getnick(void) const;
	void	settarget(const cstring &);
	cstring	gettarget(void) const;
	cstring	gettargetlist(const int = -1) const;

	cstring	getinput(void) const;
	void	delinput(void);

	void	refocus(void);
	void	add_target(const cstring &);
	void	ren_target(const cstring &, const cstring &);
	void	del_target(const cstring &);

	void	mode_toggle(const int, const int = -1);
	void	mode_enable_disable(const int, const int);
	void	mode_enable(const int mode);
	void	mode_disable(const int mode);
	void	popup_menu(int, int, int, int);

	listbox		*targetslist;		// public for masterevent...
private:
	// private methods

	tixNBFrame	*addTab(tixNoteBook *parent);
	void	bindings();

	int		eventloop;

	// private variables
	
	cstring		name;

	// widgets
	tixNoteBook	*parent;

	tixNBFrame	*tabframe;
	frame		*leftframe, *statusframe, *targetframe, 
			*outframe, *listframe;
	irctext		*out;
	entry		*targetentry, *nickentry;
	plushHistEntry	*in;
	label		*targetlabel, *nicklabel, *usermodelabel;
	checkbutton	*mode_w, *mode_i, *mode_r;

	label		*lbl_w, *lbl_i, *lbl_r;

	scrollbar	*targetscroll, *outscroll;
	menu		*mainmenu, *ctcpmenu, *setmodemenu, *delmodemenu,
			*invitemenu;
};

#endif // _SERVERGUI_H
