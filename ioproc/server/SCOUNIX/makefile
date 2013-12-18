#------------------------------------------------------------------------------
# Helios generic make system - NATIVE HOST MAKEFILE -
#-----------------------------------------------------------------------------
# SCOUNIX Host specific make rules to make NATIVE HOST version of server component.
#
# File: /HSRC/ioproc/server/SCOUNIX/makefile
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
# RcsId: $Id: makefile,v 1.1 1993/07/22 15:45:36 bart Exp $
#		(C) Copyright 1990 Perihelion Software
#
# WARNING: assumes you are using GNUmake.
#-----------------------------------------------------------------------------

InProcMake = TRUE

include ../makefile

sys 	  = /usr/include
SUN = $(HSRC)/$(COMPONENT)/sun
SCOUNIX = $(HSRC)/$(COMPONENT)/scounix

HOSTCC = cc
HOSTCFLAGS := -DSCOUNIX $(HOSTCFLAGS)

#-----------------------------------------------------------------------------
# Note all final targets here

XTARGETS = server hydra hydramon serverwinxaw

Xdefault : $(XTARGETS)

Xinstall : $(XTARGETS)
	   $(CP) server $(HPROD)/server.sco
	   $(CP) serverwinxaw $(HPROD)/serverwinxaw.sco
	   $(CP) hydra $(HPROD)/hydra.sco
	   $(CP) hydramon $(HPROD)/hydramon.sco
	   $(TOUCH) Xinstall
Xclean	:


# Note all sources
XSOURCES	= $(XSOURCES.win) \
                  $(XSOURCES.co) \
                  $(XSOURCES.odd)

XSOURCES.win	= serverxaw.c
XSOURCES.co     = $(SCOUNIX)/colib.c $(SCOUNIX)/coalloc.c

XSOURCES.odd	= makefile ioserv.bm

# Object files

XOBJECTS 	= $(XOBJECTS.win) \
	          $(XOBJECTS.co) \
		  $(SERVOBJ) \
		  $(LINKOBJS) \
                  $(HYDRAOBJ) \
                  $(HYDRAMON)


XOBJECTS.win   = $(XSOURCES.win:.c=.o) 
XOBJECTS.co    = colib.o coalloc.o


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
	$(sys)/dirent.h  $(sys)/pwd.h $(sys)/sys/wait.h $(sys)/termios.h \
	$(sys)/limits.h $(SUN)/sunlocal.h $(SCOUNIX)/scounix.h \
	$(SCOUNIX)/un.h $(GENHEADERS.c) 
	> helios.h

colib_link = colib.o coalloc.o -ltermcap -lsocket

colib.o : $(SCOUNIX)/colib.c
	$(HOSTCC) -c $(HOSTCFLAGS) -D$(HHOST) $(SCOUNIX)/colib.c

coalloc.o : $(SCOUNIX)/coalloc.c
	$(HOSTCC) -c $(HOSTCFLAGS) -D$(HHOST) $(SCOUNIX)/coalloc.c

kparix.o : $(SUN)/kparix.c
	$(HOSTCC) -c $(HOSTCFLAGS) -D$(HHOST) $(SUN)/kparix.c

server : helios.h $(SERVOBJ) $(XOBJECTS.co) $(LINKOBJS)
	$(HOSTCC) -o $@ $(SERVOBJ) $(LINKOBJS) $(colib_link)


serverwinxaw : serverxaw.o
	$(HOSTCC) -o $@ serverxaw.o -lX11 -lXt -lXaw -lXmu -lXext

hydra : $(HYDRAOBJ) $(LINKOBJS)
	$(HOSTCC) -o $@ $(HYDRAOBJ) $(LINKOBJS) 

hydramon : $(HYDRAMON)
	$(HOSTCC) -o $@ $(HYDRAMON)

#-----------------------------------------------------------------------------
