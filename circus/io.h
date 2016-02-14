#ifndef _IO_H
#define _IO_H

#define NBLOCK_POSIX
#define IO_BUFFER_SIZE	1024

class	io_param 
{
public:
	io_param(int i)
	{
		_p = i;
	};
	~io_param()
	{};
	operator int()
	{
		return _p;
	};
private:
	int	_p;
};

#define FLUSH_READ	1
#define FLUSH_WRITE	2
#define FLUSH		3

const	io_param	ioflush_read(FLUSH_READ);
const	io_param	ioflush_write(FLUSH_WRITE);
const	io_param	ioflush(FLUSH);

class	io
{
public:
	io();
	io(int);
	~io();
	void	close(void);
	int	connect(char *, int);
	int	listen(int &, char * = NULL);
	int	accept(int = 0);
	int	set_nonblocking(void);
	int	read(char *, int = -1);
	int	write(const char *, int = -1);
	int	flush(int);
	// params
	enum	{io_read, io_write};
	enum	{nblock};

	// operators
	io	&operator=(const io &);

	// internals
	int	fd(void) const;
	int	hasread(void) const;
	int	haswrite(void) const;

	// operators
	io	&operator<<(char *);
	io	&operator<<(io_param);

	// static utility functions
	static	char 			*make_ip(char *);
	static	unsigned        long    my_ip(void);
	static	char			*gethostname(void);
	static	char			*resolvehost(char *);
private:
	static	unsigned	long	_myip;

	int	_connect(char *, int, char * = NULL);
	int	_copy(char *, const char *, int);
	char	*_rbuf;
	char	*_wbuf;
	int	_rsize;
	int	_wsize;
	int	_rbytes;
	int	_wbytes;
	int	_sock;
	int	_params;
};

#endif // _IO_H
