#
# cIRCus makefile v1.00 
#
# (c) 1997/1998 Ivo van der Wijk
#

#
# 
# Possible extentions:
# make ln-solaris to symlink solaris cfg to default
#

all:	
	@echo "Please use make <target>"
	@echo "where <target> is one of:"
	@echo
	@echo "linux		Linux"
	@echo "solaris		Solaris 2.5 with gcc"
	@echo
	@echo "default		Use cfg/default.cfg"
	@echo
	@echo "Or edit this makefile, or change the"
	@echo "default symlink in cfg/"

default: 
	(cd plush && make all)
	(cd circus && make all)

solaris:
	(cd plush && make CFG=solaris all)
	(cd circus && make CFG=solaris all)

linux:
	(cd plush && make CFG=linux all)
	(cd circus && make CFG=linux all)

deps:
	(cd plush && make deps)
	(cd circus && make deps)

clean:
	(cd plush && make clean)
	(cd circus && make clean)
