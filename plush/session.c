#include "session.h"

session::session(): handler()
{
	root = new toplevel(".", "");
	prelude();
}
session::~session()
{
	delete root;
}

void	session::prelude(void)
{
}

int	session::main(void)
{
	return 0;
}

void	session::run(void)
{
	main();
	tk->loop();
}

void	session::exit(int code)
{
	kit::exit(code);
}

#define stat_fmt(type, c, d)	printf(" %-15s %-7d  %-7d       %-7d\n", \
                                       type, c, d, c-d);
void	session::statistics(void)
{
	cout << "Garbage statistics" << endl;
	cout << "---------------------------------------------" << endl;
	cout << "                 created  deleted       remaining" << endl;
	stat_fmt("KitObject", KitObject::created, KitObject::deleted);
	stat_fmt("kit", kit::created, kit::deleted);
	stat_fmt("event", event::created, event::deleted);
	stat_fmt("binding", binding::created, binding::deleted);
	stat_fmt("handler", handler::created, handler::deleted);
	stat_fmt("widget", widget::created, widget::deleted);
	stat_fmt("button", button::created, button::deleted);
	stat_fmt("checkbutton", checkbutton::created, checkbutton::deleted);
	stat_fmt("entry", entry::created, entry::deleted);
	stat_fmt("frame", frame::created, frame::deleted);
	stat_fmt("label", label::created, label::deleted);
	stat_fmt("listbox", listbox::created, listbox::deleted);
	stat_fmt("menu", menu::created, menu::deleted);
	stat_fmt("menubutton", menubutton::created, menubutton::deleted);
	stat_fmt("message", message::created, message::deleted);
	stat_fmt("radiobutton", radiobutton::created, radiobutton::deleted);
	stat_fmt("scrollbar", scrollbar::created, scrollbar::deleted);
	stat_fmt("text", text::created, text::deleted);
	stat_fmt("tixBalloon", tixBalloon::created, tixBalloon::deleted);
	stat_fmt("tixLabelFrame", tixLabelFrame::created, 
	                          tixLabelFrame::deleted);
	stat_fmt("tixNoteBook", tixNoteBook::created, 
	                        tixNoteBook::deleted);
	stat_fmt("tixPanedWindow", tixPanedWindow::created, 
	                           tixPanedWindow::deleted);
	stat_fmt("toplevel", toplevel::created, toplevel::deleted);

}
