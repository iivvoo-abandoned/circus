#ifndef _TARGET_H
#define _TARGET_H

#include <vector.h>
#include <map.h>

#include "cstring.h"
#include "channelwin.h"
#include "querywin.h"
#include "user.h"
#include "misc.h"

#include "targetif.h"

// Channelmodes

#define		CMODE_INVITE	0x0001
#define		CMODE_KEY	0x0002
#define		CMODE_LIMIT	0x0004
#define		CMODE_MODERATE	0x0008
#define		CMODE_NO_MSG	0x0010
#define		CMODE_PRIVATE	0x0020
#define		CMODE_SECRET	0x0040
#define		CMODE_TOPIC	0x0080

// Channelstates

#define 	STATE_CLEAR	0x0000
#define		STATE_KICKED	0x0001
#define		STATE_NOTJOIN	0x0002

/*
 * Implement the target-object
 *
 */

class	channel_target: public channelwin, public targetif
{
public:
	channel_target(cstring w, int sessionid);
	~channel_target();
	user_info	*find(cstring);
	void		disable(void);
	int		wantinfo(int, char **);
	void		setstate(const int); 
	int		getstate(void);
	void		set_mymode(int);
	void		del_mymode(int);
	int		get_mymode(void);
	void		set_op(int);
	void		set_voice(const int);
	void		set_cmode(int);
	void		del_cmode(int);
	int		get_cmode(void);
	void		set_key(cstring);
	cstring		&get_key(void);
	void		set_topic(const cstring);
	cstring		&get_topic(void);
	void		set_limit(const int);
	int		get_limit(void);
	vector<cstring *>	&get_ban_list(void);
	void		add_ban(const cstring);
	void		del_ban(const cstring &);
	void		add_user(cstring, const int index = 0,
				 const int mode = 0);
	void		del_user(const cstring &);
	cstring		get_userlogin(const cstring &);
	cstring		get_userhost(const cstring &);
	void		change_user(const cstring &, const cstring &);
	void		set_umode(const cstring &, const int);
	int		get_umode(const cstring &);
	void		del_umode(const cstring &, const int);
	void		setfocus(int);
	void		toggle_autoraise(void);
	void		toggle_lock(void);
	int		get_deiconify(void);
	int		get_autoraise(void);

	void	insert(const cstring &, const cstring & = "");
	void	insert(const cstring &, const cstring &, 
	                const unsigned int, const cstring & = "");
	void	info_insert(const cstring &);

	char	*getinput(void) { return channelwin::getinput(); }
	void	delinput(void) { channelwin::delinput(); }
	void	nickchange(const cstring &, const cstring &);
	void	quituser(const cstring &, const cstring &);

	void	clear(void) { channelwin::clear(); }
	void	destroy(void) { channelwin::destroy(); }

	void	current(int a, int b=-1) { channelwin::current(a, b);}

	map<cstring, KitObject *, mapcmp>	usr_list;
	cstring		channelname;
protected:
	int		do_autoraise;
	int		do_deiconify;
private:
	int		info_mode;
	int		info_who;
	int		info_names;
	int		info_bans;
	int		user_count;
	int		my_mode;
	int		mode, limit;
	int		status;
	int		havefocus;
	cstring		key, topic;
	vector<cstring *>	ban_list;
};

#endif // _TARGET_H
