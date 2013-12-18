#------------------------------------------------------------------------------
# Helios generic make system  - NATIVE HOST MAKEFILE
#-----------------------------------------------------------------------------
# SUN4 Host specific make rules to make NATIVE HOST version of c component.
#
# File: /HSRC/cmds/private/HELIOSTRAN/makefile
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
# RcsId: $Id: makefile,v 1.1 1991/12/06 12:24:44 martyn Exp $
#		(C) Copyright 1990 Perihelion Software
#
# WARNING: assumes you are using GNUmake.
#-----------------------------------------------------------------------------
# Manditory variables for a component:
# name of COMPONENT directory relative to $(HSRC)
COMPONENT = cmds/private

# Manditory includes
include $(HSRC)/makeinc/$(HHOST).mak	# Default host system variables

# Manditory phoney targets
.PHONEY: default clean
# "default" - should make this component in its proc subdir
# "install" - copy the new component to $(HHOSTBIN) directory
# "clean"   - remove all intermediate and executable files from host subdir

VPATH := $(HSRC)/$(COMPONENT)

#-----------------------------------------------------------------------------
# Note all final targets here

TARGETS =

default : $(TARGETS)

install	: $(TARGETS)
	$(TOUCH) install

clean	:
	-$(RM)	$(OBJECTS) $(TARGETS) mkout core ../mkout install


#-----------------------------------------------------------------------------
# Source and object files:

# All Object files
OBJECTS		= $(SOURCES.c:.c=.o)

# C source files
SOURCES.c	=

# AMPP source files
SOURCES.a	= 

# C header files
HEADERS.c	=

# AMPP macro files
HEADERS.m	=

# Any other source files
SOURCES.odd	= makefile

#-----------------------------------------------------------------------------
# Other generic component variables:

# Set any flags required

%.o: %.c
	$(HOSTBIN)/cc -wadsv -i,../,../../support/,/helios/include/ -j,../,../../support/,/helios/include/ $(NCFLAGS) $< -s$(TEMP)/$*.s
	$(ASM) -p -o $@ $(TEMP)/$*.s
	-$(RMTEMP) $(TEMP)/$*.s
