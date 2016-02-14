#ifndef EVENT_H
#define EVENT_H

#include "KitObject.h"
class	event: public KitObject
{
public:
	event();
	virtual ~event();

	int	argc(void) const;
	char	*arg(const int) const;
// available in hush:
//	char	**argv(int idx = 0);
	char	**argv(void) const;

	char	*name(void) const;

	int	mask(void) const;
	int	fd(void) const;

	int	x(void) const;
	int	y(void) const;

	int	x_root(void) const;
	int	y_root(void) const;

	void	setarg(int, char **);
	void	setfd(const int, const int);
	
static	int	created;
static	int	deleted;
private:
	int	_argc;
	char	**_argv;
	int	_fd;
	int	_mask;
};

#endif // EVENT_H
