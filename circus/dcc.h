#ifndef _DCC_H
#define _DCC_H

#include <plush.h>
#include <plush/garbage.h>
#include <map.h>
#include <vector.h>

#include "if_dcc.h"

#include "cstring.h"
#include "misc.h"

#include "io.h"

class	dcc_session: public handler
{
public:
	dcc_session(const cstring &);
	virtual ~dcc_session();

	int	type(void);
	// dcc types
	enum	{ none, chat, send, get };
	// dcc states
	enum	{ dcc_wait = 1, dcc_offered = 2, 
	          dcc_active = 4, dcc_delete = 8 };

	virtual	int	accept_request(char *, int, char * ="");
	virtual	int	start_connection(const cstring &, 
	                                 const cstring & = "chat",
					 int = 0);
	virtual	void	close(void) = 0;
	char	*nick(void);
	void	uhost(char *);
	char	*uhost(void);
	int	state(void);
	void	state(int);
protected:
	int	operator()();
	int	_type;
	int	_read;
	int	_sent;
	int	_sock;
	int	_state;		
	
	io	link;
	cstring	_nick;
	cstring	_uhost;
};

class	dcc_chat: public dcc_session
{
public:
	dcc_chat(const cstring &);
	~dcc_chat();
	int	accept_request(char *, int, char * = "");
	int	start_connection(const cstring &, const cstring & = "chat",
	                         int = 0);
	int 	operator()();
	void	close(void);
	int	msg(const char *);
private:
	cstring	_buf;
	dcc_meter	*meter;
};

class	dcc_send: public dcc_session
{
public:
	dcc_send(const cstring &);
	~dcc_send();

	int	start_connection(const cstring &, const cstring &file, 
	                         int = 0);
	int 	operator()();
	int	size;
	char	*file(void);
	void	close(void);
private:
	int	transferblock(void);
	cstring	_local;			// filename
	int	_file;			// fd of file to up-/download
	dcc_meter	*meter;
};

class	dcc_get: public dcc_session
{
public:
	dcc_get(const cstring &);
	~dcc_get();

	int	accept_request(char *, int, char *file);
	int 	operator()();
	int	size;
	char	*file(void);
	void	close(void);
private:
	int	transferblock(void);
	cstring	_local, _remote;	// filename
	int	_file;			// fd of file to up-/download
	dcc_meter	*meter;
};

class	dcc_handler: public handler
{
	class	func_packer: public KitObject
	{
	public:
		func_packer(dcc_handler *h, 
	            int (dcc_handler:: *f)(int, char **)): KitObject()
		{
			_c = h;		// class
			_m = f;		// method
			_c->_register(this);
		};
		~func_packer()
		{
		};
		int	execute(int argc, char **argv)
		{
			return (_c->*_m)(argc, argv);
		};
	private:
		dcc_handler	*_c;
		int	(dcc_handler::* _m)(int, char **);
	};
public:
	dcc_handler();
	~dcc_handler();

	void	init(void);
	dcc_session	*find(char *, char * = NULL);
	int 	operator()();
private:
	int			dcc_handle_chat(int, char **);
	int			dcc_handle_close(int, char **);
	int			dcc_handle_get(int, char **);
	int			dcc_handle_msg(int, char **);
	int			dcc_handle_send(int, char **);
	int			dcc_handle_status(int, char **);

	map<cstring, KitObject *, mapcmp>	dcc_table;
	vector<KitObject *>	sessions;
};

#endif // _DCC_H
