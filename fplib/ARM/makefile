#------------------------------------------------------------------------------
# Helios generic make system
#-----------------------------------------------------------------------------
# ARM processor specific make rules for the fplib component.
#
# File: /HSRC/fplib/ARM/makefile
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
# RcsId: $Id: makefile,v 1.9 1994/01/20 09:41:48 nickc Exp $
#		(C) Copyright 1990 Perihelion Software
#
# WARNING: assumes you are using GNUmake.
#-----------------------------------------------------------------------------

InProcMake = TRUE

# Get generic component makefile
include ../makefile

#-----------------------------------------------------------------------------
# Add processor specific targets:
XTARGETS = fplib.i

Xdefault: $(XTARGETS)


# Install the extra targets:
Xinstall: $(XTARGETS)
ifeq ($(HPLATFORM),ABC)
	sysstrip fplib.i -o$(LIB)/FpLib
else
	$(CP) fplib.i $(LIB)/FpLib
endif
	$(TOUCH) Xinstall

# do nothing!
Xinstalldef:

# Remove any processor specific files other than targets and objects here:
Xclean:
#	-$(RM)


#-----------------------------------------------------------------------------
# Add to lists of files, used for archive, backup and cleaning purposes.

# All extra processor specific sources
XSOURCES	= $(XSOURCES.c) $(XSOURCES.a) $(XSOURCES.s) \
		  $(XHEADERS.c) $(XHEADERS.m) \
		  $(XSOURCES.odd)


#Full set of library object files
#XOBJECTS	= $(XSOURCES.c:.c=.p) $(XSOURCES.a:.a=.p)
# Or if you are not a library:
#XOBJECTS	= $(XSOURCES.c:.c=.o) $(XSOURCES.a:.a=.o)


# Fill in processor specific sources here:
XSOURCES.c	=

XSOURCES.a	=

XSOURCES.s	= fpdirect.s

XHEADERS.c	=

XHEADERS.m	=

XSOURCES.odd	= makefile


#-----------------------------------------------------------------------------
# Any additional processor specific rules and generic rule overides go here:

# Extra ARM Commands (these should probably be defined in the .mak files):
ASMHDRS	=	$(HINCLUDE)/listopts.s  $(HINCLUDE)/arm.s \
		$(HINCLUDE)/exmacros.s
ARMHAS = $(NONDIST)/bin/$(HHOST)/armasm

XOBJECTS = fpstart.p fpdirect.p math.p modend.p

fpdirect.p : fpdirect.s $(ASMHDRS)
	${ARMHAS} -I$(HINCLUDE) -li $< $@

fplib.i: $(XOBJECTS) $(LIBDEFS)
#	$(LINK.LIBRARY) -nFpLib $^ -o$@
# Note: We need to ensure that any AOF objects given to the linker are
#       converted properly.
	$(LINK.LIBRARY) -nFpLib -Alt $^ -o$@
