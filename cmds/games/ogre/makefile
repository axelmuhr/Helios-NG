#-----------------------------------------------------------------------------
# Helios generic make system - GENERIC COMPONENT MAKEFILE -
#-----------------------------------------------------------------------------
# Helios generic component make rules for: Games
#
# File: /HSRC/cmds/games/ogre/makefile
#
# These are the host/processor independent rules and variables for this
# component of Helios. It is included by the processor specific makefile
# That may add to some of its functionality. You should only alter this
# makefile by using "ifneq ($(HPROC),YYY)" clauses for a specific processor.
#
# SccsId: %W% %G%
# RcsId: $Id: makefile,v 1.2 1991/12/02 15:28:40 martyn Exp $
#		(C) Copyright 1990 Perihelion Software
#
# WARNING: assumes you are using GNUmake.
#
#
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
COMPONENT = cmds/games/ogre

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
TARGETS = ogre

newdefault: default


#-----------------------------------------------------------------------------
# for archive, backup and cleaning purposes,
# note all generic SOURCE and OBJECT files:

# AMPP source files
SOURCES.a	=

# C header files
HEADERS.c	= ext.h ogre.h

# AMPP macro files
HEADERS.m	=

# Any other source files
SOURCES.odd	= makefile

# Bog standard games
SOURCES.c	= attack.c init.c initroun.c main.c map.c move.c ogrecom.c \
		ogrestat.c resolve.c termcap.c

default: $(TARGETS) Xdefault

# Note all sources
SOURCES		= $(SOURCES.c) $(SOURCES.a) \
		   $(HEADERS.c) $(HEADERS.m) \
		   $(SOURCES.odd)

OBJECTS	= $(SOURCES.c:.c=.o)

#-----------------------------------------------------------------------------
# Other generic component variables:

# Set any flags required
ifeq ($(HPROC),ARM)
  NCFLAGS := $(NCFLAGS)
else
  NCFLAGS := $(NCFLAGS) -wadfsv -ecpz -D_BSD
endif

LINKFLAGS := $(LINKFLAGS) -l$(LIB)/bsd.lib -l$(LIB)/curses.lib -l$(LIB)/termcap.lib

#-----------------------------------------------------------------------------
# Due to the use of ':=', rather than '=', these rules have to appear here.

default: $(TARGETS) Xdefault

install	: $(TARGETS) Xinstall
	$(TOUCH) install

installdef : Xinstalldef
	$(TOUCH) installdef

clean	: Xclean
	-$(RM)	$(OBJECTS) $(TARGETS) $(XOBJECTS) $(XTARGETS) mkout \
		core ../mkout install installdef Xinstall

#-----------------------------------------------------------------------------

ogre : attack.o init.o initroun.o main.o map.o move.o ogrecom.o \
		ogrestat.o resolve.o termcap.o
	$(LINK) -o$@ -n$@ -s2000 -h4000 $(CSTART) $^ $(LINKFLAGS)

endif #InProcMake	# This line must be at EOF
