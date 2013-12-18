#-----------------------------------------------------------------------------
# Helios generic make system - GENERIC COMPONENT MAKEFILE -
#-----------------------------------------------------------------------------
# Helios generic component make rules for: Emacs Editor
#
# File: /HSRC/cmds/emacs/makefile
#
# These are the host/processor independent rules and variables for this
# component of Helios. It is included by the processor specific makefile
# That may add to some of its functionality. You should only alter this
# makefile by using "ifneq ($(HPROC),YYY)" clauses for a specific processor.
#
# SccsId: %W% %G%
# RcsId: $Id: makefile,v 1.15 1994/05/12 11:39:17 nickc Exp $
#		(C) Copyright 1990 Perihelion Software
#
# WARNING: assumes you are using GNUmake.
#
# $Header: /users/nickc/RTNucleus/cmds/emacs/RCS/makefile,v 1.15 1994/05/12 11:39:17 nickc Exp $
#
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
COMPONENT = cmds/emacs

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
TARGETS = emacs

default : $(TARGETS) Xdefault

install	: $(TARGETS) Xinstall
	$(CP) emacs $(BIN)/emacs.old
	cat ../emacshlp.dos > $(ETC)/emacs.hlp.dos
	cat ../emacshlp.dft > $(ETC)/emacs.hlp.dft
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

OBJECTS	= $(SOURCES.c:.c=.o) $(SOURCES.a:.a=.o)

# C source files
SOURCES.c	= main.c ansi.c basic.c bind.c buffer.c \
		display.c eval.c exec.c file.c fileio.c \
		input.c isearch.c line.c \
		random.c region.c search.c spawn.c \
		termio.c window.c word.c

# C header files
HEADERS.c	= estruct.h edef.h efunc.h epath.h ebind.h evar.h

# Any other source files
SOURCES.odd	= makefile emacs.rc emacshlp.dft emacshlp.dos TAGS me.msx

#-----------------------------------------------------------------------------
# Other generic component variables:
# @@@ should remove -DHELIOS and change source to reference __HELIOS
# Set any flags required

ifeq ($(HPROC),TRAN)
# @@@ why -dT800?
  NCFLAGS := $(NCFLAGS) -DHELIOS -wadfsv -ecpz -t4 -dT800
else
  NCFLAGS := $(NCFLAGS) -DHELIOS -wadfvn -fq
endif

#-----------------------------------------------------------------------------
# Generic targets and their dependencies:

$(OBJECTS): $(HEADERS.c)

emacs	: $(OBJECTS)
	$(LINK) $(LINKFLAGS) -o$@ -n$@ -s16000 -h4000 $(LIB)/cstart.o $^


#-----------------------------------------------------------------------------
endif #InProcMake	# This line must be at EOF
