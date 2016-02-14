#include "plushButton.h"

/*
 * plushButton: 
 *
 * Add the possibility to display a different image when the button gets
 * focus, and sink/raise the button if the right button is clicked on it.
 */

plushButton::plushButton(): button()
{ 
	init(); 
}

plushButton::plushButton(char* p, char* options): button(p, options)
{
	init();
}

plushButton::plushButton(widget* w, char* p, char* options): 
	           button(w, p, options)
{
	init();
}

plushButton::~plushButton()
{
}

void 	plushButton::image(char *s)
{
	if(s[0] == '@')
	{
		cstring	f = s+1;
		img = tk->evaluate("image create photo -palette 7/7/5 -file " + f);
		tk->evaluate(img + " redither");
	}
	else
		img = s;
	button::image(img);
}

void	plushButton::help_image(char *s)
{
	if(s[0] == '@')
	{
		cstring	f = s+1;
		help_img = tk->evaluate("image create photo -file "+f);
	}
	else
		help_img = s;
}

event	*plushButton::dispatch(event *_e)
{
	int	result = TCL_OK;

	_event = _e;

	if(_event->argc() > 1)
	{
		cstring	ev = _event->arg(1);

		if(ev == "mouse_enter")
		{
			if(help_img != (const char *)"")
				button::image(help_img);
		}
		if(ev == "mouse_leave")
		{
			if(img != (const char *)"")
				button::image(img);
		}
	}

	result = operator()();
	return (result == TCL_OK)? TCL_OK: _event;
}

void 	plushButton::init(void)
{
	img = "";
	help_img = "";
	bind("<Any-Enter>", this, "mouse_enter");
	bind("<Any-Leave>", this, "mouse_leave");
}

