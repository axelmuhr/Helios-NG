#-----------------------------------------------------------------------------
# Helios generic make system - GENERIC COMPONENT MAKEFILE -
#-----------------------------------------------------------------------------
# Helios generic component make rules for: curses
#
# File: /HSRC/scanlibs/curses/makefile
#
# These are the host/processor independent rules and variables for this
# component of Helios. It is included by the processor specific makefile
# That may add to some of its functionality. You should only alter this
# makefile by using "ifneq ($(HPROC),YYY)" clauses for a specific processor.
#
# SccsId: %W% %G%
# RcsId: $Id: makefile,v 1.6 1994/06/07 12:36:58 nickc Exp $
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
COMPONENT = scanlibs/curses

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
TARGETS = curses.lib

default : $(TARGETS) Xdefault

install	: $(TARGETS) Xinstall
	$(CP) $(TARGETS) $(LIB)
	$(TOUCH) install

clean	: Xclean
	-$(RM)	$(OBJECTS) $(TARGETS) $(XOBJECTS) $(XTARGETS) mkout \
		core ../mkout install Xinstall


#-----------------------------------------------------------------------------
# for archive, backup and cleaning purposes,
# note all generic SOURCE and OBJECT files:

# Note all sources
SOURCES		= $(SOURCES.c) $(SOURCES.a) \
		  $(HEADERS.c) $(HEADERS.m) \
		  $(SOURCES.odd)

OBJECTS	= $(SOURCES.c:.c=.o) $(SOURCES.a:.a=.o)

# C source files
SOURCES.c	=  addbytes.c addch.c addstr.c box.c clear.c clrtobot.c \
		   clrtoeol.c putchar.c cr_put.c cr_tty.c curses.c delch.c deleteln.c \
		   delwin.c endwin.c erase.c fullname.c getch.c getstr.c \
		   idlok.c id_subwins.c tstp.c initscr.c insch.c insertln.c \
		   longname.c move.c mvprintw.c mvscanw.c mvwin.c newwin.c \
		   overlay.c overwrite.c printw.c refresh.c scanw.c \
		   scroll.c toucholap.c standout.c touchwin.c unctrl.c

# AMPP source files
SOURCES.a	= 

# C header files
HEADERS.c	= curses.ext

# AMPP macro files
HEADERS.m	=

# Any other source files
SOURCES.odd	= makefile makefile.old


#-----------------------------------------------------------------------------
# Other generic component variables:

# Set any flags required
NCFLAGS := $(NCFLAGS) -DSTACK_DIRECTION=-1 -DPOSIX -D_BSD 


#-----------------------------------------------------------------------------
# Generic targets and their dependencies:

LOBJECTS = $(patsubst %,-l%,$(OBJECTS))
GOBJECTS = $(OBJECTS:.o=.o.ghof)

# uncomment if you have compiler or makedepend generated dependencies
#include depends

curses.lib: $(OBJECTS)
ifeq ($(HPROC),TRAN)
	$(LINK) $(LINKFLAGS) -p -o curses.lib $(OBJECTS)
else
	@ echo Creating scanned library $@ ...
  ifeq ($(COMPILER_TYPE),ARMLtd)
	@ $(LINK) $(LIB)/posix.def $(LOBJECTS) -o fred
	@ cat $(GOBJECTS) > $@
  else
	@ cat $^ > $@
  endif
endif
ifeq ($(HHOST),HELIOSTRAN)
	$(RM) /helios/lib/curses.lib
	$(CP) curses.lib /helios/lib/curses.lib
endif

#-----------------------------------------------------------------------------
endif #InProcMake	# This line must be at EOF

