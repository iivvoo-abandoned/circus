#ifndef TARGETS_H
#define TARGETS_H

#include <plush/garbage.h>

#include "connection.h"
#include "cstring.h"
#include "channel_target.h"
#include "querywin.h"		// vs target.h
#include "misc.h"

#include "target_list.h"

class	targets: public connection
{
	class	func_packer: public KitObject
	{
	public:
		func_packer(targets *h, 
	                    int (targets:: *f)(int, char **)): KitObject()
		{
			_c = h;		// class
			_m = f;		// method
			_c->_register(this);
		};
		~func_packer()
		{};
		int	execute( int argc, char **argv)
		{
			return (_c->*_m)(argc, argv);
		};
	private:
		targets	*_c;
		int		(targets::* _m)(int, char **);
	};

public:
	targets();
	targets(tixNoteBook *, int);
	~targets();
	void	reset(void);
	int	operator()();
	int	addchannel(const cstring &, const cstring & = "");
	int	delchannel(const cstring &);
	channel_target	*createchannel(const cstring &);
	void	removechannel(const cstring &, const cstring & = "");
	void	userjoin(const cstring &, const cstring &);
	void	userleave(const cstring &, const cstring &, 
	                  const cstring & = "");
	void	kickuser(const cstring &, const cstring &,
	                 const cstring &, const cstring &);

	int	insert_info(cstring, const cstring &);
	int	insert_msg(cstring, const cstring &);

	void	recvmsg(const cstring &, const cstring &, const cstring &);
	void	sendmsg(const cstring &, const cstring &);
	void	recvnotice(const cstring &, const cstring &, const cstring &);
	void	sendnotice(const cstring &, const cstring &);
	void	recvaction(const cstring &, const cstring &, const cstring &);
	void	sendaction(const cstring &, const cstring &);

	int	selection_togglemode(const cstring &, const int);
	int	selection_setmode(const cstring &, const int, const int);
	int	selection_operate(const cstring &, const int &);

/*
 * queries stuff
 */
	int		addquery(const cstring &which);
	int		delquery(const cstring &which);
	
	void		toggle_message_window(void);
	void		close_message(void);
	void		close_query(cstring);
	void		toggle_dcc(const cstring &);
	void		toggle_lock(const cstring &);
	void		toggle_chat(const cstring &);
	void		handle_input(const cstring &);

	void	set_current(const cstring &, int = true);

	int		whoisinfo(int, char **);

	int		wantinfo(int, char **);
	void		nickchange(const cstring &, const cstring &);
	messagewin	*create_message_window(void);
	void		clear(const cstring &);
	void		recv(const cstring &, const cstring &, const cstring &);
	void		send(const char, const cstring &, const cstring &);
	void		recvchat(const cstring &, const cstring &);
	void		sendchat(const cstring &, const cstring &);
	void		popup_menu(int, int);

	int		handle_command(int, char **);

/*
 * end of queries stuff
 */
	void	bind(handler *);

protected:
	target_list		targetlist;

private:
	handler			*targetcallback;

	// Handlers for channel window events
	int	chan_topic_changed(int, char **);
	int     chan_key_changed(int, char **);
	int     chan_limit_changed(int, char **);
	int     start_query(int, char **);
	int	chan_cmode_i_changed(int, char **);
	int	chan_cmode_k_changed(int, char **);
	int	chan_cmode_l_changed(int, char **);
	int	chan_cmode_m_changed(int, char **);
	int	chan_cmode_n_changed(int, char **);
	int	chan_cmode_p_changed(int, char **);
	int	chan_cmode_s_changed(int, char **);
	int	chan_cmode_t_changed(int, char **);
	int	leave_channel(int, char **);
	int	take_focus(int, char **);
	int	lose_focus(int, char **);
	int	toggleraise(int, char **);
	int	togglelock(int, char **);
	int	openmenu(int, char **);
	int	whoisselected(int, char **);
	int	banselected(int, char **);
	int	queryselected(int, char **);
	int	opselected(int, char **);
	int	deopselected(int, char **);
	int	kickselected(int, char **);
	int	kickbanselected(int, char **);
	int	voiceselected(int, char **);
	int	muteselected(int, char **);
	int	ctcpselected(int, char **);
	int	handlemode(int, char **, int);

	int	handle_input(int, char **);


	map<cstring, KitObject *, mapcmp>	dispatch_table;
	void		selection_operate(const int&, const cstring & = "");
	void		selection_ctcp(const cstring &);
	messagewin		*messages;

	int	handlechanwinevent(void);
static	int	chanwinevent(void *h, event *e);
};

#endif // TARGETS_H
