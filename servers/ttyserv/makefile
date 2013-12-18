#-----------------------------------------------------------------------------
# Helios generic make system - GENERIC COMPONENT MAKEFILE -
#-----------------------------------------------------------------------------
# Helios generic component make rules for: TTY server
#
# File: /HSRC/servers/ttyserv/makefile
#
# These are the host/processor independent rules and variables for this
# component of Helios. It is included by the processor specific makefile
# That may add to some of its functionality. You should only alter this
# makefile by using "ifneq ($(HPROC),YYY)" clauses for a specific processor.
#
# SccsId: %W% %G%
# RcsId: $Id: makefile,v 1.9 1991/06/25 13:24:31 ben Exp $
#		(C) Copyright 1991 Perihelion Software
#
# WARNING: assumes you are using GNUmake.
#
#
#-----------------------------------------------------------------------------

ifndef InProcMake
# This part of the makefile is run in the components generic source directory

# The variables HPROC, HHOST, HSRC, and HPROD should already be set by 
# your make script or environment, if not then get local defaults from
# makeinc/DFLT.mak. Note that the following pathname cannot use $(HSRC).
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
COMPONENT = servers/ttyserv

# Manditory includes
include $(HSRC)/makeinc/$(HHOST).mak	# Default host system variables
include $(HSRC)/makeinc/$(HPROC).mak	# Default processor rules and vars.

# Manditory phoney targets
.PHONY: default clean Xdefault Xclean ttydevs ttyserv
# "default"    - should make this component in its proc subdir
# "install"    - copy the new component to $(HPROD) production directory
# "clean"      - remove all intermediate and executable files from proc subdir
# Phoney targets prefixed by X are for adding processor specific targets.
#
# Processor specific sources are held in the processor specific subdir.,
# generic ones in the directory above it.
HVPATH := $(HSRC)/$(COMPONENT)

ifdef RSRC
HVPATH := $(RSRC)/$(COMPONENT)/$(HPROC):$(HVPATH):$(RSRC)/$(COMPONENT)
endif

vpath %.h $(HVPATH)
vpath %.c $(HVPATH)
vpath %.a $(HVPATH)


#-----------------------------------------------------------------------------
# Generic component make rules and variables added below here

# Note all final generic targets here
TARGETS = ttyserv ttydevs

default : $(TARGETS) Xdefault

install	: $(TARGETS) Xinstall
	$(MAKE) -C ../devs/$(HPROC) install
	$(MAKE) -C ../ttyserv/$(HPROC) install
	$(TOUCH) install

clean	: Xclean
	$(MAKE) -C ../devs/$(HPROC) clean
	$(MAKE) -C ../ttyserv/$(HPROC) clean
	-$(RM)	mkout core ../mkout install installdef Xinstall

#-----------------------------------------------------------------------------
# for archive, backup and cleaning purposes,
# note all generic SOURCE and OBJECT files:

# Note all sources
SOURCES		= $(SOURCES.c) $(SOURCES.a) \
		   $(HEADERS.c) $(HEADERS.m) \
		   $(SOURCES.odd)

# C sources
SOURCES.c	= 

# C header files
HEADERS.c	= 

# Any other source files
SOURCES.odd	= makefile 

#-----------------------------------------------------------------------------
# Other generic component variables:
# Set any flags required

NCFLAGS := $(NCFLAGS)

#-----------------------------------------------------------------------------
# Generic targets and their dependencies:


ttyserv:
	$(MAKE) -C ../ttyserv/$(HPROC)

ttydevs:
	$(MAKE) -C ../devs/$(HPROC)


#-----------------------------------------------------------------------------
endif #InProcMake	# This line must be at EOF
