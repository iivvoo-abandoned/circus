#ifndef BUTTON_H
#define BUTTON_H

#include "widget.h"

class	button: public widget
{
public:
	button(const char * = NULL, const char * = NULL, int = true);
	button(widget *, const char * = NULL, const char * = NULL, int = true);
        virtual ~button();
	void	text(const char *) const;
	void	flash(void) const;
	void	invoke(void) const;

static	int	created = 0;
static	int	deleted = 0;

private:
};

#endif // BUTTON_H
