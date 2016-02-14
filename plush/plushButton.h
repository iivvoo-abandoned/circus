#ifndef PLUSHBUTTON_H
#define PLUSHBUTTON_H

#include "button.h"

/*
 * plushButton: 
 *
 */

class	plushButton: public button
{
public:
	plushButton();
	plushButton(char* p, char* options = "");
	plushButton(widget* w, char* p, char* options = "");
	~plushButton();
	void 	image(char *);
	void	help_image(char *);

protected:
	virtual	event	*dispatch(event *);

private:
	void	init(void);
	widget	*b;
	cstring	img;
	cstring	help_img;
};

#endif // PLUSHBUTTON_H
