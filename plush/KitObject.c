#include "KitObject.h"

#include "widget.h"
#include "cstring.h"

int	KitObject::created = 0;
int	KitObject::deleted = 0;

KitObject::KitObject(): tk(), _strings()
{
	created++;
}

KitObject::~KitObject()
{
	deleted++;
}

void	KitObject::_register(cstring *rubbish)
{
	_strings._register(rubbish);
}

void	KitObject::_register(KitObject *rubbish)
{
	_kits._register(rubbish);
}

void	KitObject::unregister(cstring *rubbish)
{
	_strings.unregister(rubbish);
}

void	KitObject::unregister(KitObject *rubbish)
{
	_kits.unregister(rubbish);
}
