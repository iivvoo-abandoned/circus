#include <iostream.h>

#include <tcl.h>
#include <tk.h>

#include "session.h"

#include "widgets.h"
#include "handler.h"

#include "filechooser.h"

class	c_handler: public handler
{
public:
	c_handler(): handler()
	{}
	~c_handler() {}
	int operator()()
	{
		cout << "I'm c_handler::operator()()" << endl;

		filesel load("/",
		            filesel::require_nodir|filesel::require_exist);

		cstring name;
		
		if(load.get(name))
			cout << "SELECTED " << name << endl;
		return TCL_OK;
	}
private:
};

class	e_handler: public handler
{
public:
	e_handler(entry *w): handler()
	{
		_w = w;
		m = NULL;
	}
	~e_handler() {
		if(m)
			delete m;
	}
	int	operator()()
	{
		cout << "I'm e_handler::operator()()" << endl;
		cout << "The entry contained " << _w->get() << endl;
		_w->del();

		m = new menu(".menu");
		menu	*m2 = new menu(m, ".menu");

		m->entry("Optie 1", "option1");
		m->entry("Optie 2", "option2");
		m->entry("Optie 3", "option3");
		m->cascade("Submenu!!", m2);

		m2->entry("Suboption 1");
		m2->entry("Suboption 2");
		m2->separator();
		m2->entry("Suboption 3");
		m2->entry("Suboption 4");

		m->entry("Optie 4", "option4");
		m->separator();
		m->entry("Bye", "last_option");

		m->post(_event->x_root(), _event->y_root());
		return TCL_OK;
	}
private:
	entry	*_w;
	menu	*m;
};

/*
 * Handler that opens a filechooser. Can be bound to, for example, buttons.
 */
class	browse_handler: public handler
{
public:
	browse_handler()
	{}
	~browse_handler()
	{}

	int operator()()
	{
		filesel load("/",
		            filesel::require_nodir|filesel::require_exist);

		cstring name;
		
		if(load.get(name))
			cout << "SELECTED " << name << endl;
		return TCL_OK;
	}
};

class	application: public session
{
public:
	application(): session()
	{
		tk->trace(KIT_TRACE);
	}
	~application()
	{}
	void	create_menu(widget *container)
	{
		frame *menuframe = new frame(container, ".menuframe", 
		                             "-relief raised -border 1");
		menubutton *m_file = new menubutton(menuframe, ".file");
		m_file->text("File");
		menu *filemenu = new menu(m_file, ".fm");
		filemenu->entry("Open...");
		filemenu->separator();
		filemenu->entry("Exit");
		filemenu->bind(this);

		m_file->menu(filemenu);

		menubutton *m_widget = new menubutton(menuframe, ".widget");
		m_widget->text("Widget");

		menubutton *m_help = new menubutton(menuframe, ".help");
		m_help->text("Help");
		menu *helpmenu = new menu(m_help, ".mh");
		helpmenu->entry("About");
		helpmenu->bind(this);

		m_help->menu(helpmenu);

		m_file->pack("-side left");
		m_widget->pack("-side left");
		m_help->pack("-side right");

		menuframe->pack("-side top -expand true -fill x");
	}

	void	intro(tixNoteBook *n)
	{
		frame *f = n->add("intro", "-label Intro");

		label *l = new label(f, ".info", "-relief sunken -border 1");

		l->text("\n"
		  "\n"
                  "                 Plush Demo\n"
                  "\n"
                  "   What you are about to see is a short demonstration of  \n"
                  "   the widgets that are currently implemented in PLUSH,   \n"
                  "   the Portable Lightweight Utility Shell.\n"
                  "\n"
                  "   More information can be found at:\n"
                  "\n"
                  "   http://www.nip.nl/~ivo/plush\n"
                  "\n");

		l->pack("-expand true -fill both");
		// nothing else to pack (!!)
	}
	void	tcltk(tixNoteBook *n)
	/*
	 * Display the implemented tk widgets
	 */
	{
		frame *f = n->add("tcltk", "-label {TCL/TK}");

		frame *buttonframe = new frame(f, ".buttonframe");
		button *b1 = new button(buttonframe, ".b");
		b1->text("Buttons");
		button *b2 = new button(buttonframe, ".b2");
		b2->text("Browse");

		handler *h = new browse_handler();
		_register(h);		// explicit garbage collection
		b2->bind(h);

		frame *radioframe = new frame(f, ".radioframe",
		                              "-border 1 -relief sunken");
		label *l1 = new label(radioframe, ".l");
		l1->text("radiobuttons: ");

		radiobutton *r1 = new radiobutton(radioframe, ".b1", 
		                                  "-text radio1");
		radiobutton *r2 = new radiobutton(radioframe, ".b2", 
		                                  "-text radio2");
		radiobutton *r3 = new radiobutton(radioframe, ".b3", 
		                                  "-text radio3");
		// group them
		r2->group(r1);
		r3->group(r1);

		frame *checkframe = new frame(f, ".checkframe",
		                              "-border 1 -relief sunken");
		label *l2 = new label(checkframe, ".l");
		l2->text("checkbuttons: ");
		checkbutton *c1 = new checkbutton(checkframe, ".c1");
		c1->text("a");
		checkbutton *c2 = new checkbutton(checkframe, ".c2");
		c2->text("b");
		checkbutton *c3 = new checkbutton(checkframe, ".c3");
		c3->text("c");

		entry *e = new entry(f, ".ent", "-width 20");
		e->insert("Type input here...");

		text *t = new text(f, ".txt", "-width 20 -height 10");
		t->insert("end", "A text widget. Very powerfull!");

		scrollbar *s = new scrollbar(f, ".scroll");
		s->yview(t);
		t->yscroll(s);

		listbox *l = new listbox(f, ".list");
		l->insert("tcl");
		l->insert("tk");
		l->insert("tix");
		l->insert("PLUSH!");

		// pack everything
		b1->pack("-side left -expand true");
		b2->pack("-side right -expand true");

		buttonframe->pack("-expand true -fill both");
		l1->pack("-side left");
		r1->pack("-side right");
		r2->pack("-side right");
		r3->pack("-side right");
		radioframe->pack("-side top -expand true -fill x");

		l2->pack("-side left");
		c1->pack("-side right");
		c2->pack("-side right");
		c3->pack("-side right");
		checkframe->pack("-side top -expand true -fill x");

		e->pack("-expand true -fill x");
		t->pack("-side left -expand true -fill both");
		s->pack("-side left -expand true -fill y");
		l->pack("-side right -expand true");

		
	}
	void	tix(tixNoteBook *n)
	{
		frame *f = n->add("tix", "-label {Tix}");

	}
	void	create_notebook(widget *container)
	{
		tixNoteBook *n = new tixNoteBook(container, ".note");

		intro(n);
		tcltk(n);
		
		frame *demoframe = n->add("demo1", "-label {Misc stuff}");

		tixPanedWindow *p = new tixPanedWindow(demoframe, ".p");

		frame *f = p->add("f1");
		frame *ff = p->add("ff");

		tixLabelFrame *lf = new tixLabelFrame(demoframe, ".lf");
		frame *f2 = lf->subframe();

		lf->setlabel("Radio buttons");

		button	*b = new button(f, ".b", "-text \"Hello World!\"");
		b->pack("-side left");

		button *c = new button(f);
		c->text("Hello everyone");

		handler	*ch = new c_handler();
		c->_register(ch);
		c->bind(ch);
		c->pack("-side top");

		entry *e = new entry(ff, ".entry", "-relief raised");
		e->insert("Gaws gash gosh gish geez");

		handler *eh = new e_handler(e);
		e->_register(eh);
		e->bind(eh, "InputTyped");
		e->pack("-side bottom -fill x -expand true");
		p->pack("-expand true -fill both");

		radiobutton *b1 = new radiobutton(f2, ".b1", "-text radio1");
		radiobutton *b2 = new radiobutton(f2, ".b2", "-text radio2");
		radiobutton *b3 = new radiobutton(f2, ".b3", "-text radio3");
		radiobutton *b4 = new radiobutton(f2, ".b4", "-text radio4");

		// Create balloon
		tixBalloon *bal = new tixBalloon(b3, "-msg {This is radio 3}");
		b2->group(b1);
		b3->group(b1);
		b4->group(b1);
		
		b2->select();

		b1->pack("-side left");
		b2->pack("-side left");
		b3->pack("-side left");
		b4->pack("-side left");
		lf->pack("-expand true -fill both");

		n->pack("-expand true -fill both");
	}

	int	operator()()
	{
cout << "Poep: " << _event->argc() << endl;

		char **argv = _event->argv();
		int argc = _event->argc();

		if(argc == 2)
		{
			cstring ev = argv[1];

			if(ev == "Exit")
				tk->quit();
			else if(ev == "About")
				; // fill in later.
		}
		return TCL_OK;
	}
	virtual	int	main(void)
	{
		create_menu(root);
		create_notebook(root);

		return 0;
	}
private:
};


// user should overload class session (derive class from it) to create
// app (which is done by overloading session::main

int	main(int, char *)
{
	application	*a = new application();
	a->run();
	delete a;

	session::statistics();
	session::exit();
}
