#ifndef CHECKBUTTON_H
#define CHECKBUTTON_H

#include "button.h"

//
// Hush derives checkbutton from class button, which seems reasonable, as
// they share some common properties


class	checkbutton: public button
{
public:
	checkbutton(const char * = NULL, const char * = NULL);
	checkbutton(widget *, const char * = NULL, const char * = NULL);
        virtual ~checkbutton();

	void	variable(const char *);
	void	select(void) const;
	void	deselect(void) const;
	void	toggle(void) const;

	char	*value(void) const;
static	int	created = 0;
static	int	deleted = 0;

private:
	cstring	_var;

};

#endif // CHECKBUTTON_H
