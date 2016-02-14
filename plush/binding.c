#include "binding.h"

int	binding::created = 0;
int	binding::deleted = 0;

binding::binding()
{
	h = (handler *)0L;
	fd = -1;
	mask = 0;
	type = other;
	created++;
}
binding::~binding()
{
	deleted++;
}
