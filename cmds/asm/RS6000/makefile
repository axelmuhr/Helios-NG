#------------------------------------------------------------------------------
# Helios generic make system - NATIVE HOST MAKEFILE -
#-----------------------------------------------------------------------------
# RS6000 Host specific make rules to make NATIVE HOST version of ASM component.
#
# File: /HSRC/cmds/asm/RS6000/makefile
#
# This file contains ALL the definitions and rules that are
# required to make a NATIVE HOST version of this component.
# IT IS NOT COMPLIMENTARY TO THE GENERIC VERSION.
#
# This makefile is always executed within this host specific subdir.
# All host specific source files should be kept here.
# All intermediate, object and binary files should be made in here.
#
# RcsId: $Id: makefile,v 1.1 1993/06/10 14:04:59 bart Exp $
#		(C) Copyright 1990 Perihelion Software
#
# WARNING: assumes you are using GNUmake.
#-----------------------------------------------------------------------------
# Manditory variables for a component:
# name of COMPONENT directory relative to $(HSRC)
COMPONENT = cmds/asm

# Manditory includes
include $(HSRC)/makeinc/$(HHOST).mak	# Default host system variables

# Manditory phoney targets
.PHONY: default clean
# "default" - should make this component in its proc subdir
# "install" - copy the new component to $(HHOSTBIN) directory
# "clean"   - remove all intermediate and executable files from host subdir

VPATH := $(HSRC)/$(COMPONENT):../../support

#-----------------------------------------------------------------------------
# Note all final targets here

TARGETS = asm objdump

default : $(TARGETS)

install	: $(TARGETS)
	$(CP) $(TARGETS) $(HHOSTBIN)
	$(TOUCH) install

clean	:
	-$(RM) keyform keytab.c $(OBJECTS) $(TARGETS) mkout core ../mkout install

# Note all sources
SOURCES		= $(SOURCES.c) $(HEADERS.c) $(SOURCES.odd)

# Object files ( Note that keytab.c is created from keyform.c)
OBJECTS		=  asm.o gencode.o genimage.o growdata.o growcode.o keytab.o \
		objed.o	lex.o encode.o mem.o module.o preasm.o sym.o syn.o \
		vm.o queue.o

# C source files
SOURCES.c	= asm.c gencode.c genimage.c gencsc.c growcode.c keyform.c \
		objed.c lex.c mem.c module.c preasm.c sym.c syn.c encode.c \
		vm.c growdata.c objdump.c

# C header files
HEADERS.c	= asm.h vm.h


# Any other source files
SOURCES.odd	= makefile


#-----------------------------------------------------------------------------
# Native Host targets and their dependencies:

HOSTCFLAGS  := -DHOSTISBIGENDIAN -DRS6000 -D__TRAN -D__HELIOSTRAN -I/usr/include -I. -I.. -I../../support -I$(INCLUDE) -DVM
HOSTCC = cc

LOCALHOSTCFLAGS  := -DHOSTISBIGENDIAN -I/usr/include -I. -I.. -I../../support -I$(INCLUDE) -DVM

$(OBJECTS) : asm.h vm.h
	
#keytab.o : keyform.o keyform keytab.c 

keytab.c : keyform
	keyform

keyform : keyform.c
	$(HOSTCC) $< $(LOCALHOSTCFLAGS) -DSWAPKEYTABLE -o keyform

objdump : objdump.c
	$(HOSTCC) $< $(HOSTCFLAGS) -o objdump

asm :	$(OBJECTS)
	$(HOSTCC) -o asm $(OBJECTS)


#-----------------------------------------------------------------------------
