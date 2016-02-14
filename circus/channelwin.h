#ifndef CHANNELWIN_H
#define CHANNELWIN_H

#include <plush.h>

#include "cstring.h"

#include "irctext.h"

#define	OP		0x0001
#define	DEOP		0x0002
#define	VOICE		0x0004
#define	MUTE		0x0008

class	channelwin: public toplevel
{
public:
	channelwin(char  *, char * = "");
	~channelwin();
	int	operator()();
	void	current(int, int = false);
	void	disable(void);
	void	enable(void);
	virtual void	insert(const cstring &, const cstring & = "");
	virtual void	insert(const cstring &, const cstring &, 
	                const unsigned int, const cstring & = "");
	virtual void	info_insert(const cstring &);
	void	set_topicentry(const cstring &);
	cstring	get_topicentry(void) const;
	void	set_keyentry(const cstring &);
	cstring	get_keyentry(void) const;
	void	set_limitentry(const cstring &);
	cstring	get_limitentry(void) const;
	void	add_user(const cstring &, const int = 0, const int = 0);
	int	del_user(const cstring &);
	int	user_selected(const cstring &) const;
	void	user_select(const int);
	cstring	get_user(void) const;
	void	set_op(int on);
	void	mode_toggle(int mode, int on = -1);
	void	mode_enable_disable(int mode, int on);
	void	mode_enable(int mode);
	void	mode_disable(int mode);
	void	mode_enable_all(void);
	void	mode_disable_all(void);
	void	topic_enable(void);
	void	topic_disable(void);
	void	limit_enable(void);
	void	limit_disable(void);
	void	key_enable(void);
	void	key_disable(void);

	void	toggle_lock(void);
	void	bind(void *h, int (*func)(void *, event *));
// I need this to get the selections..
	listbox		*userlist;

	void	popup_menu(cstring, int, int, int, int);
	char	*getinput(void);
	void	delinput(void);

	void	clear(void);
protected:
	int	sessionid;
private:
	int	handleInput(void);
static	int	StaticHandleInput(void *h, event *e);

	cstring	cname;
	cstring	eventname;		// obsolete

	void	buildwindow(widget *w);
	void	bindhelp(widget *, const cstring &);

	tixPanedWindow	*split;
	frame		*upper, *lower;
	frame		*topicframe;
	frame		*leftframe, *infoframe, *statusframe,
			*outputframe, *statframe, *userframe;
	menubutton	*chanmode;
	irctext		*output, *info;
	label		*topiclabel, *modelabel, *keylabel, *limitlabel;
	entry		*topicentry, *keyentry, *limitentry;
	plushHistEntry	*in;
	scrollbar	*outputscroll, *infoscroll, *userscroll;
};

#endif // CHANNELWIN_H
