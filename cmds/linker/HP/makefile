#------------------------------------------------------------------------------
# Helios generic make system - NATIVE HOST MAKEFILE -
#-----------------------------------------------------------------------------
# HPUX Host specific make rules to make NATIVE HOST version of Generic Linker
#
# File: /HSRC/cmds/linker/HP/makefile
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
# RcsId: $Id: makefile,v 1.7 1993/01/15 15:38:59 paul Exp $
#		(C) Copyright 1990 Perihelion Software
#
# WARNING: assumes you are using GNUmake.
#-----------------------------------------------------------------------------
# Manditory variables for a component:
# name of COMPONENT directory relative to $(HSRC)
COMPONENT = cmds/linker

# Manditory includes
include $(HSRC)/makeinc/$(HHOST).mak	# Default host system variables

# Manditory phoney targets
.PHONY: default clean
# "default" - should make this component in its proc subdir
# "install" - copy the new component to $(HHOSTBIN) directory
# "clean"   - remove all intermediate and executable files from host subdir

VPATH := $(HSRC)/$(COMPONENT)

#-----------------------------------------------------------------------------
# Note all final targets here

# Note all target CPUs that are built on this host (prefixed with 'build_')
POSSIBLECPUTARGETS = build_ARM build_C40

ifeq ($(HPROC),C40)
TARGETS = build_C40 ldc40 objdumpc40 imdump disasc40
else
 ifeq ($(HPROC),ARM)
 TARGETS = build_ARM ldarm objdumparm imdump disasarm
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
L.OBJECTS	= sym.o objed.o module.o mem.o scancode.o link.o readfile.o \
		vm.o genimage.o queue.o stubs.o

OBJECTS		= $(L.OBJECTS) c40disas.o c40objdis.o imdump.o objdump.o stubs.o

# C source files
SOURCES.c	= sym.c objed.c module.c mem.c scancode.c link.c readfile.c \
		vm.c genimage.c queue.c hash.c stubs.c

# C header files
HEADERS.c	= vm.h link.h c40ops.h c40target.h

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

$(L.OBJECTS) : link.h 
vm.o : vm.h
link.o : hash.c

ldc40 : $(L.OBJECTS)
	$(HOSTCC) $(HOSTCFLAGS) -o$@ $^

ldarm: $(L.OBJECTS)
	$(HOSTCC) $(HOSTCFLAGS) -o$@ $^

# C40 version (includes disassembler)
objdumpc40 : objdump.c c40objdis.c
	$(HOSTCC) $(HOSTCFLAGS) -o$@ $^

# ARM version (includes disassembler)
objdumparm : objdump.c ARMshape.o
	$(HOSTCC) $(HOSTCFLAGS) -c $<
	$(HOSTCC) $(HOSTCFLAGS) -o$@ ARMshape.o objdump.o 

imdump : imdump.c
	$(HOSTCC) $(HOSTCFLAGS) -o$@ $<

disasc40 : c40disas.c
	$(HOSTCC) $(HOSTCFLAGS) -o$@ $<

disasarm : armdisas.c ARMshape.o
	$(HOSTCC) $(HOSTCFLAGS) -c $<
	$(HOSTCC) $(HOSTCFLAGS) -o$@ armdisas.o ARMshape.o

#-----------------------------------------------------------------------------
