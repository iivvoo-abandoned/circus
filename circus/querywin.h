#ifndef _IRCQUERY_H
#define _IRCQUERY_H

#include <plush.h>

#include "cstring.h"
#include "irctext.h"

#include "targetif.h"

class	querywin: public targetif, public toplevel
{
public:
	querywin(cstring, cstring = "");
	~querywin(void);
	int	get_autoraise(void);
	void	current(int, int = false);
	cstring	get_person(void) const;
	void	insert(const cstring &, const cstring & = "");
	void	insert(const cstring &, const cstring &, const unsigned int, 
	               const cstring & = "");
	int	operator()();
	void	bind(handler *);
	char	*getinput(void);
	void	delinput(void);

	void	setdcc(int);
	void	setchat(int);
	int	getdcc(void);
	int	getchat(void);

	void	clear(void);

	void	toggle_lock(void);
	enum	{ none, started, active };

	void	nickchange(const cstring &);

	virtual	void	info_insert(const cstring &) {};

	virtual void	nickchange(const cstring &, const cstring &) {};
	virtual void	quituser(const cstring &, const cstring &) {};

	virtual void	change_user(const cstring &, const cstring &) {};
	virtual	void	del_user(const cstring &) {};

	virtual	void	disable(void) {};
	virtual	void	destroy(void) {};

	int		wantinfo(int, char **);
	checkbutton	*autoraise;

	frame		*nickframe, *serverframe, *miscframe;
	label		*nicklabel, *serverlabel, *misclabel;
	entry		*nickentry, *serverentry, *miscentry;
	int		info;
	
	int		sessionid;
private:
	int		handleInput(void);
static	int		StaticHandleInput(void *, event *);

	int	has_dcc;
	int	use_dcc;
	cstring	person;
	void	build_querywindow(widget *w);
	void	bindings();

	checkbutton	*dcc_button, *chat_button;
	frame		*queryframe, *buttonframe;
	plushButton	*stopbutton;
	plushHistEntry	*in;
	irctext		*query;
	scrollbar	*queryscroll;
};

class	messagewin: public targetif, public toplevel
{
public:
	messagewin(cstring, cstring = "");
	~messagewin(void);
	int	get_autoraise(void);
	int	operator()();
	void	bind(handler *);
	void	insert(const cstring &, const cstring & = "");
	void	insert(const cstring &, const cstring &, const unsigned int, 
	               const cstring & = "");
	void	add_person(const cstring &);
	void	del_person(const cstring &);
	cstring	selected_person(void);
	listbox		*targetlist;
	char	*getinput(void);
	void	delinput(void);
	int	getdcc(void);
	int	getchat(void);

	void	clear(void);

	void	toggle_lock(void);



	virtual	void	info_insert(const cstring &) {};

	virtual void	nickchange(const cstring &, const cstring &) {};
	virtual void	quituser(const cstring &, const cstring &) {};

	virtual void	change_user(const cstring &, const cstring &) {};
	virtual	void	del_user(const cstring &) {};

	virtual	void	disable(void) {};
	virtual	void	destroy(void) {};

	virtual void	current(int, int) {};

	int		wantinfo(int, char **);
	checkbutton	*autoraise;

	frame		*nickframe, *serverframe, *miscframe;
	label		*nicklabel, *serverlabel, *misclabel;
	entry		*nickentry, *serverentry, *miscentry;
	int		info;

	int		sessionid;

private:
	void	build_messagewindow(widget *w);
	void	bindings();

//	int		handleInput(void);
//	int		StaticHandleInput(void *, event *);

	frame		*messageframe, *rightframe, *listframe, *buttonframe,
	                *outframe;
	irctext		*message;
	scrollbar	*messagescroll, *targetscroll;
	plushHistEntry	*in;
};

#endif // _IRCQUERY_H
