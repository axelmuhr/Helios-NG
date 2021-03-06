#-----------------------------------------------------------------------------
# Helios generic make system - GENERIC COMPONENT MAKEFILE -
#-----------------------------------------------------------------------------
# Helios generic component make rules for: Servers
#
# File: /HSRC/demos/pi/makefile
#
# These are the host/processor independent rules and variables for this
# component of Helios. It is included by the processor specific makefile
# That may add to some of its functionality. You should only alter this
# makefile by using "ifneq ($(HPROC),YYY)" clauses for a specific processor.
#
# SccsId: %W% %G%
# RcsId: $Id: makefile,v 1.8 1993/08/13 10:50:47 nickc Exp $
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
COMPONENT = demos/pi

# Manditory includes
include $(HSRC)/makeinc/$(HHOST).mak	# Default host system variables
include $(HSRC)/makeinc/$(HPROC).mak	# Default processor rules and vars.

# Manditory phoney targets
.PHONY: default clean Xdefault Xclean pi_farm pi_fast pi_fort pi_mix pi_mod2 pi_pasc pi_ring
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
ifeq ($(HPROC),TRAN)
 TARGETS = pi_farm pi_fast pi_fort pi_mix pi_mod2 pi_pasc pi_ring
else
 TARGETS = pi_farm pi_fast pi_ring
endif

DEMO = $(HPROD)/users/guest/examples/pi

# Note individual demos in own directories

default : Xdefault
	$(foreach SUBTARGET, $(TARGETS), $(MAKE) -C ../$(SUBTARGET)/$(HPROC);)


install	: Xinstall $(TARGETS)
	test -d $(DEMO) || $(MKDIR) $(DEMO)
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
SOURCES.odd	= makefile


#-----------------------------------------------------------------------------
# Other generic component variables:

# Set any flags required
ifeq ($(HPROC),ARM)
  NCFLAGS := $(NCFLAGS)
else
  NCFLAGS := $(NCFLAGS)
endif

# Note any library def files to be linked with
LIBDEFS	:=

LINKFLAGS := $(LINKFLAGS)

#-----------------------------------------------------------------------------

# Start and completion messages:
inmessage:	; @echo "                                 [[[ Making the Helios/$(HPROC) pi demos ]]]"
outmessage:	; @echo "                                 [[[ Helios/$(HPROC) pi demos constructed ]]]"

#-----------------------------------------------------------------------------
# Individual targets in their own directories go here:

pi_farm:
	$(MAKE) -C ../pi_farm/$(HPROC)

pi_fast:
	$(MAKE) -C ../pi_fast/$(HPROC)

pi_fort:
	$(MAKE) -C ../pi_fort/$(HPROC)

pi_mix:
	$(MAKE) -C ../pi_mix/$(HPROC)

pi_mod2:
	$(MAKE) -C ../pi_mod2/$(HPROC)

pi_pasc:
	$(MAKE) -C ../pi_pasc/$(HPROC)

pi_ring:
	$(MAKE) -C ../pi_ring/$(HPROC)


#-----------------------------------------------------------------------------
endif #InProcMake	# This line must be at EOF
