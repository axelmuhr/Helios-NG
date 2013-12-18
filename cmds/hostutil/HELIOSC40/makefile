#------------------------------------------------------------------------------
# Helios generic make system - NATIVE HOST MAKEFILE -
#-----------------------------------------------------------------------------
# SUN4 Host specific make rules to make NATIVE HOST version of Host Utilities.
#
# File: /HSRC/cmds/hostutil/SUN4/makefile
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
# RcsId: $Id: makefile,v 1.2 1993/05/26 15:07:49 bart Exp $
#		(C) Copyright 1990 Perihelion Software
#
# WARNING: assumes you are using GNUmake.
#-----------------------------------------------------------------------------
# Manditory variables for a component:
# name of COMPONENT directory relative to $(HSRC)
COMPONENT = cmds/hostutil

# Manditory includes
include $(HSRC)/makeinc/$(HHOST).mak	# Default host system variables

# Manditory phoney targets
.PHONY: default clean
# "default" - should make this component in its proc subdir
# "install" - copy the new component to $(HHOSTBIN) directory
# "clean"   - remove all intermediate and executable files from host subdir

VPATH := $(HSRC)/$(COMPONENT):$(CMDS)/com

#-----------------------------------------------------------------------------
# Note all final targets here

TARGETS := sysbuild objed newmotd gencodes squash

default : $(TARGETS)

install	: $(TARGETS)
	$(CP) $(TARGETS) $(HHOSTBIN)
	$(TOUCH) install

clean	:
	-$(RM)	$(OBJECTS) $(TARGETS) mkout core ../mkout install

# Note all sources
SOURCES		= $(SOURCES.c) $(HEADERS.c) $(SOURCES.odd)

# Object files
OBJECTS		= $(SOURCES.c:.c=.o) sysbuild.o objed.o

# C source files
SOURCES.c	:= gencodes.c squash.c testenc.c newmotd.c

# C header files
HEADERS.c	:=

# Any other source files
SOURCES.odd	:= makefile


#-----------------------------------------------------------------------------
# Native Host targets and their dependencies:

#----------------------------------------------------------------------------
# use the Helios sources (sysbuild/sysstrip/objed) directly from native srcs
# directory

# system image (nucleus) construction tool
sysbuild : sysbuild.c
	$(HOSTCC) -c -D$(HHOST) $<
	$(HOSTCC) -o $@ $@.o

# Helios object editor (display/change a program's stack/heap/name)
objed : objed.c
	$(HOSTCC) -c -D$(HHOST) $<
	$(HOSTCC) -o $@ $@.o

#---------------------------------------------------------------------------
# Construct a standard "motd" file

newmotd: newmotd.c
	$(HOSTCC) -c -D$(HHOST) $<
	$(HOSTCC) -o $@ $@.o


#---------------------------------------------------------------------------
# Test encoding functions for helios object files.

testenc: testenc.c
	$(HOSTCC) -c -D$(HHOST) $<
	$(HOSTCC) -o $@ $@.o


#-----------------------------------------------------------------------------
# Fault library host utilities:

gencodes : gencodes.c
	$(HOSTCC) -c -D$(HHOST) $<
	$(HOSTCC) -o $@ $@.o

squash : squash.c
	$(HOSTCC) -c -D$(HHOST) $<
	$(HOSTCC) -o $@ $@.o


#-----------------------------------------------------------------------------
