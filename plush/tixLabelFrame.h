#ifndef TIXLABELFRAME_H
#define TIXLABELFRAME_H

#include "cstring.h"
#include "frame.h"

class	tixLabelFrame: public frame
{
public:
	tixLabelFrame(const char * = NULL, const char * = NULL, int = true);
	tixLabelFrame(widget *, const char * = NULL, const char * = NULL, 
	              int = true);
	
	virtual ~tixLabelFrame();

	void	title(const char *);

	void	pack(const char * = "") const;
	virtual cstring container(void) const;

static	int	created;
static	int	deleted;

private:

};

#endif // TIXLABELFRAME_H
