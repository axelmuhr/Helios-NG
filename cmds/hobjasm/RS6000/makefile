#------------------------------------------------------------------------------
# Helios generic make system - NATIVE HOST MAKEFILE -
#-----------------------------------------------------------------------------
# RS6000 Host specific make rules to make NATIVE HOST version of HOBJASM.
#
# File: /HSRC/cmds/hobjasm/RS6000/makefile
#
# This file contains ALL the definitions and rules that are
# required to make a NATIVE HOST version of this component.
# IT IS NOT COMPLIMENTARY TO THE GENERIC VERSION.
#
# This makefile is always executed within this host specific subdir.
# All host specific source files should be kept here.
# All intermediate, object and binary files should be made in here.
#
# SccsId: %W% %G%
# RcsId: $Id: makefile,v 1.1 1993/07/26 15:12:40 nickc Exp nickc $
#               (C) Copyright 1990 Perihelion Software
#
# WARNING: assumes you are using GNUmake.
#-----------------------------------------------------------------------------
# Manditory variables for a component:
# name of COMPONENT directory relative to $(HSRC)
COMPONENT = cmds/hobjasm

# Manditory includes
include $(HSRC)/makeinc/$(HHOST).mak    # Default host system variables

# Manditory phoney targets
.PHONY: default clean
# "default" - should make this component in its proc subdir
# "install" - copy the new component to $(HHOSTBIN) directory
# "clean"   - remove all intermediate and executable files from host subdir

VPATH := $(HSRC)/$(COMPONENT)

#-----------------------------------------------------------------------------
# Note all final targets here

TARGETS = hobjasm

default : $(TARGETS)

install : $(TARGETS)
	$(CP) $(TARGETS) $(HHOSTBIN)
	$(TOUCH) install

clean   :
	-$(RM)  $(OBJECTS) $(TARGETS) mkout core ../mkout install

# Note all sources
SOURCES         = $(SOURCES.c) $(HEADERS.c) $(SOURCES.odd)

# Object files
OBJECTS         = $(SOURCES.c:.c=.o)

# C source files
# C source files
SOURCES.c =     aasm.c apply.c applysub.c asm.c code.c conds.c \
		errors.c expr.c exstore.c extypes.c formatio.c \
		fpio.c getdir.c getline.c globvars.c initdir.c listing.c \
		literals.c llabel.c mactypes.c nametype.c occur.c \
		opcode.c osdepend.c p1dirs.c p1hand.c p2dirs.c \
		p2hand.c stats.c store.c stubs.c symbol.c \
		tables.c tokens.c vars.c iface.c

# C header files
HEADERS.c       = asm.h code.h conds.h constant.h errors.h \
		exstore.h extypes.h formatio.h fpio.h getdir.h getline.h  \
		globvars.h helios.h initdir.h iface.h listing.h \
		llabel.h mactypes.h nametype.h asmvars.h opcode.h  \
		osdepend.h p1dirs.h p1hand.h p2dirs.h \
		stats.h store.h stubs.h symbol.h tables.h tokens.h  \
		vars.h version.h apply.h applysub.h occur.h \
		expr.h literals.h p2hand.h

# Any other source files
SOURCES.odd     = makefile DOC README \
		examples/libend.s examples/libhdr.s examples/library.s \
		examples/libtest.c examples/makefile examples/minimum.s \
		examples/sample.c examples/standard.s 

#-----------------------------------------------------------------------------

HOSTCFLAGS := $(HOSTCFLAGS) -I..

#-----------------------------------------------------------------------------
# Native Host targets and their dependencies:

# Always recompile "aasm.o" to get the correct version information
# (NOTE: It currently uses a local path reference rather than a global
#        one like the rest of the compilations)
hobjasm:        $(OBJECTS)
		$(HOSTCC) $(HOSTCFLAGS) -c ../aasm.c
		$(HOSTCC) $(HOSTCFLAGS) -o $@ $(OBJECTS)

#-----------------------------------------------------------------------------
