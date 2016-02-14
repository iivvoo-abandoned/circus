#include <iostream.h>	// debugging

#include "widget.h"

int	widget::created = 0;
int	widget::deleted = 0;

widget::widget(const char *path, const char *options): handler()
{
	_type = "undefined-widget";
	_parent = NULL;
	// no parent defined
	_path = (path? path: (char *)uniq());
	_options = (options? options: "");
	_seq = "<Button-1>";		// mouseclick as default bindsequence
	created++;
}

widget::widget(widget *w, const char *path, 
              const char *options): handler()
// If the user doesn't specify a parent widget (w==NULL), we'll crash!!
//
// abspath is a flag that defines whether the pathname is derived from the
// parents, or not (see tixLabelFrame for a case where this is needed)
{
	_type = "undefined-widget";
	_parent = w;
	_path = _parent->container();

	// Register this widget for garbage collection at it's parent,
	// i.e. when the parent gets destroyed, this widget should
	// be collected...
	_parent->_register(this);

	// is parent rootwindow? 
	// then we would end up with '..pathname'
	if(_path == ".")
		_path = "";
		
	_path += (path? path: (char *)uniq());
	_options = (options? options: "");
	_seq = "<Button-1>";		// mouseclick as default bindsequence

	created++;
}

widget::~widget()
{
	if(_parent)
		_parent->unregister(this);
	deleted++;
}

const	char	*widget::cget(const char *option) const
{
	tk << _path << " cget " << option << end;
	return tk.result();
}

void	widget::configure(const char *options) const
{
	tk << _path << " configure " << options << end;
}

void	widget::destroy(void) const
{
	tk << "destroy " << _path << end;
}

void	widget::pack(const char *how) const
{
	tk << "pack " << _path << " " << how << end;
}

const	char	*widget::path(void) const
{
	return _path;
}

const	char	*widget::type(void) const
{
	return _type;
}

int	widget::eval(char *script) const
{
	cstring cmd;

	cmd << _path << " " << script;
	return tk.eval(cmd);
}

char	*widget::evaluate(char *script) const
{
	cstring cmd;

	cmd << _path << " " << script;
	return tk.evaluate(cmd);
}

void	widget::image(const char *name) const
{
	tk << _path << " configure -image " << name << end;
}

void	widget::enable(void) const
{
	tk << _path << " configure -state normal" << end;
}

void	widget::disable(void) const
{
	tk << _path << " configure -state disabled" << end;
}

void	widget::state(char *_state) const
{
	tk << _path << " configure -state " << _state << end;
}

/*
 * Hush implements six versions of bind, the two below, two that take
 * a binding as an argument, and two that take a void *proc as an
 * argument. I don't know what the other four do, and as long as noone 
 * needs them, I won't/can't implement them.
 */

void	widget::bind(handler *h, char *arg)
{
	arg = (arg?arg:"");
	binding	*b;

	b = tk->bind(h);
	
	tk << "bind " << _path << " " << _seq 
	   << " {" << b->cmd << " " << arg << "}" << end;
}

void	widget::bind(void *h, int (*func)(void *, event *), char *arg)
{
	arg = (arg?arg:"");
	binding	*b;

	b = tk->bind(h, func);
	
	tk << "bind " << _path << " " << _seq 
	   << " {" << b->cmd << " " << arg << "}" << end;
}

void	widget::bind(char *sequence, handler *h, char *arg)
{
	arg = (arg?arg:"");
	binding	*b;

	b = tk->bind(h);
	
	tk << "bind " << _path << " " << sequence 
	   << " {" <<  b->cmd << " " << arg << "}" << end;
}

void	widget::bind(char *sequence, void *h, 
		     int (*func)(void *, event *), char *arg)
{
	arg = (arg?arg:"");
	binding	*b;

	b = tk->bind(h, func);
	
	tk << "bind " << _path << " " << sequence 
	   << " {" << b->cmd << " " << arg << "}" << end;
}

void	widget::bind(const char *script)
{
	tk << "bind " << _path << " " << _seq << " {"
	   << script << "}" << end;
}

void	widget::bind(const char *sequence, const char *script)
{
	tk << "bind " << _path << " " << sequence << " {"
	   << script << "}" << end;
}

void	widget::xscroll(const widget *w) const
{
	tk << _path << " configure -xscrollcommand {" << w->path()
	   << " set }" << end;
}

void	widget::yscroll(const widget *w) const
{
	tk << _path << " configure -yscrollcommand {" << w->path()
	   << " set }" << end;
}

void	widget::focus(void) const
{
	tk << "focus " << _path << end;
}

void	widget::wait(void) const
{
	tk << "tkwait window " << _path << end;
}

void	widget::grab(void) const
{
	tk << "grab set " << _path << end;
}

cstring	widget::container(void) const
/*
 * Return the path in which child-widgets should be packed.
 *
 * Usually, this is simply '_path', but this method may be overloaded
 * for those widgets that need it (for example, tixWidgets like
 * tixLabelFrame that require childwidgets to be packed into subwidgets
 */
{
	return _path;
}

cstring	widget::uniq(void)
// this function creates a unique pathname for the widget, 
// if the user didn't specify one
{
	cstring	pathname;

	pathname << "." << _type << created;

	return pathname;
}
