CFG=default

include ../cfg/${CFG}.cfg

all:	
	make CFG=${CFG} deps
	make CFG=${CFG} circus

static:
	@${CIRCUS_CFG} static
	
shared:
	@${CIRCUS_CFG} shared

SOURCES  = main.c ircclient.c servergui.c ircserver.c connection.c  \
           target_list.c targets.c cmdhandler.c guihandler.c ctcphandler.c \
	   serverhandler.c ircsession.c masterwindow.c channel_target.c \
	   irctext.c channelwin.c querywin.c misc.c images.c cstring.c  \
	   dcc.c if_dcc.c io.c fnmatch.c
OBJECTS  = main.o ircclient.o servergui.o ircserver.o connection.o  \
           target_list.o targets.o cmdhandler.o guihandler.o ctcphandler.o \
	   serverhandler.o ircsession.o masterwindow.o channel_target.o \
	   irctext.o channelwin.o querywin.o misc.o images.o cstring.o \
	   dcc.o if_dcc.o io.o fnmatch.o

circus:  ${OBJECTS} ../plush/libplush.a
	${CC} ${CFLAGS} ${LDFLAGS} -o circus ${OBJECTS} ${LIBS}

.c.o: 
	${CC} ${CFLAGS} ${DEFINES} ${INCLUDES} -c $<

fnmatch.o: 
	${GCC} ${CFLAGS} ${DEFINES} ${INCLUDES} -c fnmatch.c

clean:
	${RM} circus circus.old core *.o *.bak 

deps:	
	${MAKEDEPEND} ${SOURCES} > .deps

# Dependencies

include .deps
