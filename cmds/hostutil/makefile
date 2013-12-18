#-----------------------------------------------------------------------------
# Helios generic make system
#-----------------------------------------------------------------------------
# Helios generic component make rules for: Host Utilities
#
# File: /HSRC/cmds/hostutil/makefile
#
# *This makefile basically used to note what the sources are so we can tar them*
#
# These are the host/processor independent rules and variables for this
# component of Helios. It is included by the processor specific makefile
# That may add to some of its functionality. You should only alter this
# makefile by using "ifneq ($(HPROC),YYY)" clauses for a specific processor.
#
# SccsId: %W% %G%
# RcsId: $Id: makefile,v 1.8 1994/06/08 09:05:25 vlsi Exp $
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
COMPONENT = cmds/hostutil

# Manditory includes
include $(HSRC)/makeinc/$(HHOST).mak	# Default host system variables
include $(HSRC)/makeinc/$(HPROC).mak	# Default processor rules and vars.

# Manditory phoney targets
.PHONY: default clean Xdefault Xclean
# "default" - should make this component in its proc subdir
# "install" - copy the new component to $(HPROD) production directory
# "clean"   - remove all intermediate and executable files from proc subdir
# Phoney targets prefixed by X are for adding processor specific targets.
#
# Processor specific sources are held in the processor specific subdir.,
# generic ones in the directory above it.
VPATH := $(HSRC)/$(COMPONENT)

#-----------------------------------------------------------------------------
# Generic component make rules and variables added below here

# Note all final generic targets here
TARGETS = newmotd gencodes squash buildtime

default : $(TARGETS) Xdefault

install	: $(TARGETS) Xinstall
	$(CP) $(TARGETS) $(HHOSTBIN)
	$(TOUCH) install

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

OBJECTS	= $(SOURCES.c:.c=.o)

# C source files
SOURCES.c	= gencodes.c newmotd.c squash.c testenc.c buildtime.c

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
LIBDEFS	=


#-----------------------------------------------------------------------------
# Generic targets and their dependencies:

# These may be used by a self building version of Helios


#---------------------------------------------------------------------------
# Construct a standard "motd" file

newmotd: newmotd.o
	$(LINK) $(LINKFLAGS) -o $@ -n$@ $(CSTART) $^

#-----------------------------------------------------------------------------
# Assembler header file generation

buildtime: buildtime.o
	$(LINK) $(LINKFLAGS) -o $@ -n$@ $(CSTART) $^

#-----------------------------------------------------------------------------
# Fault library host utilities:

gencodes : gencodes.o
	$(LINK) $(LINKFLAGS) -o $@ -n$@ $(CSTART) $^

squash : squash.o
	$(LINK) $(LINKFLAGS) -o $@ -n$@ $(CSTART) $^


#---------------------------------------------------------------------------
# Test encoding functions for helios object files.

testenc: testenc.o
	$(LINK) $(LINKFLAGS) -o $@ -n$@ $(CSTART) $^


#-----------------------------------------------------------------------------
endif #InProcMake	# This line must be at EOF
