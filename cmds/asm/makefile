#-----------------------------------------------------------------------------
# Helios generic make system
#-----------------------------------------------------------------------------
# Helios generic component make rules for: Transputer Assembler/Linker
#
# File: /HSRC/cmds/asm/makefile
#
# These are the host/processor independent rules and variables for this
# component of Helios. It is included by the processor specific makefile
# That may add to some of its functionality. You should only alter this
# makefile by using "ifneq ($(HPROC),YYY)" clauses for a specific processor.
#
# RcsId: $Id: makefile,v 1.24 1993/06/11 14:24:00 bart Exp $
#		(C) Copyright 1990 Perihelion Software
#
# $Header: /hsrc/cmds/asm/RCS/makefile,v 1.24 1993/06/11 14:24:00 bart Exp $
#
# WARNING: assumes you are using GNUmake.
#-----------------------------------------------------------------------------

ifndef InProcMake
# This part of the makefile is run in the components generic source directory

# The variables HPROC, HHOST, HSRC, and HPROD should already be set by 
# your make script or environment, if not then get local defaults from
# $(HSRC)/makeinc/DFLT.mak. Note that the following pathname cannot use $(HSRC).
ifndef HPROC
include ../../makeinc/DFLT.mak
endif

.PHONY: notarget
notarget:
	@$(MAKE) -C $(HPROC)

.DEFAULT:
	@$(MAKE) -C $(HPROC) $@

else
#-----------------------------------------------------------------------------
# This part of the makefile is always run in a processor specific subdirectory.

# Manditory variables for a component:
# name of this dir relative to $(HSRC)
COMPONENT = cmds/asm

# Manditory includes
include $(HSRC)/makeinc/$(HHOST).mak	# Default host system variables
include $(HSRC)/makeinc/$(HPROC).mak	# Default processor rules and vars.

# Manditory phoney targets
.PHONY: default clean Xdefault Xclean
# "default" - should make this component in its proc subdir
# "install" - copy the new component to $(HPROD) production directory
# "clean"   - remove all intermediate and executable files from proc subdir
# Phoney targets prefixed by X are for adding processor specific targets.
#
# Processor specific sources are held in the processor specific subdir.,
# generic ones in the directory above it.
HVPATH := $(HSRC)/$(COMPONENT):../../support

ifdef RSRC
HVPATH := $(RSRC)/$(COMPONENT)/$(HPROC):$(HSRC)/$(COMPONENT):$(RSRC)/$(COMPONENT)
endif

vpath %.h $(HVPATH)
vpath %.a $(HVPATH)
vpath %.c $(HVPATH)

#-----------------------------------------------------------------------------
# Generic component make rules and variables added below here

# Note all final generic targets here
TARGETS := asm

default : $(TARGETS) Xdefault

install: $(TARGETS) Xinstall
	$(CP) asm $(BIN)
	$(TOUCH) install

clean	: Xclean
	-$(RM)	$(OBJECTS) $(TARGETS) $(XOBJECTS) $(XTARGETS) mkout \
		keytab.c keyform core ../mkout install installdef Xinstall

#-----------------------------------------------------------------------------
# for archive, backup and cleaning purposes,
# note all generic SOURCE and OBJECT files:

# Note all sources
SOURCES		= $(SOURCES.c) $(SOURCES.a) \
		   $(HEADERS.c) $(HEADERS.m) \
		   $(SOURCES.odd)

# C source files
SOURCES.c	= asm.c gencode.c genimage.c gencsc.c growcode.c \
                   keyform.c objed.c lex.c mem.c module.c preasm.c \
                   sym.c syn.c encode.c vm.c growdata.c objdump.c
#  keytab.c is automatically generated

OBJECTS		= asm.o gencode.o genimage.o growdata.o growcode.o keytab.o \
		objed.o lex.o encode.o mem.o module.o preasm.o sym.o syn.o \
		vm.o

# AMPP source files
SOURCES.a	= 

# C header files
HEADERS.c	= asm.h vm.h

# AMPP macro files
HEADERS.m	=

# Any other source files
SOURCES.odd	= makefile hash.c


#-----------------------------------------------------------------------------
# Other generic component variables:

INCLUDES := -j$(HSRC)/include/ -i,../,../../support/

ifdef RSRC
INCLUDES := -j$(HSRC)/include/,$(RSRC)/include/ -i,../,$(RSRC)/$(COMPONENT)/,../../support/
endif

# Set any flags required
  NCFLAGS := $(NCFLAGS) -wasdfv -ecpz -t4 $(INCLUDES) -DT800 -DNORCROFT -DVM

# Note any library def files to be linked with
LIBDEFS	:=

# must change to compile on any other host system!!!
ifeq ($(HHOST),SUN4)
 HOSTCFLAGS  := $(HOSTCFLAGS) -w -I/usr/include -I. -I.. -I../../support -I$(INCLUDE) -DVM -DSUN4
 HOSTCC = gcc

# AxM: changed cc to gcc
else
ifeq ($(HHOST),RS6000)
 HOSTCFLAGS  := $(HOSTCFLAGS) -I/usr/include -I. -I.. -I../../support -I$(INCLUDE) -DVM -DRS6000
 HOSTCC = cc
else
 HOSTCFLAGS  := $(HOSTCFLAGS) -wadsv -I. -I.. -I../../support -DVM
endif
endif

$(OBJECTS) : asm.h vm.h

#-----------------------------------------------------------------------------
# Generic targets and their dependencies:

keytab.c : keyform
	keyform

keyform : keyform.c
	$(HOSTCC) $< $(HOSTCFLAGS) -o keyform

asm:  $(OBJECTS)
	$(ASM) -v -f -s20000 -h4000 -nasm -mvmfile $(CSTART) $(OBJECTS) -oasm


#-----------------------------------------------------------------------------
endif #InProcMake	# This line must be at EOF

