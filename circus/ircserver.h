#ifndef _IRCSERVER_H
#define _IRCSERVER_H

#include <plush.h>
#include <plush/event.h>

#include "io.h"
#include "cstring.h"

class 	ircserver: public virtual handler, public io
{
public:
	ircserver();
	~ircserver();
	ircserver(const cstring &, unsigned int);
	void	init(const cstring &, unsigned int);
	void	abort(void);
	int	establish(void);
	int	operator()();

	void	bind(unsigned int, handler *);
	void	bindfd(void);

	int	state(int = -1);
	void	setstate(int);

	// Server commands stuff
	void	away(const cstring &);
	void	invite(const cstring &, const cstring &);
	void	ison(const cstring &);
	void	join(const cstring &, const cstring &);
	void	kick(const cstring &, const cstring &, const cstring &);
	void	nick(const cstring &);
	void	mode(const cstring &, const cstring & = "");
	void	notice(const cstring &, const cstring &);
	void	oper(const cstring &, const cstring &);
	void	part(const cstring &, const cstring & = "");
	void	pong(const cstring &);
	void	privmsg(const cstring &, const cstring &);
	void	raw(const cstring &);
	void	reguser(const cstring &, const cstring &, const cstring &);
	void	topic(const cstring &, const cstring & = "");
	void	quit(const cstring &);
	void	whois(const cstring &);
	void	who(const cstring &);

	// Less direct servercommands
	void	ctcpreply(const cstring &, const cstring &, const cstring &);
	void	ctcp(const cstring &, const cstring &, const cstring & = "");

	int	geterror(void) const;
	const 	char	*geterrorstr(void) const;
	enum	{ disconnected, connecting, connected };

protected:
	int		fd;
	int		last_error;
	int		_state;

	cstring		_host;
	unsigned int 	_port;
};

#endif // _IRCSERVER_H
