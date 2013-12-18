#
# Makefile for the Helios C40 compiler
#
# Copyright (c) 1992 - 1993 by Perihelion Software Ltd.
#  All Rights Reserved
#
# Author: N Clifton
# RCS Id: $Id: makefile,v 1.39 1994/06/03 09:28:49 nickc Exp $
#

#-----------------------------------------------------------------------------
# Helios generic make system - GENERIC COMPONENT MAKEFILE -
#-----------------------------------------------------------------------------
# Helios generic component make rules for: Norcroft C compiler
#
# File: /HSRC/cmds/cc/ncc/makefile
#
# These are the host/processor independent rules and variables for this
# component of Helios. It is included by the processor specific makefile
# That may add to some of its functionality. You should only alter this
# makefile by using "ifneq ($(HPROC),YYY)" clauses for a specific processor.
#
# RcsId: $Id: makefile,v 1.39 1994/06/03 09:28:49 nickc Exp $
#		(C) Copyright 1990 Perihelion Software
#	(C) Copyright 1994 Perihelion Distributed Software
#
# WARNING: assumes you are using GNUmake.
#-----------------------------------------------------------------------------

ifndef InProcMake
# This part of the makefile is run in the components generic source directory

# The variables HPROC, HHOST, HSRC, and HPROD should already be set by 
# your make script or environment, if not then get local defaults from
# $(HSRC)/makeinc/DFLT.mak. Note that the following pathname cannot use $(HSRC).
ifndef HPROC
include ../makeinc/DFLT.mak
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
COMPONENT = cmds/cc/ncc/cc350

# Manditory includes
include $(HSRC)/makeinc/$(HHOST).mak	# Default host system variables
include $(HSRC)/makeinc/$(HPROC).mak	# Default processor rules and vars.

# Manditory phoney targets
.PHONY: default clean Xdefault Xclean
# "default"    - should make this component in its proc subdir
# "install"    - copy the new component to $(HPROD) production directory
# "clean"      - remove all intermediate and executable files from proc subdir
# Phoney targets prefixed by X are for adding processor specific targets.
#
# Processor specific sources are held in the processor specific subdir.,
# generic ones in the directory above it.
HVPATH := $(HSRC)/$(COMPONENT)

ifdef RSRC
 HVPATH := $(RSRC)/$(COMPONENT)/$(HPROC):$(HSRC)/$(COMPONENT):$(RSRC)/$(COMPONENT)
endif

vpath %.h $(HVPATH)
vpath %.a $(HVPATH)
vpath %.c $(HVPATH)

#-----------------------------------------------------------------------------
# Generic component make rules and variables added below here

# for archive, backup and cleaning purposes,
# note all generic SOURCE and OBJECT files:

# Note all sources
SOURCES		= $(SOURCES.c) $(SOURCES.a) \
		  $(HEADERS.c) $(HEADERS.m) \
		  $(SOURCES.odd)

OBJECTS	= $(SOURCES.c:.c=.o) $(SOURCES.a:.a=.o)

# C source files
SOURCES.c	= mip/aetree.c mip/aoutobj.c mip/bind.c mip/builtin.c mip/cg.c mip/codebuf.c \
		mip/coffobj.c mip/compiler.c mip/cse.c mip/csescan.c mip/dbx.c mip/driver.c \
		mip/flowgraf.c mip/fname.c mip/hdrproto.c mip/ieeeflt.c mip/jopprint.c mip/main.c \
		mip/misc.c mip/regalloc.c mip/regsets.c mip/s370flt.c mip/store.c mip/version.c \
		cfe/lex.c cfe/pp.c cfe/sem.c cfe/simplify.c cfe/syn.c cfe/vargen.c \
		mbe/asm.c mbe/debug.c mbe/gen.c mbe/heliobj.c mbe/mcdep.c mbe/peep.c \
		include/headers.c

# C header files
HEADERS.c	= mip/aeops.h mip/aetree.h mip/aout.h mip/bind.h mip/builtin.h mip/cg.h \
		mip/cgdefs.h mip/codebuf.h mip/coff.h mip/compiler.h mip/cse.h mip/cseguts.h \
		mip/defaults.h mip/defs.h mip/drivhelp.h mip/flowgraf.h mip/fname.h mip/globals.h \
		mip/host.h mip/jopcode.h mip/mcdep.h mip/miperrs.h mip/mipvsn.h mip/optproto.h \
		mip/regalloc.h mip/regsets.h mip/sixchar.h mip/store.h mip/util.h mip/version.h \
		mip/xrefs.h \
		cfe/feerrs.h cfe/fevsn.h cfe/lex.h cfe/pp.h cfe/sem.h cfe/simplify.h cfe/syn.h \
		cfe/vargen.h \
		mbe/errors.h mbe/mcdpriv.h mbe/mcerrs.h mbe/mcvsn.h mbe/ops.h mbe/peep.h \
		mbe/target.h
		include/c40.h include/helios.h include/module.h include/options.h

# AMPP macro files
HEADERS.m	=

# Any other source files
SOURCES.odd	= makefile mip/makefile cfe/makefile mbe/makefile include/makefile include/h.errors

#-----------------------------------------------------------------------------
# Generic targets and their dependencies:

.SUFFIXES:
.SUFFIXES: .src .rs6000_obj .snake_obj .sparc_obj .c40_obj

# programs

SNAKE_CC	= cc
SPARC_CC	= gcc
C40_CC		= ccc40
RS6000_CC	= xlc

SNAKE_LN	= cc
SPARC_LN	= gcc
C40_LN		= ldc40
RS6000_LN	= xlc

RM		= rm
MV		= mv
CD		= cd
CP		= cp

# command line flags for the programs

SNAKE_CFLAGS	= -Aa +w1
SPARC_CFLAGS	= -ansi -g -I/opt/gnu/include
C40_CFLAGS	= -Ffao -Zpl1 -Zps1 -Zpz1 -Zpw0
RS6000_CFLAGS	= 

#SNAKE_DEFINES	= -DDEBUG
SNAKE_DEFINES	=
SPARC_DEFINES	= -DCOMPILING_ON_UNIX -D__SPARC -DVERSION=`uname -r`
C40_DEFINES	= -I$(HPROD)/include -DCOMPILING_ON_UNIX -DNO_VERSION_STRINGS
RS6000_DEFINES	= -DCOMPILING_ON_UNIX

# NB/ we must use -Bdynamic as Solaris 2.0 is only able to run dymanically linked SunOs 4.0 executables

SNAKE_LDOPTS	= -Aa 
SPARC_LDOPTS	= -ansi # -Bdynamic
C40_LDOPTS	= $(HPROD)/lib/c0.o -l$(HPROD)/lib/helios.lib -l$(HPROD)/lib/c.lib
RS6000_LDOPTS	= 

# generic command line flags

CFLAGS		= -I../include -I../mbe -I../mip -I../cfe
DEFINES		= -D__SMT
LDOPTS		=

# compiler names

SNAKE_NCC	= ccc40
SPARC_NCC	= ncc-SUN4 
C40_NCC		= ncc
RS6000_NCC	= ccc40

#rules

.src.snake_obj:
	@ $(CD) ../$* ; $(MAKE) all CC=$(SNAKE_CC) CFLAGS="$(SNAKE_CFLAGS) $(CFLAGS)" \
			  DEFINES="$(SNAKE_DEFINES) $(DEFINES)" \
			  DEST=../HP

.src.sparc_obj:
	@ $(CD) ../$* ; $(MAKE) all CC=$(SPARC_CC) CFLAGS="$(SPARC_CFLAGS) $(CFLAGS)" \
			  DEFINES="$(SPARC_DEFINES) $(DEFINES)" \
			  DEST=../SUN4

.src.c40_obj:
	@ $(CD) ../$* ; $(MAKE) all CC=$(C40_CC) CFLAGS="$(C40_CFLAGS) $(CFLAGS)" \
			  DEFINES="$(C40_DEFINES) $(DEFINES)" \
			  DEST=../C40

.src.rs6000_obj:
	@ $(CD) ../$* ; $(MAKE) all CC=$(RS6000_CC) CFLAGS="$(RS6000_CFLAGS) $(CFLAGS)" \
			  DEFINES="$(RS6000_DEFINES) $(DEFINES)" \
			  DEST=../RS6000

# components

SNAKE_OBJS	= mbe.snake_obj mip.snake_obj cfe.snake_obj 
SPARC_OBJS	= mbe.sparc_obj mip.sparc_obj cfe.sparc_obj 
C40_OBJS	= mbe.c40_obj   mip.c40_obj   cfe.c40_obj   
RS6000_OBJS	= mbe.rs6000_obj mip.rs6000_obj cfe.rs6000_obj

#targets

ifeq ($(HHOST),HP)
    all: $(SNAKE_NCC) 

    $(SNAKE_NCC): $(SNAKE_OBJS)
	$(CD) ../HP; $(SNAKE_LN) $(SNAKE_LDOPTS) $(LDOPTS) *.o -o $(SNAKE_NCC)

else
  ifeq ($(HHOST),SUN4)
    all: $(SPARC_NCC) 

    $(SPARC_NCC): $(SPARC_OBJS)
	$(CD) ../SUN4; $(SPARC_LN) $(SPARC_LDOPTS) $(LDOPTS) *.o -o $(SPARC_NCC)

  else
    all: $(RS6000_NCC) 

    $(RS6000_NCC): $(RS6000_OBJS)
	$(CD) ../RS6000; $(RS6000_LN) $(RS6000_LDOPTS) $(LDOPTS) *.o -o $(RS6000_NCC)
  endif
endif

$(C40_NCC):   $(C40_OBJS)
	$(CD) ../C40; $(C40_LN) -s10000 $(C40_LDOPTS) $(LDOPTS) *.o -o $(C40_NCC)


install-C40: $(C40_NCC)
	-$(MV) $(C40_NCC) $(HPROD)/bin/ncc

install-SUN4: $(SPARC_NCC)

install-HP: $(SNAKE_NCC)
	-$(MV) $(HHOSTBIN)/$(SNAKE_NCC) $(HHOSTBIN)/$(SNAKE_NCC).old
	$(CP) ../HP/$(SNAKE_NCC) $(HHOSTBIN)

install-RS6000: $(RS6000_NCC)
	-$(MV) $(HHOSTBIN)/$(RS6000_NCC) $(HHOSTBIN)/$(RS6000_NCC).old
	$(CP) ../RS6000/$(RS6000_NCC) $(HHOSTBIN)

install: install-C40 # install-HP install-SUN4

clean-C40:
	$(CD) ../C40 ; $(RM) -f *.o $(C40_NCC)

clean-HP:
	$(CD) ../HP ; $(RM) -f *.o $(SNAKE_NCC)

clean-RS6000:
	$(CD) ../RS6000 ; $(RM) -f *.o $(RS6000_NCC)

clean-SUN4:
	$(CD) ../SUN4 ; $(RM) -f *.o $(SPARC_NCC)

squeekyclean: clean-C40 clean-HP clean-SUN4 clean-RS6000
	$(CD) ../mip   ; $(MAKE) clean
	$(CD) ../cfe   ; $(MAKE) clean
	$(CD) ../mbe   ; $(MAKE) clean

clean: clean-C40

tags:
	etags -e -t mbe/*.[ch] mip/*.[ch] cfe/*.[ch]

# dependencies

mip.snake_obj: mip.src
cfe.snake_obj: cfe.src
mbe.snake_obj: mbe.src

mip.sparc_obj: mip.src
cfe.sparc_obj: cfe.src
mbe.sparc_obj: mbe.src

mip.c40_obj: mip.src
cfe.c40_obj: cfe.src
mbe.c40_obj: mbe.src

mip.rs6000_obj: mip.src
cfe.rs6000_obj: cfe.src
mbe.rs6000_obj: mbe.src

cfe.src:
mip.src:
mbe.src:

#-----------------------------------------------------------------------------
endif #InProcMake	# This line must be at EOF
