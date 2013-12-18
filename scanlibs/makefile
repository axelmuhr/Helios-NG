#-----------------------------------------------------------------------------
# Helios generic make system - GENERIC COMPONENT MAKEFILE -
#-----------------------------------------------------------------------------
# Helios generic component make rules for: SCAN libraries
#
# File: /HSRC/scanlibs/makefile
#
# These are the host/processor independent rules and variables for this
# component of Helios. It is included by the processor specific makefile
# That may add to some of its functionality. You should only alter this
# makefile by using "ifneq ($(HPROC),YYY)" clauses for a specific processor.
#
# SccsId: %W% %G%
# RcsId: $Id: makefile,v 1.3 1992/12/02 14:58:42 nickc Exp $
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

#.PHONY: notarget cmds
notarget:
	@$(MAKE) -C $(HPROC)

.DEFAULT:
	@$(MAKE) -C $(HPROC) $@

else
#-----------------------------------------------------------------------------
# This part of the makefile is always run in a processor specific subdirectory.

# Manditory variables for a component:
# name of this dir relative to $(HSRC)
COMPONENT = scanlibs

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
vpath %.c $(HVPATH)
vpath %.a $(HVPATH)

#-----------------------------------------------------------------------------
# Generic component make rules and variables added below here

# Note all final generic targets here

.PHONY: notarget

TARGETS := inmessage bsd curses termcap 
ifeq ($(HPROC),TRAN)
 TARGETS := $(TARGETS) vectlib
endif
ifeq ($(HPROC),C40)
 TARGETS := $(TARGETS) vectlib IEEE64
endif
TARGETS := $(TARGETS) outmessage 

default: $(TARGETS) Xdefault

install: Xinstall $(TARGETS)
	$(MAKE) -C ../bsd/$(HPROC) install
	$(MAKE) -C ../curses/$(HPROC) install
	$(MAKE) -C ../termcap/$(HPROC) install
ifeq ($(HPROC),TRAN)
	$(MAKE) -C ../vectlib/$(HPROC) install
endif
ifeq ($(HPROC),C40)
	$(MAKE) -C ../vectlib/$(HPROC) install
	$(MAKE) -C ../IEEE64/$(HPROC) install
endif
	$(TOUCH) install

clean: Xclean
	-$(RM) mkout core install
	$(MAKE) -C ../termcap/$(HPROC) clean
	$(MAKE) -C ../curses/$(HPROC) clean
	$(MAKE) -C ../bsd/$(HPROC) clean
ifeq ($(HPROC),TRAN)
	$(MAKE) -C ../vectlib/$(HPROC) clean
endif
ifeq ($(HPROC),C40)
	$(MAKE) -C ../vectlib/$(HPROC) clean
	$(MAKE) -C ../IEEE64/$(HPROC) clean
endif

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
# Start and completion messages:
inmessage:	; @echo "          [[[ Making the Helios/$(HPROC) scan libraries ]]]"
outmessage:	; @echo "          [[[ Helios/$(HPROC) scan libraries constructed ]]]"

#-----------------------------------------------------------------------------
# Individual servers in their own directories go here:

bsd :
	$(MAKE) -C ../bsd/$(HPROC) 

curses :
	$(MAKE) -C ../curses/$(HPROC) 

termcap :
	$(MAKE) -C ../termcap/$(HPROC) 

vectlib :
	$(MAKE) -C ../vectlib/$(HPROC)

IEEE64 :
	$(MAKE) -C ../IEEE64/$(HPROC)

#-----------------------------------------------------------------------------

endif #InProcMake	# This line must be at EOF
