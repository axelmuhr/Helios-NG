#-----------------------------------------------------------------------------
# Helios generic make system
#-----------------------------------------------------------------------------
# Helios generic component make rules for:
#
# File: /HSRC/ioproc/mswin/printer/makefile
#
# These are the host/processor independent rules and variables for this
# component of Helios. It is included by the processor specific makefile
# That may add to some of its functionality. You should only alter this
# makefile by using "ifneq ($(HPROC),YYY)" clauses for a specific processor.
#
# SccsId: %W% %G%
# RcsId: $Id: makefile,v 1.2 1993/08/13 16:52:33 bart Exp $
#		(C) Copyright 1990 Perihelion Software
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
COMPONENT = ioproc/mswin/printer

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

TARGETS	= dummy

default : $(TARGETS) Xdefault

install	: installdef $(TARGETS) Xinstall
#	$(CP) $(TARGETS) $(BIN)
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

OBJECTS = 	$(SOURCES.c:.c=.o)

# C source files
SOURCES.c =

SOURCES.a =

# C header files
HEADERS.c =

# AMPP macro files
HEADERS.m =

SOURCES.odd = makefile

#-----------------------------------------------------------------------------
# Generic targets and their dependencies:

# uncomment if you have compiler or makedepend generated dependencies
#include depends

# Turn off stack check and function header generation.
ifeq ($(HPROC),ARM)
  NCFLAGS := $(NCFLAGS) -Zps1
else
  ifeq ($(HPROC),C40)
    NCFLAGS := $(NCFLAGS)
  else
    ifdef SYSDEB
      NCFLAGS := $(NCFLAGS) -ps1
    endif
    NCFLAGS := $(NCFLAGS) -wd
  endif
endif

# Note any library def files to be linked with
LIBDEFS	:=

LINKFLAGS := $(LINKFLAGS)

#-----------------------------------------------------------------------------
# Generic targets and their dependencies:

# uncomment if you have compiler or makedepend generated dependencies
#include depends

dummy:
	touch dummy

#-----------------------------------------------------------------------------
endif #InProcMake	# This line must be at EOF
