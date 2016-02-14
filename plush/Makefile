CFG=default

include ../cfg/${CFG}.cfg

all:	
	make CFG=${CFG} deps
	make CFG=${CFG} libplush.a
#
# Widget source and objects

S_WIDGET = widget.c button.c checkbutton.c entry.c frame.c label.c listbox.c \
           menu.c menubutton.c message.c text.c radiobutton.c scrollbar.c    \
	   toplevel.c 
S_TIX    = tixBalloon.c tixLabelFrame.c tixNoteBook.c tixNBFrame.c 	\
           tixPanedWindow.c
S_PLUSH	 = plushButton.c plushHistEntry.c
S_EXTRA  = filechooser.c dialog.c

O_WIDGET = widget.o button.o checkbutton.o entry.o frame.o label.o listbox.o \
           menu.o menubutton.o message.o text.o radiobutton.o scrollbar.o    \
	   toplevel.o 
O_TIX    = tixBalloon.o tixLabelFrame.o tixNoteBook.o tixNBFrame.o	\
           tixPanedWindow.o
O_PLUSH	 = plushButton.o plushHistEntry.o
O_EXTRA  = filechooser.o dialog.o


SOURCES = kit.c KitObject.c event.c binding.c session.c \
          handler.c ${S_WIDGET} ${S_TIX} ${S_PLUSH} ${S_EXTRA} 
OBJECTS = kit.o KitObject.o event.o binding.o session.o \
	  handler.o ${O_WIDGET} ${O_TIX} ${O_PLUSH} ${O_EXTRA}

demo: deps demo.o cstring.o libplush.a
	@echo "*** Creating PLUSH DEMO"
	${CC} ${PLUSHFLAGS} ${LDFLAGS} -o demo demo.o cstring.o \
	                                  libplush.a ${LIBS}

libplush.a: ${OBJECTS}
	@echo "*** Creating archive"
	ar rcs libplush.a ${OBJECTS}


.c.o: ; ${CC} ${PLUSHFLAGS} ${DEFINES} ${INCLUDES} -c $<

clean:
	@echo "*** Removing unnecessary files"
	rm -f demo demo.o libplush.a ${OBJECTS} core *.bak

deps:
	@echo "*** Checking dependencies"
	${MAKEDEPEND} ${SOURCES} > .deps

# Dependencies
include .deps

