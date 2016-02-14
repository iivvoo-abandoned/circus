#ifndef RADIOBUTTON_H
#define RADIOBUTTON_H

#include "button.h"

//
// Hush derives radiobutton from class button, which seems reasonable, as
// they share some common properties

// hush also implements toggle and value,
// however, hush's radiobutton implementation does not work.
// First of all, there is no widgetcommand 'toggle', and you
// need to specify a variablename associated with each radiobutton
// (radiobuttons only make sense if they're grouped!)
//
// This basically means I will have to redesign this class..
//

class	radiobutton: public button
{
public:
	radiobutton(const char * = NULL, const char * = NULL);
	radiobutton(widget *, const char * = NULL, const char * = NULL);
        virtual ~radiobutton();

	void	select(void) const;
	void	deselect(void) const;
	void	group(radiobutton *);
	void	value(const char *);
	char	*value(void) const;

static	int	created = 0;
static	int	deleted = 0;

private:

	cstring	_var;
};

#endif // RADIOBUTTON_H
