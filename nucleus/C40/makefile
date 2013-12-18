#------------------------------------------------------------------------------
# Helios generic make system
#-----------------------------------------------------------------------------
# C40 processor specific make rules for the nucleus component.
#
# File: /HSRC/nucleus/C40/makefile
#
# This file contains the extra definitions of variables and rules that are
# required to make a processor specific version of this component.
#
# If you wish to overide a targets rule that has been defined in the generic
# part of the makefile, you should bracket it with "ifneq ($(HPROC),YYY)" and
# supply the new rule in this makefile.
#
# This makefile is always executed within this processor specific subdir.
# All processor specific source files should be kept here.
# All intermediate, object and binary files should be made in here.
#
# SccsId: %W% %G%
# RcsId: $Id: makefile,v 1.2 1992/04/21 17:47:16 paul Exp $
#		(C) Copyright 1990 Perihelion Software
#
# WARNING: assumes you are using GNUmake.
#-----------------------------------------------------------------------------

InProcMake = TRUE

BOOT 		= $(KERNEL)/C40/c40boot.i

# Get generic component makefile
include ../makefile

#-----------------------------------------------------------------------------
# Add processor specific targets:
XTARGETS =

Xdefault: $(XTARGETS)


# Install the extra targets:
Xinstall: $(XTARGETS)
	$(TOUCH) Xinstall

# Remove any processor specific files other than targets and objects here:
Xclean:
#	-$(RM) xxx


#-----------------------------------------------------------------------------
# Add to lists of files, used for archive, backup and cleaning purposes.

# All extra processor specific sources
XSOURCES	= $(XSOURCES.c) $(XSOURCES.a) $(XSOURCES.s) \
		  $(XHEADERS.c) $(XHEADERS.m) \
		  $(XSOURCES.odd)


XOBJECTS	=


# Fill in processor specific sources here:
XSOURCES.c	=

XSOURCES.a	=

XSOURCES.s	=

XHEADERS.c	=

XHEADERS.m	=

XSOURCES.odd	= makefile


#-----------------------------------------------------------------------------
# Any additional processor specific rules and generic rule overides go here:
