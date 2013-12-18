# @@@ Need to be updated to cope with target processors other than ARM
#------------------------------------------------------------------------------
# Helios generic make system - NATIVE HOST MAKEFILE -
#-----------------------------------------------------------------------------
# R140 Host specific make rules to make NATIVE HOST version of Generic Linker
#
# File: /HSRC/cmds/linker/R140/makefile
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
# RcsId: $Id: makefile,v 1.8 1993/01/15 15:50:39 paul Exp $
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

ifeq ($(HPROC),ARM)
TARGETS = build_ARM ldarm objdumparm imdump disasarm
else
  ifeq ($(HPROC),C40)
     TARGETS = build_C40 ldc40 objdumpc40 imdump disasc40
  endif
endif

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
L.OBJECTS	= sym.o objed.o module.o mem.o scancode.o link.o readfile.o \
		vm.o genimage.o queue.o

OBJECTS		= $(L.OBJECTS) armdisas.o ARMshape.o imdump.o objdump.o

# C source files
SOURCES.c	= sym.c objed.c module.c mem.c scancode.c link.c readfile.c \
		vm.c genimage.c queue.c hash.c ARMshape.c armdisas.c

# C header files
HEADERS.c	= vm.h link.h ARMshape.h

# Any other source files
SOURCES.odd	= makefile


#-----------------------------------------------------------------------------
# Native Host targets and their dependencies:

NEWINCLUDE = .,/usr/include/ansi,/usr/include,$(INCLUDE)
#NEWINCLUDE = .,$(INCLUDE)
ifeq ($(HPROC),ARM)
  # platform = arm bsd4.3 -> target = ARM
  HOSTCFLAGS  := $(HOSTCFLAGS) -j$(NEWINCLUDE) -ansi -DNOPOSIX -DARM -DARMBSD -D__HELIOSARM
else
  ifeq ($(HPROC),C40)
      HOSTCFLAGS  := $(HOSTCFLAGS) -j$(NEWINCLUDE) -ansi -DNOPOSIX -DARM -DARMBSD -D__HELIOSC40
  endif
endif

$(L.OBJECTS) : link.h 
vm.o : vm.h
link.o : hash.c

ARMshape.o: ARMshape.c
	$(HOSTCC) $(HOSTCFLAGS) -c $<

ldarm : $(L.OBJECTS)
	$(HOSTCC) $(HOSTCFLAGS) -o$@ $^

ldc40 : $(L.OBJECTS)
	$(HOSTCC) $(HOSTCFLAGS) -o$@ $^

# ARM version (includes disassembler)
objdumparm : objdump.c ARMshape.o
	$(HOSTCC) $(HOSTCFLAGS) -c $<
	$(HOSTCC) $(HOSTCFLAGS) -o$@ ARMshape.o objdump.o 

# C40 version (includes disassembler)
objdumpc40 : objdump.c c40objdis.c
	$(HOSTCC) $(HOSTCFLAGS) -o$@ c40objdis.c objdump.c

imdump : imdump.c
	$(HOSTCC) $(HOSTCFLAGS) -o$@ $<

disasarm : armdisas.c ARMshape.o
	$(HOSTCC) $(HOSTCFLAGS) -c $<
	$(HOSTCC) $(HOSTCFLAGS) -o$@ armdisas.o ARMshape.o

disasc40 : c40disas.c
	$(HOSTCC) $(HOSTCFLAGS) -o$@ c40disas.c


#-----------------------------------------------------------------------------
