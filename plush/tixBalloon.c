#include "tixBalloon.h"

int	tixBalloon::created = 0;
int	tixBalloon::deleted = 0;

/*
 * tixBalloon is a Tix mega-widget. It needs special care, as it doesn't
 * really behave like a standard tk widget. For example, there's only
 * one possible binding.
 *
 * From the manpage: 'The Balloon widget supports all the standard 
 *                    options of a frame widget'
 * so, derive this widget from class frame
 */

/*
 * tixBalloon bugs?
 *
 * .path configure .. does not except options such as -balloonmsg or
 * -statusmsg, it seems. This is rather inconsistent, and the only way
 * to specify these is by using the 'bind' command.
 *
 * It's not really a bug, it's documented this way. The tixBalloon command
 * only accepts tixBalloon (frame) options. So does tixBalloon configure.
 * When using 'bind', you can supply different options.
 *
 */
/*
 * tixBalloon behaves somewhat weird. You only create ONE balloon per app,
 * and bind it to more that one widget. This is completely unlike all other
 * widgets, and requirs special action
 *
 * it's even more complicated... you don't need one *per app*, but
 * per statusbar!
 */

tixBalloon::tixBalloon(const char *path, const char *options):
        frame(path, options, false)
{
	_type = "tixBalloon";
	_seq = "";	// No bindings definable

	tk << "tixBalloon " << _path << _options << end;

	// events have to be passed to class specific handlers first..

	tk << "bindtags " << _path << " { TixBalloon " 
	   << _path << " . all }" << end;

	created++;
}

tixBalloon::tixBalloon(widget *w, const char *path, const char *options): 
            frame(w, path, options, false)
/*
 * Duh, 'options' should be frame-options. But I also want balloonwidget
 * options to pass to 'bind'. So, just skip options when creating the 
 * balloon and use them later when 'bind'-ing
 */
{
	_type = "tixBalloon";
	_seq = "";	// No bindings definable

	tk << "tixBalloon " << _path << end;
	tk << "bindtags " << _path << " { TixBalloon " 
	   << _path << " . all }" << end;

	created++;
}

tixBalloon::~tixBalloon()
{
	deleted++;
}

void	tixBalloon::initwait(int t)
{
	tk << _path << " configure -initwait " << t << end;
}

void	tixBalloon::bind(widget *w, const char *msg)
/*
 * bind this balloon to a specific widget, using 'msg' for both
 * balloon and (if any) statusbar
 */
{
	tk << _path << " bind " << w->path() << " " 
	   << "-msg \"" << esc(msg) << "\"" << end;
}

void	tixBalloon::bind(widget *w, const char *status,
                         const char *balloon)
/*
 * bind this balloon to a specific widget, with a separate message
 * for the statusbar and for the balloon
 */
// Bind to widget 'w' using 'options', and _msg etc if they're set.
{
	if(status && balloon)
	{
		tk << _path << " bind " << w->path() << " " 
	     	   << "-statusmsg \"" << esc(status) << "\" " 
	           << "-balloonmsg \"" << esc(balloon) << "\"" << end;
	}
	else if(status)
	{
		tk << _path << " bind " << w->path() << " " 
	     	   << "-statusmsg \"" << esc(status) << "\"" << end;

	}
	else	// assume just balloon, both NULL doesn't make sense
	{
		tk << _path << " bind " << w->path() << " " 
	           << "-balloonmsg \"" << esc(balloon) << "\"" << end;
	}
}

void	tixBalloon::unbind(widget *w)
{
	tk << _path << " unbind " << w->path() << end;
}

void	tixBalloon::statusbar(widget *w)
/*
 * use 'w' as statusbar. It should probably be a label or a message
 */
{
	tk << _path << " configure -statusbar " << w->path() << end;
}
