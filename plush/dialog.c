#include "dialog.h"

/*
 * Dialog box, complex widget with lots of buttons, entries, a bitmap etc.
 *
 * TODO:
 *
 * - Optional helpbutton
 * - Delete buttons, entries (lists) (proper garbage collection)
 * - (done?) Fix problem with path-names if value="" etc. (or contains spaces)
 * - (done?) Handle entries, add a way to retrieve value, fix (weird) aws_entry 
 * - Add comboboxes
 * - (done) behaviour when removing mouse
 * - Default button when pressing enter
 * - textfont for message
 * - other options for entries, buttons etc. that are within the dialog
 * - optional positioning (i.e. middle of screen) of dialog
 */
/*
 * What should this class do?
 *
 * - Build a dialog
 * - Add buttons
 * - Define standard dialogs (OK, YesNo, OkCancel)
 * - Optional helpbutton
 */
aws_dialog::aws_dialog(): toplevel()
{
	init();
}

aws_dialog::aws_dialog(char *p, char *options): toplevel(p, options)
{
	init();
}

aws_dialog::aws_dialog(widget *w, char *p, char *options): toplevel(w,p,options)
{
	init();
}

aws_dialog::~aws_dialog()
{
}

void	aws_dialog::init()
{
	widget_count = 0; // to create unique widgetnames

	choice = "";
	img = "";
	def_button = "";

	textframe = new frame(this, ".textFrame", "-relief raised -border 1");
	pairframe = new frame(textframe, ".pairFrame");
	labelframe = new frame(pairframe, ".labelFrame");
	entryframe = new frame(pairframe, ".entryFrame");
	buttonframe = new frame(this, ".buttonFrame", "-relief raised -border 1");

	imglabel = new label(textframe, ".imgtextLabel");
	textlabel = new label(textframe, ".textLabel");
}


void	aws_dialog::build_dialog(void)
//
// Build the dialog window
//
// Lots of work has been done to make it look properly, i.e. adding
// a seperate frame to seperate text from buttons, lots of positioning, etc.
//
// Why can't I use ipad on the textframe to internally pad the widgets?
{

	// Only pack imagelabel if an image is present, else
	// an empy label will be placed with useless padx/pady
	if(!(img == ""))
		imglabel->pack("-side left -expand true -padx 10 -pady 10");
	textlabel->pack("-expand true -padx 10 -pady 10");

	labelframe->pack("-side left -fill both");
	entryframe->pack("-side left -fill both");
	pairframe->pack("-padx 10 -pady 10");


	textframe->pack("-expand true -fill both");
	buttonframe->pack("-expand true -fill both");
	cstring p = path();
	tk->eval("wm resizable " + p + " false false");

	// Add some bindings, asuming all widgets have been added
	// to the window (we can also do this in wacht()
	unsigned int i, entry_count = entries.size();

	if(entry_count > 0)
	{
		d_entry *d = (d_entry *) entries[0];

		if(d->type == ENTRY)
			d->e->focus();
////		else	// COMBOBOX
////			d->c->focus();
	}

	for(i = 0; i < entry_count; i++)
	{
		d_entry *d = (d_entry *) entries[i];

		if(d->type == ENTRY)
		{
			cstring p = d->e->path();

			if(def_button == "" || i < entries.size() - 1)
				d->e->bind("<Return>",
		                            "focus [ tk_focusNext " + p + " ]");
		}
//		else	// COMBOBOX
//		{
//			cstring p = d->c->path();
//
//			if(def_button == "" || i < entries.size() - 1)
//				d->c->bind("<Return>",
//		                            "focus [ tk_focusNext " + p + ".button ]");
//		}
	}
	if(!(def_button == "") && entry_count > 0)
	{
		d_entry *d = (d_entry *) entries[i-1];

		if(d->type == ENTRY)
			d->e->bind("<Return>", def_button + " invoke");
//		else
//			d->c->bind("<Return>", //			                      def_button + " invoke");
	}
}

int	aws_dialog::operator()()
{
	if(_event->argc() > 1)
		choice = _event->arg(1);

	// store entries, we can't do it after the dialog widget
	// has been destroyed..
	for(unsigned int i = 0; i < entries.size(); i++)
	{
		d_entry *d = (d_entry *) entries[i];

		if(d->type == ENTRY)
			d->value = d->e->get();
//		else	// combobox
//			d->value = d->c->get();
	}
	destroy();
	return TCL_OK;
}

cstring	aws_dialog::wacht(void)
{
	build_dialog();
//	grab();
	wait();
	return choice;
}
	
label	*aws_dialog::message(const cstring &msg, const cstring &labelopts)
{
	textlabel->configure(labelopts);
	textlabel->text((char *)msg);
	return textlabel;
}

aws_button	*aws_dialog::button(const cstring &text, const cstring &value, 
                                    const int isdef, const cstring &buttonopts)
/*
 * Eventhough there can only be one defaultbutton, multiple could be defined.
 * We will always use the last defined default button.
 */
{
	char	buttonpath[64];

	sprintf(buttonpath, ".button%d", widget_count++);

	aws_button *b = new aws_button(buttonframe, buttonpath, buttonopts);
	b->text(text);
	b->bind(this, value);
	b->pack("-side left -expand true -padx 10 -pady 10");
	buttons.insert(buttons.end(), b);
	if(isdef)
	{
		def_button = b->path();
		b->focus();
	}
	return b;
}

aws_entry	*aws_dialog::entry(const cstring &text, const cstring &value,
                                   const cstring &entryopts)
{
	char	ep[64];
	char	lp[64];

	sprintf(ep, ".ep%d", widget_count++);

	sprintf(lp, ".lp%d", widget_count++);

        frame *fl = new frame(labelframe, lp);
	label *l = new label(fl, lp, "-justify right");
	l->text((char *)text);
        frame *fe = new frame(entryframe, ep);
	aws_entry *e = new aws_entry(fe, ep, entryopts);
	e->insert(value);

	l->pack("-side right -pady 2");
	e->pack("-side left -pady 2");
	fl->pack("-fill x");
	fe->pack("-fill x");

	d_entry *d = new d_entry();
//	collector._register(d);
	d->e = e;
	d->value = "";
	d->type = ENTRY;

	entries.insert(entries.end(), d);
	return e;
}

#if 0
aws_combobox	*aws_dialog::combobox(const cstring &text, const cstring &value,
                                      const cstring &comboopts)
{
	char	ep[64];
	char	lp[64];

	sprintf(ep, ".ep%d", widget_count++);

	sprintf(lp, ".lp%d", widget_count++);

        frame *fl = new frame(labelframe, lp);
	label *l = new label(fl, lp, "-justify right");
	l->text((char *)text);
        frame *fe = new frame(entryframe, ep);
	aws_combobox *c = new aws_combobox(fe, ep, comboopts);
	c->insert(value);

	l->pack("-side right -pady 2");
	c->pack("-side left -pady 2");
	fl->pack("-fill x");
	fe->pack("-fill x");

	d_entry *d = new d_entry();
//	collector._register(d);
	d->c = c;
	d->value = "";
	d->type = COMBOBOX;

	entries.insert(d);
	return c;
}
#endif

void	aws_dialog::image(const char *s)
{
	if(s[0] == '@')
	{
		cstring	f = s+1;
		img = tk->evaluate("image create photo -file "+f);
	}
	else
		img = s;
	imglabel->image(img);
}

char	*aws_dialog::getentry(const aws_entry *e)
{
	for(unsigned int i = 0; i < entries.size(); i++)
	{
		d_entry *d = (d_entry *) entries[i];

		if(d->type == ENTRY && e == d->e)
			return d->value;
	}
	return "Invalid Entry";
}

#if 0
char	*aws_dialog::getcombobox(const aws_combobox *c)
{
	for(int i = 0; i < entries.length(); i++)
		if(entries[i]->type == COMBOBOX && c == entries[i]->c)
			return entries[i]->value;
	return "Invalid Entry";
}
#endif

//
// Just an OK button

aws_OKdialog::aws_OKdialog(): aws_dialog()
{
	aws_dialog::button("Ok", "OkPressed", true);
}

aws_OKdialog::aws_OKdialog(char *p, char *options): 
              aws_dialog(p, options)
{
	aws_dialog::button("Ok", "OkPressed", true);
}

aws_OKdialog::aws_OKdialog(widget *w, char *p, char *options): 
              aws_dialog(w,p,options)
{
	aws_dialog::button("Ok", "OkPressed", true);
}

aws_OKdialog::~aws_OKdialog()
{
}

//
// Okay or Cancel

aws_OkCanceldialog::aws_OkCanceldialog(): aws_dialog()
{
	aws_dialog::button("Ok", "OkPressed", true);
	aws_dialog::button("Cancel", "CancelPressed");
}

aws_OkCanceldialog::aws_OkCanceldialog(char *p, char *options): 
                    aws_dialog(p, options)
{
	aws_dialog::button("Ok", "OkPressed", true);
	aws_dialog::button("Cancel", "CancelPressed");
}

aws_OkCanceldialog::aws_OkCanceldialog(widget *w, char *p, char *options): 
                    aws_dialog(w,p,options)
{
	aws_dialog::button("Ok", "OkPressed", true);
	aws_dialog::button("Cancel", "CancelPressed");
}

aws_OkCanceldialog::~aws_OkCanceldialog()
{
}

//
// Yes or No

aws_YesNodialog::aws_YesNodialog(): aws_dialog()
{
	aws_dialog::button("Yes", "YesPressed", true);
	aws_dialog::button("No", "NoPressed");
}

aws_YesNodialog::aws_YesNodialog(char *p, char *options): 
                 aws_dialog(p, options)
{
	aws_dialog::button("Yes", "YesPressed", true);
	aws_dialog::button("No", "NoPressed");
}

aws_YesNodialog::aws_YesNodialog(widget *w, char *p, char *options): 
                 aws_dialog(w,p,options)
{
	aws_dialog::button("Yes", "YesPressed", true);
	aws_dialog::button("No", "NoPressed");
}

aws_YesNodialog::~aws_YesNodialog()
{
}
