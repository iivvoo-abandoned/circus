#ifndef CTCPHANDLER_H
#define CTCPHANDLER_H

#include <plush.h>
#include <map.h>

#include "cstring.h"
#include "misc.h"

#include "targets.h"

class	ctcphandler: public virtual targets
{
	class	func_packer: public KitObject
	{
	public:
		func_packer(ctcphandler *h, 
	            int (ctcphandler:: *f)(const cstring &, const cstring &,
		         const cstring &, const cstring &)): KitObject()
		{
			_c = h;		// class
			_m = f;		// method
			_c->_register(this);
		};
		~func_packer()
		{};
		int	execute(const cstring &from, const cstring &to,
		                const cstring &cmd, const cstring &rest)
		{
			return (_c->*_m)(from, to, cmd, rest);
		};
	private:
		ctcphandler	*_c;
		int	(ctcphandler::* _m)(const cstring &, const cstring &,
		                             const cstring &, const cstring &);
	};
public:
	ctcphandler();
	~ctcphandler();
	int dispatch_ctcp(const cstring &, const cstring &, 
	                  const cstring &, const cstring &);
	virtual	int operator()();

private:
	int		ctcp_action(const cstring &, const cstring &, 
	                            const cstring &, const cstring &);
	int		ctcp_version(const cstring &, const cstring &, 
	                            const cstring &, const cstring &);
	int		ctcp_clientinfo(const cstring &, const cstring &, 
	                            const cstring &, const cstring &);
	int		ctcp_finger(const cstring &, const cstring &, 
	                            const cstring &, const cstring &);
	int		ctcp_dcc(const cstring &, const cstring &, 
	                            const cstring &, const cstring &);
	int		ctcp_ping(const cstring &, const cstring &, 
	                            const cstring &, const cstring &);
	int		ctcp_reply(const cstring &, const cstring &,
	                              const cstring &, const cstring &);
	map<cstring, KitObject *, mapcmp>	ctcp_table;
};


#endif // CTCPHANDLER_H
