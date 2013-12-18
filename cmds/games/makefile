#-----------------------------------------------------------------------------
# Helios generic make system - GENERIC COMPONENT MAKEFILE -
#-----------------------------------------------------------------------------
# Helios generic component make rules for: Games
#
# File: /HSRC/cmds/games/makefile
#
# These are the host/processor independent rules and variables for this
# component of Helios. It is included by the processor specific makefile
# That may add to some of its functionality. You should only alter this
# makefile by using "ifneq ($(HPROC),YYY)" clauses for a specific processor.
#
# SccsId: %W% %G%
# RcsId: $Id: makefile,v 1.20 1991/08/14 11:16:35 martyn Exp $
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
COMPONENT = cmds/games

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
TARGETS = arithmet back bj craps fish maze moo number random ttt wump \
	battship rain mz paranoia puzzle greed cg rot ski

newdefault: default


#-----------------------------------------------------------------------------
# for archive, backup and cleaning purposes,
# note all generic SOURCE and OBJECT files:

# AMPP source files
SOURCES.a	=

# C header files
HEADERS.c	= ski.h

# AMPP macro files
HEADERS.m	=

# Any other source files
SOURCES.odd	= makefile

# Bog standard games
SOURCES.c	= arithmet.c back.c bj.c craps.c fish.c maze.c moo.c \
		number.c random.c ttt.c wump.c battship.c rain.c \
		mz.c mzio.c paranoia.c puzzle.c greed.c cg.c rot.c \
		ski.c do_level.c

default: $(TARGETS) Xdefault

# Note all sources
SOURCES		= $(SOURCES.c) $(SOURCES.a) \
		   $(HEADERS.c) $(HEADERS.m) \
		   $(SOURCES.odd)

OBJECTS	= $(SOURCES.c:.c=.o)

SUBDIRS = ogre

#-----------------------------------------------------------------------------
# Other generic component variables:

# Set any flags required
ifeq ($(HPROC),ARM)
  NCFLAGS := $(NCFLAGS)
else
  NCFLAGS := $(NCFLAGS) -wadfsv -ecpz -D_BSD
endif

LINKFLAGS := $(LINKFLAGS) -l$(LIB)/bsd.lib -l$(LIB)/curses.lib -l$(LIB)/termcap.lib

%: %.o
	$(LINK) $(LINKFLAGS) -o$@ -n$@ -s2000 -h4000 $(CSTART) $@.o

$(OBJECTS):

#-----------------------------------------------------------------------------
# Special targets and their dependencies:

battship: battship.c
	$(CC) -o$@ $< -wA -D_BSD -DNDEBUG -DPENGUIN -l$(LIB)/bsd.lib \
		-l$(LIB)/curses.lib -l$(LIB)/termcap.lib

greed: greed.c
	$(CC) -o$@ $< -wA -D_BSD -DSCOREFILE=\"/helios/local/games/lib/greed.hs\" \
	 -l$(LIB)/bsd.lib -l$(LIB)/curses.lib -l$(LIB)/termcap.lib

mz: mz.o mzio.o
	$(LINK) $(LINKFLAGS) -o$@ -n$@ -s2000 -h4000 $(CSTART) $^

ski: ski.o do_level.o
	$(LINK) $(LINKFLAGS) -o$@ -n$@ -s2000 -h4000 $(CSTART) $^

rot: rot.c 
	$(CC) -o$@ $< -I.. -wA -D_BSD -DBSD \
	 -l$(LIB)/bsd.lib -l$(LIB)/curses.lib -l$(LIB)/termcap.lib

#-----------------------------------------------------------------------------
# Due to the use of ':=', rather than '=', these rules have to appear here.

default: $(TARGETS) Xdefault
	$(foreach x, $(SUBDIRS), make -C ../$(x)/$(HPROC))

install	: $(TARGETS) Xinstall
	$(foreach x, $(SUBDIRS), make -C ../$(x)/$(HPROC) install)
	$(TOUCH) install

installdef : Xinstalldef
	$(foreach x, $(SUBDIRS), make -C ../$(x)/$(HPROC) installdef)
	$(TOUCH) installdef

clean	: Xclean
	$(foreach x, $(SUBDIRS), make -C ../$(x)/$(HPROC) clean)
	-$(RM)	$(OBJECTS) $(TARGETS) $(XOBJECTS) $(XTARGETS) mkout \
		core ../mkout install installdef Xinstall

#-----------------------------------------------------------------------------
endif #InProcMake	# This line must be at EOF
