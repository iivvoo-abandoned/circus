#ifndef DIALOG_H
#define DIALOG_H

#include <vector.h>

#include "widgets.h"
#include "cstring.h"

#define	ENTRY		0x0001
#define	COMBOBOX	0x0002

typedef	entry	aws_entry;
typedef button	aws_button;


class	d_entry: public KitObject
{
public:
	d_entry()
	{
		e = NULL;
//		c = NULL;
		value = "";
		type = 0;
	}
	~d_entry()
	{};
	aws_entry	*e;
//	aws_combobox	*c;
	cstring		value;
	int		type;
};

class	aws_dialog: public toplevel
{
public:
	aws_dialog();
	aws_dialog(char *p, char *options = "");
	aws_dialog(widget *w, char *p, char *options = "");
	~aws_dialog();

	int		operator()();
	label		*message(const cstring &msg, const cstring & = "");
	cstring	wacht(void);
	aws_button	*button(const cstring &, const cstring & = "", 
	                        const int isdef = false, const cstring & = "");
	aws_entry	*entry(const cstring &, const cstring & = "", 
	                       const cstring & ="");
//	aws_combobox	*combobox(const cstring &, const cstring & = "",
//	                          const cstring & ="");
	void		image(const char *);
	char		*getentry(const aws_entry *);
//	char		*getcombobox(const aws_combobox *);
private:
	void		init();
	void		build_dialog(void);
	frame		*textframe, *buttonframe,
	                *labelframe, *entryframe, *pairframe;
	label		*imglabel, *textlabel;
	vector<KitObject *>	buttons;
	vector<KitObject *>	entries;
//	garbage<d_entry>	collector;
	cstring		def_button;
	cstring		choice;
	cstring		img;
	int		widget_count;
};

class	aws_OKdialog: public aws_dialog
{
public:
	aws_OKdialog();
	aws_OKdialog(char *p, char *options = "");
	aws_OKdialog(widget *w, char *p, char *options = "");
	~aws_OKdialog();
};

class	aws_OkCanceldialog: public aws_dialog
{
public:
	aws_OkCanceldialog();
	aws_OkCanceldialog(char *p, char *options = "");
	aws_OkCanceldialog(widget *w, char *p, char *options = "");
	~aws_OkCanceldialog();
};
class	aws_YesNodialog: public aws_dialog
{
public:
	aws_YesNodialog();
	aws_YesNodialog(char *p, char *options = "");
	aws_YesNodialog(widget *w, char *p, char *options = "");
	~aws_YesNodialog();
};

#endif
