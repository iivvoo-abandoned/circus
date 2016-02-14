#ifndef _CONST_H
#define _COSNT_H

#define CIRCUS_VERSION "cIRCus 0.44"
#define CTCP_VERSION   CIRCUS_VERSION " (c) 1996-97 Ivo van der Wijk, Mark de Boer. \x02http://www.nijenrode.nl/~ivo/circus/\x02"
#define CTCP_CLIENTINFO	"ACTION VERSION PING FINGER"
#define CIRCUS_QUIT    CIRCUS_VERSION " - \x02http://www.nijenrode.nl/~ivo/circus/\x02"

#define DEFAULT_NICK		"cIRCuser"
#define DEFAULT_LOGIN		"circus"
#define DEFAULT_IRCNAME		CIRCUS_QUIT
#define DEFAULT_FINGERINFO	CIRCUS_QUIT
#define DEFAULT_USERINFO	CIRCUS_QUIT

#define DEFAULT_SCRIPTDIR	"scripts"
#define DEFAULT_DOWNLOADDIR	"/tmp"


#define	DEFAULT_PORT	6667

// Available usermodes
#define	MODE_WALLOP	0x001
#define	MODE_INVISIBLE	0x002
#define	MODE_SERV_NOT	0x004		// obsolete 
#define	MODE_RESTRICTED	0x008

// Available selection operations
#define QUERY		0x001
#define	WHOIS		0x002
#define	KICK		0x004
#define	BAN		0x008
#define	INVITE		0x010

#endif // _CONST_H
