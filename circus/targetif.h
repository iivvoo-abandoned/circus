#ifndef TARGETIF_H
#define TARGETIF_H

#include "cstring.h"

class targetif
{
public:
	enum { channel_target, query_target, message_target };

	targetif(int type = targetif::channel_target)
	{
		_type = type;
	}
	virtual ~targetif()
	{}

	virtual	int	type() { return _type; }

	// different types of insert

	virtual	void	insert(const cstring &s, const cstring &t) = 0;
	virtual	void	insert(const cstring &p, const cstring &s,
                        const unsigned int l, const cstring &tagname) = 0;
	virtual	void	info_insert(const cstring &) = 0;

	virtual void	nickchange(const cstring &, const cstring &) = 0;
	virtual void	quituser(const cstring &, const cstring &) = 0;

	virtual void	change_user(const cstring &, const cstring &) = 0;
	virtual	void	del_user(const cstring &) = 0;

	virtual	void	clear(void) = 0;
	virtual	void	disable(void) = 0;
	virtual	void	destroy(void) = 0;

	virtual	void	current(int, int) = 0;

	virtual	char	*getinput(void) = 0;
	virtual	void	delinput(void) = 0;
private:
	int	_type;
};

#endif // TARGETIF_H
