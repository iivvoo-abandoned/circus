/*
 * cstring.c - simple string class. 
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
/*
 * Strimple- a simple string class.
 *
 * One that actually works. No weird referencing, and because I wrote
 * the code, I understand it and I can change it.
 *
 */

// #include <iostream.h>
#include <ctype.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "cstring.h"

int	cstring::created = 0;
int	cstring::deleted = 0;

int	cstring_iter::created = 0;
int	cstring_iter::deleted = 0;

/*
 * Goals:
 *
 * Efficiency
 * char *-like behaviour
 * complex string manipulations
 *
 * upper()
 * lower()
 */
/*
 * Is growing (and shrinking!!) of buffers efficient? (new/delete)
 */
/*
 * Ideas:
 *       - Different types of strings:
 *         - normal
 *         - upper
 *         - lower
 */
/*
 * Todo: 
 *	 istream stuff, though I don't need it.
 *	 extensive testing
 *	 operators < <= > >= (don't need it)
 */

// string(int)	-- buffer of size int?

// constructors

cstring::cstring()
// create an empty string
{
	cstring::created++;
	_buffer = NULL;
	_bufsize = 0;
	grow(0);		// 16 char buffer, ""	
	_size = 0;
}

cstring::cstring(const char c)
{
	cstring::created++;
	_buffer = NULL;
	_bufsize = 0;
	grow(1);		// one character
	_buffer[0] = c;
	_buffer[1] = '\0';
	_size = 1;
}

cstring::cstring(const char *s)
{
	assert(s != NULL);
	cstring::created++;
	_buffer = NULL;
	_bufsize = 0;
	grow(s);
	strcpy(_buffer, s);
	_size = strlen(s);
}

cstring::cstring(const cstring &s)
{
	cstring::created++;
	_buffer = NULL;
	_bufsize = 0;
	grow(s._buffer);
	strcpy(_buffer, s._buffer);
	_size = s._size;
}

cstring::~cstring()
{
	cstring::deleted++;
	delete _buffer;
}

cstring	&cstring::operator=(const char c)
{
	grow(1);
	_buffer[0] = c;
	_buffer[1] = '\0';
	_size = 1;
	return (*this);
}

cstring	&cstring::operator=(const char *s)
{
	assert(s!=NULL);
	if(s == _buffer)		// self assignment
		return (*this);
	grow(s);
	_strcpy(s);
	_size = strlen(s);
	return (*this);
}

cstring	&cstring::operator=(const cstring &s)
{
	if(&s == this)
		return (*this);
	grow(s._buffer);
	_strcpy(s._buffer);
	_size = s._size;
	return (*this);
}

// Comparison operators
int	cstring::operator==(const cstring &s) const
{
	return strcmp(_buffer, s._buffer) == 0;
}

int	cstring::operator!=(const cstring &s) const
{
	return strcmp(_buffer, s._buffer) != 0;
}

int	cstring::operator==(const char *s) const
{
	return strcmp(_buffer, s) == 0;
}

int	cstring::operator!=(const char *s) const
{
	return strcmp(_buffer, s) != 0;
}

int	cstring::compare(const char *s, int mode) const
{
	if(mode)
		return strcmp(_buffer, s) == 0;
	return strcasecmp(_buffer, s) == 0;
}

int	cstring::compare(const cstring &s, int mode) const
{
	if(mode)
		return strcmp(_buffer, s._buffer) == 0;
	return strcasecmp(_buffer, s._buffer) == 0;
}

// Indexing operators
char	&cstring::operator[](int i) const
{
	assert(i >= -_size && i <= _size);	// <=, allow scan for '\0'
	if(i < 0)
		return _buffer[_size + i];
	return _buffer[i];
}

// Addition of strings etc.
cstring	cstring::operator+(const cstring &s)
{
	cstring	tmp(_buffer);
	tmp+= s;
	return tmp;
}

cstring	cstring::operator+(const char *s)
{
	cstring	tmp(_buffer);
	tmp+= s;
	return tmp;
}

cstring	cstring::operator+(const char c)
{
	cstring	tmp(_buffer);
	tmp+= c;
	return tmp;
}

cstring	&cstring::operator<<(const char c)
{
	(*this) += c;
	return (*this);
}

cstring	&cstring::operator<<(const int i)
{
	char	buf[255];

	sprintf(buf, "%d", i);
	(*this) += buf;
	return (*this);
}

cstring	&cstring::operator<<(const long l)
{
	char	buf[255];

	sprintf(buf, "%ld", l);
	(*this) += buf;
	return (*this);
}

cstring	&cstring::operator<<(const unsigned char c)
{
	(*this) += c;
	return (*this);
}

cstring	&cstring::operator<<(const unsigned int i)
{
	char	buf[255];

	sprintf(buf, "%d", i);
	(*this) += buf;
	return (*this);
}

cstring	&cstring::operator<<(const unsigned long l)
{
	char	buf[255];

	sprintf(buf, "%lu", l);
	(*this) += buf;
	return (*this);
}

cstring	&cstring::operator<<(const char *s)
{
	(*this) += s;
	return (*this);
}

cstring	&cstring::operator<<(const cstring &s)
{
	(*this) += s;
	return (*this);
}

cstring	&cstring::operator+=(const cstring &s)
{
	grow(_size + s._size);
	strcat(_buffer, s._buffer);
	_size += s._size;
	return (*this);
}

cstring &cstring::operator--(void)
{
	_buffer[--_size] = '\0';
	return (*this);
}

cstring	&cstring::operator+=(const char *s)
{
	grow(_size + strlen(s));
	strcat(_buffer, s);
	_size += strlen(s);
	return (*this);
}

cstring	&cstring::operator+=(const char c)
{
	grow(_size + 1);
	_buffer[_size++] = c;
	_buffer[_size] = '\0';
	return (*this);
}


// misc functions
int	cstring::find(const char c, const int idx) const
// find c and return it's index, start search at idx
// return -1 if no match
{
	int	where = idx;
	
	assert(idx <= _size);	
	
	while(where <= _size)
		if(_buffer[where] == c)
			return where;
		else
			where++;
	return -1;
}

// chop
// skip

int	cstring::skip(const char c)
// skip string till first occurence (including) c, return number of characters
// skipped, -1 if not found
{
	int	idx;

	for(idx = 0; idx < _size && _buffer[idx] != c; idx++)
		;
	if(idx == _size)
		return -1;

	_strcpy(_buffer + idx + 1);		// _strcpy adjusts size
	return idx;
}

int	cstring::rskip(const char c)
// skip string till first occurence (including) c, searching from right to
// left, return number of characters skipped
{
	int	idx;

	for(idx = _size; idx >= 0 && _buffer[idx] != c; idx--)
		;
	if(idx < 0)
		return -1;

	_strcpy(_buffer + idx + 1);		// _strcpy adjusts size
	return idx;
}

int	cstring::rchomp(const char c)
// chomp everything after first occurence from the right of c, includng
// c itself
{
	int	idx, i=0;

	for(idx = _size; idx >= 0 && _buffer[idx] != c; idx--, i++)
		;
	if(idx < 0)
		return -1;
	_buffer[idx] = '\0';
	_size = idx;
	return i;
}

void	cstring::stats(void)
{
	cout << "cstring created: " << cstring::created << 
	        " cstring deleted: " << cstring::deleted <<
		" => " << cstring::created - cstring::deleted << endl;
}

int	cstring::find(const char *s, const int idx) const
// find any of the chars in s, and return it's index, start search at idx
// return -1 if no match
{
	int	where = idx;
	
	assert(idx <= _size);

	while(where < _size)
		if(strchr(s, _buffer[where]))
			return where;
		else
			where++;
	return -1;
}

int	cstring::length(void) const
{
	return _size;
}

// protected functions

void	cstring::_strcpy(const char *s)
{
	int 	i;

	for(i = 0; s[i] != '\0'; i++)
		_buffer[i] = s[i];
	_buffer[i] = '\0';
	_size = i;
}

// Private functions
int	cstring::growsize(const int size) const
{
	return (((size / GROWSIZE) + 1) * GROWSIZE) + 1;
}

void	cstring::grow(const int size)
// Create a buffer sufficiently large enough for a string of 'size'
// change if size really changes.. also, don't shrink (?)
{
	char	*_new;

	if(_bufsize > size)
		return;		// large enough
	if(_bufsize == growsize(size))
		return;		// no change;

	_new = new char[growsize(size)];
	_bufsize = growsize(size);

	if(_buffer)
	{
		strcpy(_new, _buffer);
		delete _buffer;
		_buffer = _new;
	}
	else
	{
		_new[0] = '\0';
		_buffer = _new;
	}
}

void	cstring::grow(const char *s)
// Create a buffer sufficiently large enough for a string 's'
{
	grow(strlen(s));
}

ostream	&operator<<(ostream &os, const cstring &s)
{
	os << s._buffer;
	return os;
}

cstring	operator+(const char *s1, const cstring &s2)
{
	cstring	tmp(s1);
	tmp+= s2;
	return tmp;
}

cstring	operator+(const cstring &s1, const char *s2)
{
	cstring	tmp(s1);
	tmp+= s2;
	return tmp;
}

cstring	operator+(const cstring &s1, const cstring &s2)
{
	cstring	tmp(s1);
	tmp+= s2;
	return tmp;
}

cstring	operator+(const char c, const cstring &s)
{
	cstring	tmp(c);
	tmp+= s;
	return tmp;
}

cstring	operator<<(const cstring &s1, const cstring &s2)
{
	return s1+s2;
}

cstring	operator<<(const cstring &s1, const char *s2)
{
	return s1+s2;
}

cstring	operator<<(const cstring &s1, const int i)
{
	char	buf[255];

	sprintf(buf, "%d", i);
	return s1+buf;
}

cstring	operator<<(const int i, const cstring &s1)
{
	char	buf[255];

	sprintf(buf, "%d", i);
	return buf + s1;
}

cstring	cstring::operator()(const int start, const int end) const
// if end != -1, return the string starting at 'start' until (including) end,
// else return the string starting at 'start' until the end
// if end < start, return ""
{
	int	newsize;

	if(end == -1)
		newsize = _size - start + 1;
	else if(end < start)
		newsize = 0;
	else
		newsize = end - start + 1;
	
	char	*tmp = new char[newsize + 1];
	strncpy(tmp, _buffer + start, newsize);
	tmp[newsize] = '\0';		// strncpy doesn't always terminate
	cstring 	result(tmp);

	return tmp;
}

//	cstring::operator bool() const
//{
//	return _size > 0;
//}

	cstring::operator int() const
{
	return _size;
}

	cstring::operator char *() const
{
	return _buffer;
}

	cstring::operator const char *() const
{
	return _buffer;
}


cstring	cstring::lower(void) const
{
	//char	tmp[_size + 1];		Fuck Irix
	char	*tmp = new char[_size + 1];

	int	i;
	
	for(i = 0; i < _size; i++)
		tmp[i] = tolower(_buffer[i]);
	tmp[i] = '\0';

	cstring	result(tmp);
	delete tmp;
	return result;
}

cstring	cstring::upper(void) const
{
	// char	tmp[_size + 1];
	char	*tmp = new char[_size + 1];
	int	i;
	
	for(i = 0; i < _size; i++)
		tmp[i] = toupper(_buffer[i]);
	tmp[i] = '\0';

	cstring	result(tmp);
	delete tmp;
	return result;
}


// ---------------------------------------------------------------------

cstring_iter::cstring_iter(): cstring(), _result()
{
	cstring_iter::created++;
	_idx = 0;
}

cstring_iter::cstring_iter(const char c): cstring(c), _result()
{
	cstring_iter::created++;
	_idx = 0;
}

cstring_iter::cstring_iter(const char *s): cstring(s), _result()
{
	cstring_iter::created++;
	_idx = 0;
}

cstring_iter::cstring_iter(const cstring &s): cstring(s), _result()
{
	cstring_iter::created++;
	_idx = 0;
}

cstring_iter::~cstring_iter()
{
	cstring_iter::deleted++;
}

char	cstring_iter::operator()()
{
	assert(_idx <= _size+1);

	if(_idx == _size+1)
		_idx = 0;
	return _buffer[_idx++];
}

cstring	cstring_iter::operator()(const char c)
//  split the string in parts separated by one or more 'c's
// return an empty string if there are no more 'tokens'
//
// _idx is used to remember positions between calls
{
	//char	tmp[_size+1];		// *must* be enough
	char	*tmp = new char[_size + 1];
	int	i;

	assert(_idx <= _size+1);

	if(_idx == _size+1)
		_idx = 0;
	// start by skipping 'c' at the beginning
	while(_buffer[_idx] && (_buffer[_idx] == c))
		_idx++;
	// copy until '\0' or c
	for(i = 0; _buffer[_idx] && (_buffer[_idx] != c); i++, _idx++)
		tmp[i] = _buffer[_idx];
	tmp[i] = '\0';

	// make a string
	cstring	result(tmp);
	delete tmp;
	return result;
}

cstring	*cstring_iter::operator[](const char c)
// This is the sampe as the other operator()(const char), except this
// one returns pointers to strings (and NULL on end) in stead of strings
{
	_result = cstring_iter::operator()(c);
	if((int)_result)
		return &_result;
	else
		return NULL;
}

cstring	cstring_iter::operator()(const char *s)
//  split the string in parts separated by one or more of the characters in s
// return an empty string if there are no more 'tokens'
//
// _idx is used to remember positions between calls
{
	// char	tmp[_size+1];		// *must* be enough
	char	*tmp = new char[_size + 1];
	int	i;

	assert(_idx <= _size+1);
	assert(s != NULL);

	if(_idx == _size+1)
		_idx = 0;
	// start by skipping 'c' at the beginning
	while(_buffer[_idx] && (strchr(s, _buffer[_idx])))
		_idx++;
	// copy until '\0' or c
	for(i = 0; _buffer[_idx] && (!strchr(s, _buffer[_idx])); i++, _idx++)
		tmp[i] = _buffer[_idx];
	tmp[i] = '\0';

	// make a string
	cstring	result(tmp);
	delete tmp;
	return result;
}

cstring	*cstring_iter::operator[](const char *s)
// This is the sampe as the other operator()(const char *), except this
// one returns pointers to strings (and NULL on end) in stead of strings
{
	_result = cstring_iter::operator()(s);
	if((int)_result)
		return &_result;
	else
		return NULL;
}

void	cstring_iter::stats(void)
{
	cout << "cstring_iter created: " << cstring_iter::created << 
	        " cstring_iter deleted: " << cstring_iter::deleted <<
		" => " << cstring_iter::created - cstring_iter::deleted << endl;
}

// stupid stl fix..

bool	operator!=(cstring const &a, const char *b)
{
        return strcmp((char *) a, (char *) b) != 0;
}

bool     operator!=(cstring const &a, cstring const &b)
{
        return strcmp((char *) a, (char *) b) != 0;
}
