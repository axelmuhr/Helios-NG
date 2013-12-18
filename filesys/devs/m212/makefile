#-----------------------------------------------------------------------------
# Helios generic make system - GENERIC COMPONENT MAKEFILE -
#-----------------------------------------------------------------------------
# Helios generic component make rules for: filesys/devs/m212
#
# File: /HSRC/filesys/devs/m212/makefile
#
# These are the host/processor independent rules and variables for this
# component of Helios. It is included by the processor specific makefile
# That may add to some of its functionality. You should only alter this
# makefile by using "ifneq ($(HPROC),YYY)" clauses for a specific processor.
#
# SccsId: %W% %G%
# RcsId: $Id: makefile,v 1.9 1992/10/30 11:39:41 martyn Exp $
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
include ../../../makeinc/DFLT.mak
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
COMPONENT = filesys/devs/m212

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
VPATH := $(HSRC)/$(COMPONENT)

ifdef RSRC
VPATH := $(RSRC)/$(COMPONENT)/$(HPROC):$(HSRC)/$(COMPONENT):$(RSRC)/$(COMPONENT)
endif

#-----------------------------------------------------------------------------
# Generic component make rules and variables added below here

# Note all final generic targets here
TARGETS = m212.d loadm2

EXPORTS = discdev.c loadm2.c discdevs.a modend.a m212.h

default : $(TARGETS) Xdefault

install	:  $(TARGETS) Xinstall installsources
	$(CP) m212.d $(HPROD)/lib
	$(CP) loadm2 $(HPROD)/bin
	$(TOUCH) install


clean	: Xclean
	-$(RM)	installsources loadm2.o $(OBJECTS) $(TARGETS) $(XOBJECTS) $(XTARGETS) mkout \
		core ../mkout install installdef Xinstall


#-----------------------------------------------------------------------------
# for archive, backup and cleaning purposes,
# note all generic SOURCE and OBJECT files:

# Note all sources
SOURCES		= $(SOURCES.c) $(SOURCES.a) \
		  $(HEADERS.c) $(HEADERS.m) \
		  $(SOURCES.odd)

#Full set of library object files
OBJECTS		= $(SOURCES.c:.c=.p) $(SOURCES.a:.a=.p)
# Or if you are not a library:
#OBJECTS	= $(SOURCES.c:.c=.o) $(SOURCES.a:.a=.o)

# C source files
SOURCES.c	= discdev.c loadm2.c

# AMPP source files
SOURCES.a	= discdevs.a modend.a

# C header files
HEADERS.c	= m212.h

# AMPP macro files
HEADERS.m	=

# Any other source files
SOURCES.odd	= makefile


#-----------------------------------------------------------------------------
# Other generic component variables:

# Set any flags required
ifeq ($(HPROC),ARM)
  NCFLAGS := $(NCFLAGS)
else
  NCFLAGS := $(NCFLAGS) -ps1 -pf0
endif

# Note any library def files to be linked with
LIBDEFS	:= $(LIB)/kernel.def $(LIB)/syslib.def $(LIB)/util.def

LINKFLAGS := $(LINKFLAGS) -v -f

#-----------------------------------------------------------------------------
# Generic targets and their dependencies:

# uncomment if you have compiler or makedepend generated dependencies
#include depends

m212.d : discdevs.p discdev.p modend.p
	$(LINK) $(LINKFLAGS) -n $@ -o $@ $^ $(LIBDEFS)

loadm2 : loadm2.o
	$(LINK) $(LINKFLAGS) -o $@ $(LIB)/c0.o $^ -l$(LIB)/helios.lib -l$(LIB)/c.lib

installsources : $(EXPORTS)
	-mkdir $(HPROD)/local
	-mkdir $(HPROD)/local/src
	-mkdir $(HPROD)/local/src/hfs
	-mkdir $(HPROD)/local/src/hfs/m212
	$(foreach NAME,$(EXPORTS), $(RM) $(HPROD)/local/src/hfs/m212/$(NAME) ; \
		$(CP) ../$(NAME) $(HPROD)/local/src/hfs/m212 ; \
		$(CHMOD) gu+rw $(HPROD)/local/src/hfs/m212/$(NAME) ; )
	-$(RM) $(HPROD)/local/src/hfs/m212/makefile
	$(CP) example.mkf $(HPROD)/local/src/hfs/m212/makefile
	$(CHMOD) gu+rw $(HPROD)/local/src/hfs/m212/makefile
	$(TOUCH) $@

#-----------------------------------------------------------------------------
endif #InProcMake	# This line must be at EOF


