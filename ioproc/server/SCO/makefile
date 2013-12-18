#------------------------------------------------------------------------------
# Helios generic make system - NATIVE HOST MAKEFILE -
#-----------------------------------------------------------------------------
# SCO Unix Host specific make rules to make NATIVE HOST version of server component.
#
# File: /HSRC/ioproc/server/SCO/makefile
#
# This file contains ALL the definitions and rules that are
# required to make a NATIVE HOST version of this component.
# IT IS NOT COMPLIMENTARY TO THE GENERIC VERSION.
#
# This makefile is always executed within this host specific subdir.
# All host specific source files should be kept here.
# All intermediate, object and binary files should be made in here.
#
# SccsId: %W% %G%
# RcsId: $Id: makefile,v 1.17 1993/05/10 10:04:51 nick Exp $
#		(C) Copyright 1990 Perihelion Software
#
# WARNING: assumes you are using GNUmake.
#-----------------------------------------------------------------------------

InProcMake = TRUE

include ../makefile

sys 	  = /usr/include
SUN = $(HSRC)/$(COMPONENT)/sun

HOSTCC = cc
HOSTCFLAGS := -DSCOUNIX $(HOSTCFLAGS)


#-----------------------------------------------------------------------------
# Note all final targets here

XTARGETS = server hydra hydramon

Xdefault : $(XTARGETS)

Xinstall : $(XTARGETS)
	   $(CP) server $(HPROD)/server.sun4
	   $(CP) hydra $(HPROD)/hydra.sun4
	   $(CP) hydramon $(HPROD)/hydramon.sun4
	   $(TOUCH) Xinstall
Xclean	:


# Note all sources
XSOURCES	= $(XSOURCES.co) \
                  $(XSOURCES.odd)

XSOURCES.co     = scounix/colib.c scounix/coalloc.o

XSOURCES.odd	= makefile 

# Object files

XOBJECTS 	= $(XOBJECTS.co) \
		  $(SERVOBJ) \
		  $(LINKOBJS) \
                  $(HYDRAOBJ) \
                  $(HYDRAMON)

XOBJECTS.co    = $(XSOURCES.co:.c=.o)


$(XOBJECTS) : helios.h

#-----------------------------------------------------------------------------
# Native Host targets and their dependencies:

# uncomment if you have compiler or makedepend generated dependencies
#include depends

HOSTCFLAGS := $(HOSTCFLAGS) -I. -I$(SUN) -Ikludge

helios.h: $(GENHEADERS.c)
	cat $(sys)/stdio.h $(sys)/sys/types.h $(sys)/ctype.h $(sys)/setjmp.h \
	$(sys)/stdarg.h $(sys)/sys/time.h $(sys)/errno.h $(sys)/sys/socket.h \
	$(sys)/sys/times.h $(sys)/signal.h $(sys)/fcntl.h \
	$(sys)/unistd.h $(sys)/sys/stat.h $(sys)/sys/param.h \
	$(sys)/sys/file.h $(sys)/memory.h \
	$(sys)/netinet/in.h $(sys)/netdb.h $(sys)/sys/statfs.h \
	$(sys)/dirent.h $(sys)/pwd.h $(sys)/sys/wait.h $(sys)/termios.h \
	$(sys)/limits.h $(HSRC)/$(COMPONENT)/sun/sunlocal.h \
	$(HSRC)/$(COMPONENT)/scounix/scounix.h \
	$(HSRC)/$(COMPONENT)/scounix/un.h \
	$(GENHEADERS.c) \
	> helios.h

colib_link = colib.o coalloc.o -ltermcap =lsocket
colib.o : colib.c
	$(HOSTCC) -c $(HOSTCFLAGS) $(HSRC)/$(COMPONENT)/scounix/colib.c


server : helios.h $(SERVOBJ) $(XOBJECTS.co) $(LINKOBJS)
	$(HOSTCC) -Bstatic -o $@ $(SERVOBJ) \
                            $(LINKOBJS) $(colib_link)
#	su -c "chown root server ; chmod 04755 server"


hydra : $(HYDRAOBJ) $(LINKOBJS)
	$(HOSTCC) -o $@ $(HYDRAOBJ) $(LINKOBJS) 

hydramon : $(HYDRAMON)
	$(HOSTCC) -o $@ $(HYDRAMON)

#-----------------------------------------------------------------------------
