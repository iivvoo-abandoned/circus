#ifndef INTERNALEVENT_H
#define INTERNALEVENT_H

/*
 * The CMDLINE event is generated bij windows with a commandline, and
 * caught and handled bij the cmdhandler class in cmdhandler.[ch]
 *
 * The syntax is CMDLINEx default_target command,
 *
 * Where x identifies the connextion, default_target the default target 
 * if none can be derived from the command (i.e. /me in a messagewindow),
 * and command is the command to be parsed (those /-thingies)
 */
#define CMDLINE	"CMDLINE"

/*
 * The SERVER event is generæted by scripts and cmdhandler that need to
 * modify server connections (add, remove, change)
 * The event is handled by ircclient.[ch]
 *
 * SERVER -change connectionid serverhost ?serverport?
 * SERVER -close connectionid
 * SERVER -disconnect connectionid
 * SERVER -connect serverhost ?serverport? returns connectionid
 * More details later.
 */
#define SERVER	"SERVER"

#endif // INTERNALEVENT_H
