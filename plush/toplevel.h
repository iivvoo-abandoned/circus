#ifndef TOPLEVEL_H
#define TOPLEVEL_H

#include "cstring.h"
#include "frame.h"

// toplevel is derived from frame, because they're really alike

class	toplevel: public frame
{
public:
	toplevel(const char * = NULL, const char * = NULL, int = true);
	toplevel(widget *, const char * = NULL, 
                 const char * = NULL, int = true);
	
	virtual ~toplevel();

	void	title(const char *) const;
	void	position(const int, const int) const;
	void	resize(const int, const int) const;
	void	transient(const char * = ".") const;
	void	iconify(void);
	void	deiconify(void);
	void	raise(void);

static	int	created;
static	int	deleted;

private:

};

#endif // TOPLEVEL_H
