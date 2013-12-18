#------------------------------------------------------------------------------
# Helios generic make system - NATIVE HOST MAKEFILE -
#-----------------------------------------------------------------------------
# HP Host specific make rules to make NATIVE HOST version of assembler.
#
# File: /HSRC/cmds/assembler/HP/makefile
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
# RcsId: $Id: makefile,v 1.9 1993/06/22 17:01:41 paul Exp $
#		(C) Copyright 1990 Perihelion Software
#
# WARNING: assumes you are using GNUmake.
#-----------------------------------------------------------------------------
# Manditory variables for a component:
# name of COMPONENT directory relative to $(HSRC)

COMPONENT = cmds/assembler

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

ifeq ($(HPROC),ARM)
  THEASM = asarm#
else
 ifeq ($(HPROC),C40)
  THEASM = asc40#
 else
  THEASM =
 endif
endif

# Note all target CPUs that are built on this host (prefixed with 'build_')
POSSIBLECPUTARGETS = build_ARM build_C40

TARGETS = build_$(HPROC) $(THEASM) binhdr

default : $(TARGETS)

install	: $(TARGETS)
	$(CP) $(THEASM) $(HHOSTBIN)
	$(TOUCH) install

clean	:
	-$(RM)	*.o $(TARGETS) $(XOBJECTS) $(XTARGETS) mkout \
		core ../mkout install installdef Xinstall \
		gasm.y y.tab.c y.tab.h y.tab.o y.output \
		binary.h $(POSSIBLECPUTARGETS)

# If we build for a new processor target clean out the old target proc. objects
build_$(HPROC):
	$(MAKE) clean
	$(TOUCH) build_$(HPROC)


# Note all sources
SOURCES		= $(SOURCES.c) $(HEADERS.c) $(SOURCES.odd)

# Object files
OBJECTS		= $(SOURCES.c:.c=.o)

# C source files
SOURCES.c	=

# C header files
HEADERS.c	=

# Any other source files
SOURCES.odd	= makefile


#-----------------------------------------------------------------------------
# for archive, backup and cleaning purposes,
# note all generic SOURCE and OBJECT files:

# Note all sources
SOURCES		= $(SOURCES.c) $(SOURCES.a) \
		  $(HEADERS.c) $(HEADERS.m) \
		  $(SOURCES.odd)

ASMOBJS		= $(ASMSRCS.c:.c=.o)

# C source files
SOURCES.c	= $(ASMSRCS.c) $(ODDSRCS.c)

MAINSRCS.c	= main.c symbtab.c hash.c lex.c eval.c ghof.c ghofenc.c float.c

ASMSRCS.c	= $(MAINSRCS.c) $(TARGETSRCS.c)

# must include target sources for all possible processors
ODDSRCS.c	= binhdr.c itoabin.c alloca.c \
		lex_ARM.c lex_C40.c pass2_C40.c pass2_ARM.c

TARGETSRCS.c	= lex_$(TARGET).c pass2_$(TARGET).c

# AMPP source files
SOURCES.a	=

# C header files
HEADERS.c	= gasm.h warn.h hdr_C40.h hdr_ARM.h ghof.h

# AMPP macro files
HEADERS.m	=

# Any other source files
SOURCES.odd	= makefile gasm.doc cpu_port.doc testpseudo.s \
		gasm.ypp \
		toks_C40.ypp rules_C40.ypp testc40.s \
		c40asm.doc \
		toks_ARM.ypp rules_ARM.ypp testarm.s \
		armasm.doc

#-----------------------------------------------------------------------------
# Generic targets and their dependencies:

TARGET = $(HPROC)#		# target cpu name used in filenames and ifdefs

# run C pre-processor (CPP), dont remove comments and dont add #line directives
THECPP = $(CPP) $(DEFINES) -DHP	# defined in /hsrc/makeinc/$(HHOST).mak

# Depending on target architecture choose a set of files that
# implement the processor specific parts of the assembler.

CPU_HEADER	= -DCPU_HEADER=\"hdr_$(TARGET).h\"
CPU_YYTOKENS	= -DCPU_YYTOKENS=\"toks_$(TARGET).ypp\"
CPU_YYRULES	= -DCPU_YYRULES=\"rules_$(TARGET).ypp\"

# set if you are using bison
#BISON = -DBISON

# run YACC creating separate y.tab.c and y.tab.h files (-d),
# removing #lines (-l), optionally with debugging (-t).
# (bison -y = yacc compatibility)

ifdef BISON
 YACC = bison -v -d -y -l -t
else
# YACC = yacc -vd -Ns600000 -Nm50000 #-l -t
 YACC = yacc -vd 
endif

ifdef BISON
 ALLOCA = alloca.o
endif

# Also available: -DDEBUG
DEFINES = -D__$(TARGET)TARGET $(BISON) \
	$(CPU_HEADER) $(CPU_YYTOKENS) $(CPU_YYRULES)


ifeq ($(HHOST),R140)
 HOSTCFLAGS := $(HOSTCFLAGS) -ansi $(DEFINES) -I.
else
 ifeq ($(HHOST),HP)
  HOSTCFLAGS := $(HOSTCFLAGS) -Aa $(DEFINES) -I.
 else
 endif
endif

#-----------------------------------------------------------------------------
# Generic targets and their dependencies:

# Automatically generate binary constants header file

binhdr:	binhdr.c
	$(HOSTCC) $(HOSTCFLAGS) -o$@ $^

../binary.h : binhdr
	binhdr > ../binary.h


gasm.y:	gasm.ypp toks_$(TARGET).ypp rules_$(TARGET).ypp
	$(THECPP) $< > $@

y.tab.c y.tab.h: gasm.y
	$(YACC) $<

lex_$(TARGET).o lex.o ghof.o eval.o: y.tab.h

$(ASMOBJS) gasm.y: gasm.h hdr_$(TARGET).h ../binary.h ghof.h


$(THEASM) : $(ASMOBJS) y.tab.o itoabin.o $(ALLOCA)
	$(HOSTCC) -o$@ $(HOSTCFLAGS) $^

print:
	-laserp -l $(SOURCES.c) $(HEADERS.c) $(SOURCES.odd)


#-----------------------------------------------------------------------------
