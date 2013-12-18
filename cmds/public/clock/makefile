#-----------------------------------------------------------------------------
# Helios generic make system - GENERIC COMPONENT MAKEFILE -
#-----------------------------------------------------------------------------
# Helios generic component make rules for: cmds/public/clock/gdc
#
# File: /HSRC/cmds/public/clock/makefile
#
# These are the host/processor independent rules and variables for this
# component of Helios. It is included by the processor specific makefile
# That may add to some of its functionality. You should only alter this
# makefile by using "ifneq ($(HPROC),YYY)" clauses for a specific processor.
#
# RcsId: $Id: makefile,v 1.5 1994/05/12 17:01:56 nickc Exp $
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
COMPONENT = cmds/public/clock

# Manditory includes
include $(HSRC)/makeinc/$(HHOST).mak	# Default host system variables
include $(HSRC)/makeinc/$(HPROC).mak	# Default processor rules and vars.

# Manditory phoney targets
.PHONY: default clean Xdefault Xclean
# "default"    - should make this component in its proc subdir
# "install"    - copy the new component to $(HPROD) production directory
# "clean"      - remove all intermediate and executable files from proc subdir
# Phoney targets prefixed by X are for adding processor specific targets.
#
# Processor specific sources are held in the processor specific subdir.,
# generic ones in the directory above it.
VPATH := $(HSRC)/$(COMPONENT)

ifdef RSRC
VPATH := $(RSRC)/$(COMPONENT)/$(HPROC):$(VPATH):$(RSRC)/$(COMPONENT)
endif

	#-----------------------------------------------------------------------------
# Generic component make rules and variables added below here

# Note all final generic targets here
TARGETS = gdc

default : $(TARGETS) Xdefault

install	: $(TARGETS) Xinstall
	$(CP) $(TARGETS) $(HPROD)/local/bin
	$(TOUCH) install

clean	: Xclean
	-$(RM)	$(OBJECTS) $(TARGETS) $(XOBJECTS) $(XTARGETS) mkout \
		core ../mkout install Xinstall *.ghof


#-----------------------------------------------------------------------------
# for archive, backup and cleaning purposes,
# note all generic SOURCE and OBJECT files:

# Note all sources
SOURCES		= $(SOURCES.c) $(SOURCES.a) \
		  $(HEADERS.c) $(HEADERS.m) \
		  $(SOURCES.odd)

OBJECTS	= $(SOURCES.c:.c=.o) $(SOURCES.a:.a=.o)

# C source files
SOURCES.c	=  gdc.c

# AMPP source files
SOURCES.a	= 

# C header files
HEADERS.c	= 

# AMPP macro files
HEADERS.m	=

# Any other source files
SOURCES.odd	= makefile Makefile.old gdc.6

#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# Other generic component variables:

# Set any flags required
NCFLAGS := $(NCFLAGS) -DSTACK_DIRECTION=-1 -D_BSD


#-----------------------------------------------------------------------------
# Generic targets and their dependencies:

gdc: $(OBJECTS)
	@ echo linking $@ ...
	@ $(LINK) $(LINKFLAGS) -o gdc $(LIB)/cstart.o $(OBJECTS) -l$(LIB)/curses.lib -l$(LIB)/bsd.lib -l$(LIB)/termcap.lib



#-----------------------------------------------------------------------------
endif #InProcMake	# This line must be at EOF


