#ifndef TARGETLIST_H
#define TARGETLIST_H

#include <map.h>
#include "targetif.h"
#include "cstring.h"

#include "misc.h"		// mapicmp defined there

typedef	map<cstring, targetif *, mapicmp> target_map;
typedef	map<cstring, targetif *, mapicmp>::iterator target_iterator;

#define iter_key(u)	((*u).first)
#define iter_value(u)	((*u).second)

class	target_list: public target_map
{
public:
	target_list();
	~target_list();

	targetif	*add(const cstring &, targetif *);
	int	del(const cstring &);
	targetif	*findtarget(const cstring &);
	targetif	*operator[](const cstring &);
	targetif	*operator[](const int);
	void	disable(void);
	void	clear(const cstring &);
	void	active(const cstring &, int, int);
	void	nickchange(const cstring &, const cstring &);
	void	quituser(const cstring &, const cstring &);
private:
};
 
#endif // TARGETLIST_H
