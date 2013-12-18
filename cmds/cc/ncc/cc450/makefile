#
# Makefile for the Helios C compilers
#
# Copyright (c) 1992-1993 by Perihelion Software Ltd.
#  All Rights Reserved
#
# Author: N Clifton
# RCS Id: $Id: makefile,v 1.38 1993/11/17 17:17:29 nickc Exp $
#

#-----------------------------------------------------------------------------
# Helios generic make system - GENERIC COMPONENT MAKEFILE -
#-----------------------------------------------------------------------------
# Helios generic component make rules for: Norcroft C compiler
#
# File: /HSRC/cmds/cc/ncc/cc450/makefile
#
# These are the host/processor independent rules and variables for this
# component of Helios. It is included by the processor specific makefile
# That may add to some of its functionality. You should only alter this
# makefile by using "ifneq ($(HPROC),YYY)" clauses for a specific processor.
#
# SccsId: %W% %G%
# RcsId: $Id: makefile,v 1.38 1993/11/17 17:17:29 nickc Exp $
#		(C) Copyright 1990 Perihelion Software
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
COMPONENT = cmds/cc/ncc/cc450

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

SOURCES.c = mip/aetree.c mip/aoutobj.c mip/bind.c mip/builtin.c mip/cg.c mip/codebuf.c \
	  mip/coffobj.c mip/compiler.c mip/cse.c mip/csescan.c mip/dbx.c mip/driver.c \
	  mip/flowgraf.c mip/fname.c mip/heliobj.c mip/ieeeflt.c mip/jopprint.c mip/misc.c \
	  mip/prgname.c mip/regalloc.c mip/regsets.c mip/store.c mip/version.c \
	  cfe/lex.c cfe/pp.c cfe/sem.c cfe/simplify.c cfe/syn.c cfe/vargen.c \
	  util/datenow.c util/genhdrs.c util/peepgen.c util/striphdr.c util/toansi.c util/topcc.c

HEADERS.c = mip/aeops.h mip/aetree.h mip/aout.h mip/bind.h mip/builtin.h mip/cg.h mip/cgdefs.h \
	  mip/codebuf.h mip/coff.h mip/compiler.h mip/cse.h mip/cseguts.h mip/defaults.h \
	  mip/defs.h mip/drivhelp.h mip/flowgraf.h mip/fname.h mip/globals.h mip/helios.h \
	  mip/host.h mip/jopcode.h mip/mcdep.h mip/miperrs.h mip/mipvsn.h mip/module.h \
	  mip/optproto.h mip/prgname.h mip/regalloc.h mip/regsets.h mip/sixchar.h mip/store.h \
	  mip/util.h mip/version.h mip/xrefs.h \
	  cfe/feerrs.h cfe/fevsn.h cfe/lex.h cfe/pp.h cfe/sem.h cfe/simplify.h cfe/syn.h \
	  cfe/vargen.h 

SOURCES.odd = makefile mip/makefile cfe/makefile util/Makefile RS6000/makefile HP/makefile \
	    SUN4/makefile

# Target specific sources

ifeq ($(HPROC),ARM)
  SOURCES.c	:= $(SOURCES.c) arm/aaof.c arm/asd.c arm/asm.c arm/gen.c arm/mcdep.c \
		   arm/peephole.c arm/peeppat.c arm/peeptest.c arm/uasm.c
  HEADERS.c	:= $(HEADERS.c) arm/errors.h arm/mcdpriv.h arm/mcerrs.h arm/mcvsn.h arm/ops.h \
		   arm/options.h arm/target.h
  SOURCES.odd   := $(SOURCES.odd) arm/makefile ARM/makefile
else
  ifeq ($(HPROC),C40)
    SOURCES.c	:= $(SOURCES.c) c40/asm.c c40/debug.c c40/gen.c c40/mcdep.c c40/peep.c
    HEADERS.c	:= $(HEADERS.c) c40/errors.h c40/mcdpriv.h c40/mcerrs.h c40/mcvsn.h c40/ops.h \
		   c40/options.h c40/peep.h c40/target.h
    SOURCES.odd := $(SOURCES.odd) c40/makefile C40/makefile

  else
    ifeq ($(HPROC),M68K)
      SOURCES.c	:= $(SOURCES.c) 68k/asm.c 68k/decins.c 68k/gen.c 68k/mcdep.c 68k/unixins.c
      HEADERS.c	:= $(HEADERS.c) 68k/errors.h 68k/mcdpriv.h 68k/mcerrs.h 68k/mcvsn.h 68k/ops.h \
		   68k/options.h 68k/target.h
      SOURCES.odd := $(SOURCES.odd) 68k/makefile M68K/makefile
    else
      echo *** unknown target for compiler
    endif
  endif
endif

#-----------------------------------------------------------------------------
# Other generic component variables:

#-----------------------------------------------------------------------------
# Generic targets and their dependencies:

.SUFFIXES:
.SUFFIXES: .src .rs6000_obj .snake_obj .sparc_obj .c40_obj .arm_obj .68k_obj

# programs

SNAKE_CC	= cc
SPARC_CC	= gcc
RS6000_CC	= xlc
C40_CC		= ccc40
ARM_CC		= ccarm
68K_CC		= ccm68k

SNAKE_LN	= cc
SPARC_LN	= gcc
RS6000_LN	= xlc
C40_LN		= ldc40
ARM_LN		= ldarm
68K_LN		= ldm68k

RM		= rm
MV		= mv
CD		= cd
CP		= cp

# command line flags for the programs

SNAKE_CFLAGS	= -Aa +w1
SPARC_CFLAGS	= -ansi  
RS6000_CFLAGS	= 
C40_CFLAGS	= -Ffao -Zpz1 -Zpl1 -Zps1 -Zpw0
ARM_CFLAGS      = -D__HELIOS
68K_CFLAGS      =

#SNAKE_DEFINES	= -DDEBUG
SNAKE_DEFINES	=
SPARC_DEFINES	= -D__SPARC
RS6000_DEFINES	= 
C40_DEFINES	= -I$(HPROD)/include
ARM_DEFINES	= -I$(HPROD)/include
68K_DEFINES     = -I$(HPROD)/include

# NB/ we must use -Bdynamic as Solaris 2.0 is only able to run dymanically linked SunOs 4.0 executables

SNAKE_LDOPTS	= -Aa 
SPARC_LDOPTS	= -ansi -Bdynamic
RS6000_LDOPTS	= 
C40_LDOPTS	= $(HPROD)/lib/c0.o -l$(HPROD)/lib/helios.lib -l$(HPROD)/lib/c.lib
ARM_LDOPTS	= $(HPROD)/lib/c0.o -l$(HPROD)/lib/helios.lib -l$(HPROD)/lib/c.lib
68K_LDOPTS	= $(HPROD)/lib/c0.o -l$(HPROD)/lib/helios.lib -l$(HPROD)/lib/c.lib

# generic command line flags

ifeq ($(HPROC),ARM)
  CFLAGS		= -I../mip -I../cfe -I../arm
else
  ifeq ($(HPROC),C40)
    CFLAGS		= -I../mip -I../cfe -I../c40
  else
    ifeq ($(HPROC),M68K)
      CFLAGS		= -I../mip -I../cfe -I../68k
    endif
  endif
endif

ifeq ($(HPROC),M68K)
  DEFINES		= -DCOMPILING_ON_UNIX -DNO_VERSION_STRINGS
else
  DEFINES		= -DCOMPILING_ON_UNIX -DNO_VERSION_STRINGS -D__SMT
endif

LDOPTS		=

# compiler names

ifeq ($(HPROC),ARM)
  SNAKE_NCC	= ccarm
  SPARC_NCC	= ccarm 
  RS6000_NCC	= ccarm
  FOREIGN_CC    = ccc40 ccm68k
else
  ifeq ($(HPROC),C40)
    SNAKE_NCC	= ccc40
    SPARC_NCC	= ccc40 
    RS6000_NCC	= ccc40
    FOREIGN_CC  = ccarm ccm68k
  else
    ifeq ($(HPROC),M68K)
      SNAKE_NCC	 = ccm68k
      SPARC_NCC	 = ccm68k
      RS6000_NCC = ccm68k
      FOREIGN_CC = ccarm ccc40
    endif
  endif
endif

C40_NCC		= ncc
ARM_NCC		= ncc
68K_NCC         = ncc

#rules

.src.snake_obj:
	@ $(CD) ../$* ; $(MAKE) all CC=$(SNAKE_CC) CFLAGS="$(SNAKE_CFLAGS) $(CFLAGS)" \
			  DEFINES="$(SNAKE_DEFINES) $(DEFINES)" \
			  DEST=../HP

.src.sparc_obj:
	@ $(CD) ../$* ; $(MAKE) all CC=$(SPARC_CC) CFLAGS="$(SPARC_CFLAGS) $(CFLAGS)" \
			  DEFINES="$(SPARC_DEFINES) $(DEFINES)" \
			  DEST=../SUN4

.src.rs6000_obj:
	@ $(CD) ../$* ; $(MAKE) all CC=$(RS6000_CC) CFLAGS="$(RS6000_CFLAGS) $(CFLAGS)" \
			  DEFINES="$(RS6000_DEFINES) $(DEFINES)" \
			  DEST=../RS6000

.src.c40_obj:
	@ $(CD) ../$* ; $(MAKE) all CC=$(C40_CC) CFLAGS="$(C40_CFLAGS) $(CFLAGS)" \
			  DEFINES="$(C40_DEFINES) $(DEFINES)" \
			  DEST=../C40

.src.arm_obj:
	@ $(CD) ../$* ; $(MAKE) all CC=$(ARM_CC) CFLAGS="$(ARM_CFLAGS) $(CFLAGS)" \
			  DEFINES="$(ARM_DEFINES) $(DEFINES)" \
			  DEST=../ARM

.src.68k_obj:
	@ $(CD) ../$* ; $(MAKE) all CC=$(68K_CC) CFLAGS="$(68K_CFLAGS) $(CFLAGS)" \
			  DEFINES="$(68K_DEFINES) $(DEFINES)" \
			  DEST=../M68K

# components

ifeq ($(HPROC),ARM)
  SNAKE_OBJS	= arm.snake_obj  mip.snake_obj  cfe.snake_obj 
  SPARC_OBJS	= arm.sparc_obj  mip.sparc_obj  cfe.sparc_obj 
  RS6000_OBJS	= arm.rs6000_obj mip.rs6000_obj cfe.rs6000_obj
else
  ifeq ($(HPROC),C40)
    SNAKE_OBJS	= c40.snake_obj  mip.snake_obj  cfe.snake_obj 
    SPARC_OBJS	= c40.sparc_obj  mip.sparc_obj  cfe.sparc_obj 
    RS6000_OBJS	= c40.rs6000_obj mip.rs6000_obj cfe.rs6000_obj
  else
    ifeq ($(HPROC),M68K)
      SNAKE_OBJS  = 68k.snake_obj  mip.snake_obj  cfe.snake_obj 
      SPARC_OBJS  = 68k.sparc_obj  mip.sparc_obj  cfe.sparc_obj 
      RS6000_OBJS = 68k.rs6000_obj mip.rs6000_obj cfe.rs6000_obj
    endif
  endif
endif

C40_OBJS	= c40.c40_obj    mip.c40_obj    cfe.c40_obj   
ARM_OBJS	= arm.arm_obj    mip.arm_obj    cfe.arm_obj   
68K_OBJS	= 68k.68k_obj    mip.68k_obj    cfe.68k_obj   


#targets

ifeq ($(HHOST),HP)
    all: $(SNAKE_NCC) 

    $(SNAKE_NCC): test-clean-HP $(SNAKE_OBJS)
	$(CD) ../HP; $(SNAKE_LN) $(SNAKE_LDOPTS) $(LDOPTS) *.o -o $(SNAKE_NCC)
else
  ifeq ($(HHOST),SUN4)
    all: $(SPARC_NCC) 

    $(SPARC_NCC): test-clean-SUN4 $(SPARC_OBJS)
	$(CD) ../SUN4; $(SPARC_LN) $(SPARC_LDOPTS) $(LDOPTS) *.o -o $(SPARC_NCC)
  else
    ifeq ($(HHOST),RS6000)
      all: $(RS6000_NCC) 

      $(RS6000_NCC): test-clean-RS6000 $(RS6000_OBJS)
	  $(CD) ../RS6000; $(RS6000_LN) $(RS6000_LDOPTS) $(LDOPTS) *.o -o $(RS6000_NCC)
    endif
  endif
endif

ifeq ($(HPROC),ARM)
  $(ARM_NCC):   $(ARM_OBJS)
	$(CD) ../ARM; $(ARM_LN) -s20000 $(ARM_LDOPTS) $(LDOPTS) *.o -o $(ARM_NCC)
else
  ifeq ($(HPROC),C40)
    $(C40_NCC):   $(C40_OBJS)
	$(CD) ../C40; $(C40_LN) -s20000 $(C40_LDOPTS) $(LDOPTS) *.o -o $(C40_NCC)
  else
    ifeq ($(HPROC),M68K)
      $(68K_NCC):   $(68K_OBJS)
	$(CD) ../M68K; $(68K_LN) -s20000 $(68K_LDOPTS) $(LDOPTS) *.o -o $(68K_NCC)
    endif
  endif
endif


install-C40: $(C40_NCC)
	-$(MV) $(C40_NCC) $(HPROD)/bin/ncc

install-ARM: $(ARM_NCC)
	-$(MV) $(ARM_NCC) $(HPROD)/bin/ncc

install-68K: $(68K_NCC)
	-$(MV) $(68K_NCC) $(HPROD)/bin/ncc

install-SUN4: $(SPARC_NCC)

install-HP: $(SNAKE_NCC)
	-$(MV) $(HHOSTBIN)/$(SNAKE_NCC) $(HHOSTBIN)/$(SNAKE_NCC).old
	$(CP) ../HP/$(SNAKE_NCC) $(HHOSTBIN)

install-RS6000: $(RS6000_NCC)
	-$(MV) $(HHOSTBIN)/$(RS6000_NCC) $(HHOSTBIN)/$(RS6000_NCC).old
	$(CP) ../RS6000/$(RS6000_NCC) $(HHOSTBIN)

ifeq ($(HPROC),ARM)
  DEF_INSTALL = install-ARM
  DEF_CLEAN   = clean-ARM
else
  ifeq ($(HPROC),C40)
    DEF_INSTALL = install-C40
    DEF_CLEAN   = clean-C40
  else
    ifeq ($(HPROC),M68K)
      DEF_INSTALL = install-68K
      DEF_CLEAN   = clean-68K
    endif
  endif
endif

install: $(DEF_INSTALL)


clean-C40:
	$(CD) ../C40 ; $(RM) -f *.o $(C40_NCC)

clean-ARM:
	$(CD) ../ARM ; $(RM) -f *.o $(ARM_NCC)

clean-68K:
	$(CD) ../M68K ; $(RM) -f *.o $(68K_NCC)

clean-HP:
	$(CD) ../HP ; $(RM) -f *.o $(SNAKE_NCC)

clean-RS6000:
	$(CD) ../RS6000 ; $(RM) -f *.o $(RS6000_NCC)

clean-SUN4:
	$(CD) ../SUN4 ; $(RM) -f *.o $(SPARC_NCC)

squeekyclean: clean-C40 clean-HP clean-SUN4 clean-RS6000 clean-ARM clean-68K
	$(CD) ../mip   ; $(MAKE) clean
	$(CD) ../cfe   ; $(MAKE) clean
	$(CD) ../c40   ; $(MAKE) clean
	$(CD) ../arm   ; $(MAKE) clean
	$(CD) ../M68k  ; $(MAKE) clean

clean: $(DEF_CLEAN)

test-clean-RS6000:
	$(CD) ../RS6000; $(foreach FOREIGN, $(FOREIGN_CC), if [ -f $(FOREIGN) ] ; then rm *.o $(FOREIGN) ; fi ; )

test-clean-HP:
	$(CD) ../HP; $(foreach FOREIGN, $(FOREIGN_CC), if [ -f $(FOREIGN) ] ; then rm *.o $(FOREIGN) ; fi ; )

test-clean-SUN4:
	$(CD) ../SUN4; $(foreach FOREIGN, $(FOREIGN_CC), if [ -f $(FOREIGN) ] ; then rm *.o $(FOREIGN) ; fi ; )

tags:
	etags -e -t c40/*.[ch] mip/*.[ch] cfe/*.[ch]

# dependencies

mip.snake_obj: mip.src
cfe.snake_obj: cfe.src
c40.snake_obj: c40.src
arm.snake_obj: arm.src
68k.snake_obj: 68k.src

mip.sparc_obj: mip.src
cfe.sparc_obj: cfe.src
c40.sparc_obj: c40.src
arm.sparc_obj: arm.src
68k.sparc_obj: 68k.src

mip.c40_obj: mip.src
cfe.c40_obj: cfe.src
c40.c40_obj: c40.src
arm.c40_obj: arm.src
68k.c40_obj: 68k.src

mip.rs6000_obj: mip.src
cfe.rs6000_obj: cfe.src
c40.rs6000_obj: c40.src
arm.rs6000_obj: arm.src
68k.rs6000_obj: 68k.src

cfe.src:
mip.src:
c40.src:
arm.src:
68k.src:

#-----------------------------------------------------------------------------
endif #InProcMake	# This line must be at EOF
