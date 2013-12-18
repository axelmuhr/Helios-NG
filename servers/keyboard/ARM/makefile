#------------------------------------------------------------------------------
# Helios generic make system - PROCESSOR SPECIFIC COMPONENT MAKEFILE -
#-----------------------------------------------------------------------------
# ARM processor specific make rules for the keyboard server component.
#
# File: /HSRC/servers/keyboard/makefile
#
# This file contains the extra definitions of variables and rules that are
# required to make a processor specific version of this component.
#
# If you wish to overide a targets rule that has been defined in the generic
# part of the makefile, you should bracket it with "ifneq ($(HPROC),YYY)" and
# supply the new rule in this makefile.
#
# This makefile is always executed within this processor specific subdir.
# All processor specific source files should be kept here.
# All intermediate, object and binary files should be made in here.
#
# SccsId: %W% %G%
# RcsId: $Id: makefile,v 1.4 1993/08/06 13:13:45 paul Exp $
#		(C) Copyright 1990 Perihelion Software
#
# WARNING: assumes you are using GNUmake.
#-----------------------------------------------------------------------------

InProcMake = TRUE

# Get generic component makefile
include ../makefile

#-----------------------------------------------------------------------------
# Add processor specific targets:

ifeq ($(HPLATFORM), ABC)
 XTARGETS = inkey44.bkm exkey44.bkm
else
 ifeq ($(HPLATFORM), ARCHIMEDES)
  XTARGETS = keyboard.bkm
 endif
endif

Xdefault: $(XTARGETS)


# Install the extra targets:
Xinstall: $(XTARGETS)
	$(CP) $(XTARGETS) $(LIB)
	$(TOUCH) Xinstall

# Remove any processor specific files other than Xtargets and Xobjects here:
Xclean:
	-$(RM) $(XTARGETS) Xinstall


#-----------------------------------------------------------------------------
# Add to lists of files, used for archive, backup and cleaning purposes.

# All extra processor specific sources
XSOURCES	= $(XSOURCES.c) $(XSOURCES.a) \
		  $(XHEADERS.c) $(XHEADERS.m) \
		  $(XSOURCES.odd)


XOBJECTS	= $(XSOURCES.c:.c=.o) $(XSOURCES.a:.a=.o)


# Fill in processor specific sources here:
XSOURCES.c	=

XSOURCES.a	=

XHEADERS.c	=

XHEADERS.m	=

XSOURCES.odd	= makefile inkey44.akm exkey44.akm cherry.akm archi.akm \
		akbdproto.doc akbdsrc.s


#-----------------------------------------------------------------------------
# Any additional processor specific rules and generic rule overides go here:
# The buildkeymap is currently held in /hsrc/cmds/com/ARM
# A host version being built in /hsrc/cmds/hostutil

# build the internal uk keyboard map
inkey44.bkm: inkey44.akm
	buildkeymap -o$@ $<

# build the external uk keyboard map
exkey44.bkm: exkey44.akm
	buildkeymap -o$@ $<

# build the standard Archimedes keyboard map
keyboard.bkm: archi.akm
	buildkeymap -o$@ $<


#-----------------------------------------------------------------------------
