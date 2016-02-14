/*
 * images.c - implements cached image lookups
 * Copyright (C) 1998 Ivo van der Wijk (ivo@cs.vu.nl) and others
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#include "images.h"
#include "misc.h"

images::images(char *picdir)
{
	base_path = picdir;
	if(base_path == "")
		base_path = "pics";
}

images::~images()
{
}

void	images::dir(char *s)
{
	base_path = s;
}

cstring	images::operator[](cstring which)
{
	cstring	*s = img_list[which];
//	char	*picpath = skit->getvar("picturedir");

	cstring	img_id;
	
//	if(picpath)
//		base_path = picpath;

	if(s == NULL)		// load image
	{
		cstring	pic_path = base_path + '/' + which;
		img_id = tk->evaluate("image create photo -file " +
		                               escape(pic_path));
		s = new cstring(img_id);
		img_list[which] = s;
	}
	else
		img_id = *s;
	return img_id;
}
