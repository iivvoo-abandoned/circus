#ifndef _IMAGE_H
#define _IMAGE_H

#include <map.h>		// use STL map
#include <plush/KitObject.h>

#include "cstring.h"
#include "misc.h"

class	images: public KitObject
{
public:
	images(char * = "");
	~images();
	void	dir(char *);
	cstring	operator[](cstring);
private:
	map<cstring, cstring *, mapcmp>	img_list;
	cstring			base_path;
};

#endif
