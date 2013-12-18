#-----------------------------------------------------------------------------
# Helios generic make system - GENERIC COMPONENT MAKEFILE -
#-----------------------------------------------------------------------------
# Helios generic component make rules for: include server
#
# File: /HSRC/servers/include/makefile
#
# These are the host/processor independent rules and variables for this
# component of Helios. It is included by the processor specific makefile
# That may add to some of its functionality. You should only alter this
# makefile by using "ifneq ($(HPROC),YYY)" clauses for a specific processor.
#
# SccsId: %W% %G%
# RcsId: $Id: makefile,v 1.5 1993/08/10 13:14:43 nickc Exp $
#		(C) Copyright 1990 Perihelion Software
#
# WARNING: assumes you are using GNUmake.
#-----------------------------------------------------------------------------

ifndef InProcMake
# This part of the makefile is run in the components generic source directory

# The variables HPROC, HHOST, HSRC, and HPROD should already be set by 
# your make script or environment, if not then get local defaults from
# $(HSRC)/makeinc/DFLT.mak. Note that the following pathname cannot use $(HSRC).
ifndef HPROC
include ../../makeinc/DFLT.mak
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
COMPONENT = servers/include

# Manditory includes
include $(HSRC)/makeinc/$(HHOST).mak	# Default host system variables
include $(HSRC)/makeinc/$(HPROC).mak	# Default processor rules and vars.

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
vpath %.a $(HVPATH)
vpath %.c $(HVPATH)

#-----------------------------------------------------------------------------
# Generic component make rules and variables added below here

# Note all final generic targets here
TARGETS = include buildinc stopinc

default : $(TARGETS) Xdefault

install	: installdef $(TARGETS) Xinstall
	test -d $(HPROD)/users/guest/examples/servers || mkdir $(HPROD)/users/guest/examples/servers
	test -d $(HPROD)/users/guest/examples/servers/include || mkdir $(HPROD)/users/guest/examples/servers/include
	-rm -f $(HPROD)/users/guest/examples/servers/include/*
	cp $(HSRC)/$(COMPONENT)/include.c $(HPROD)/users/guest/examples/servers/include
	cp $(HSRC)/$(COMPONENT)/include.h $(HPROD)/users/guest/examples/servers/include
	cp $(HSRC)/$(COMPONENT)/buildinc.c $(HPROD)/users/guest/examples/servers/include
	cp $(HSRC)/$(COMPONENT)/stopinc.c $(HPROD)/users/guest/examples/servers/include
	cp $(HSRC)/$(COMPONENT)/makefile.hel $(HPROD)/users/guest/examples/servers/include/makefile
	cp $(HSRC)/$(COMPONENT)/readme $(HPROD)/users/guest/examples/servers/include
	$(TOUCH) install

installdef:	# If this is a library, enter all .def files here (to install)
	$(TOUCH) installdef

clean	: Xclean
	-$(RM)	$(OBJECTS) $(TARGETS) $(XOBJECTS) $(XTARGETS) mkout \
		core ../mkout install installdef Xinstall


#-----------------------------------------------------------------------------
# for archive, backup and cleaning purposes,
# note all generic SOURCE and OBJECT files:

# Note all sources
SOURCES		= $(SOURCES.c) $(SOURCES.a) \
		  $(HEADERS.c) $(HEADERS.m) \
		  $(SOURCES.odd)

#Full set of library object files
#OBJECTS		= $(SOURCES.c:.c=.p) $(SOURCES.a:.a=.p)
# Or if you are not a library:
OBJECTS	= $(SOURCES.c:.c=.o) $(SOURCES.a:.a=.o)

# C source files
SOURCES.c	=	include.c buildinc.c stopinc.c

# AMPP source files
SOURCES.a	=

# C header files
HEADERS.c	=	include.h

# AMPP macro files
HEADERS.m	=

# Any other source files
SOURCES.odd	= makefile makefile.hel readme


#-----------------------------------------------------------------------------
# Other generic component variables:

# Set any flags required
ifeq ($(HPROC),ARM)
  NCFLAGS := $(NCFLAGS)
else
  NCFLAGS := $(NCFLAGS)
endif

# Note any library def files to be linked with
LIBDEFS	:= -l$(LIB)/helios.lib -l$(LIB)/c.lib

LINKFLAGS := $(LINKFLAGS)

#-----------------------------------------------------------------------------
# Generic targets and their dependencies:

include: include.o
	$(LINK) -o$@ $(LIB)/s0.o $^ $(LIBDEFS)

buildinc: buildinc.o
	$(LINK) -o$@ $(LIB)/c0.o $^ $(LIBDEFS)

stopinc: stopinc.o
	$(LINK) -o$@ $(LIB)/c0.o $^ $(LIBDEFS)

#-----------------------------------------------------------------------------
endif #InProcMake	# This line must be at EOF

