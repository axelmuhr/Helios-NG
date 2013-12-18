#-----------------------------------------------------------------------------
# Helios generic make system - GENERIC COMPONENT MAKEFILE -
#-----------------------------------------------------------------------------
# Helios generic component make rules for: pi examples
#
# File: /HSRC/demos/pi/pi_mod2/makefile
#
# These are the host/processor independent rules and variables for this
# component of Helios. It is included by the processor specific makefile
# That may add to some of its functionality. You should only alter this
# makefile by using "ifneq ($(HPROC),YYY)" clauses for a specific processor.
#
# SccsId: %W% %G%
# RcsId: $Id: makefile,v 1.8 1993/08/17 16:47:44 bart Exp $
#		(C) Copyright 1990 Perihelion Software
#
# WARNING: assumes you are using GNUmake.
#
# $Header: /hsrc/demos/pi/pi_mod2/RCS/makefile,v 1.8 1993/08/17 16:47:44 bart Exp $
#
#-----------------------------------------------------------------------------

MV = mv

ifndef InProcMake
# This part of the makefile is run in the components generic source directory

  # These variables should already be set by your make script or environment.
  ifndef HPROC
    HPROC = TRAN
  endif

  ifndef HHOST
    HHOST = SUN4
  endif

  ifndef HSRC
    HSRC = /usr/perihelion/Helios
  endif

  ifndef HPROD
    HPROD = $(HSRC)/slashhelios/$(HPROC)
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
COMPONENT = demos/pi/pi_mod2

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

#-----------------------------------------------------------------------------
# Generic component make rules and variables added below here

DEMO = $(HPROD)/users/guest/examples/pi/pi_mod2

# Note all final generic targets here
TARGETS := ../makefile.hel ../pi.cdl ../worker.mod ../control.mod ../termbase.mod

default : $(TARGETS) Xdefault

install	: $(TARGETS) Xinstall
	-$(MKDIR) $(DEMO)
	-$(RM) $(DEMO)/*
	$(CP) $(TARGETS) $(DEMO)
	$(CHMOD) 0666 $(DEMO)/*
	$(MV) $(DEMO)/makefile.hel $(DEMO)/makefile
	$(TOUCH) install

clean	: Xclean
	-$(RM)	$(OBJECTS) $(XOBJECTS) $(XTARGETS) mkout \
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
SOURCES.c	= 

# AMPP source files
SOURCES.a	=


# C header files
HEADERS.c	= 

# AMPP macro files
HEADERS.m	=

# Any other source files
SOURCES.odd	= control.mod makefile makefile.hel pi.cdl termbase.mod worker.mod


#-----------------------------------------------------------------------------
# Other generic component variables:
# @@@ should remove -DHELIOS and change source to reference __HELIOS
# Set any flags required
ifeq ($(HPROC),ARM)
  NCFLAGS := $(NCFLAGS) -DHELIOS -w
else
  NCFLAGS := $(NCFLAGS) -DHELIOS -wadfsv -ecpz -t4
endif


#-----------------------------------------------------------------------------
# Generic targets and their dependencies:

# uncomment if you have compiler or makedepend generated dependencies
#include depends

#$(OBJECTS): $(HEADERS.c)

#-----------------------------------------------------------------------------
endif #InProcMake	# This line must be at EOF
