#------------------------------------------------------------------------------
# Helios generic make system - NATIVE HOST MAKEFILE -
#-----------------------------------------------------------------------------
# HP Host specific make rules to make NATIVE HOST version of Host Utilities.
#
# File: /HSRC/cmds/hostutil/HP/makefile
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
# RcsId: $Id: makefile,v 1.6 1993/08/17 09:50:22 paul Exp $
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

TARGETS = sysbuild objed newmotd gencodes squash c buildkeymap

default : $(TARGETS)

install	: $(TARGETS)
	$(CP) $(TARGETS) $(HHOSTBIN)
	$(TOUCH) install

clean	:
	-$(RM)	$(OBJECTS) $(TARGETS) mkout core ../mkout install

# Note all sources
SOURCES		= $(SOURCES.c) $(HEADERS.c) $(SOURCES.odd)


# Object files
OBJECTS		= $(SOURCES.c:.c=.o) sysbuild.o objed.o buildkeymap.o

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
	$(HOSTCC) -o $@ $(HOSTCFLAGS) -I/usr/include -I$(INCLUDE) \
		-DBYTE_SEX_ODD $<

# Helios object editor (display/change a program's stack/heap/name)
objed : objed.c
	$(HOSTCC) -o $@ $(HOSTCFLAGS) -I/usr/include -I$(INCLUDE) $<

# Helios keyboard /console server keymap builder
buildkeymap : $(CMDS)/com/buildkeymap.c
	$(HOSTCC) -o$@ $(HOSTCFLAGS) $<


#---------------------------------------------------------------------------
# Construct a standard "motd" file

newmotd: newmotd.c
	$(HOSTCC) $(HOSTCFLAGS) -o $@ $<


#---------------------------------------------------------------------------
# Test encoding functions for helios object files.

testenc: testenc.c
	$(HOSTCC) $(HOSTCFLAGS) -o $@ $<


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

# Set any flags required
WHERE := -DC_COMPILER=\"ccc40\" \
	-DASSEMBLER=\"asc40\" \
	-DLINKER=\"ldc40\" \
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

