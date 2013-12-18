#-----------------------------------------------------------------------------
# Helios generic make system - GENERIC COMPONENT MAKEFILE -
#-----------------------------------------------------------------------------
# Helios generic component make rules for: TTY server
#
# File: /HSRC/servers/ttyserv/ttyserv/makefile
#
# These are the host/processor independent rules and variables for this
# component of Helios. It is included by the processor specific makefile
# That may add to some of its functionality. You should only alter this
# makefile by using "ifneq ($(HPROC),YYY)" clauses for a specific processor.
#
# RcsId: $Id: makefile,v 1.12 1994/06/10 11:13:25 nickc Exp $
#		(C) Copyright 1991 Perihelion Software
#
# WARNING: assumes you are using GNUmake.
#
#
#-----------------------------------------------------------------------------

ifndef InProcMake
# This part of the makefile is run in the components generic source directory

# The variables HPROC, HHOST, HSRC, and HPROD should already be set by 
# your make script or environment, if not then get local defaults from
# makeinc/DFLT.mak. Note that the following pathname cannot use $(HSRC).
ifndef HPROC
include ../../../makeinc/DFLT.mak
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
COMPONENT = servers/ttyserv/ttyserv

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
HVPATH := $(RSRC)/$(COMPONENT)/$(HPROC):$(HVPATH):$(RSRC)/$(COMPONENT)
endif

vpath %.h $(HVPATH)
vpath %.c $(HVPATH)
vpath %.a $(HVPATH)


#-----------------------------------------------------------------------------
# Generic component make rules and variables added below here

# Note all final generic targets here
TARGETS = ttyserv setterm

default : $(TARGETS) Xdefault

install	: $(TARGETS) Xinstall
	$(CP) ttyserv $(LIB)
	$(CP) setterm $(BIN)
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

TTY.OBJECTS	= $(TTY.SOURCES.c:.c=.o)

OBJECTS		=  $(TTY.OBJECTS) setterm.o

# C source files
TTY.SOURCES.c	= globals.c termcap.c input.c terminal.c ansi.c tty.c \
			ttydev.c

SOURCES.c	= $(TTY.SOURCES.c) setterm.c

# C header files
HEADERS.c	= tty.h ttydev.h

# Any other source files
SOURCES.odd	= makefile 

#-----------------------------------------------------------------------------
# Other generic component variables:
# Set any flags required

ifndef SYSDEB
  ifeq ($(HPROC),TRAN)
    NCFLAGS := $(NCFLAGS) -ps1
  endif
endif

ifeq ($(HPROC),FRED)
  NCFLAGS := $(NCFLAGS) -DDEBUG
  TTY.OBJECTS := $(TTY.OBJECTS) debug.o
endif

# these are processor independent flags ??!!
# -wadfsv -ecpz -t4

#-----------------------------------------------------------------------------
# Generic targets and their dependencies:

$(TTY.OBJECTS): $(HEADERS.c)

ttyserv	: $(TTY.OBJECTS)
	@ echo Linking $@ ...
	@ $(LINK) $(LINKFLAGS) -o$@ -n$@ -h4000 $(LIB)/cstart.o $^ $(LIB)/servlib.def -l$(LIB)/bsd.lib

setterm : setterm.o
	@ echo compiling $^ ...
	@ $(LINK) $(LINKFLAGS) -o$@ -n$@ -h4000 $(LIB)/cstart.o $^

#-----------------------------------------------------------------------------
endif #InProcMake	# This line must be at EOF
