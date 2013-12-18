#------------------------------------------------------------------------------
# Helios generic make system - NATIVE HOST MAKEFILE -
#-----------------------------------------------------------------------------
# RS6000 Host specific make rules to make NATIVE HOST version of server component.
#
# File: /HSRC/ioproc/server/RS6000/makefile
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
# RcsId: $Id: makefile,v 1.4 1993/09/24 11:51:19 bart Exp $
#		(C) Copyright 1990 Perihelion Software
#
# WARNING: assumes you are using GNUmake.
#-----------------------------------------------------------------------------

InProcMake = TRUE

include ../makefile

sys 	  = /usr/include
SUN = $(HSRC)/$(COMPONENT)/sun
RS6000 = $(HSRC)/$(COMPONENT)/rs6000

HOSTCC = cc
HOSTCFLAGS := -DRS6000 $(HOSTCFLAGS)

#-----------------------------------------------------------------------------
# Note all final targets here

XTARGETS = server hydra hydramon serverwinxaw serverwinxt

Xdefault : $(XTARGETS)

Xinstall : $(XTARGETS)
	   $(CP) server $(HPROD)/server.rs6000
	   $(CP) serverwinxaw $(HPROD)/serverwinxaw.rs6000
	   $(CP) serverwinxt $(HPROD)/serverwinxt.rs6000
	   $(CP) hydra $(HPROD)/hydra.rs6000
	   $(CP) hydramon $(HPROD)/hydramon.rs6000
	   $(TOUCH) Xinstall
Xclean	:


# Note all sources
XSOURCES	= $(XSOURCES.win) \
                  $(XSOURCES.co) \
                  $(XSOURCES.odd)

XSOURCES.win	= serverxaw.c serverxt.c
XSOURCES.co     = $(RS6000)/colib.c $(RS6000)/ctx.s

XSOURCES.odd	= makefile ioserv.bm

# Object files

XOBJECTS 	= $(XOBJECTS.win) \
	          $(XOBJECTS.co) \
		  $(SERVOBJ) \
		  $(LINKOBJS) \
                  $(HYDRAOBJ) \
                  $(HYDRAMON)


XOBJECTS.win   = $(XSOURCES.win:.c=.o) 
XOBJECTS.co    = colib.o ctx.o


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
	$(sys)/sys/file.h $(sys)/memory.h $(sys)/sys/vfs.h \
	$(sys)/netinet/in.h $(sys)/netdb.h $(sys)/sys/un.h \
	$(sys)/dirent.h  $(sys)/termio.h $(sys)/pwd.h $(sys)/sys/wait.h \
	$(sys)/sys/select.h $(sys)/sys/statfs.h \
	$(HSRC)/$(COMPONENT)/sun/sunlocal.h \
	$(GENHEADERS.c) \
	> helios.h

colib_link = colib.o ctx.o -ltermcap

colib.o : $(RS6000)/colib.c
	$(HOSTCC) -c $(HOSTCFLAGS) -D$(HHOST) $(RS6000)/colib.c

ctx.o: $(RS6000)/ctx.s
	as -o ctx.o $(RS6000)/ctx.s

server : helios.h $(SERVOBJ) $(XOBJECTS.co) $(LINKOBJS)
	$(HOSTCC) -o $@ $(SERVOBJ) $(LINKOBJS) $(colib_link) -lbsd

serverwinxaw : serverxaw.o
	$(HOSTCC) -o $@ serverxaw.o -lX11 -lXt -lXaw -lXmu -lXext

serverwinxt : serverxt.o
	$(HOSTCC) -o $@ serverxt.o -lX11 -lXt -lXaw -lXmu -lXext -lm -ltermcap

hydra : $(HYDRAOBJ) $(LINKOBJS)
	$(HOSTCC) -o $@ $(HYDRAOBJ) $(LINKOBJS) 

hydramon : $(HYDRAMON)
	$(HOSTCC) -o $@ $(HYDRAMON)

#-----------------------------------------------------------------------------
