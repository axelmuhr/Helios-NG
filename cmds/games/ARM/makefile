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
# RcsId: $Id: makefile,v 1.14 1991/05/31 13:44:45 paul Exp $
#		(C) Copyright 1990 Perihelion Software
#
# WARNING: assumes you are using GNUmake.
#-----------------------------------------------------------------------------

InProcMake = TRUE

# Get generic component makefile
include ../makefile

#-----------------------------------------------------------------------------
# Add processor specific targets:
XTARGETS = whetstone showroot showmi sigtest testFPoverflow testFP testexecerr \
	sleeper whatpri testport showprocs testlowalloc testrealloc mex \
	flashwrite testrmm tst resetCPU testFPdivzero testdivzero enquire43 \
	shorttest forcegc alarmtst sysstrip mltest buildkeymap bash bashverify


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
		testport.c testlowalloc.c testrealloc.c mex.c \
		testrmm.c tst.c shorttest.c showmi.c forcegc.c alarmtst.c \
		sysstrip.c buildkeymap.c

XSSRCS.c	= whetstone.c testFPoverflow.c testFP.c flash.c showprocs.c \
		resetCPU.c testFPdivzero.c testdivzero.c enquire43.c mltest.c \
		bash.c bashverify.c bashcommon.c

XSOURCES.a	=

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

showprocs: $(CSTART) $(LIB)/abclib.def showprocs.o
	$(LINK) $(LINKFLAGS) -o$@ -n$@ -s2000 -h1000 $^

mltest: $(CSTART) $(LIB)/abclib.def mltest.o
	$(LINK) $(LINKFLAGS) -o$@ -n$@ -s2000 -h1000 $^

# C Library and Compiler environment information display utility
enquire43: enquire43.o $(CSTART)
	$(LINK) $(LINKFLAGS) -o$@ -n$@ -s2000 -h2000 $(CSTART) $<

bash.c bashcomon.c bashverify.c: bash.h
bash.o: bash.c
	$(CC) -c -D_BSD -I$(INCLUDE) $<

bashcommon.o: bashcommon.c
	$(CC) -c -D_BSD -I$(INCLUDE) $<

bashverify.o: bashverify.c
	$(CC) -c -D_BSD -I$(INCLUDE) $<

# File system test programs
bash:	$(CSTART) bash.o bashcommon.o
	$(LINK) $(LINKFLAGS) -o$@ -n$@ -s2000 -h2000 $^ -l$(LIB)/bsd.lib

bashverify: $(CSTART) bashverify.o bashcommon.o
	$(LINK) $(LINKFLAGS) -o$@ -n$@ -s2000 -h2000 $^ -l$(LIB)/bsd.lib

NCFLAGS := $(NCFLAGS) -DC_COMPILER=\"ncc\" \
	-DASSEMBLER=\"hobjasm\" \
	-DLINKER=\"armlink\" \
	-DAMPP=\"ampp\" \
	-DREMOVER=\"rm\" \
	-DC_INCDIR=\"/helios/include/\" \
	-DC_LIBDIR=\"/helios/lib/\" \
	-DMACDIR=\"/helios/include/ampp/\" 

c: c.o
	$(LINK) $(LINKFLAGS) -o$@ -n$@ -s2000 -h2000 $(CSTART) $<


#-----------------------------------------------------------------------------
