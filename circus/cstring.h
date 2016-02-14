#ifndef _CSTRING_H
#define _CSTRING_H

#include <iostream.h>

#define GROWSIZE	16

class	cstring
{
public:
	friend	ostream &operator<<(ostream &, const class cstring &);
	friend	cstring	operator+(const char *, const cstring &);
	friend	cstring	operator+(const cstring &, const cstring &);
	friend	cstring	operator+(const cstring &, const char *);
	friend	cstring	operator+(const char, const cstring &);
	friend	cstring	operator<<(const cstring &, const cstring &);
	friend	cstring	operator<<(const cstring &, const char *);
	friend	cstring	operator<<(const cstring &, const int);
	friend	cstring	operator<<(const int, const cstring &);

	cstring();
	cstring(const char);
	cstring(const char *);
	cstring(const cstring &);
	virtual ~cstring();

// operators - assignment
	cstring	&operator=(const char);
	cstring	&operator=(const char *);
	cstring	&operator=(const cstring &);

// operators - comparison
	int	operator==(const cstring &) const;
	int	operator!=(const cstring &) const;
	int	operator==(const char *) const;
	int	operator!=(const char *) const;

// not an operator (use && or ||?)
	int	compare(const char *, int = 0) const;
	int	compare(const cstring &, int = 0) const;

// operators - indexing
	char	&operator[](int) const;
// operators - addition

	cstring	operator+(const cstring &);
	cstring	operator+(const char *);
	cstring	operator+(const char);
	cstring	&operator<<(const char);
	cstring	&operator<<(const int);
	cstring	&operator<<(const long);
	cstring	&operator<<(const unsigned char);
	cstring	&operator<<(const unsigned int);
	cstring	&operator<<(const unsigned long);
	cstring	&operator<<(const char *);
	cstring	&operator<<(const cstring &);
	cstring	&operator+=(const cstring &);
	cstring	&operator+=(const char *);
	cstring	&operator+=(const char);

	cstring &operator--(void);
// misc operators
	virtual cstring	operator()(const int, const int = -1) const;
// conversion
	cstring	upper(void) const;
	cstring	lower(void) const;
	operator char *() const;
	operator const char *() const;
        operator int() const;
//	operator bool() const;
	int	skip(const char);
	int	rskip(const char);
	int	rchomp(const char);
// misc functions
	int	find(const char, const int = 0) const;
	int	find(const char *, const int = 0) const;
	int	length(void) const;
static	void	stats(void);
protected:
	void	_strcpy(const char *);
	void	grow(const int);
	void	grow(const char *);
	int	growsize(const int) const;
	char	*_buffer;
	int	_size;
	int	_bufsize;
private:
	static	int	created;
	static	int	deleted;
};

class	cstring_iter: public cstring
{
public:
	cstring_iter();
	cstring_iter(const char);
	cstring_iter(const char *);
	cstring_iter(const cstring &);
	~cstring_iter();

	char	operator()();
	cstring	operator()(const char);
	cstring	*operator[](const char);
	cstring	operator()(const char *);
	cstring	*operator[](const char *);
static	void	stats(void);
protected:
	int	_idx;
	cstring	_result;
private:
	static	int	created;
	static	int	deleted;
};

bool	operator!=(cstring const &a, const char *b);
bool     operator!=(cstring const &a, cstring const &b);

#endif // _CSTRING_H
