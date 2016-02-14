#ifndef IRCSESSION_H
#define IRCSESSION_H

#include "guihandler.h"
#include "serverhandler.h"

class	ircsession: public guihandler, public serverhandler
{
public:
	ircsession(tixNoteBook *, int sessionid);
	~ircsession();
	virtual int operator()();
};

#endif // IRCSESSION_H
