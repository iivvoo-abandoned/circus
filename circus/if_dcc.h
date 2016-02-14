#ifndef _IF_DCC_H
#define _IF_DCC_H

#include <plush.h>
#include "cstring.h"

int	dcc_acceptchat(const cstring &, const cstring &, const cstring &);
int	dcc_acceptfile(const cstring &, cstring &, const cstring &, 
                       const cstring &, const int);

class dcc_meter: public toplevel
{
public:
	dcc_meter();
	~dcc_meter();
	void	message(char *);
	void	build(void);
	void	setmax(int);
	void	update(float, int, int);
	void	info(const cstring &, const cstring &);
	int	operator()();
	void	bind(handler *h);
private:
	float	_prevperc;
	int	_cwidth, _cheight;
	frame	*topframe, *buttonframe;
	button	*cancelbutton;
	label	*msg, *state;

	cstring	nick;
	cstring	file;
static	int	_serial;
};

#endif // _IF_DCC_H
