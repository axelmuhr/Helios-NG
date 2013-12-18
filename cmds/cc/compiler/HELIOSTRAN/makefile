#-----------------------------------------------------------------------------
# Helios generic make system - GENERIC COMPONENT MAKEFILE -
#-----------------------------------------------------------------------------
# Helios generic component make rules for: cc
#
# File: /HSRC/cmds/cc/compiler/makefile
#
# These are the host/processor independent rules and variables for this
# component of Helios. It is included by the processor specific makefile
# That may add to some of its functionality. You should only alter this
# makefile by using "ifneq ($(HPROC),YYY)" clauses for a specific processor.
#
# SccsId: %W% %G%
# RcsId: $Id: makefile,v 1.2 1991/03/20 15:05:10 martyn Exp $
#		(C) Copyright 1990 Perihelion Software
#
# WARNING: assumes you are using GNUmake.
#-----------------------------------------------------------------------------
#-----------------------------------------------------------------------------
# This part of the makefile is always run in a processor specific subdirectory.

# Manditory variables for a component:
# name of this dir relative to $(HSRC)
COMPONENT = cmds/cc/compiler

# Manditory includes
include $(HSRC)/makeinc/$(HHOST).mak	# Default host system variables
include $(HSRC)/makeinc/$(HPROC).mak	# Default processor rules and vars.

# Manditory phoney targets
.PHONY: default clean install
# "default"    - should make this component in its proc subdir
# "install"    - copy the new component to $(HPROD) production directory
# "clean"      - remove all intermediate and executable files from proc subdir
# Phoney targets prefixed by X are for adding processor specific targets.
#
# Processor specific sources are held in the processor specific subdir.,
# generic ones in the directory above it.
VPATH := $(HSRC)/$(COMPONENT)

ifdef RSRC
 VPATH := $(RSRC)/$(COMPONENT)/$(HPROC):$(VPATH):$(RSRC)/$(COMPONENT)
endif

#-----------------------------------------------------------------------------
# Generic component make rules and variables added below here

# Note all final generic targets here
TARGETS = nc

default : $(TARGETS)

install	: $(TARGETS)
	$(CP) nc $(HHOSTBIN)/cc
	$(TOUCH) install

clean	: Xclean
	-$(RM)	$(OBJECTS) $(TARGETS) mkout \
		core ../mkout install Xinstall


#-----------------------------------------------------------------------------
# for archive, backup and cleaning purposes,
# note all generic SOURCE and OBJECT files:

# Note all sources
SOURCES		= $(SOURCES.c) $(SOURCES.a) \
		  $(HEADERS.c) $(HEADERS.m) \
		  $(SOURCES.odd)

# Or if you are not a library:
OBJECTS	= $(SOURCES.c:.c=.o) $(SOURCES.a:.a=.o)

# C source files
SOURCES.c	= bind.c builtin.c driver.c stubs.c pp.c preparse.c \
		  debug.c lex.c misc.c IEEEflt.c cg.c cg1.c cg2.c emit.c \
		  sem.c syn.c synprint.c vargen.c showcode.c optimise.c

# AMPP source files
SOURCES.a	=

# C header files
HEADERS.c	= AEops.h cchdr.h cg.h modes.h system.h util.h xpuops.h \
		  xpuspec.h xrefs.h

# AMPP macro files
HEADERS.m	=

# Any other source files
SOURCES.odd	= makefile


#-----------------------------------------------------------------------------
# Other generic component variables:

NCFLAGS := $(NCFLAGS) -DTARGET_IS_XPUTER -DOFFSETOF_FIX -DDBX -DNEWCAST -DCOMPILING_ON_XPUTER

LINKFLAGS := $(LINKFLAGS)
#-----------------------------------------------------------------------------
# Generic targets and their dependencies:

%.o: %.c
	cc -wadsv -i,../,../../support/,/helios/include/ -j,../,../../support/,/helios/include/ $(NCFLAGS) $< -s$(TEMP)/$*.s
	$(ASM) -p -o $@ $(TEMP)/$*.s
	-$(RMTEMP) $(TEMP)/$*.s

nc:  $(OBJECTS)
	$(LINK) -v -f $(LINKFLAGS) -s20000 -o $@ /helios/lib/c0.o $(OBJECTS) \
		-l/helios/lib/helios.lib -l/helios/lib/c.lib -ncc
