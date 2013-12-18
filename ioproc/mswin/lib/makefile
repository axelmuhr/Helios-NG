#-----------------------------------------------------------------------------
# Helios generic make system
#-----------------------------------------------------------------------------
# Helios generic component make rules for: KERNEL
#
# File: /HSRC/ioproc/mswin/lib/makefile
#
# These are the host/processor independent rules and variables for this
# component of Helios. It is included by the processor specific makefile
# That may add to some of its functionality. You should only alter this
# makefile by using "ifneq ($(HPROC),YYY)" clauses for a specific processor.
#
# RcsId: $Id: makefile,v 1.14 1993/11/26 15:19:42 nickc Exp nickc $
#		(C) Copyright 1990 Perihelion Software
#	(C) Copyright 1994 Perihelion Distributed Software
#
# WARNING: assumes you are using GNUmake.
#-----------------------------------------------------------------------------

ifndef InProcMake
# This part of the makefile is run in the components generic source directory

# The variables HPROC, HHOST, HSRC, and HPROD should already be set by 
# your make script or environment, if not then get local defaults from
# makeinc/DFLT.mak. Note that the following pathname cannot use $(HSRC).
ifndef HPROC
include ../makeinc/DFLT.mak
endif

.PHONY: notarget
notarget:
	@$(MAKE) -C $(HPROC)

.DEFAULT:
	@$(MAKE) -C $(HPROC) $@

else
#-----------------------------------------------------------------------------
# This part of the makefile is always run in a processor specific subdirectory.

# Manditory variables for a component:
# name of this dir relative to $(HSRC)
COMPONENT = ioproc/mswin/lib

# Manditory includes
include $(HSRC)/makeinc/$(HHOST).mak
include $(HSRC)/makeinc/$(HPROC).mak

# Manditory phoney targets
.PHONY: default clean Xdefault Xclean
# "default"    - should make this component in its proc subdir
# "install"    - copy the new component to $(HPROD) production directory
# "installdef" - If component is a library, make .def file and install in LIB
# "clean"      - remove all intermediate and executable files from proc subdir
# Phoney targets prefixed by X are for adding processor specific targets.
#
# Processor specific sources are held in the processor specific subdir.,
# generic ones in the directory above it.
HVPATH := $(HSRC)/$(COMPONENT)

ifdef RSRC
HVPATH := $(RSRC)/$(COMPONENT)/$(HPROC):$(HSRC)/$(COMPONENT):$(RSRC)/$(COMPONENT)
endif

vpath %.h $(HVPATH)
vpath %.c $(HVPATH)
vpath %.a $(HVPATH)


#-----------------------------------------------------------------------------
# Generic component make rules and variables added below here

# Note all final generic targets here

TARGETS	= windows.lib startws mslights ddetest gditest hello

default : $(TARGETS) Xdefault

install	: installdef $(TARGETS) Xinstall
	@ test -d $(LIB) || $(MKDIR) -p $(LIB)
	$(CP) windows.lib $(LIB)
	@ test -d $(HPROD)/local/bin || $(MKDIR) -p $(HPROD)/local/bin
	$(CP) startws  $(HPROD)/local/bin
	$(CP) mslights $(HPROD)/local/bin
	@ test -d $(HPROD)/include || $(MKDIR) -p $(HPROD)/include
	$(RM) $(HPROD)/include/windows.h
	$(RM) $(HPROD)/include/ddeml.h
	$(CP) ../windows.h $(HPROD)/include
	$(CP) ../ddeml.h $(HPROD)/include
	$(CHMOD) ug+w $(HPROD)/include/windows.h
	$(CHMOD) ug+w $(HPROD)/include/ddeml.h
	@ test -d $(HPROD)/users/guest || $(MKDIR) -p $(HPROD)/users/guest
	$(RM) -f $(HPROD)/users/guest/login.msw
	$(CP) ../login.msw $(HPROD)/users/guest
	$(CHMOD) ug+w $(HPROD)/users/guest/login.msw
	@ test -d $(HPROD)/users/guest/examples/mswin || $(MKDIR) -p $(HPROD)/users/guest/examples/mswin
	-$(RM) $(HPROD)/users/guest/examples/mswin/*
	$(CP) ../makefile.hel $(HPROD)/users/guest/examples/mswin/makefile
	$(CP) ../hello.c $(HPROD)/users/guest/examples/mswin
	$(CP) ../gditest.c $(HPROD)/users/guest/examples/mswin
	$(CP) ../ddetest.c $(HPROD)/users/guest/examples/mswin
	$(CP) ../mslights.c $(HPROD)/users/guest/examples/mswin
	$(CP) ../mslights.h $(HPROD)/users/guest/examples/mswin
	$(CP) ../readme.txt $(HPROD)/users/guest/examples/mswin
	$(CHMOD) ug+w $(HPROD)/users/guest/examples/mswin/*
	$(TOUCH) install

installdef:
	$(TOUCH) installdef

clean	: Xclean
	-$(RM)	$(OBJECTS) $(TARGETS) $(XOBJECTS) $(XTARGETS) \
		mkout core ../mkout install installdef Xinstall


#-----------------------------------------------------------------------------
# for archive, backup and cleaning purposes,
# note all generic SOURCE and OBJECT files:

SOURCES	=	$(SOURCES.c) $(SOURCES.a) $(HEADERS.c) $(HEADERS.m) \
		$(SOURCES.odd)


SOURCES.c=	ddeml.c gdi.c mslights.c startws.c windows.c ddetest.c \
		hello.c gditest.c

SOURCES.a =

# C header files
HEADERS.c =	ddeml.h windefs.h windows.h mslights.h

# AMPP macro files
HEADERS.m =

SOURCES.odd = makefile winfull.h login.msw readme.txt makefile.hel mswi.ins

OBJECTS = $(SOURCES.c:.c=.o)

#-----------------------------------------------------------------------------
# Generic targets and their dependencies:

# uncomment if you have compiler or makedepend generated dependencies
#include depends

# Turn off stack check and function header generation.
ifeq ($(HPROC),ARM)
  NCFLAGS := $(NCFLAGS) -Zps1
else
  ifeq ($(HPROC),C40)
    NCFLAGS := $(NCFLAGS) -D_BSD 
  else
    ifdef SYSDEB
      NCFLAGS := $(NCFLAGS) -ps1
    endif
    NCFLAGS := $(NCFLAGS) -wd -D_BSD
  endif
endif

# Note any library def files to be linked with
LIBDEFS	:= -l$(LIB)/servlib.def -l$(LIB)/RmLib.def

LINKFLAGS := $(LINKFLAGS)

#-----------------------------------------------------------------------------
# Generic targets and their dependencies:

# uncomment if you have compiler or makedepend generated dependencies
#include depends

W_OBJS  = windows.o gdi.o ddeml.o
LW_OBJS = $(patsubst %,-l%,$(W_OBJS))
GW_OBJS = $(W_OBJS:.o=.o.ghof)

windows.lib : $(W_OBJS) 
ifeq ($(HPROC),TRAN)
	$(LINK) -p -o$@ $^
else
ifeq ($(HPROC),ARM)
	@ echo Creating scanned library $@ ...
	@ $(LINK) $(LW_OBJS) -o fred
	@ cat $(GW_OBJS) > $@
else
	@ echo Creating scanned library $@ ...
	@ cat $^ > $@
endif

endif
		
startws: startws.o windows.lib
	$(LINK) $(LINKFLAGS) -o$@ $(CSTART) startws.o $(LIBDEFS) -l./windows.lib

mslights : mslights.o windows.lib
	$(LINK) -o$@ $(CSTART) mslights.o $(LIBDEFS) -l./windows.lib

ddetest : ddetest.o windows.lib
	$(LINK) -o$@ $(CSTART) ddetest.o $(LIBDEFS) -l./windows.lib -l$(LIB)/fplib.def

gditest : gditest.o windows.lib
	$(LINK) -o$@ $(CSTART) gditest.o $(LIBDEFS) -l./windows.lib

hello : hello.o windows.lib
	$(LINK) -o$@ $(CSTART) hello.o $(LIBDEFS) -l./windows.lib

#-----------------------------------------------------------------------------
endif #InProcMake	# This line must be at EOF
