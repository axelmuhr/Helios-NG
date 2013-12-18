#------------------------------------------------------------------------------
# Helios generic make system
#-----------------------------------------------------------------------------
# TRAN processor specific make rules for the network software
#
# File: /HSRC/network/TRAN/makefile
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
# RcsId: $Id: makefile,v 1.9 1992/08/17 11:12:54 paul Exp $
#		(C) Copyright 1990 Perihelion Software
#
# WARNING: assumes you are using GNUmake.
#-----------------------------------------------------------------------------

InProcMake = TRUE

BOOT 		= $(KERNEL)/TRAN/nboot.i

# Get generic component makefile
include ../makefile

#-----------------------------------------------------------------------------
# Add processor specific targets:
XTARGETS = rboot pa_rboot tram_ra.d pa_ra.d \
	tr_reset pa_reset

Xdefault: $(XTARGETS)


# Install the extra targets:
Xinstall: $(XTARGETS)
	$(CP) rboot $(BIN)
	$(CP) pa_rboot $(BIN)
	$(CP) tram_ra.d $(LIB)
	$(CP) pa_ra.d $(LIB)
	$(CP) tr_reset $(BIN)
	$(CP) pa_reset $(BIN)
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
XOBJECTS	= $(XSOURCES.c:.c=.p) $(XSOURCES.a:.a=.p) \
		  $(XSOURCES.c:.c=.o) $(XSOURCES.a:.a=.o) \
		  pa_rboot.o


# Fill in processor specific sources here:
XSOURCES.c	= rboot.c pa_reset.c tr_reset.c tram_ra.c pa_ra.c 

XSOURCES.a	=

XHEADERS.c	=

XHEADERS.m	=

XSOURCES.odd	= makefile


#-----------------------------------------------------------------------------
# Any additional processor specific rules and generic rule overides go here:

#LIBS := -lsession.lib -lRmLib.def -lnetutils.lib -l$(LIB)/c.lib -l$(LIB)/helios.lib

%.p : %.c
	$(NC) -l $(NCFLAGS) $(DEFINES) $< -s$(TEMP)/$*.s
	$(ASM) $(ASMFLAGS) -o $*.p $(TEMP)/$*.s
	-$(RMTEMP) $(TEMP)/$*.s

#-----------------------------------------------------------------------------
# Transputer specific commands

rboot : rboot.c
	$(NC) $(NCFLAGS) $(DEFINES) -DHANDBOOT $< -s$(TEMP)/$@.s
	$(ASM) $(ASMFLAGS) -o $@.o $(TEMP)/$@.s
	-$(RMTEMP) $(TEMP)/$@.s
	$(LINK) $(LINKFLAGS) -o $@ -n$@ -s1000 -h2000 $(SSTART) $@.o $(LIBS)

pa_rboot : rboot.c
	$(NC) $(NCFLAGS) $(DEFINES) -DHANDBOOT -DPARSYTEC \
		$< -s$(TEMP)/$@.s
	$(ASM) $(ASMFLAGS) -o $@.o $(TEMP)/$@.s
	-$(RMTEMP) $(TEMP)/$@.s
	$(LINK) $(LINKFLAGS) -o $@ -n$@ -s1000 -h2000 $(SSTART) $@.o $(LIBS)

tr_reset : $(SSTART) tr_reset.o
	$(LINK) $(LINKFLAGS) -o $@ -n$@ -s1000 -h2000 $^ $(LIBS)

pa_reset : $(SSTART) pa_reset.o
	$(LINK) $(LINKFLAGS) -o $@ -n$@ -s1000 -h2000 $^ $(LIBS)

#-----------------------------------------------------------------------------
# device drivers
tram_ra.d : driver.p tram_ra.p modend.p $(LIBDEFS) RmLib.def
	$(LINK.LIBRARY) -o $@ $^

pa_ra.d : driver.p pa_ra.p modend.p $(LIBDEFS) RmLib.def
	$(LINK.LIBRARY) -o $@ $^

