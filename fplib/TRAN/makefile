#------------------------------------------------------------------------------
# Helios generic make system
#-----------------------------------------------------------------------------
# TRANSPUTER processor specific make rules for the FpLib component.
#
# File: /HSRC/fplib/TRAN/makefile
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
# RcsId: $Id: makefile,v 1.6 1990/11/16 16:26:03 nick Exp $
#		(C) Copyright 1990 Perihelion Software
#
# WARNING: assumes you are using GNUmake.
#-----------------------------------------------------------------------------

InProcMake = TRUE

# Get generic component makefile
include ../makefile

#-----------------------------------------------------------------------------
# Add processor specific targets:
XTARGETS = fplib.t8 fplib.t4 fpclib.i fpclib.def

Xdefault: $(XTARGETS)


# Install the extra targets:
Xinstall: $(XTARGETS)
	$(CP) fplib.t8 $(LIB)/FpLib.t8
	$(CP) fplib.t4 $(LIB)/FpLib.t4
	$(CP) fpclib.i $(LIB)/fpclib
	$(CP) fpclib.def $(LIB)
	$(TOUCH) Xinstall

Xinstalldef: $(LIB)/fpclib.def
	$(TOUCH) Xinstalldef

$(LIB)/fpclib.def : fpclib.def
ifeq (HELIOS,$(findstring HELIOS,$(HHOST))) #csh default on Helios hosts
	(cmp $^ $@ ; if ( $$status ) cp $^ $@ )
else
	if cmp $^ $@; then echo "$^ not changed"; else $(CP) $^ $@ ;fi
endif

# Remove any processor specific files other than targets and objects here:
Xclean:
	-$(RM) *.p8 *.p4 *.x4 *.x8 *.o 


#-----------------------------------------------------------------------------
# Add to lists of files, used for archive, backup and cleaning purposes.

# All extra processor specific sources
XSOURCES	= $(XSOURCES.c) $(XSOURCES.a) \
		  $(XHEADERS.c) $(XHEADERS.m) \
		  $(XSOURCES.odd)


#Full set of library object files
XOBJECTS	= $(XSOURCES.c:.c=.p) $(XSOURCES.a:.a=.p)
# Or if you are not a library:
#XOBJECTS	= $(XSOURCES.c:.c=.o) $(XSOURCES.a:.a=.o)


# Fill in processor specific sources here:
XSOURCES.c	= fpcstubs.c

XSOURCES.a	= t8sim.a t4sim.a fpcstart.a

XHEADERS.c	=

XHEADERS.m	=

XSOURCES.odd	= makefile


#-----------------------------------------------------------------------------
# Any additional processor specific rules and generic rule overides go here:

.SUFFIXES: .p4 .p8

%.p4: %.c
	$(NC) -l -t4 $(NCFLAGS) $(DEFINES) $< -s$(TEMP)/$*
	$(ASM) -p $(TEMP)/$* -o$*.p4
	$(RMTEMP) $(TEMP)/$*

%.p8: %.c
	$(NC) -l -t8 $(NCFLAGS) $(DEFINES) $< -s$(TEMP)/$*
	$(ASM) -p $(TEMP)/$* -o$*.p8
	$(RMTEMP) $(TEMP)/$*


#------------------------------------------------------------------------------
# Language Independant FP library

FPLIBDEFS = $(LIB)/kernel.def $(LIB)/syslib.def $(LIB)/posix.def \
	$(LIB)/util.def

# T4 version
FP4OBJS = fpstart.p t4sim.p math.p4 modend.p

fplib.t4 : $(FP4OBJS) $(FPLIBDEFS)
	$(LINK.LIBRARY) -nFpLib $(FP4OBJS) $(FPLIBDEFS) -ofplib.t4


# T8 version
FP8OBJS = fpstart.p t8sim.p math.p8 modend.p

fplib.t8 : $(FP8OBJS) 
	$(LINK.LIBRARY) -nFpLib $(FP8OBJS) $(FPLIBDEFS) -ofplib.t8


#------------------------------------------------------------------------------
# C language FP library
# This is always compiled for T4, what little floating point maths it does
# is always emulated.

FPCLIBDEFS = $(LIB)/kernel.def $(LIB)/syslib.def $(LIB)/util.def \
	     $(LIB)/clib.def $(LIB)/fplib.def

FPCOBJS = fpcstart.p fpcstubs.p4 modend.p

fpclib.i : $(FPCOBJS) $(FPCLIBDEFS) 
	$(ASM) -nFpClib $^ -ofpclib.i

fpclib.def: fpcstart.a
	$(MAKE.DEF)

