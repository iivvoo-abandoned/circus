#ifndef CONNECTION_H
#define CONNECTION_H

#include "servergui.h"          // the gui of this connection
#include "ircserver.h"          // the tcp connection of this connection

class	connection: public servergui, public ircserver
{
public:
	// constructors
	connection();				// default, not to be used!
	connection(tixNoteBook *, int);

	// destructor
	virtual ~connection();

	// do it
	void		init(const char *, const char *, unsigned int);
	void		start(void);
	void		reregister(void);
	void		connected(void);
	void		disconnected(const cstring &, int = true);
	void		setnick(const cstring &);
	void		setaway(const cstring &);

	// public accessible strings
	
	cstring		nick;
	cstring		login;
	cstring		ircname;
	cstring		fingerinfo;
	cstring		userinfo;

	cstring		scriptdir;
	cstring		downloaddir;
	cstring		signoff;

	int		get_mode(void) const;
	void		set_mode(const int m);
	void		del_mode(const int m);

	int		operator()();

	void		add_target(const cstring &);
	void		del_target(const cstring &);
	void		ren_target(const cstring &, const cstring &);
	void		set_current(const cstring);

	int		connect_server(void);
	void		update_title(void);
	int		reconnect_dialog(const cstring &);

	int		sessionid;
protected:			
	cstring		current;
	cstring		awaymsg;
	int		deathwish;

private:

	int		umode;

	cstring		serverhost;
	unsigned int	serverport;

static	int		count;
};

#endif // CONNECTION_H
