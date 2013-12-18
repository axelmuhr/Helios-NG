#------------------------------------------------------------------------------
# Helios generic make system - NATIVE HOST MAKEFILE -
#-----------------------------------------------------------------------------
# RS6000 Host specific make rules to make NATIVE HOST version of Berkeley
# public domain YACC.
#
# File: /HSRC/cmds/public/yacc-1.4/RS6000/makefile
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
# RcsId: $Id: makefile,v 1.1 1993/05/21 16:38:10 paul Exp $
#		(C) Copyright 1990 Perihelion Software
#
# WARNING: assumes you are using GNUmake.
#-----------------------------------------------------------------------------
# Manditory variables for a component:
# name of COMPONENT directory relative to $(HSRC)
COMPONENT = cmds/public/yacc-1.4

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

TARGETS = byacc

default : $(TARGETS)

install	: $(TARGETS)
	$(CP) $(TARGETS) $(HHOSTBIN)
	$(TOUCH) install

clean	:
	-$(RM)	$(OBJECTS) $(TARGETS) mkout core ../mkout install

# Note all sources
SOURCES		= $(SOURCES.c) $(HEADERS.c) $(SOURCES.odd)

# Object files
OBJECTS		= $(SOURCES.c:.c=.o)

# C source files
SOURCES.c	=  closure.c error.c lalr.c lr0.c main.c mkpar.c output.c \
		reader.c skeleton.c symtab.c verbose.c warshall.c

# AMPP source files
SOURCES.a	= 

# C header files
HEADERS.c	= defs.h

# AMPP macro files
HEADERS.m	=

# Any other source files
SOURCES.odd	= makefile Makefile.unix MANIFEST ACKNOWLEDFEMEN NEW_FEATURES \
		NO_WARRANTY README manpage ftp.y


#-----------------------------------------------------------------------------
# Native Host targets and their dependencies:


$(SOURCES.c): $(HEADERS.c)

byacc: $(SOURCES.c)
	$(HOSTCC) $(HOSTCFLAGS) -o $@ $^


#----------------------------------------------------------------------------


