#-----------------------------------------------------------------------------
# Helios generic make system
#-----------------------------------------------------------------------------
# Helios generic component make rules for: telmat
#
# File: /HSRC/network/telmat/makefile
#
# This directory MUST NOT be shipped to customers other than Telmat.
#
# These are the host/processor independent rules and variables for this
# component of Helios. It is included by the processor specific makefile
# That may add to some of its functionality. You should only alter this
# makefile by using "ifneq ($(HPROC),YYY)" clauses for a specific processor.
#
# SccsId: %W% %G%
# RcsId: $Id: makefile,v 1.3 1992/01/14 14:26:06 bart Exp $
#		(C) Copyright 1990 Perihelion Software
#
# WARNING: assumes you are using GNUmake.
#-----------------------------------------------------------------------------

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
COMPONENT = network/telmat

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
HVPATH := $(HSRC)/$(COMPONENT)/$(HPROC):$(RSRC)/$(COMPONENT)/$(HPROC):$(HSRC)/$(COMPONENT):/$(RSRC)/$(COMPONENT)
endif

vpath %.h $(HVPATH)
vpath %.a $(HVPATH)
vpath %.c $(HVPATH)

#-----------------------------------------------------------------------------
# Generic component make rules and variables added below here

# Note all final generic targets here
TARGETS := 

default : $(TARGETS) Xdefault
ifdef BartsWorld		# nobody else should define this
	export telmat_c.d telmat_r.d /helios/lib
endif

install	: installdef $(TARGETS) Xinstall
	$(TOUCH) install

installdef: 
	$(TOUCH) installdef

clean	: Xclean
	-$(RM) $(OBJECTS) 
	-$(RM) $(TARGETS) 
	-$(RM) $(XOBJECTS)
	-$(RM) $(XTARGETS)
	-$(RM) mkout core install installdef Xinstall


#-----------------------------------------------------------------------------
# for archive, backup and cleaning purposes,
# note all generic SOURCE and OBJECT files:

# Note all sources
SOURCES		= $(SOURCES.c) $(SOURCES.a) \
		   $(HEADERS.c) $(HEADERS.m) \
		   $(SOURCES.odd)

# Full set of object files - cheat a little - all possibles
OBJECTS		= $(SOURCES.c:.c=.p) $(SOURCES.a:.a=.p) \
		  $(SOURCES.c:.c=.o) $(SOURCES.a:.a=.o)

SOURCES.c	=

# AMPP source files
SOURCES.a 	= 

# C header files
HEADERS.c	= tc_def.h tc_erreu.h tc_liste.h tc_types.h

# AMPP macro files
HEADERS.m	=

# Any other source files
SOURCES.odd	= makefile


#-----------------------------------------------------------------------------
# Other generic component variables:

DEFINES	:= $(DEFINES)

# Set any flags required
ifeq ($(HPROC),ARM)
  NCFLAGS := $(NCFLAGS)
else
 ifeq ($(HPROC),TRAN)
  NCFLAGS := $(NCFLAGS) -t4
  DEFINES := $(DEFINES)
  LINKFLAGS := $(LINKFLAGS) -f
 else
  NCFLAGS := $(NCFLAGS)
 endif
endif


ifndef SSTART
  SSTART = $(LIB)/s0.o
endif

ifndef CSTART
  CSTART = $(LIB)/c0.o
endif

# Note any library def files to be linked with
# in this case for the resource management library
LIBDEFS	:= $(LIB)/kernel.def $(LIB)/syslib.def $(LIB)/util.def $(LIB)/posix.def

# Libraries needed by most of the software


#-----------------------------------------------------------------------------
# Generic targets and their dependencies:

# uncomment if you have compiler or makedepend generated dependencies
#include depends

#-----------------------------------------------------------------------------
endif #InProcMake	# This line must be at EOF
