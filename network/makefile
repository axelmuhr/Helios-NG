#-----------------------------------------------------------------------------
# Helios generic make system
#-----------------------------------------------------------------------------
# Helios generic component make rules for: network
#
# File: /HSRC/network/makefile
#
# These are the host/processor independent rules and variables for this
# component of Helios. It is included by the processor specific makefile
# That may add to some of its functionality. You should only alter this
# makefile by using "ifneq ($(HPROC),YYY)" clauses for a specific processor.
#
# SccsId: %W% %G%
# RcsId: $Id: makefile,v 1.44 1994/06/07 12:37:29 nickc Exp $
#		(C) Copyright 1990 Perihelion Software
#
# WARNING: assumes you are using GNUmake.
#-----------------------------------------------------------------------------

ifndef InProcMake
# This part of the makefile is run in the components generic source directory

  # These variables should already be set by your make script or environment.
  ifndef HPROC
    HPROC = TRAN
  endif

  ifndef HHOST
    HHOST = SUN4
  endif

  ifndef HSRC
    HSRC = /usr/perihelion/Helios
  endif

  ifndef HPROD
    HPROD = $(HSRC)/slashhelios/$(HPROC)
  endif

.PHONY: notarget
notarget:
	@$(MAKE) -C $(HPROC)

.DEFAULT:
	@$(MAKE) -C $(HPROC) $@

else
#-----------------------------------------------------------------------------
# This part of the makefile is always run in a processor specific subdirectory.

# Manditory variables for a component:
# name of this dir relative to $(HSRC)
COMPONENT = network

# Manditory includes
include $(HSRC)/makeinc/$(HHOST).mak	# Default host system variables
include $(HSRC)/makeinc/$(HPROC).mak	# Default processor rules and vars.

# Manditory phoney targets
.PHONY: default clean Xdefault Xclean
# "default"    - should make this component in its proc subdir
# "install"    - copy the new component to $(HPROD) production directory
# "installdef" - If component is a library, make .def file and install in LIB
# "clean"      - remove all intermediate and executable files from proc subdir
# Phoney targets prefixed by X are for adding processor specific targets.
#
# Processor specific sources are held in the processor specific subdir.,
# generic ones in the directory above it.
HVPATH := $(HSRC)/$(COMPONENT)

ifdef RSRC
HVPATH := $(HSRC)/$(COMPONENT)/$(HPROC):$(RSRC)/$(COMPONENT)/$(HPROC):$(HSRC)/$(COMPONENT):/$(RSRC)/$(COMPONENT)
endif

vpath %.h $(HVPATH)
vpath %.a $(HVPATH)
vpath %.c $(HVPATH)

#-----------------------------------------------------------------------------
# The networking software can produce three different systems:
#    SS : single-user single-processor	(Tiny Helios)
#    MS : multi-user  single-processor	(ARM)
#    MM : multi-user  multi-processor   (The real system)
#

ifeq ($(HPROC),TRAN)
  SYSTEMS := SS MS MM
endif
ifeq ($(HPROC),ARM)
  SYSTEMS := SS
endif
ifeq ($(HPROC),C40)
  SYSTEMS := SS MS MM
endif
ifndef SYSTEMS
  Please define the target systems for your processor.
endif

#
# The different systems require different sets of targets, with some
# overlaps
#
GENERIC_TARGETS_LIB := RmLib RmLib.def netutils.lib session.lib
GENERIC_TARGETS_BIN := stopio rebootio clink logname tty id uptime \
		run passadm remote.nrm

SS_TARGETS_LIB := tfm.sp
SS_TARGETS_BIN := login.ss newuser.su ps.sp loaded.sp wsh.sp kill.sp \
		findtfm diag_tfm

MS_TARGETS_LIB := session.sp tfm.sp
MS_TARGETS_BIN := startns.sp login.sp newuser write wall users whoami \
		who sendto findsm ps.sp loaded.sp wsh.sp kill.sp \
		findtfm diag_tfm diag_sm

MM_TARGETS_LIB := netagent netserv session tfm null_ra.d
MM_TARGETS_BIN := startns startns.64 startns.any login newuser \
		write wall users whoami who sendto diag_ns findns findsm \
		domain ps loaded network elink plink dlink lstatus \
		rmgen wsh remote joinnet kill crash \
		findtfm diag_tfm diag_sm

#
# Determine the actual targets for the processor
#
TARGETS_LIB := 
TARGETS_BIN := 

ifneq (,$(findstring SS,$(SYSTEMS)))
 TARGETS_LIB := $(TARGETS_LIB) $(SS_TARGETS_LIB)
 TARGETS_BIN := $(TARGETS_BIN) $(SS_TARGETS_BIN)
endif
ifneq (,$(findstring MS,$(SYSTEMS)))
 TARGETS_LIB := $(TARGETS_LIB) $(MS_TARGETS_LIB)
 TARGETS_BIN := $(TARGETS_BIN) $(MS_TARGETS_BIN)
endif
ifneq (,$(findstring MM,$(SYSTEMS)))
 TARGETS_LIB := $(TARGETS_LIB) $(MM_TARGETS_LIB)
 TARGETS_BIN := $(TARGETS_BIN) $(MM_TARGETS_BIN)
endif

#
# Eliminate duplicates from the list. The generic targets have to be made
# first under all circumstances, because these include things like the
# various libraries
TARGETS_LIB := $(GENERIC_TARGETS_LIB) $(sort $(TARGETS_LIB))
TARGETS_BIN := $(GENERIC_TARGETS_BIN) $(sort $(TARGETS_BIN))

# 
# Check for the presence of OEM-specific sources, and for various useful
# little packages that have their own subdirectories
#
dirname := $(HPROC)

TELMAT_TEST   := ../telmat/$(dirname)
PARSYTEC_TEST := ../parsytec/$(dirname)
MEIKO_TEST    := ../meiko/$(dirname)
FARMLIB_TEST  := ../packages/farmlib/$(dirname)
HWTESTS1_TEST := ../packages/hwtests1/$(dirname)
RMTESTS_TEST  := ../packages/rmtests/$(dirname)
TFTESTS_TEST  := ../packages/tftests/$(dirname)

TELMAT        := $(strip $(wildcard $(TELMAT_TEST)))
PARSYTEC      := $(strip $(wildcard $(PARSYTEC_TEST)))
MEIKO         := $(strip $(wildcard $(MEIKO_TEST)))
FARMLIB       := $(strip $(wildcard $(FARMLIB_TEST)))
HWTESTS1      := $(strip $(wildcard $(HWTESTS1_TEST)))
RMTESTS	      := $(strip $(wildcard $(RMTESTS_TEST)))
TFTESTS       := $(strip $(wildcard $(TFTESTS_TEST)))

default : $(TARGETS_LIB) $(TARGETS_BIN) Xdefault
ifeq ($(TELMAT),$(TELMAT_TEST))
	$(MAKE) -C $(TELMAT)
endif
ifeq ($(PARSYTEC),$(PARSYTEC_TEST))
	$(MAKE) -C $(PARSYTEC)
endif
ifeq ($(MEIKO),$(MEIKO_TEST))
	$(MAKE) -C $(MEIKO)
endif
ifeq ($(FARMLIB),$(FARMLIB_TEST))
	$(MAKE) -C $(FARMLIB)
endif
ifeq ($(HWTESTS1),$(HWTESTS1_TEST))
	$(MAKE) -C $(HWTESTS1)
endif
ifeq ($(RMTESTS),$(RMTESTS_TEST))
	$(MAKE) -C $(RMTESTS)
endif
ifeq ($(TFTESTS),$(TFTESTS_TEST))
	$(MAKE) -C $(TFTESTS)
endif

install	: installdef $(TARGETS_LIB) $(TARGETS_BIN) Xinstall
	$(CP) $(TARGETS_BIN) $(BIN)
	$(CP) $(TARGETS_LIB) $(LIB)
ifeq ($(TELMAT),$(TELMAT_TEST))
		$(MAKE) -C $(TELMAT) install
endif
ifeq ($(PARSYTEC),$(PARSYTEC_TEST))
	$(MAKE) -C $(PARSYTEC) install
endif
ifeq ($(MEIKO),$(MEIKO_TEST))
	$(MAKE) -C $(MEIKO) install
endif
ifeq ($(FARMLIB),$(FARMLIB_TEST))
	$(MAKE) -C $(FARMLIB) install
endif
ifeq ($(HWTESTS1),$(HWTESTS1_TEST))
	$(MAKE) -C $(HWTESTS1) install
endif
ifeq ($(RMTESTS),$(RMTESTS_TEST))
	$(MAKE) -C $(RMTESTS) install
endif
ifeq ($(TFTESTS),$(TFTESTS_TEST))
	$(MAKE) -C $(TFTESTS) install
endif

ifeq ($(HPROC),ARM)		# special kludge as abcarm doesn't use "Files"
	mv $(LIB)/tfm.sp $(LIB)/tfm
	mv $(BIN)/loaded.sp $(BIN)/loaded
	mv $(BIN)/kill.sp $(BIN)/kill
	mv $(BIN)/ps.sp $(BIN)/ps
	mv $(BIN)/wsh.sp $(BIN)/wsh
	mv $(BIN)/login.ss $(BIN)/login
	mv $(BIN)/newuser.su $(BIN)/newuser
endif
	$(TOUCH) install

installdef: RmLib.def
ifeq ($(TELMAT),$(TELMAT_TEST))
	$(MAKE) -C $(TELMAT) installdef
endif
ifeq ($(PARSYTEC),$(PARSYTEC_TEST))
	$(MAKE) -C $(PARSYTEC) installdef
endif
ifeq ($(MEIKO),$(MEIKO_TEST))
	$(MAKE) -C $(MEIKO) installdef
endif
ifeq ($(FARMLIB),$(FARMLIB_TEST))
	$(MAKE) -C $(FARMLIB) installdef
endif
ifeq ($(HWTESTS1),$(HWTESTS1_TEST))
	$(MAKE) -C $(HWTESTS1) installdef
endif
ifeq ($(RMTESTS),$(RMTESTS_TEST))
	$(MAKE) -C $(RMTESTS) installdef
endif
ifeq ($(TFTESTS),$(TFTESTS_TEST))
	$(MAKE) -C $(TFTESTS) installdef
endif
	$(CP) RmLib.def $(LIB)
	$(TOUCH) installdef

clean	: Xclean
	-$(RM) $(OBJECTS)
	-$(RM) $(TARGETS_BIN)
	-$(RM) $(TARGETS_LIB)
	-$(RM) $(XOBJECTS)
	-$(RM) $(XTARGETS) 
	-$(RM) mkout core ../mkout install installdef Xinstall
ifeq ($(TELMAT),$(TELMAT_TEST))
	$(MAKE) -C $(TELMAT) clean
endif
ifeq ($(PARSYTEC),$(PARSYTEC_TEST))
	$(MAKE) -C $(PARSYTEC) clean
endif
ifeq ($(MEIKO),$(MEIKO_TEST))
	$(MAKE) -C $(MEIKO) clean
endif
ifeq ($(FARMLIB),$(FARMLIB_TEST))
	$(MAKE) -C $(FARMLIB) clean
endif
ifeq ($(HWTESTS1),$(HWTESTS1_TEST))
	$(MAKE) -C $(HWTESTS1) clean
endif
ifeq ($(RMTESTS),$(RMTESTS_TEST))
	$(MAKE) -C $(RMTESTS) clean
endif
ifeq ($(TFTESTS),$(TFTESTS_TEST))
	$(MAKE) -C $(TFTESTS) clean
endif


#-----------------------------------------------------------------------------
# for archive, backup and cleaning purposes,
# note all generic SOURCE and OBJECT files:

# Note all sources
SOURCES		= $(SOURCES.c) $(SOURCES.a) \
		   $(HEADERS.c) $(HEADERS.m) \
		   $(SOURCES.odd)

# Full set of object files - cheat a little - all possibles
# Some files are not generated using obvious sources
OBJECTS		= $(SOURCES.c:.c=.o) $(SOURCES.a:.a=.o) \
		  $(SOURCES.c:.c=.p) $(SOURCES.a:.a=.p) \
		  $(SOURCES.c:.c=.q) $(SOURCES.a:.a=.q) \
		 sessionsp.o tfmsp.o tfmjobss.o newuser2.o \
		 startns6.o startnsa.o startnss.o \
		 loginss.o loginsp.o \
		 wshsp.o pssp.o loadedsp.o killsp.o remoten.o

SOURCES.c	=	nuconfig.c nuprtnet.c nuprttf.c nuheap.c \
			nuqrtns.c nulocks.c nucount.c nuioc.c nuobjs.c \
			nuwalk.c nuports.c nuinfo.c numon.c nupasswd.c \
			rmlib1.c rmlib2.c rmlib3.c rmlhost.c \
			startns.c readmap.c \
			netserv.c netboot.c netmon.c netalloc.c \
			netnativ.c netmisc.c \
			tfm.c readcdl.c tfmjobs.c tfmmap.c \
			tfmmisc.c tfmrun.c tfmwoe.c \
			session.c userdb.c \
			stopio.c \
			rebootio.c \
			domain.c \
			newuser.c newuser2.c \
			findns.c \
			findsm.c \
			findtfm.c \
			login.c \
			write.c \
			wall.c \
			sendto.c \
			users.c \
			who.c \
			logname.c \
			tty.c \
			id.c \
			whoami.c \
			slnewusr.c sltests.c sllogin.c sltfm.c slnames.c \
			slwindow.c \
			diag_tfm.c diag_ns.c diag_sm.c \
			run.c \
			ps.c \
			loaded.c \
			uptime.c \
			clink.c \
			netagent.c \
			network.c \
			lstatus.c \
			elink.c \
			plink.c \
			dlink.c \
			rmgen.c \
			wsh.c \
			remote.c \
			joinnet.c \
			kill.c \
			null_ra.c \
			crash.c \
			passadm.c

# AMPP source files
SOURCES.a 	= 	resource.a 	\
			modend.a driver.a

# C header files
HEADERS.c	=	session.h rmlib.h netutils.h private.h exports.h \
			netaux.h tfmaux.h sessaux.h

# AMPP macro files
HEADERS.m	=

# Any other source files
SOURCES.odd	= makefile


#-----------------------------------------------------------------------------
# Other generic component variables:

DEFINES	:= $(DEFINES)

# Set any flags required
NCFLAGS := $(NCFLAGS) -D_BSD
# Optional flags:
#	-DSTACKCHECK		used by parts of the software to override 
#				-ps1 pragma
#	-DSTACKEXTENSION	use small stacks, relies on -DSTACKCHECK

CCFLAGS := $(CCFLAGS) -I$(INCLUDE) -D_BSD
ifeq ($(HPROC),TRAN)
  # -pg0 remove names from code
  NCFLAGS   := $(NCFLAGS) -t4 -pg0 -D_BSD
  LINKFLAGS := $(LINKFLAGS) -f
  TFMHEAP   := -h30000
  NSHEAP    := -h30000
  SMHEAP    := -h50000
endif
ifeq ($(HPROC),C40)
  # -zpc1	= enable mem check, -zpm1 = enable backtrace
  NCFLAGS   := $(NCFLAGS) -DSTACKCHECK -DSTACKEXTENSION
  TFMHEAP   := -h30000
  NSHEAP    := -h30000
  SMHEAP    := -h40000
endif
ifeq ($(HPROC),ARM)
  NCFLAGS := $(NCFLAGS) -DSTACKCHECK
endif

ifndef SSTART
 ifeq ($(HPROC),ARM)
   SSTART = $(LIB)/sstart.o
 else
   SSTART = $(LIB)/s0.o
 endif
endif

CSTART = $(LIB)/c0.o

# Note any library def files to be linked with
# in this case for the resource management library
LIBDEFS	:= $(LIB)/kernel.def $(LIB)/syslib.def $(LIB)/util.def $(LIB)/posix.def

# Libraries needed by most of the software
LIBS := -lsession.lib -lRmLib.def -lnetutils.lib \
		-l$(LIB)/c.lib -l$(LIB)/helios.lib -l$(LIB)/bsd.lib

# libraries needed for running on bare machine
 SYSLIBS := -l$(LIB)/helios.lib -lsession.lib

#-----------------------------------------------------------------------------
# Resource Management Library. This is the same for all systems, a little bit
# unfortunate since it is quite big and some of the facilities are useless
# unless you have a multi-processor network. However, it is too much effort
# to untangle everything.

RmLib :	resource.p rmlib1.p rmlib2.p rmlib3.p modend.p 
	$(LINK.LIBRARY) -nRmLib $^ $(LIBDEFS) -o$@

RmLib.def :	resource.a
	$(MAKE.DEF)

#-----------------------------------------------------------------------------
# Session Manager library, a scanned library for now. This is only useful
# in a multi-user environment.

session.lib : slnewusr.o sltests.o sllogin.o sltfm.o slnames.o slwindow.o
ifeq ($(HPROC),TRAN)
	$(LINK) -p -o$@ $^
else
	cat $^ > $@
endif

#-----------------------------------------------------------------------------
# netutils.lib, another scanned library. This is always made, it contains
# things like PrintNetwork() which are needed all over the place.

netutils.lib : nuconfig.o nuprtnet.o nuprttf.o nuheap.o nuqrtns.o \
		nulocks.o nucount.o nuioc.o nuobjs.o nuwalk.o nuports.o \
		nuinfo.o numon.o nupasswd.o
ifeq ($(HPROC),TRAN)
	$(LINK) -p -o$@ $^
else
	cat $^ > $@
endif

#-----------------------------------------------------------------------------
# The three main servers are the Network Server, the Session Manager, and
# the Taskforce Manager. The Network Server is only useful in a multi-processor
# environment, and there is only one version.
# Note: the heap size is the same as that of the Processor Manager to reduce
# fragmentation problems
netserv : netserv.o netmisc.o netboot.o netmon.o netalloc.o netnativ.o
	$(LINK) $(LINKFLAGS) -o$@ -n$@ $(NSHEAP) -s2000 $(CSTART) $^ $(LIBS)

#
# The Session Manager comes in two flavours, session.sp for the MS system and
# session for the MM system. In the MS system multiple users can log in, they
# may or may not use TFM's, but all TFM's run on the same processor.
session : session.o userdb.o
	$(LINK) $(LINKFLAGS) -o$@ -n$@ $(SMHEAP) -s2000 $(CSTART) $^ $(LIBS)

session.sp : sessionsp.o userdb.o
	$(LINK) $(LINKFLAGS) -o$@ -n$@ $(SMHEAP) -s2000 $(CSTART) $^ $(LIBS)

sessionsp.o : session.c
ifeq ($(HPROC),ARM)
	$(NC) -c $(NCFLAGS) -o$@ -I$(HSRC)/$(COMPONENT) -DSingleProcessor $<
else
	$(CC) -c $(CCFLAGS) -o$@ -I$(HSRC)/$(COMPONENT) -DSingleProcessor $<
endif

#
# The TFM can come in two flavours as well. There is tfm.sp for the single
# processor version, and tfm for the multi-processor version.
# The heap sizes match the heap of the Processor Manager to reduce fragmentation
tfm : tfm.o readcdl.o tfmmap.o tfmmisc.o tfmrun.o tfmwoe.o tfmjobs.o
	$(LINK) $(LINKFLAGS) -o$@ -n$@ $(NSHEAP) -s2000 $(CSTART) $^ $(LIBS)

tfm.sp : tfmrun.o tfmmap.o tfmjobss.o tfmsp.o readcdl.o tfmmisc.o tfmwoe.o nulocks.o
	$(LINK) $(LINKFLAGS) -o$@ -n$@ $(NSHEAP) -s2000 $(CSTART) $^ $(LIBS)

tfmsp.o : tfm.c
ifeq ($(HPROC),ARM)
	$(NC) -c $(NCFLAGS) -o$@ -I$(HSRC)/$(COMPONENT) -DSingleProcessor $<
else
	$(CC) -c $(CCFLAGS) -o$@ -I$(HSRC)/$(COMPONENT) -DSingleProcessor $<
endif

tfmjobss.o : tfmjobs.c
ifeq ($(HPROC),ARM)
	$(NC) -c $(NCFLAGS) -o$@ -I$(HSRC)/$(COMPONENT) -DSingleProcessor $<
else
	$(CC) -c $(CCFLAGS) -o$@ -I$(HSRC)/$(COMPONENT) -DSingleProcessor $<
endif

#-----------------------------------------------------------------------------
# device drivers
null_ra.d : driver.q null_ra.q modend.q RmLib.def
	$(LINK.LIBRARY) -o $@ $^ $(LIBDEFS)

#-----------------------------------------------------------------------------
# `system' commands, these do not need the C library. Newuser is complicated,
# there are two versions of this depending on whether or not there is a
# Session Manager

netagent : netagent.o
	$(LINK) $(LINKFLAGS) -o $@ -n$@ -s1500 -h5000 $(SSTART) $^ $(SYSLIBS)

clink : clink.o
	$(LINK) $(LINKFLAGS) -o $@ -n$@ -s1000 -h2000 $(SSTART) $^ $(SYSLIBS) -l$(LIB)/c.lib

newuser : newuser.o
	$(LINK) $(LINKFLAGS) -o $@ -n$@ -s2000 -h2000 $(SSTART) $^ $(SYSLIBS) -l$(LIB)/c.lib

newuser.su : newuser2.o
	$(LINK) $(LINKFLAGS) -o $@ -n$@ -s2000 -h2000 $(CSTART) $^ $(LIBS)

#-----------------------------------------------------------------------------
# Startns comes in umpteen delicious flavours.

startns : startns.o readmap.o
	$(LINK) $(LINKFLAGS) -o$@ -n$@ -s4000 -h4000 $(CSTART) $^ $(LIBS)

startns.64 : startns6.o readmap.o
	$(LINK) $(LINKFLAGS) -o$@ -n$@ -s4000 -h4000 $(CSTART) $^ $(LIBS)

startns.any : startnsa.o readmap.o
	$(LINK) $(LINKFLAGS) -o$@ -n$@ -s4000 -h4000 $(CSTART) $^ $(LIBS)

startns.sp : startnss.o
	$(LINK) $(LINKFLAGS) -o$@ -n$@ -s4000 -h4000 $(CSTART) $^ $(LIBS)

startns.o : startns.c
ifeq ($(HPROC),ARM)
	$(NC) -c $(NCFLAGS) -o$@ -DLimit20 -I$(HSRC)/$(COMPONENT) $<
else
	$(CC) -c $(CCFLAGS) -o$@ -DLimit20 -I$(HSRC)/$(COMPONENT) $<
endif

startns6.o : startns.c
ifeq ($(HPROC),ARM)
	$(NC) -c $(NCFLAGS) -o$@ -DLimit64 -I$(HSRC)/$(COMPONENT) $<
else
	$(CC) -c $(CCFLAGS) -o$@ -DLimit64 -I$(HSRC)/$(COMPONENT) $<
endif

startnsa.o : startns.c
ifeq ($(HPROC),ARM)
	$(NC) -c $(NCFLAGS) -o$@ -I$(HSRC)/$(COMPONENT) $<
else
	$(CC) -c $(CCFLAGS) -o$@ -I$(HSRC)/$(COMPONENT) $<
endif

startnss.o : startns.c
ifeq ($(HPROC),ARM)
	$(NC) -c $(NCFLAGS) -o$@ -DSingleProcessor -I$(HSRC)/$(COMPONENT) $<
else
	$(CC) -c $(CCFLAGS) -o$@ -DSingleProcessor -I$(HSRC)/$(COMPONENT) $<
endif

#-----------------------------------------------------------------------------
# Login also needs rather a lot of versions.
login : login.o
	$(LINK) $(LINKFLAGS) -o$@ -n$@ -s4000 -h10000 $(CSTART) $^ $(LIBS)

login.ss : loginss.o
	$(LINK) $(LINKFLAGS) -o$@ -n$@ -s4000 -h10000 $(CSTART) $^ $(LIBS)

login.sp : loginsp.o
	$(LINK) $(LINKFLAGS) -o$@ -n$@ -s4000 -h10000 $(CSTART) $^ $(LIBS)

loginss.o : login.c
ifeq ($(HPROC),ARM)
	$(NC) -c $(NCFLAGS) -o$@ -DSingleSingle -I$(HSRC)/$(COMPONENT) $<
else
	$(CC) -c $(CCFLAGS) -o$@ -DSingleSingle -I$(HSRC)/$(COMPONENT) $<
endif

loginsp.o : login.c
ifeq ($(HPROC),ARM)
	$(NC) -c $(NCFLAGS) -o$@ -DSingleProcessor -I$(HSRC)/$(COMPONENT) $<
else
	$(CC) -c $(CCFLAGS) -o$@ -DSingleProcessor -I$(HSRC)/$(COMPONENT) $<
endif

#-----------------------------------------------------------------------------
# And so on for the various complicated commands.

wsh : wsh.o
	$(LINK) $(LINKFLAGS) -o$@ -n$@ -s4000 -h4000 $(CSTART) $^ $(LIBS)

wsh.sp : wshsp.o
	$(LINK) $(LINKFLAGS) -o$@ -n$@ -s4000 -h4000 $(CSTART) $^ $(LIBS)

wshsp.o : wsh.c
ifeq ($(HPROC),ARM)
	$(NC) -c $(NCFLAGS) -o$@ -DSingleProcessor -I$(HSRC)/$(COMPONENT) $<
else
	$(CC) -c $(CCFLAGS) -o$@ -DSingleProcessor -I$(HSRC)/$(COMPONENT) $<
endif

ps :	ps.o
	$(LINK) $(LINKFLAGS) -o$@ -n$@ -s4000 -h4000 $(CSTART) $^ $(LIBS)

ps.sp : pssp.o
	$(LINK) $(LINKFLAGS) -o$@ -n$@ -s4000 -h4000 $(CSTART) $^ $(LIBS)

pssp.o : ps.c
ifeq ($(HPROC),ARM)
	$(NC) -c $(NCFLAGS) -o$@ -DSingleProcessor -I$(HSRC)/$(COMPONENT) $<
else
	$(CC) -c $(CCFLAGS) -o$@ -DSingleProcessor -I$(HSRC)/$(COMPONENT) $<
endif

loaded : loaded.o
	$(LINK) $(LINKFLAGS) -o$@ -n$@ -s4000 -h4000 $(CSTART) $^ $(LIBS)

loaded.sp : loadedsp.o
	$(LINK) $(LINKFLAGS) -o$@ -n$@ -s4000 -h4000 $(CSTART) $^ $(LIBS)

loadedsp.o : loaded.c
ifeq ($(HPROC),ARM)
	$(NC) -c $(NCFLAGS) -o$@ -DSingleProcessor -I$(HSRC)/$(COMPONENT) $<
else
	$(CC) -c $(CCFLAGS) -o$@ -DSingleProcessor -I$(HSRC)/$(COMPONENT) $<
endif

kill : kill.o
	$(LINK) $(LINKFLAGS) -o$@ -n$@ -s4000 -h4000 $(CSTART) $^ $(LIBS)

kill.sp : killsp.o
	$(LINK) $(LINKFLAGS) -o$@ -n$@ -s4000 -h4000 $(CSTART) $^ $(LIBS)

killsp.o : kill.c
ifeq ($(HPROC),ARM)
	$(NC) -c $(NCFLAGS) -o$@ -DSingleProcessor -I$(HSRC)/$(COMPONENT) $<
else
	$(CC) -c $(CCFLAGS) -o$@ -DSingleProcessor -I$(HSRC)/$(COMPONENT) $<
endif

remote : remote.o
	$(LINK) $(LINKFLAGS) -o$@ -n$@ -s4000 -h4000 $(CSTART) $^ $(LIBS)

remote.nrm : remoten.o
	$(LINK) $(LINKFLAGS) -o$@ -n$@ -s4000 -h4000 $(CSTART) $^ $(LIBS)

remoten.o : remote.c
ifeq ($(HPROC),ARM)
	$(NC) -c $(NCFLAGS) -o$@ -DNoRmLib -I$(HSRC)/$(COMPONENT) $<
else
	$(CC) -c $(CCFLAGS) -o$@ -DNoRmLib -I$(HSRC)/$(COMPONENT) $<
endif

#-----------------------------------------------------------------------------
# And finally some commands that are simple, and for which only one
# version is needed.

rmgen : rmgen.o
	$(LINK) $(LINKFLAGS) -o$@ -n$@ -s6000 -h4000 $(CSTART) $^ $(LIBS)

joinnet : joinnet.o
	$(LINK) $(LINKFLAGS) -o$@ -n$@ -s4000 -h4000 $(CSTART) $^ $(LIBS)

stopio : stopio.o
	$(LINK) $(LINKFLAGS) -o$@ -n$@ -s4000 -h4000 $(CSTART) $^ $(LIBS)

rebootio : rebootio.o
	$(LINK) $(LINKFLAGS) -o$@ -n$@ -s4000 -h4000 $(CSTART) $^ $(LIBS)

write : write.o
	$(LINK) $(LINKFLAGS) -o$@ -n$@ -s4000 -h4000 $(CSTART) $^ $(LIBS)

wall : wall.o
	$(LINK) $(LINKFLAGS) -o$@ -n$@ -s4000 -h4000 $(CSTART) $^ $(LIBS)

users : users.o
	$(LINK) $(LINKFLAGS) -o$@ -n$@ -s4000 -h4000 $(CSTART) $^ $(LIBS)

whoami : whoami.o
	$(LINK) $(LINKFLAGS) -o$@ -n$@ -s4000 -h4000 $(CSTART) $^ $(LIBS)

who :	who.o
	$(LINK) $(LINKFLAGS) -o$@ -n$@ -s4000 -h4000 $(CSTART) $^ $(LIBS)

diag_tfm : diag_tfm.o
	$(LINK) $(LINKFLAGS) -o$@ -n$@ -s4000 -h4000 $(CSTART) $^ $(LIBS)

logname : logname.o
	$(LINK) $(LINKFLAGS) -o$@ -n$@ -s4000 -h4000 $(CSTART) $^ $(LIBS)

tty :	tty.o
	$(LINK) $(LINKFLAGS) -o$@ -n$@ -s4000 -h4000 $(CSTART) $^ $(LIBS)

sendto : sendto.o
	$(LINK) $(LINKFLAGS) -o$@ -n$@ -s4000 -h4000 $(CSTART) $^ $(LIBS)

id :	id.o
	$(LINK) $(LINKFLAGS) -o$@ -n$@ -s4000 -h4000 $(CSTART) $^ $(LIBS)

network : network.o
	$(LINK) $(LINKFLAGS) -o$@ -n$@ -s4000 -h4000 $(CSTART) $^ $(LIBS)

domain : domain.o
	$(LINK) $(LINKFLAGS) -o$@ -n$@ -s4000 -h4000 $(CSTART) $^ $(LIBS)

uptime : uptime.o
	$(LINK) $(LINKFLAGS) -o$@ -n$@ -s4000 -h4000 $(CSTART) $^ $(LIBS)

diag_ns : diag_ns.o
	$(LINK) $(LINKFLAGS) -o$@ -n$@ -s4000 -h4000 $(CSTART) $^ $(LIBS)

diag_sm : diag_sm.o
	$(LINK) $(LINKFLAGS) -o$@ -n$@ -s4000 -h4000 $(CSTART) $^ $(LIBS)

findns : findns.o
	$(LINK) $(LINKFLAGS) -o$@ -n$@ -s4000 -h4000 $(CSTART) $^ $(LIBS)

findsm : findsm.o
	$(LINK) $(LINKFLAGS) -o$@ -n$@ -s4000 -h4000 $(CSTART) $^ $(LIBS)

findtfm : findtfm.o
	$(LINK) $(LINKFLAGS) -o$@ -n$@ -s4000 -h4000 $(CSTART) $^ $(LIBS)

run :	run.o
	$(LINK) $(LINKFLAGS) -o$@ -n$@ -s4000 -h4000 $(CSTART) $^ $(LIBS)

elink : elink.o
	$(LINK) $(LINKFLAGS) -o$@ -n$@ -s4000 -h4000 $(CSTART) $^ $(LIBS)

dlink : dlink.o
	$(LINK) $(LINKFLAGS) -o$@ -n$@ -s4000 -h4000 $(CSTART) $^ $(LIBS)

plink : plink.o
	$(LINK) $(LINKFLAGS) -o$@ -n$@ -s4000 -h4000 $(CSTART) $^ $(LIBS)

lstatus : lstatus.o
	$(LINK) $(LINKFLAGS) -o$@ -n$@ -s4000 -h4000 $(CSTART) $^ $(LIBS)

crash : crash.o
	$(LINK) $(LINKFLAGS) -o$@ -n$@ -s4000 -h4000 $(CSTART) $^ $(LIBS)

passadm : passadm.o
	$(LINK) $(LINKFLAGS) -o$@ -n$@ -s4000 -h4000 $(CSTART) $^ $(LIBS)

#-----------------------------------------------------------------------------
endif #InProcMake	# This line must be at EOF
