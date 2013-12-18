#-----------------------------------------------------------------------------
# Helios generic make system - GENERIC COMPONENT MAKEFILE -
#-----------------------------------------------------------------------------
# Helios generic component make rules for: Servers
#
# File: /HSRC/demos/makefile
#
# These are the host/processor independent rules and variables for this
# component of Helios. It is included by the processor specific makefile
# That may add to some of its functionality. You should only alter this
# makefile by using "ifneq ($(HPROC),YYY)" clauses for a specific processor.
#
# RcsId: $Id: makefile,v 1.12 1994/06/21 08:35:15 nickc Exp $
#		(C) Copyright 1990 Perihelion Software
#	(c) Copyright 1994 Perihelion Distributed Software
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
COMPONENT = demos

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
TARGETS := tut hello servers signal

# Note individual demos in own directories

DEMO = $(HPROD)/users/guest/examples

ifndef HSINGLEPROC
TARGETS := $(TARGETS) factor lb pi rmlib
endif

ifdef DEBUG_LIC
TARGETS := $(TARGETS) tiddles
endif

ifeq ($(HPROC),C40)
TARGETS := $(TARGETS) c40-dma
endif

default : Xdefault
	$(foreach SUBTARGET, $(TARGETS), $(MAKE) -C ../$(SUBTARGET)/$(HPROC);)


install	: Xinstall $(TARGETS)
	-$(RM) $(DEMO)/readme
	$(CP) ../readme $(DEMO)
	$(CHMOD) 0666 $(DEMO)/readme
	$(foreach SUBTARGET, $(TARGETS), $(MAKE) -C ../$(SUBTARGET)/$(HPROC) install;)
	$(TOUCH) install


clean	: Xclean
	-$(RM)	$(OBJECTS) $(XOBJECTS) $(XTARGETS) mkout \
		core ../mkout install installdef Xinstall
	$(foreach SUBTARGET, $(TARGETS), $(MAKE) -C ../$(SUBTARGET)/$(HPROC) clean;)

#-----------------------------------------------------------------------------
# for archive, backup and cleaning purposes,
# note all generic SOURCE and OBJECT files that are not in individual server
# directories.

# Note all sources
SOURCES		= $(SOURCES.c) $(SOURCES.a) \
		  $(HEADERS.c) $(HEADERS.m) \
		  $(SOURCES.odd)

OBJECTS	= $(SOURCES.c:.c=.o) $(SOURCES.a:.a=.o)

# C source files
SOURCES.c	=

# AMPP source files
SOURCES.a	=

# C header files
HEADERS.c	=

# AMPP macro files
HEADERS.m	=

# Any other source files
SOURCES.odd	= makefile readme


#-----------------------------------------------------------------------------
# Other generic component variables:

# Set any flags required

# Note any library def files to be linked with
LIBDEFS	:=

LINKFLAGS := $(LINKFLAGS)

#-----------------------------------------------------------------------------

# Start and completion messages:
inmessage:	; @echo "                                 [[[ Making the Helios/$(HPROC) demos ]]]"
outmessage:	; @echo "                                 [[[ Helios/$(HPROC) demos constructed ]]]"

#-----------------------------------------------------------------------------
# Individual targets in their own directories go here:


hello:
	$(MAKE) -C ../hello/$(HPROC)

tut:
	$(MAKE) -C ../tut/$(HPROC)

servers:
	$(MAKE) -C ../servers/$(HPROC)

signal:
	$(MAKE) -C ../signal/$(HPROC)

ifeq ($(HPROC),ARM)
 HSINGLEPROC = TRUE
endif

ifndef HSINGLEPROC

factor:
	$(MAKE) -C ../factor/$(HPROC)

lb:
	$(MAKE) -C ../lb/$(HPROC)

pi:
	$(MAKE) -C ../pi/$(HPROC)

rmlib:
	$(MAKE) -C ../rmlib/$(HPROC)

endif

tiddles:
	$(MAKE) -C ../tiddles/$(HPROC)

ifeq ($(HPROC),C40)
c40-dma:
	$(MAKE) -C ../c40-dma/$(HPROC)
endif

#-----------------------------------------------------------------------------
endif #InProcMake	# This line must be at EOF
