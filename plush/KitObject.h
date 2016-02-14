#ifndef KITOBJECT_H
#define KITOBJECT_H

#include "kit.h"
#include "garbage.h"

// 
// Provide each class a toolkit and a uniform interface by deriving from
// KitObject. Also provide each derived class with a garbage collector


class KitObject
{
public:
	KitObject();
	virtual ~KitObject();
	void	_register(cstring *);
	void	_register(KitObject *);

	void	unregister(cstring *);
	void	unregister(KitObject *);
static	int			created;
static	int			deleted;

protected:
	kit	tk;
private:
	garbage<cstring *>	_strings;
	garbage<KitObject *>	_kits;
};

#endif // KITOBJECT_H
