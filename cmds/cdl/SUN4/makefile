#------------------------------------------------------------------------------
# Helios generic make system - NATIVE HOST MAKEFILE -
#-----------------------------------------------------------------------------
# SUN Host specific make rules to make NATIVE HOST version of Generic Linker
#
# File: /HSRC/cmds/linker/SUN/makefile
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
# RcsId: $Id: makefile,v 1.3 1994/06/07 12:33:07 nickc Exp $
#		(C) Copyright 1990 Perihelion Software
#
# WARNING: assumes you are using GNUmake.
#-----------------------------------------------------------------------------
# Manditory variables for a component:
# name of COMPONENT directory relative to $(HSRC)
COMPONENT = network

# Manditory includes
include $(HSRC)/makeinc/$(HHOST).mak	# Default host system variables

HOSTCC      = gcc 
HOSTCFLAGS  = -DHOSTISBIGENDIAN -D__SMT -DSUN4 -D__SUN4 -D__HELIOSSUN4 -D__C40 -ansi \
	      -I/giga/lib/gcc-include -I/opt/gnu/include -I/hsrc/include -DCOMPATIBLE
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

# Note all target CPUs that are built on this host (prefixed with 'build_')
POSSIBLECPUTARGETS = build_C40

ifeq ($(HPROC),C40)
TARGETS = build_C40 cdl
else
 ifeq ($(HPROC),ARM)
 TARGETS = build_ARM cdl
 endif
endif

default : $(TARGETS)

install	: $(TARGETS)
	$(CP) $(TARGETS) $(HHOSTBIN)
	$(TOUCH) install

clean	:
	-$(RM)	$(OBJECTS) $(TARGETS) mkout core ../mkout install \
	$(POSSIBLECPUTARGETS)

# If we build for a new processor target clean out the old target proc. objects
build_$(HPROC):
	$(MAKE) clean
	touch	build_$(HPROC)

# Note all sources
SOURCES		= $(SOURCES.c) $(HEADERS.c) $(SOURCES.odd)

# Object files
OBJECTS		= alloc.o cdl.o code.o data.o exec.o eval.o \
		  expand.o parse.o valid.o queue.o stubs.o

# C source files
SOURCES.c	= alloc.c cdl.c code.c data.c exec.c eval.c \
		  expand.c parse.c valid.c queue.c stubs.c

# C header files
HEADERS.c	= cdl.h cdlobj.h cdlobject.h list.h table.h 

# Any other source files
SOURCES.odd	= makefile


#-----------------------------------------------------------------------------
# Native Host targets and their dependencies:

ifeq ($(HPROC),C40)
HOSTCFLAGS	:= $(HOSTCFLAGS) -D__C40 -D__HELIOSC40 -I/opt/gnu/include -I$(INCLUDE) -DCOMPATIBLE
else
  ifeq ($(HPROC),ARM)
  HOSTCFLAGS	:= $(HOSTCFLAGS) -D__ARM -D__HELIOSARM -I/opt/gnu/include -I$(INCLUDE) -DCOMPATIBLE
  endif
endif

alloc.o: ../alloc.c ../cdl.h
	$(HOSTCC) $(HOSTCFLAGS) -c $<

cdl.o: ../cdl.c ../cdl.h
	$(HOSTCC) $(HOSTCFLAGS) -c $<

code.o: ../code.c ../cdl.h
	$(HOSTCC) $(HOSTCFLAGS) -c $<

data.o: ../data.c ../cdl.h
	$(HOSTCC) $(HOSTCFLAGS) -c $<

exec.o: ../exec.c ../cdl.h
	$(HOSTCC) $(HOSTCFLAGS) -c $<

eval.o: ../eval.c ../cdl.h
	$(HOSTCC) $(HOSTCFLAGS) -c $<

expand.o: ../expand.c ../cdl.h
	$(HOSTCC) $(HOSTCFLAGS) -c $<

parse.o: ../parse.c ../cdl.h
	$(HOSTCC) $(HOSTCFLAGS) -c $<

valid.o: ../valid.c ../cdl.h
	$(HOSTCC) $(HOSTCFLAGS) -c $<

queue.o: queue.c
	$(HOSTCC) $(HOSTCFLAGS) -c $<

stubs.o: stubs.c
	$(HOSTCC) $(HOSTCFLAGS) -c $<

cdl : $(OBJECTS)
	$(HOSTLD) $(HOSTLDFLAGS) -o $@ $^

#-----------------------------------------------------------------------------
