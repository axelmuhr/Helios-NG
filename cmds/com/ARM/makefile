#------------------------------------------------------------------------------
# Helios generic make system - PROCESSOR SPECIFIC COMPONENT MAKEFILE -
#-----------------------------------------------------------------------------
# ARM processor specific make rules for the General Commands component.
#
# File: /HSRC/cmds/com/ARM/makefile
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
# RcsId: $Id: makefile,v 1.18 1993/12/09 23:38:28 nickc Exp $
#		(C) Copyright 1990 Perihelion Software
#
# WARNING: assumes you are using GNUmake.
#-----------------------------------------------------------------------------

InProcMake = TRUE

# Get generic component makefile
include ../makefile

#-----------------------------------------------------------------------------
# Add processor specific targets:

FP_TARGETS = whetstone testFPoverflow testFP testFPdivzero enquire43

ifeq ($(HLICENSEE),ABC)
XTARGETS = showmi sigtest testexecerr \
	sleeper whatpri testport testlowalloc testrealloc mex mwr \
	flashwrite testrmm tst resetCPU testdivzero \
	shorttest forcegc alarmtst sysstrip mltest bash bashverify
else
XTARGETS = sigtest testexecerr \
	sleeper whatpri testport testlowalloc mex mwr \
	testdivzero shorttest alarmtst pidpanic
endif


Xdefault: $(XTARGETS)


# Install the extra targets:
Xinstall: $(XTARGETS)
	$(CP) $(XTARGETS) $(BIN)/private
	$(TOUCH) Xinstall

# Remove any processor specific files other than targets and objects here:
Xclean:
	-$(RM) Xinstall


#-----------------------------------------------------------------------------
# Add to lists of files, used for archive, backup and cleaning purposes.

# All extra processor specific sources
XSOURCES	= $(XSOURCES.c) $(XSOURCES.a) $(XSOURCES.s) \
		   $(XHEADERS.c) $(XHEADERS.m) \
		   $(XSOURCES.odd)

XOBJECTS	= $(XROBJS) $(XSOBJS)

XROBJS 		= $(XRSRCS.c:.c=.o)
XSOBJS		= $(XSSRCS.c:.c=.o)
XRBINS		= $(XROBJS:.o=)

XSOURCES.c	= $(XRSRCS.c) $(XSSRCS.c)

XRSRCS.c	= showroot.c sigtest.c testexecerr.c sleeper.c whatpri.c \
		testport.c testlowalloc.c testrealloc.c mex.c mwr.c \
		testrmm.c tst.c shorttest.c showmi.c forcegc.c alarmtst.c \
		sysstrip.c

XSSRCS.c	= whetstone.c testFPoverflow.c testFP.c flash.c showprocs.c \
		resetCPU.c testFPdivzero.c testdivzero.c enquire43.c mltest.c \
		bash.c bashverify.c bashcommon.c pidpanic.c

XSOURCES.a	= pidapanic.a

XSOURCES.s	=

XHEADERS.c	= bash.h

XHEADERS.m	=

XSOURCES.odd	= makefile


#-----------------------------------------------------------------------------
# Any additional processor specific rules and generic rule overides go here:

$(XROBJS):

$(XRBINS): $(CSTART)

#-----------------------------------------------------------------------
# Private specials

tps: $(CSTART) tps.o
	$(LINK) $(LINKFLAGS) -o$@ -n$@ -s4000 -h2000 $^

pidpanic: $(CSTART) pidapanic.o pidpanic.o
	$(LINK) $(LINKFLAGS) -o$@ -n$@ -s4000 -h2000 $^

testFP: testFP.o $(CSTART)
	$(LINK) $(LINKFLAGS) -o$@ -n$@ -s4000 -h2000 \
		$(CSTART) $(LIB)/fplib.def $<

testFPoverflow: testFPoverflow.o $(CSTART)
	$(LINK) $(LINKFLAGS) -o$@ -n$@ -s4000 -h2000 \
		$(CSTART) $(LIB)/fplib.def $<

testFPdivzero: testFPdivzero.o $(CSTART)
	$(LINK) $(LINKFLAGS) -o$@ -n$@ -s4000 -h2000 \
		$(CSTART) $(LIB)/fplib.def $<

testdivzero: testdivzero.o $(CSTART)
	$(LINK) $(LINKFLAGS) -o$@ -n$@ -s4000 -h2000 \
		$(CSTART) $<

whetstone: whetstone.o $(CSTART)
	$(LINK) $(LINKFLAGS) -o$@ -n$@ -s4000 -h2000 \
		$(CSTART) $(LIB)/fplib.def $<

resetCPU: resetCPU.o $(CSTART)
	$(LINK) $(LINKFLAGS) -o$@ -n$@ -s2000 -h2000 \
		$(CSTART) $(LIB)/abclib.def $<

# FlashEPROM update program (could probably drop stack more)
flashwrite: $(CSTART) $(LIB)/abclib.def flash.o
	$(LINK) $(LINKFLAGS) -o$@ -n$@ -s2000 -h1000 $^

mltest: $(CSTART) $(LIB)/abclib.def mltest.o
	$(LINK) $(LINKFLAGS) -o$@ -n$@ -s2000 -h1000 $^

# C Library and Compiler environment information display utility
enquire43: enquire43.o $(CSTART)
	$(LINK) $(LINKFLAGS) -o$@ -n$@ -s2000 -h2000 $(CSTART) $<

bash.c bashcomon.c bashverify.c: bash.h
bash.o: bash.c
ifeq ($(HPROC),ARM)
	$(NC) -c $(NCFLAGS) -D_BSD -I$(INCLUDE) $<
else
	$(CC) -c -D_BSD -I$(INCLUDE) $<
endif

bashcommon.o: bashcommon.c
ifeq ($(HPROC),ARM)
	$(NC) -c $(NCFLAGS) -D_BSD -I$(INCLUDE) $<
else
	$(CC) -c -D_BSD -I$(INCLUDE) $<
endif

bashverify.o: bashverify.c
ifeq ($(HPROC),ARM)
	$(NC) -c $(NCFLAGS) -D_BSD -I$(INCLUDE) $<
else
	$(CC) -c -D_BSD -I$(INCLUDE) $<
endif

# File system test programs
bash:	$(CSTART) bash.o bashcommon.o
	$(LINK) $(LINKFLAGS) -o$@ -n$@ -s2000 -h2000 $^ -l$(LIB)/bsd.lib

bashverify: $(CSTART) bashverify.o bashcommon.o
	$(LINK) $(LINKFLAGS) -o$@ -n$@ -s2000 -h2000 $^ -l$(LIB)/bsd.lib

C_CFLAGS := $(NCFLAGS) -DC_COMPILER=\"ncc\" \
	-DASSEMBLER=\"as\" \
	-DLINKER=\"ld\" \
	-DAMPP=\"ampp\" \
	-DREMOVER=\"rm\" \
	-DC_INCDIR=\"/helios/include/\" \
	-DC_LIBDIR=\"/helios/lib/\" \
	-DMACDIR=\"/helios/include/ampp/\" 

NCFLAGS := $(NCFLAGS)

c.o: c.c
	$(NC) $(NCFLAGS) $(C_CFLAGS) -c $<

c: c.o
	$(LINK) $(LINKFLAGS) -o$@ -n$@ -s2000 -h2000 $(CSTART) $<


#-----------------------------------------------------------------------------
