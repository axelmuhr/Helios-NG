#------------------------------------------------------------------------------
# Helios generic make system - NATIVE HOST MAKEFILE -
#-----------------------------------------------------------------------------
# SUN Host specific make rules to make NATIVE HOST version of Generic Commands
#
# File: /HSRC/cmds/com/SUN4/makefile
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
# RcsId: $Id: makefile,v 1.3 1994/06/07 12:31:51 nickc Exp $
#		(C) Copyright 1990 Perihelion Software
#
# WARNING: assumes you are using GNUmake.
#-----------------------------------------------------------------------------
# Manditory variables for a component:
# name of COMPONENT directory relative to $(HSRC)
COMPONENT = cmds/com

# Manditory includes
include $(HSRC)/makeinc/$(HHOST).mak	# Default host system variables

HOSTCC      = gcc 
HOSTCFLAGS  = -DHOSTISBIGENDIAN -D__SMT -DSUN4 -D__SUN4 -D__HELIOSSUN4 -ansi \
	      -I$(GCCINCLUDE) -D__$(HPROC) -D__HELIOS$(HPROC) -I/usr/include -I/hsrc/include
HOSTLD      = gcc
HOSTLDFLAGS = -ansi

# Manditory phoney targets
.PHONY: default clean
# "default" - should make this component in its proc subdir
# "install" - copy the new component to $(HHOSTBIN) directory
# "clean"   - remove all intermediate and executable files from host subdir

VPATH := $(HSRC)/$(COMPONENT)

#-----------------------------------------------------------------------------
# Note all final targets here

TARGETS =  gdi sysbreak sysbuild

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
OBJECTS		= gdi.o sysbreak.o

# C source files
SOURCES.c	= gdi.c sysbreak.c

# C header files
HEADERS.c	= 

# Any other source files
SOURCES.odd	= makefile


#-----------------------------------------------------------------------------
# Native Host targets and their dependencies:

ifeq ($(HPROC),C40)
HOSTCFLAGS	:= $(HOSTCFLAGS) -D__C40 -D__HELIOSC40 -I/usr/include -I$(INCLUDE)
else
  ifeq ($(HPROC),ARM)
  HOSTCFLAGS	:= $(HOSTCFLAGS) -D__ARM -D__HELIOSARM -I/usr/include -I$(INCLUDE)
  endif
endif

gdi : gdi.c
	$(HOSTCC) $(HOSTCFLAGS) -o $@ $<

sysbreak : sysbreak.c
	$(HOSTCC) $(HOSTCFLAGS) -o $@ $<

#-----------------------------------------------------------------------------
