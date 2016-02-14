#ifndef CMDHANDLER_H
#define CMDHANDLER_H

#include <map.h>
#include <plush/garbage.h>

#include "cstring.h"
#include "targets.h"

#include "misc.h"

class	cmdhandler: public virtual targets
{
	class	func_packer: public KitObject
	{
	public:
		func_packer(cmdhandler *h, 
	            int (cmdhandler:: *f)(const cstring &, const cstring &)): KitObject()
		{
			_c = h;		// class
			_m = f;		// method
			_c->_register(this);
		};
		~func_packer()
		{
		};
		int	execute(const cstring &cmd, const cstring &rest)
		{
			return (_c->*_m)(cmd, rest);
		};
	private:
		cmdhandler	*_c;
		int	(cmdhandler::* _m)(const cstring &, const cstring &);
	};
public:
	cmdhandler();
	~cmdhandler();
	virtual int operator()();

private:
	void	init_command_table(void);
	int 	dispatch_command(const cstring &, const cstring &);
	int	do_away(const cstring &, const cstring &);
	int	do_clear(const cstring &, const cstring &);
	int	do_dcc(const cstring &, const cstring &);
	int	do_ctcp(const cstring &, const cstring &);
	int	do_ping(const cstring &, const cstring &);
	int	do_disconnect(const cstring &, const cstring &);
	int	do_join(const cstring &, const cstring &);
	int	do_kick(const cstring &, const cstring &);
	int	do_leave(const cstring &, const cstring &);
	int	do_load(const cstring &, const cstring &);
	int	do_me(const cstring &, const cstring &);
	int	do_mode(const cstring &, const cstring &);
	int	do_msg(const cstring &, const cstring &);
	int	do_nick(const cstring &, const cstring &);
	int	do_notice(const cstring &, const cstring &);
	int	do_oper(const cstring &, const cstring &);
	int	do_query(const cstring &, const cstring &);
	int	do_quit(const cstring &, const cstring &);
	int	do_server(const cstring &, const cstring &);
	int	do_topic(const cstring &, const cstring &);
	int	do_umode(const cstring &, const cstring &);
	int	do_whois(const cstring &, const cstring &);
	int	do_invite(const cstring &, const cstring &);

	/*
	 * handlers
	 */

	int	handleCmdline(void);
	int	handleCmdbind(void);
static	int	statHandleCmdline(void *h, event *e);
static	int	statHandleCmdbind(void *h, event *e);

	garbage<cstring *>	tclgarb;
	map<cstring, KitObject *, mapcmp>	command_table;
	map<cstring, cstring *, mapcmp>		tcl_table;
};

#endif // CMDHANDLER_H
