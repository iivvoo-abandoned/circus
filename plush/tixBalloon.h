#ifndef TIXBALLOON_H
#define TIXBALLOON_H

#include "frame.h"
#include "cstring.h"

class	tixBalloon: public frame
{
public:
	tixBalloon(const char *, const char * = NULL);
	tixBalloon(widget *, const char *, const char * = NULL);
        virtual ~tixBalloon();
	void	initwait(int i);
	void	bind(widget *, const char *);
	void	bind(widget *, const char *, const char *);
	void	unbind(widget *);
	void	statusbar(widget *);

static	int	created = 0;
static	int	deleted = 0;

private:
};

#endif // TIXBALLOON_H
