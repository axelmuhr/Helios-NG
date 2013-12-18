# *BEWARE*: Never tested - probably will never even be used!
#------------------------------------------------------------------------------
# Helios generic make system - NATIVE HOST MAKEFILE -
#-----------------------------------------------------------------------------
# YYY Host specific make rules to make NATIVE HOST version of XXX component.
#
# File: /HSRC/XXX/YYY/makefile
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
# RcsId: $Id: makefile,v 1.4 91/03/07 19:29:01 paul Exp $
#		(C) Copyright 1990 Perihelion Software
#
# WARNING: assumes you are using GNUmake.
#-----------------------------------------------------------------------------
# Manditory variables for a component:
# name of COMPONENT directory relative to $(HSRC)
COMPONENT = cmds/cc/compiler

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

TARGETS = nc

default : $(TARGETS)

install	: $(TARGETS)
	$(CP) nc $(HHOSTBIN)/nc
	$(TOUCH) install
clean	:
	-$(RM)	$(OBJECTS) $(TARGETS) mkout core ../mkout install

# Note all sources
SOURCES		= $(SOURCES.c) $(HEADERS.c) $(SOURCES.odd)

# Object files
OBJECTS		= $(SOURCES.c:.c=.o)

# C source files
SOURCES.c	= bind.c builtin.c driver.c stubs.c pp.c preparse.c \
		  debug.c lex.c misc.c IEEEflt.c cg.c cg1.c cg2.c emit.c \
		  sem.c syn.c synprint.c vargen.c showcode.c optimise.c

# C header files
HEADERS.c	= AEops.h cchdr.h cg.h modes.h system.h util.h xpuops.h \
		  xpuspec.h xrefs.h

# Any other source files
SOURCES.odd	= makefile


#-----------------------------------------------------------------------------
# Native Host targets and their dependencies:

# uncomment if you have compiler or makedepend generated dependencies
#include depends

DEFINES := $(DEFINES) -DTARGET_IS_XPUTER -DOFFSETOF_FIX -DDBX -DNEWCAST
DEFINES := $(DEFINES) -DCOMPILING_ON_R140

HOSTCFLAGS := -w -I. -I.. -Iinclude $(DEFINES)

nc : $(OBJECTS)
	cc -o nc $(OBJECTS)



#-----------------------------------------------------------------------------
