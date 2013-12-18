#------------------------------------------------------------------------------
# Helios generic make system - NATIVE HOST MAKEFILE -
#-----------------------------------------------------------------------------
# Hp9000 Host specific make rules to make NATIVE HOST version of server component.
#
# File: /HSRC/ioproc/server/HP9000/makefile
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
# RcsId: $Id: makefile,v 1.3 1993/09/24 11:54:57 bart Exp $
#		(C) Copyright 1990 Perihelion Software
#
# WARNING: assumes you are using GNUmake.
#-----------------------------------------------------------------------------

InProcMake = TRUE

include ../makefile

sys 	  = /usr/include
SUN = $(HSRC)/$(COMPONENT)/sun
HP9000 = $(HSRC)/$(COMPONENT)/hp9000

HOSTCC = cc
HOSTCFLAGS := -DHP9000 $(HOSTCFLAGS) -I/usr/include/X11R4

#-----------------------------------------------------------------------------
# Note all final targets here

XTARGETS = server hydra hydramon serverwinxaw serverwinxt

Xdefault : $(XTARGETS)

Xinstall : $(XTARGETS)
	   $(CP) server $(HPROD)/server.hp9000
	   $(CP) serverwinxaw $(HPROD)/serverwinxaw.hp9000
	   $(CP) serverwinxt $(HPROD)/serverwinxt.hp9000
	   $(CP) hydra $(HPROD)/hydra.hp9000
	   $(CP) hydramon $(HPROD)/hydramon.hp9000
	   $(TOUCH) Xinstall
Xclean	:


# Note all sources
XSOURCES	= $(XSOURCES.win) \
                  $(XSOURCES.co) \
                  $(XSOURCES.odd)

XSOURCES.win	= serverxaw.c serverxt.c
XSOURCES.co     = $(hp9000)/colib.c

XSOURCES.odd	= makefile ioserv.bm

# Object files

XOBJECTS 	= $(XOBJECTS.win) \
	          $(XOBJECTS.co) \
		  $(SERVOBJ) \
		  $(LINKOBJS) \
                  $(HYDRAOBJ) \
                  $(HYDRAMON)


XOBJECTS.win   = $(XSOURCES.win:.c=.o) 
XOBJECTS.co    = colib.o


$(XOBJECTS) : helios.h

#-----------------------------------------------------------------------------
# Native Host targets and their dependencies:

# uncomment if you have compiler or makedepend generated dependencies
#include depends

HOSTCFLAGS := $(HOSTCFLAGS) -Aa -D_HPUX_SOURCE -I. -I$(SUN) -Ikludge

helios.h: $(GENHEADERS.c)
	cat $(sys)/stdio.h $(sys)/sys/types.h $(sys)/ctype.h $(sys)/setjmp.h \
	$(sys)/stdarg.h $(sys)/sys/time.h $(sys)/errno.h $(sys)/sys/socket.h \
	$(sys)/sys/times.h $(sys)/signal.h $(sys)/fcntl.h \
	$(sys)/unistd.h $(sys)/sys/stat.h $(sys)/sys/param.h \
	$(sys)/sys/file.h $(sys)/memory.h $(sys)/sys/vfs.h \
	$(sys)/netinet/in.h $(sys)/netdb.h $(sys)/sys/un.h \
	$(sys)/dirent.h  $(sys)/pwd.h $(sys)/sys/wait.h $(sys)/termios.h \
	$(sys)/limits.h $(HSRC)/$(COMPONENT)/sun/sunlocal.h \
	$(HSRC)/$(COMPONENT)/hp9000/hp9000.h \
	$(GENHEADERS.c) \
	> helios.h

colib_link = colib.o -ltermcap

colib.o : $(HP9000)/colib.c
	$(HOSTCC) -c $(HOSTCFLAGS) -D$(HHOST) $(HP9000)/colib.c

server : helios.h $(SERVOBJ) $(XOBJECTS.co) $(LINKOBJS)
	$(HOSTCC) -o $@ $(SERVOBJ) $(LINKOBJS) $(colib_link)

serverwinxaw : serverxaw.o
	$(HOSTCC) -o $@ serverxaw.o -L/usr/lib/X11R4 -lX11 -lXt -lXaw -lXmu -lXext

serverwinxt : serverxt.o
	$(HOSTCC) -o $@ serverxt.o -L/usr/lib/X11R4 -lX11 -lXt -lXaw -lXmu -lXext -lm -ltermcap

hydra : $(HYDRAOBJ) $(LINKOBJS)
	$(HOSTCC) -o $@ $(HYDRAOBJ) $(LINKOBJS) 

hydramon : $(HYDRAMON)
	$(HOSTCC) -o $@ $(HYDRAMON)

#-----------------------------------------------------------------------------
