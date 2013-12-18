#------------------------------------------------------------------------------
# Helios generic make system - NATIVE HOST MAKEFILE -
#-----------------------------------------------------------------------------
# RS6000 Host specific make rules to make NATIVE HOST version of network software
#
# File: /HSRC/network/RS6000/makefile
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
# RcsId: $Id: makefile,v 1.1 1993/08/06 10:19:47 nickc Exp $
#		(C) Copyright 1990 Perihelion Software
#
# WARNING: assumes you are using GNUmake.
#-----------------------------------------------------------------------------
# Manditory variables for a component:
# name of COMPONENT directory relative to $(HSRC)
COMPONENT = network

# Manditory includes
include $(HSRC)/makeinc/$(HHOST).mak	# Default host system variables

HOSTCC      = xlc
HOSTCFLAGS  = -DHOSTISBIGENDIAN -D__SMT -DRS6000 -D__C40 -D_ALL_SOURCE \
	      -I/usr/include -I$(HSRC)/include
HOSTLD      = xlc
HOSTLDFLAGS = 

# Manditory phoney targets
.PHONY: default clean
# "default" - should make this component in its proc subdir
# "install" - copy the new component to $(HHOSTBIN) directory
# "clean"   - remove all intermediate and executable files from host subdir

VPATH := $(HSRC)/$(COMPONENT)

#-----------------------------------------------------------------------------
# Note all final targets here

TARGETS = rmgen

default : $(TARGETS)

install	: $(TARGETS)
	$(CP) $(TARGETS) $(HHOSTBIN)
	$(TOUCH) install

clean	:
	-$(RM)	$(OBJECTS) $(TARGETS) mkout core ../mkout install \
	$(POSSIBLECPUTARGETS)

# Note all sources
SOURCES		= $(SOURCES.c) $(HEADERS.c) $(SOURCES.odd)

# Object files
OBJECTS		= rmgen.o rmlib1.o rmlib2.o rmlhost.o

# C source files
SOURCES.c	= rmgen.c rmlib1.c rmlib2.c rmlhost.c

# C header files
HEADERS.c	= 

# Any other source files
SOURCES.odd	= makefile


#-----------------------------------------------------------------------------
# Native Host targets and their dependencies:

rmgen : rmgen.o rmlib1.o rmlib2.o rmlhost.o
	$(HOSTLD) $(HOSTLDFLAGS) -o $@ $^

#-----------------------------------------------------------------------------
