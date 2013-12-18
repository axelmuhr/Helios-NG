#------------------------------------------------------------------------------
# Helios generic make system - NATIVE HOST MAKEFILE -
#-----------------------------------------------------------------------------
# SUN386 Host specific make rules to make NATIVE HOST version of server component.
#
# File: /HSRC/ioproc/server/SUN386/makefile
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
# RcsId: $Id: nativehost.template,v 1.1 91/02/12 17:12:59 nick Exp $
#		(C) Copyright 1990 Perihelion Software
#
# WARNING: assumes you are using GNUmake.
#-----------------------------------------------------------------------------

InProcMake = TRUE

include ../makefile

sys 	  = /usr/include
SUN = $(HSRC)/$(COMPONENT)/sun

#-----------------------------------------------------------------------------
# Note all final targets here

XTARGETS = helios.h 

Xdefault : $(XTARGETS)

Xinstall : $(XTARGETS)
	   $(CP) server $(HPROD)/server.sun386
	   $(CP) serverwindow $(HPROD)/serverwindow.sun386
	   $(CP) hydra $(HPROD)/hydra.sun386
	   $(CP) hydramon $(HPROD)/hydramon.sun386
	   $(TOUCH) install
Xclean	:
	-$(RM)	$(XOBJECTS) $(XTARGETS) mkout core ../mkout install

# Note all sources
XSOURCES	= $(XSOURCES.win) \
                  $(XSOURCES.co) \
                  $(XSOURCES.odd)

XSOURCES.win	= serverwi.c 
XSOURCES.co     = colib.c 

SOURCES.odd	= makefile 

# Object files

XOBJECTS 	= $(XOBJECTS.win) \
	          $(XOBJECTS.co) \
		  $(SERVOBJ) \
		  $(LINKOBJS) \
                  $(HYDRAOBJ) \
                  $(HYDRAMON)


XOBJECTS.win   = $(XSOURCES.win:.c=.o) 
XOBJECTS.co    = $(XSOURCES.co:.c=.o)


$(XOBJECTS) : helios.h

#-----------------------------------------------------------------------------
# Native Host targets and their dependencies:

# uncomment if you have compiler or makedepend generated dependencies
#include depends

HOSTCFLAGS := $(HOSTCFLAGS) -I. -I$(SUN) -Ikludge  

helios.h: $(HEADERS.c)
	cat $(sys)/stdio.h $(sys)/sys/types.h $(sys)/ctype.h $(sys)/setjmp.h \
	$(sys)/sys/time.h $(sys)/errno.h $(sys)/sys/socket.h \
	$(sys)/sys/times.h $(sys)/signal.h $(sys)/fcntl.h \
	$(sys)/unistd.h $(sys)/sys/stat.h $(sys)/sys/param.h \
	$(sys)/sys/file.h $(sys)/memory.h $(sys)/sys/vfs.h \
	$(sys)/netinet/in.h $(sys)/netdb.h $(sys)/sys/un.h \
	$(sys)/dirent.h  $(sys)/termio.h $(sys)/pwd.h \
	$(HSRC)/$(COMPONENT)/sun/sunlocal.h \
	$(HEADERS.c) \
	> helios.h

colib_link = -Bstatic colib.o -ltermcap -llwp
colib.o : colib.c
	$(HOSTCC) -c $(HOSTCFLAGS) -D$(HHOST) -DLWP $(HSRC)/$(COMPONENT)/sun/colib.c


server : helios.h $(SERVOBJ) $(XOBJECTS.co) $(LINKOBJS)
	$(HOSTCC) -o $@ $(SERVOBJ) \
                            $(LINKOBJS) $(colib_link)


serverwindow : $(XOBJECTS.win) 
	$(HOSTCC) -o $@ $(XOBJECTS.win) -lsuntool \
                      -lsunwindow -lpixrect

hydra : $(HYDRAOBJ) $(LINKOBJS)
	$(HOSTCC) -o $@ $(HYDRAOBJ) $(LINKOBJS) 

hydramon : $(HYDRAMON)
	$(HOSTCC) -o $@ $(HYDRAMON)

#-----------------------------------------------------------------------------
