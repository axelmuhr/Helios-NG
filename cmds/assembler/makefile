#-----------------------------------------------------------------------------
# Helios generic make system - GENERIC COMPONENT MAKEFILE -
#-----------------------------------------------------------------------------
# Helios generic component make rules for: generic assembler
#
# File: /HSRC/cmds/assembler/makefile
#
# These are the host/processor independent rules and variables for this
# component of Helios. It is included by the processor specific makefile
# That may add to some of its functionality. You should only alter this
# makefile by using "ifneq ($(HPROC),YYY)" clauses for a specific processor.
#
# SccsId: %W% %G%
# RcsId: $Id: makefile,v 1.11 1994/01/05 14:04:36 paul Exp $
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
COMPONENT = cmds/assembler

# Manditory includes
include $(HSRC)/makeinc/$(HHOST).mak	# Default host system variables
include $(HSRC)/makeinc/$(HPROC).mak	# Default processor rules and vars.

# Manditory phoney targets
.PHONY: default clean Xdefault Xclean
# "default"    - should make this component in its proc subdir
# "install"    - copy the new component to $(HPROD) production directory
# "installdef" - If component is a library, make .def file and install in LIB
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
vpath %.ypp $(HVPATH)

#-----------------------------------------------------------------------------
# Generic component make rules and variables added below here

# Note all final generic targets here
TARGETS = as

default : $(TARGETS) Xdefault

install	: $(TARGETS) Xinstall
	$(CP) $(TARGETS) $(BIN)
	$(TOUCH) install

clean	: Xclean
	-$(RM)	*.o $(TARGETS) $(XOBJECTS) $(XTARGETS) mkout \
		core ../mkout install installdef Xinstall \
		gasm.y y.tab.c y.tab.h y.tab.o y.output \
		binhdr ../binary.h


#-----------------------------------------------------------------------------
# for archive, backup and cleaning purposes,
# note all generic SOURCE and OBJECT files:

# Note all sources
SOURCES		= $(SOURCES.c) $(SOURCES.a) \
		  $(HEADERS.c) $(HEADERS.m) \
		  $(SOURCES.odd)

ASMOBJS		= $(ASMSRCS.c:.c=.o)

# C source files
SOURCES.c	= $(MAINSRCS.c) $(ODDSRCS.c)

MAINSRCS.c	= main.c symbtab.c hash.c lex.c eval.c ghof.c ghofenc.c float.c

ASMSRCS.c	= $(MAINSRCS.c) $(TARGETSRCS.c)

# must include target sources for all possible processors
ODDSRCS.c	= binhdr.c itoabin.c alloca.c \
		lex_C40.c pass2_C40.c \
		lex_ARM.c pass2_ARM.c \
		lex_M68K.c pass2_M68K.c

TARGETSRCS.c	= lex_$(TARGET).c pass2_$(TARGET).c

# AMPP source files
SOURCES.a	=

# C header files
HEADERS.c	= gasm.h warn.h ghof.h \
		hdr_ARM.h \
		hdr_C40.h \
		hdr_M68K.h

# AMPP macro files
HEADERS.m	=

# Any other source files
SOURCES.odd	= makefile gasm.doc cpu_port.doc testpseudo.s gasm.ypp \
		toks_C40.ypp rules_C40.ypp testc40.s c40asm.doc \
		toks_ARM.ypp rules_ARM.ypp testarm.s armasm.doc \
		toks_M68K.ypp rules_M68K.ypp m68kasm.doc


#-----------------------------------------------------------------------------
# Other generic component variables:

TARGET = $(HPROC)#		# target cpu name used in filenames and ifdefs

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
 YACC := $(YACC) -v -d #-l -t
endif

ifdef BISON
 ALLOCA = alloca.o
endif

DEFINES = -D__$(TARGET)TARGET $(BISON) \
	$(CPU_HEADER) $(CPU_YYTOKENS) $(CPU_YYRULES)

# Also available: -DDEBUG, -ZC = signed chars as default
NCFLAGS := $(NCFLAGS) $(DEFINES) -I. #-ZC

ifeq ($(HHOST),R140)
 HOSTCFLAGS := $(HOSTCFLAGS) -ansi
else
 ifeq ($(HHOST),HP)
  HOSTCFLAGS := $(HOSTCFLAGS) -Aa
 endif
endif

# run C pre-processor (CPP), dont remove comments and dont add #line directives
THECPP = $(CPP) $(DEFINES)	# defined in /hsrc/makeinc/$(HHOST).mak

ifeq ($(HHOST),SUN4) # ensure that the Helios/ARM include files are used
 THECPP := $(THECPP) -P -I$(HSRC)/$(COMPONENT)/$(HPROC) -I$(HSRC)/$(COMPONENT) -I$(INCLUDE)#
endif

#-----------------------------------------------------------------------------
# Generic targets and their dependencies:

# Automatically generate binary constants header file
binhdr:	binhdr.o
	$(LINK) $(LINKFLAGS) -o$@ -n$@ $(CSTART) $^

../binary.h : binhdr
	../$(HHOST)/binhdr > ../binary.h

gasm.y:	gasm.ypp toks_$(TARGET).ypp rules_$(TARGET).ypp
	$(THECPP) $< > $@

y.tab.c y.tab.h: gasm.y
	$(YACC) $<

lex_$(TARGET).o lex.o ghof.o eval.o: y.tab.h

$(ASMOBJS) gasm.y: gasm.h hdr_$(TARGET).h ../binary.h ghof.h

as:	$(CSTART) $(ASMOBJS) y.tab.o itoabin.o $(ALLOCA)
	$(LINK) -o$@ $(LDFLAGS) -s20000 $^


#-----------------------------------------------------------------------------
endif #InProcMake	# This line must be at EOF
