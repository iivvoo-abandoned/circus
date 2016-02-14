#ifndef SERVERHANDLER_H
#define SERVERHANDLER_H

#include <map.h>
#include <plush/handler.h>

#include "cstring.h"
#include "ctcphandler.h"

const   int     buffersize = 1024;

class	serverhandler: public ctcphandler
{
	class	func_packer: public KitObject
	{
	public:
		func_packer(serverhandler *h, 
	            	int (serverhandler:: *f)(int, char **)): KitObject()
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
		serverhandler	*_c;
		int		(serverhandler::* _m)(int, char **);
	};
public:
	serverhandler();
	virtual ~serverhandler();

	virtual	int operator()();
	void		error(const char *);
private:
	int     	dispatch_msg(int, char **);
	int		handlemessage(const cstring &);
	char		buffer[buffersize + 1];
	unsigned int 	bytes;

	// Handlers for server commands

	int	dispatch_error(int, char **);
	int	dispatch_join(int, char **);
	int	dispatch_kick(int, char **);
	int	dispatch_leave(int, char **);
	int	dispatch_mode(int, char **);
	int	dispatch_nick(int, char **);
	int	dispatch_ping(int, char **);
	int	dispatch_privmsg(int, char **);
	int	dispatch_topic(int, char **);
	int	dispatch_quit(int, char **);
	int	dispatch_invite(int, char **);

	// numeric replies
	int	dispatch_welcome(int, char **);
	int	dispatch_namereply(int, char **);
	int	dispatch_rpltopic(int, char **);
	int	dispatch_chancreated(int, char **);
	int	dispatch_topicset(int, char **);
	int	dispatch_rplchanmodeis(int, char **);
	int	dispatch_rplumodeis(int, char **);
	int	dispatch_isonreply(int, char **);
	int	dispatch_awayreply(int, char **);
	int	dispatch_whoisreply(int, char **);
	int	dispatch_whoreply(int, char **);
	int	dispatch_banlistreply(int, char **);
	int	dispatch_nickchanlock(int, char **);
	int	dispatch_nojoin(int, char **);

	int	handle_reply(int, char **);
	int	expand_reply(cstring &, int, char **);

	cstring			*reg_string(const cstring &);
	void			init_server_dispatch_table(void);
	connection		*c;
	map<cstring, KitObject *, mapcmp>	dispatch_table;
	map<cstring, cstring *, mapcmp>		replies;
	garbage<cstring *>	strings;
};

#endif // SERVERHANDLER_H
