#------------------------------------------------------------------------------
# Helios generic make system  - NATIVE HOST MAKEFILE
#-----------------------------------------------------------------------------
# SUN4 Host specific make rules to make NATIVE HOST version of AMPP component.
#
# File: /HSRC/cmds/ampp/SUN4/makefile
#
# This file contains ALL the definitions and rules that are
# required to make a NATIVE HOST version of this component. i.e. It is not
# complimentary to the generic version.
#
# This makefile is always executed within this host specific subdir.
# All host specific source files should be kept here.
# All intermediate, object and binary files should be made in here.
#
# SccsId: %W% %G%
# RcsId: $Id: makefile,v 1.3 91/03/05 13:15:56 martyn Exp $
#		(C) Copyright 1990 Perihelion Software
#
# WARNING: assumes you are using GNUmake.
#-----------------------------------------------------------------------------
# Manditory variables for a component:
# name of COMPONENT directory relative to $(HSRC)
COMPONENT = cmds/ampp

# Manditory includes
include $(HSRC)/makeinc/$(HHOST).mak	# Default host system variables

# Manditory phoney targets
.PHONEY: default clean
# "default" - should make this component in its proc subdir
# "install" - copy the new component to $(HHOSTBIN) directory
# "clean"   - remove all intermediate and executable files from host subdir

VPATH := $(HSRC)/$(COMPONENT):../../support

#-----------------------------------------------------------------------------
# Note all final targets here

TARGETS = ampp

default : $(TARGETS)

install	: $(TARGETS)
	$(CP) ampp $(HHOSTBIN)
	$(TOUCH) install

clean	:
	-$(RM)	$(OBJECTS) $(TARGETS) mkout core ../mkout install


#-----------------------------------------------------------------------------
# Source and object files:

# All Object files
OBJECTS		= $(SOURCES.c:.c=.o)

# C source files
SOURCES.c	= ampp.c macro.c lex.c sym.c builtin.c defs.c \
		putback.c charstack.c output.c queue.c

# AMPP source files
SOURCES.a	= t.a			# test file

# C header files
HEADERS.c	= ampp.h

# AMPP macro files
HEADERS.m	=

# Any other source files
SOURCES.odd	= makefile

#-----------------------------------------------------------------------------

.SUFFIXES: .o .c

%.o: %.c
	$(HOSTCC) -w -c -I.. -I../../support -D$(HHOST) -DSUN4 $<

$(OBJECTS): ampp.h

ampp:	$(OBJECTS) 
	$(HOSTCC) -Bstatic -o ampp $^



#-----------------------------------------------------------------------------
