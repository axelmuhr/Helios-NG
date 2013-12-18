#------------------------------------------------------------------------------
# Helios generic make system - PROCESSOR SPECIFIC COMPONENT MAKEFILE -
#-----------------------------------------------------------------------------
# C40 processor specific make rules for private commands
#
# File: /HSRC/cmds/private/C40/makefile
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
# RcsId: $Id: makefile,v 1.6 1994/02/17 11:45:13 nickc Exp $
#		(C) Copyright 1990 Perihelion Software
#
# WARNING: assumes you are using GNUmake.
#-----------------------------------------------------------------------------

InProcMake = TRUE

# Get generic component makefile
include ../makefile

#-----------------------------------------------------------------------------
# Add processor specific targets:
XTARGETS = maxlat atst artst

Xdefault: $(XTARGETS)


# Install the extra targets:
Xinstall: $(XTARGETS)
	@ test -d $(BIN)/private || mkdir -p $(BIN)/private
	$(CP) $(XTARGETS) $(BIN)/private
	$(TOUCH) Xinstall


# Remove any processor specific files other than targets and objects here:
Xclean:
	-$(RM) $(XTARGETS) $(XOBJECTS)


#-----------------------------------------------------------------------------
# Add to lists of files, used for archive, backup and cleaning purposes.

# All extra processor specific sources
XSOURCES	= $(XSOURCES.c) $(XSOURCES.a) \
		  $(XHEADERS.c) $(XHEADERS.m) \
		  $(XSOURCES.odd)

XOBJECTS	= $(XSOURCES.c:.c=.o) $(XSOURCES.a:.a=.o)
XBINS		= $(XOBJECTS:.o=)


# Fill in processor specific sources here:
XSOURCES.c	= maxlat.c atst.c artst.c

XSOURCES.a	=

XHEADERS.c	=

XHEADERS.m	=

XSOURCES.odd	= makefile


#-----------------------------------------------------------------------------
# Any additional processor specific rules and generic rule overides go here:

$(XOBJECTS):

$(XBINS): $(CSTART)

NCFLAGS := $(NCFLAGS) -Zpz1 -Zpw1
