#------------------------------------------------------------------------------
# Helios generic make system - NATIVE HOST MAKEFILE -
#-----------------------------------------------------------------------------
# RS6000 Host specific make rules to make NATIVE HOST version of Generic Linker
#
# File: /HSRC/cmds/linker/RS6000/makefile
#
# This file contains ALL the definitions and rules that are
# required to make a NATIVE HOST version of this component.
# IT IS NOT COMPLIMENTARY TO THE GENERIC VERSION.
#
# This makefile is always executed within this host specific subdir.
# All host specific source files should be kept here.
# All intermediate, object and binary files should be made in here.
#
# RcsId: $Id: makefile,v 1.13 1994/04/07 10:56:37 nickc Exp $
#		(C) Copyright 1990 - 1994 Perihelion Software
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
POSSIBLECPUTARGETS = build_ARM build_C40 build_M68K

ifeq ($(HPROC),C40)
  TARGETS = ldc40 objdumpc40 imdump disasc40
  XTARGETS = build_C40 $(TARGETS) 
else
  ifeq ($(HPROC),ARM)
    TARGETS = ldarm objdumparm imdump disasarm
    XTARGETS = build_ARM $(TARGETS) 
  else
    ifeq ($(HPROC),M68K)
      TARGETS = ldm68k objdumpm68k imdump disasm68k
      XTARGETS = build_M68K $(TARGETS) 
    endif
  endif
endif

default : $(XTARGETS)

install	: $(XTARGETS)
	$(CP) $(TARGETS) $(HHOSTBIN)
	$(TOUCH) install

clean	:
	-$(RM)	$(L.OBJECTS) $(TARGETS) mkout core ../mkout install \
	$(POSSIBLECPUTARGETS)

# If we build for a new processor target clean out the old target proc. objects
build_$(HPROC):
	$(MAKE) clean
	touch	build_$(HPROC)

# Note all sources
SOURCES		= $(SOURCES.c) $(HEADERS.c) $(SOURCES.odd)

# Object files
L.OBJECTS	= sym.o objed.o module.o mem.o scancode.o link.o readfile.o \
		vm.o genimage.o queue.o stubs.o objdump.o imdump.o

ifeq ($(HPROC),M68K)
  OBJECTS		= $(L.OBJECTS) 68kdisas.o 
else
  ifeq ($(HPROC),C40)
    OBJECTS		= $(L.OBJECTS) c40disas.o c40objdis.o
  endif
endif

# C source files
SOURCES.c	= sym.c objed.c module.c mem.c scancode.c link.c readfile.c \
		vm.c genimage.c queue.c hash.c stubs.c

# C header files
ifeq ($(HPROC),M68K)
  HEADERS.c	= vm.h link.h 
else
  HEADERS.c	= vm.h link.h c40ops.h c40target.h
endif

# Any other source files
SOURCES.odd	= makefile


#-----------------------------------------------------------------------------
# Native Host targets and their dependencies:

ifeq ($(HPROC),C40)
  HOSTCFLAGS	:= $(HOSTCFLAGS) -I/usr/include -I$(INCLUDE)
else
  ifeq ($(HPROC),ARM)
    HOSTCFLAGS	:= $(HOSTCFLAGS) -g -I/usr/include -I$(INCLUDE) -I$(EXTINCLUDE)
  else
    ifeq ($(HPROC),M68K)
      HOSTCFLAGS := $(HOSTCFLAGS) -I/usr/include -I$(INCLUDE)
    endif
  endif
endif


$(L.OBJECTS) : link.h 
vm.o : vm.h
link.o : hash.c
convert.o: convert.h endian.h

ldc40: $(L.OBJECTS)
	$(HOSTCC) $(HOSTCFLAGS) -o$@ $^
ldarm: $(L.OBJECTS) convert.o
	$(HOSTCC) $(HOSTCFLAGS) -o$@ $^ $(EXTLIB)/clxlib.a
ldm68k: $(L.OBJECTS)
	$(HOSTCC) $(HOSTCFLAGS) -o$@ $^

# C40 version (includes disassembler)
objdumpc40 : objdump.c c40objdis.c
	$(HOSTCC) $(HOSTCFLAGS) -o$@ $^

# ARM version (includes disassembler)
objdumparm : objdump.c
	$(HOSTCC) $(HOSTCFLAGS) -c $<
	$(HOSTCC) $(HOSTCFLAGS) -o$@ objdump.o $(EXTLIB)/clxlib.a

# M68K version
objdumpm68k : objdump.c 
	$(HOSTCC) $(HOSTCFLAGS) -o$@ $^

imdump : imdump.c
	$(HOSTCC) $(HOSTCFLAGS) -o$@ $<

disasc40 : c40disas.c
	$(HOSTCC) $(HOSTCFLAGS) -o$@ $<

disasarm : armdisas.c
	$(HOSTCC) $(HOSTCFLAGS) -c $<
	$(HOSTCC) $(HOSTCFLAGS) -o$@ armdisas.o $(EXTLIB)/clxlib.a

disasm68k : 68kdisas.c
	$(HOSTCC) $(HOSTCFLAGS) -o$@ $<

#-----------------------------------------------------------------------------
