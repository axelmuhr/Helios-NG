#-----------------------------------------------------------------------------
# Helios generic make system - GENERIC COMPONENT MAKEFILE -
#-----------------------------------------------------------------------------
# Helios generic component make rules for: MicroEmacs Editor
#
# File: /HSRC/cmds/emacs.311/makefile
#
# These are the host/processor independent rules and variables for this
# component of Helios. It is included by the processor specific makefile
# That may add to some of its functionality. You should only alter this
# makefile by using "ifneq ($(HPROC),YYY)" clauses for a specific processor.
#
# SccsId: %W% %G%
# RcsId: $Id: makefile,v 1.8 1994/06/10 11:46:36 nickc Exp $
#		(C) Copyright 1990 Perihelion Software
#
# WARNING: assumes you are using GNUmake.
#
# $Header: /pds/nickc/RTNucleus/cmds/emacs.311/RCS/makefile,v 1.8 1994/06/10 11:46:36 nickc Exp $
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
COMPONENT = cmds/emacs.311

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
TARGETS = emacs

default : $(TARGETS) Xdefault

install	: $(TARGETS) ../emacs.hlp Xinstall
	$(CP) emacs $(BIN)
	cat ../emacs.hlp > $(ETC)/emacs.hlp
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
SOURCES.c	= word.c main.c basic.c random.c bind.c eval.c \
		char.c exec.c folding.c \
		file.c fileio.c heliosio.c window.c \
		input.c search.c isearch.c display.c mouse.c screen.c \
		region.c buffer.c line.c

# C header files
HEADERS.c	= estruct.h edef.h efunc.h epath.h ebind.h evar.h \
		elang.h english.h eproto.h efolding.h

EXTRAS.c	= amigados.c ansi.c aosvs.c cmdfix.c crypt.c dg10.c dolock.c \
		fmr.c fmrdos.c history.c hp110.c hp150.c ibmpc.c \
		lock.c me.c mpe.c msdos.c nec.c necdos.c os2.c \
		os2npm.c smg.c st52.c tags.c tcap.c tipc.c tos.c unix.c \
		vms.c vt52.c wmcs.c z309.c

EXTRAS.h	= dutch.h french.h german.h japan.h  latin.h platin.h spanish.h

# Any other source files
SOURCES.odd	= makefile emacs.hlp # $(EXTRAS.c) $(EXTRAS.h)



#-----------------------------------------------------------------------------
# Other generic component variables:
# Set any flags required

ifeq ($(HPROC),TRAN)
  NCFLAGS := $(NCFLAGS) -wadfsv
  LINKFLAGS := $(LINKFLAGS) -f
else
  NCFLAGS := $(NCFLAGS) -wadfpv
endif

LIBS := -l$(LIB)/helios.lib -l$(LIB)/c.lib -l$(LIB)/bsd.lib

#-----------------------------------------------------------------------------
# Generic targets and their dependencies:

emacs	: $(OBJECTS)
	@ echo Linking $@ ...
	@ $(LINK) $(LINKFLAGS) -o$@ -n$@ -s16000 -h4000 $(LIB)/c0.o $^ $(LIBS)


#-----------------------------------------------------------------------------
endif #InProcMake	# This line must be at EOF
