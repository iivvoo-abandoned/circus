/*
 * io.c - abstacts bsd sockets into a nice (?) class
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
#include <sys/types.h>		// solaris chokes without this
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>

#include <iostream.h>

#include <stdio.h>		// sscanf
#include "io.h"

extern "C" int gethostname(char *name, int namelen);

unsigned	long	io::_myip = 0L;

/*
 * TODO:
 *       Error checking and -reporting
 *       Store remote address/port (for accepted and initiated connects)
 *	 nonblocking is default. This should be enabled/disabled using params
 */

/*
 * Niet vergeten:
 *
 * Select zal niet rapporteren of er data in de buffers zit!! Deze moeten
 * dus eerst leeg zijn! (check haswrite())
 */

io::io()
{
	_sock = -1;
	_rsize = IO_BUFFER_SIZE;
	_wsize = IO_BUFFER_SIZE;
	_rbytes = 0;
	_wbytes = 0;
	_rbuf = new char[_rsize];
	_wbuf = new char[_wsize];
	_rbuf[0] = '\0';
	_wbuf[0] = '\0';
	_params = 0;
}

io::io(int fd)
{
	_sock = fd;
	_rsize = IO_BUFFER_SIZE;
	_wsize = IO_BUFFER_SIZE;
	_rbytes = 0;
	_wbytes = 0;
	_rbuf = new char[_rsize];
	_wbuf = new char[_wsize];
	_rbuf[0] = '\0';
	_wbuf[0] = '\0';
	_params = 0;
}

io::~io()
{
	if(_sock != -1)
		::close(_sock);
	delete [] _rbuf;
	delete [] _wbuf;
}

void	io::close(void)
{
	if(_sock != -1)
		::close(_sock);
	_rbytes = 0;
	_wbytes = 0;	// 'empty' buffers
	_rbuf[0] = '\0';
	_wbuf[0] = '\0';
	_sock = -1;
}
	
int	io::connect(char *host, int port)
// make a standard tcp connection
{
	_sock = _connect(host, port);
	return _sock;
}

int	io::listen(int &port, char *host)
// create a listening socket, return the socket and the portnumber
{
        int     size;
        struct  sockaddr_in     localaddr;

        if((_sock = _connect(NULL, -port, host)) < 0)
                return _sock;
        size = sizeof(struct sockaddr_in);
        getsockname(_sock, (struct sockaddr *) &localaddr, &size);
        port = (unsigned)ntohs(localaddr.sin_port);
        return _sock;
}

int	io::accept(int accept_self)
// accept_self is optional (default 0), if set to true, close the listening
// socket and use the socket returned by ::accept
// But you don't always want to close the original socket!!
{
	int	size;
	int	newsock;
	struct	sockaddr_in	raddr;

	size = sizeof(struct sockaddr_in);
	newsock = ::accept(_sock, (struct sockaddr *) &raddr, &size);
	if(accept_self)
	{
		::close(_sock);
		_sock = newsock;
	}
	return newsock;
}

int     io::set_nonblocking(void)
{
#ifndef NBLOCK_SYSV
        int     nonb = 0;
#endif
        int     res = 1;

#ifdef NBLOCK_POSIX
        nonb |= O_NONBLOCK;
#endif
#ifdef NBLOCK_BSD
        nonb |= O_NDELAY;
#endif
#ifdef NBLOCK_SYSV
        if(ioctl(_sock, FIONBIO, (char *) &res) < 0)
                return -1;
#else
        if((res = fcntl(_sock, F_GETFL, 0)) == -1)
                return -1;
        else if(fcntl(_sock, F_SETFL, res | nonb) == -1)
                return -1;
#endif
        return _sock;
}

int	io::write(const char *msg, int bytes)
// add msg to _wbuf
//
// The default value for byets is -1. In this case, use strlen(msg);
{
	if(bytes == -1)
		bytes = strlen(msg);

	// does it fit in our buffer?
	if(_wbytes + bytes > _wsize)
	{
		// pick a safe size, leave some space to grow
		char	*tmp = new char[_wsize + bytes + 64];

		_copy(tmp, _wbuf, _wbytes);
		_wsize += (bytes + 64);
		delete [] _wbuf;

		_wbuf = tmp;
	}
	
	// add msg to buffer
	_copy(_wbuf + _wbytes, msg, bytes);

	// adjust count
	_wbytes += bytes;


	// done
	return _wbytes;		// current #bytes in buffer
}

int	io::read(char *msg, int size)
//
// Read 'size' bytes (or less). If size == -1, read everything
{
	if(size == -1 || size > _rbytes)
		size = _rbytes;
	
	_copy(msg, _rbuf, size);
	
	// copy rest of buffer

	_copy(_rbuf, _rbuf+size, _rbytes-size);
	
	_rbytes -= size;

	return size;
}

int	io::flush(int mode)
// it's probably not a good idea to flush everything at once, one of the 
// two might cause an error
{
	if(_sock == -1)
		return -1;

	if(mode == io_read)
	{
		char	buf[IO_BUFFER_SIZE];
		int	bread = ::read(_sock, buf, IO_BUFFER_SIZE);

		if(bread <= 0)
			return bread;
		if(_rbytes + bread > _rsize)
		{
			char	*tmp = new char[_rsize + IO_BUFFER_SIZE];

			_copy(tmp, _rbuf, _rbytes);
			_rsize += IO_BUFFER_SIZE;

			delete [] _rbuf;

			_rbuf = tmp;
		}
		_copy(_rbuf + _rbytes, buf, bread);
		_rbytes += bread;
		return _rbytes;
	}

	if(mode == io_write)
	{
		// flush everything, if possible
		int	bwritten = ::write(_sock, _wbuf, _wbytes);

		if(bwritten <= 0)
			return bwritten;

		if(bwritten < _wbytes)
			_copy(_wbuf, _wbuf+bwritten, bwritten);
		_wbytes -= bwritten;
		return bwritten;
	}
	return -1;		// not reached
}

int	io::fd(void) const
{
	return _sock;
}

int	io::hasread(void) const
// does io have something to read?
{
	return _rbytes > 0;
}

int	io::haswrite(void) const
// does io have something to write?
{
	return _wbytes > 0;
}

// operators
io	&io::operator<<(char *s)
{
	write(s);
	return (*this);
}

io	&io::operator<<(io_param p)
{
	if(p == FLUSH_WRITE || p == FLUSH)
		flush(io_write);
	return (*this);
}

int	io::_copy(char *dest, const char *src, int count)
{
	for(int i = 0; i < count; i++)
		dest[i] = src[i];
	return count;
}

int     io::_connect(char *host, int service, char *local)
// 'low level' connect
{
	int     s = -1;
	struct  sockaddr_in server;

	if (service > 0)
	{
        	server.sin_family = AF_INET;
        	if((server.sin_addr.s_addr = inet_addr(host)) == 0xffffffff)
		// try resolving
		{
			struct	hostent	*h;

			if((h = gethostbyname(host)) == NULL)
			{
				return -1;
			}
			memset((char *) &server, 0, sizeof(server));
			memcpy((char *) &server.sin_addr, h->h_addr, h->h_length);
			server.sin_family = h->h_addrtype;
		}
         	server.sin_port = (unsigned short) htons(service);
	}
	if((s = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		return -1;
	if((_params & nblock) && (set_nonblocking() < 0))
	{
		::close(s);
		return -1;
        }
	setsockopt(s, SOL_SOCKET, SO_LINGER, 0, 0);
	setsockopt(s, SOL_SOCKET, SO_REUSEADDR, 0, 0);
	setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, 0, 0);
	if(service <= 0)
	{
		struct  sockaddr_in        localaddr;

		localaddr.sin_family = AF_INET;
		if(service < 0)
			localaddr.sin_port = (unsigned short) htons(-service);
		else
			localaddr.sin_port = 0;
		if(local)
			localaddr.sin_addr.s_addr = inet_addr(local);
		else
			localaddr.sin_addr.s_addr = INADDR_ANY;
		if(bind(s, (struct sockaddr *) &localaddr, sizeof(localaddr))
			== -1 || ::listen(s, 1) == -1)
		{
			::close(s);
			return -1;
		}
		return s;
	}

	if((::connect(s, (struct sockaddr *) & server, sizeof(server)) < 0))
	{
		if(errno == EINPROGRESS)
			return s;
		else
		{
			::close(s);
			return -1;
		}
	}

	int     len = sizeof(struct sockaddr);
	struct  sockaddr_in     localaddr;

	getsockname(s, (struct sockaddr *) &localaddr, &len);
	_myip = htonl(localaddr.sin_addr.s_addr);

	return s;
}

char	*io::make_ip(char *ia)
{
	unsigned long inetaddr;
	struct	 in_addr party;

	sscanf(ia, "%lu", &inetaddr);
	party.s_addr = htonl(inetaddr);
	return inet_ntoa(party);
}

unsigned	long	io::my_ip(void)
/*
 * This method returns the address of the interface associated with the 
 * connection. _myip is static, which means we  assume all io-connections
 * use the same interface. Perhaps this is a bit too restrictive.
 */
{
	return _myip;
}

char	*io::gethostname(void)
{
	static	char	myhost[1024];	// long 'nuff?

	if(::gethostname(myhost, 1024) == -1)
		return "<unknown>";
	return myhost;
}

char	*io::resolvehost(char *host)
// return the ip beloning to 'host', NULL on error (with herror set)
{
	if(inet_addr(host) == (unsigned long)-1)
	// try resolving
	{
		struct	hostent	*h;
		struct 	sockaddr_in result;

		if((h = gethostbyname(host)) == NULL)
{
			return NULL;
}
		memset((char *) &result, 0, sizeof(result));
		memcpy((char *) &result.sin_addr, h->h_addr, h->h_length);
		result.sin_family = h->h_addrtype;
		return inet_ntoa(result.sin_addr);
	}
	return host;
}

// advanced stuff: copying io's

io	&io::operator=(const io &source)
{
	if(&source == this)		// self assignment
		return (*this);
	
	_rsize = source._rsize;
	_wsize = source._wsize;
	_rbytes = source._rbytes;
	_wbytes = source._wbytes;
	_sock = source._sock;
	_params = source._params;

	delete [] _wbuf;
	delete [] _rbuf;

	_rbuf = new char[_rsize];
	_wbuf = new char[_wsize];

	_copy(_wbuf, source._wbuf, _wbytes);
	_copy(_rbuf, source._rbuf, _rbytes);

	return (*this);
}
