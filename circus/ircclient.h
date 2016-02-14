#ifndef IRCCLIENT_H
#define IRCCLIENT_H

#include <plush.h>
#include <vector.h>

#include "serverlist.h"
#include "ircsession.h"
#include "masterwindow.h"

class	ircclient: public masterwindow
{
public:
	ircclient(widget *w, char *path);
	~ircclient();

	void	start();

	int	operator()();

private:
	void	changeServer(int, char *, int);

	int	handleServer(void);
static	int	staticHandleServer(void *h, event *e);

	serverlist		serverList;
	vector<ircsession *>	connectionList;
	int		serverserial;

};

#endif // IRCCLIENT_H
