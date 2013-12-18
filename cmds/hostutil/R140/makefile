#------------------------------------------------------------------------------
# Helios generic make system - NATIVE HOST MAKEFILE -
#-----------------------------------------------------------------------------
# R140 Host specific make rules to make NATIVE HOST version of Host Utilities.
#
# File: /HSRC/cmds/hostutil/R140/makefile
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
# RcsId: $Id: makefile,v 1.15 1993/08/08 16:44:14 paul Exp $
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

HVPATH := $(HSRC)/$(COMPONENT):$(CMDS)/com:$(CMDS)/com/$(HPROC):$(CMDS)/support

ifdef RSRC
HVPATH := $(RSRC)/$(COMPONENT)/$(HHOST):$(HSRC)/$(COMPONENT):$(RSRC)/$(COMPONENT):$(CMDS)/com:$(CMDS)/com/$(HPROC):$(CMDS)/support
endif

vpath %.h $(HVPATH)
vpath %.c $(HVPATH)
vpath %.a $(HVPATH)

#-----------------------------------------------------------------------------
# Note all final targets here

TARGETS = sysbuild sysstrip objed newmotd dump gencodes squash buildkeymap c

ifeq ($(HLICENSEE), ABC)
 TARGETS = buildconfig cbin bromitem brom graycode linktest buildtime brompad
endif

default : $(TARGETS)

install	: $(TARGETS)
	$(CP) $(TARGETS) $(HHOSTBIN)
	$(TOUCH) install

clean	:
	-$(RM)	$(OBJECTS) $(TARGETS) mkout core ../mkout install

# Note all sources
SOURCES		= $(SOURCES.c) $(HEADERS.c) $(SOURCES.odd)

# Object files
OBJECTS		= $(SOURCES.c:.c=.o) sysbuild.o sysstrip.o objed.o \
		  buildkeymap.o c.o

# C source files
SOURCES.c	:= cbin.c bromitem.c \
		brom.c genhdr.c romsplit.c linktest.c \
		genhdr.c newmotd.c buildconfig.c dump.c graycode.c \
		buildtime.c brompad.c gencodes.c squash.c testenc.c

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
	$(HOSTCC) -o$@ $(HOSTCFLAGS) -I/usr/include,$(INCLUDE) $<

# image header stripper
sysstrip: sysstrip.c
	$(HOSTCC) -o$@ $(HOSTCFLAGS) -I/usr/include,$(INCLUDE) $<

# Helios object editor (display/change a program's stack/heap/name)
objed : objed.c
	$(HOSTCC) -o$@ $(HOSTCFLAGS) -I/usr/include,$(INCLUDE) $<

# Helios keyboard /console server keymap builder
buildkeymap : $(CMDS)/com/buildkeymap.c
	$(HOSTCC) -ansi -o$@ $(HOSTCFLAGS) $<


#---------------------------------------------------------------------------
# dump the contents of a unix a.out format file.
# Used for testing munge programs
#dumpa: dumpa.c
#	$(HOSTCC) -lansi -o dumpa dumpa.c

#---------------------------------------------------------------------------
# Rom support tools

# Splits a 32bit wide romimage into four 8bit images
romsplit: romsplit.c
	$(HOSTCC) -lansi -o $@ $<

# Generate a gray-coded ROM image
graycode:	graycode.c
		$(HOSTCC) -lansi -o $@ $<

# Creates a raw binary file from a unix a.out file
# (removes header, symbol table, etc.)
cbin: cbin.c
	$(HOSTCC) -lansi -o $@ $<

# build a ROM item for later inclusion in the ROM image
bromitem: bromitem.c
	$(HOSTCC) -lansi -o $@ $<

# Build ROM image
brom: brom.c
	$(HOSTCC) -lansi -o $@ $<

# pad an image to a given size
brompad: brompad.c
	$(HOSTCC) -ansi -lansi -o $@ $<
 
#---------------------------------------------------------------------------
# Construct a standard "motd" file
newmotd: newmotd.c
	$(HOSTCC) -ansi -o $@ $<

# tool to generate hobjasm headers from C style headers

genhdr: genhdr.c
	$(HOSTCC) -lansi -I$(INCLUDE) -o $@ $<

#---------------------------------------------------------------------------

buildtime: buildtime.c
	$(HOSTCC) -lansi -o $@ $<

#---------------------------------------------------------------------------
# Construct a default "ROMconfig" file for "/etc/config" under Helios

buildconfig:	buildconfig.c
		$(HOSTCC) -ansi -I/usr/include,$(INCLUDE) -o buildconfig \
			buildconfig.c

#---------------------------------------------------------------------------
# tool to provide HEX/ASCII dumps of binaries

dump:		dump.c
		$(HOSTCC) -o dump dump.c

#---------------------------------------------------------------------------
# r140 to abc prototype monitor terminal program.

linktest: linktest.c
	$(HOSTCC) -lansi -o $@ $<

#---------------------------------------------------------------------------
# Test encoding functions for helios object files.

testenc: testenc.c
	$(HOSTCC) -lansi -o $@ $<


#-----------------------------------------------------------------------------
# Fault library host utilities:

gencodes : gencodes.c
ifeq ($(HHOST),HELIOSTRAN)
	$(HOSTCC) $(HOSTCFLAGS) -b $< -o$@
else
	$(HOSTCC) -o $@ $(HOSTCFLAGS) $<
endif

squash : squash.c
ifeq ($(HHOST),HELIOSTRAN)
	$(HOSTCC) $(HOSTCFLAGS) -b $< -o$@
else
	$(HOSTCC) $(HOSTCFLAGS) -o $@ $<
endif

#-----------------------------------------------------------------------------

WHERE := -DC_COMPILER=\"ccarm\" \
	-DASSEMBLER=\"asarm\" \
	-DLINKER=\"ldarm\" \
	-DAMPP=\"ampp\" \
	-DREMOVER=\"rm\" \
	-DCAT=\"cat\" \
	-DTRANSPUTER_CC=\"nc\" \
	-DTRANSPUTER_ASSEMBLER=\"asm\" \
	-DTRANSPUTER_LINKER=\"asm\" \
	-DTRANSPUTER_C_LIBDIR=\"/hprod/TRAN/lib/\" \
	-DC40_CC=\"ccc40\" \
	-DC40_ASSEMBLER=\"asc40\" \
	-DC40_LINKER=\"ldc40\" \
	-DC40_C_LIBDIR=\"/hprod/C40/lib/\" \
	-DARM_CC=\"ccarm\" \
	-DARM_ASSEMBLER=\"asarm\" \
	-DARM_LINKER=\"ldarm\" \
	-DARM_C_LIBDIR=\"/hprod/ARM/lib/\" \
	-DMACRO=\"ampp\" \
	-DC_INCDIR=\"$(HSRC)/include/\" \
	-DC_LIBDIR=\"$(HPROD)/lib/\" \
	-DMACDIR=\"$(HSRC)/include/ampp/\" 

c : c.c queue.c
	$(HOSTCC) $(HOSTCFLAGS) -o c -I$(CMDS)/support -I/usr/include $(WHERE) -DUNIX $^


#-----------------------------------------------------------------------------

