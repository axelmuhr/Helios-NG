#------------------------------------------------------------------------------
# Helios generic make system
#-----------------------------------------------------------------------------
# TRAN/telmat processor specific make rules for the network software
#
# File: /HSRC/network/telmat/TRAN/makefile
#
# This file contains the extra definitions of variables and rules that are
# required to make the Telmat specific thingies
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
# RcsId: $Id: makefile,v 1.4 1992/08/17 11:14:15 paul Exp $
#		(C) Copyright 1990 Perihelion Software
#
# WARNING: assumes you are using GNUmake.
#-----------------------------------------------------------------------------

InProcMake = TRUE

# Get generic component makefile
include ../makefile

#-----------------------------------------------------------------------------
# Add processor specific targets:
XTARGETS = telmat_r.d telmat_c.d 

Xdefault: $(XTARGETS)

# Install the extra targets:
Xinstall: $(XTARGETS)
	$(CP) telmat_r.d $(LIB)
	$(CP) telmat_c.d $(LIB)
	$(TOUCH) Xinstall


# Remove any processor specific files other than targets and objects here:
Xclean:
#	-$(RM) xxx


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
XSOURCES.c	= client.c configur.c listes.c special.c telmat_c.c telmat_r.c

XSOURCES.a	=

XHEADERS.c	= cbcom.h client.h special.h tkswdefs.h

XHEADERS.m	=

XSOURCES.odd	= makefile


#-----------------------------------------------------------------------------
# Any additional processor specific rules and generic rule overides go here:

LIBS    := -l$(HSRC)/network/$(HPROC)/RmLib.def -l$(LIB)/c.lib -l$(LIB)/helios.lib
LIBDEFS := $(LIB)/kernel.def $(LIB)/syslib.def $(LIB)/util.def \
	   $(LIB)/posix.def $(HSRC)/network/$(HPROC)/RmLib.def

DEFINES := $(DEFINES) -DANSI -DHELIOS
# BLV - if extra stubs are needed, please make a copy of the network software's
# driver.a and put it into the Telmat directory
DRIVER  := $(HSRC)/network/$(HPROC)/driver.p
MODEND  := $(HSRC)/network/$(HPROC)/modend.p

%.p : %.c
	$(NC) -l $(NCFLAGS) $(DEFINES) $< -s$(TEMP)/$*.s
	$(ASM) $(ASMFLAGS) -o $*.p $(TEMP)/$*.s
	-$(RMTEMP) $(TEMP)/$*.s

#-----------------------------------------------------------------------------
# Telmat specific commands


#-----------------------------------------------------------------------------
# device drivers

telmat_r.d : $(DRIVER) telmat_r.p special.p client.p $(MODEND) \
		 $(LIBDEFS)
	$(LINK.LIBRARY) -o $@ $^

telmat_c.d :  $(DRIVER) telmat_c.p special.p client.p configur.p listes.p \
	      $(MODEND) $(LIBDEFS)
	$(LINK.LIBRARY) -o $@ $^


