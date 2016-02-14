#ifndef PLUSHHISTENTRY_H
#define PLUSHHISTENTRY_H

#include <vector.h>		// STL List Template
#include "entry.h"

#include "cstring.h"

/*
 * plushHistEntry
 *
 * An ordinary entry, with builtin history
 */

class	plushHistEntry: public entry
{
public:
	plushHistEntry();
	plushHistEntry(char* p, char* options = "");
	plushHistEntry(widget* w,char* p, char* options = "");
	~plushHistEntry();
	char	*get();
private:
	event	*dispatch(event *);
	void	init(void);
	vector<cstring *>	history;
	garbage<cstring *>	histrash;
	unsigned int		idx;
};

#endif // PLUSHHISTENTRY
