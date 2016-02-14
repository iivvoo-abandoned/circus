#ifndef _USER_H_
#define _USER_H_

#include <iostream.h>

#include <plush.h>

#include "cstring.h"

#define UMODE_OP	1
#define UMODE_VOICE	2

class user_info: public KitObject
{
public:
	user_info(void){login="";host="";ircname="";umode = 0;};
	~user_info() {
	};
	void	set_umode(int m){umode |= m;};
	void	del_umode(int m){umode &=~m;};
	int	get_umode(void){return umode;};

	int	index;
	cstring	login;
	cstring	host;
	cstring	ircname;
private:
	int 	umode;
};

#endif // _USER_H_
