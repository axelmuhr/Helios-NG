#-----------------------------------------------------------------------------
# Helios generic make system - GENERIC (NATIVE) COMPONENT MAKEFILE -
#-----------------------------------------------------------------------------
# Helios generic component make rules for: server
#
# File: /HSRC/ioproc/server/makefile
#
# These are the host/processor independent rules and variables for this
# component of Helios. It is included by the processor specific makefile
# That may add to some of its functionality. You should only alter this
# makefile by using "ifneq ($(HPROC),YYY)" clauses for a specific processor.
#
# RcsId: $Id: makefile,v 1.23 1994/01/06 15:19:22 nickc Exp $
#		(C) Copyright 1990 - 1994 Perihelion Software
#
# WARNING: assumes you are using GNUmake.
#-----------------------------------------------------------------------------

ifndef InProcMake
# This part of the makefile is run in the components generic source directory

# The variables HPROC, HHOST, HSRC, and HPROD should already be set by 
# your make script or environment, if not then get local defaults from
# $(HSRC)/makeinc/DFLT.mak. Note that the following pathname cannot use $(HSRC).
#ifndef HPROC
#nclude ../makeinc/DFLT.mak
#endif

.PHONY: notarget
notarget:
	@$(MAKE) -C $(HHOST)

.DEFAULT:
	@$(MAKE) -C $(HHOST) $@

else
#-----------------------------------------------------------------------------
# This part of the makefile is always run in a processor specific subdirectory.

# Manditory variables for a component:
# name of this dir relative to $(HSRC)

COMPONENT = ioproc/server#
SRCHOME   = $(HSRC)/$(COMPONENT)#

# Manditory includes
include $(HSRC)/makeinc/$(HHOST).mak	# Default host system variables

# Manditory phoney targets
.PHONY: default clean install Xdefault Xclean
# "default"    - should make this component in its proc subdir
# "install"    - copy the new component to $(HPROD) production directory
# "clean"      - remove all intermediate and executable files from proc subdir
# Phoney targets prefixed by X are for adding processor specific targets.
#
# Processor specific sources are held in the processor specific subdir.,
# generic ones in the directory above it.

HVPATH := $(HSRC)/$(COMPONENT):$(HSRC)/$(COMPONENT)/sun

ifdef RSRC
 HVPATH := $(RSRC)/$(COMPONENT)/$(HPROC):$(HSRC)/$(COMPONENT):$(RSRC)/$(COMPONENT)
endif

vpath %.h $(HVPATH)
vpath %.a $(HVPATH)
vpath %.c $(HVPATH)
vpath %.s $(HVPATH)

#
# These are some special rules to cope with the fact that
# although we build the Helios binaries on an RS6000 we
# ship out SUN versions of the IO server's window program
# To add new SUN4 binaries, add the name of the file to
# the Sun4Binaries variable below.  Similarly a SUN3 binary
# should be added to the Sun3Binaries variable.
#
# A quick word about the GNU make specials used below
# $(<F) expands to the file name of the first dependency
# so for example
#
#   target: dir/depend1 dir/depend2
#       echo $(<F)
#
# would echo 'depend1'.
#
# A target of the form 
#     <target names> : <pattern1> : <pattern2>
# says that the following rules should be applied to all of the targets
# in <target names> provided that they match the wildcard string <pattern1>.
# An occurance of % in <pattern1> is a wildcard operator, matching any string.
# If % occurs in <pattern2> it is replaced by the string it represents in
# <pattern1>.  When the rule executes $@ (the target) is <pattern1> and $^
# (the dependencies) is <pattern2>.  So for example
#
#   dir/fred dir/jim dir/bert : dir/% : src/%
#        echo cp $@ $^
#
# is the equivalent of
#
#    dir/fred : src/fred
#        echo di dir/fred src/fred
#    dir/jim  : src/jim
#        echo cp dir/jim src/jim
#    dir/bert  : src/bert
#        echo cp dir/bert src/bert
#

Sun4Binaries := serverwinxv serverwinsv serverwinxaw serverwinxt hydra hydramon server
Sun3Binaries := serverwinxv serverwinxaw serverwinxt hydra hydramon server

Sun4DependantBinaries := $(addprefix $(HPROD)/, $(addsuffix .sun4, $(Sun4Binaries) ) )
Sun3DependantBinaries := $(addprefix $(HPROD)/, $(addsuffix .sun3, $(Sun3Binaries) ) )

$(SRCHOME)/SUN4/$(Sun4Binaries):
$(SRCHOME)/SUN3/$(Sun3Binaries):

# ideally we should perform a remote make of these files ...
$(Sun4DependantBinaries) : $(HPROD)/%.sun4 : $(SRCHOME)/SUN4/%
	@ echo ERROR: SUN4/$(<F) is not up to date.  Please log on to Sparky and rebuild
	@ false

$(Sun3DependantBinaries) : $(HPROD)/%.sun3 : $(SRCHOME)/SUN3/%
	@ echo ERROR: SUN3/$(<F) is not up to date.  Please log on to Sparky and rebuild
	@ false

#-----------------------------------------------------------------------------
# Generic component make rules and variables added below here

# Note all final generic targets here
# The actual targets are always machine-specific

TARGETS =

default : $(TARGETS) Xdefault
install	: $(TARGETS) Xinstall $(Sun3DependantBinaries) $(Sun4DependantBinaries)
	$(TOUCH) install

clean	: Xclean
	-$(RM)	$(TARGETS) $(XOBJECTS) $(XTARGETS) mkout \
		core ../mkout install Xinstall helios.h *.o

#-----------------------------------------------------------------------------
# for archive, backup and cleaning purposes,
# note all generic SOURCE and OBJECT files:

# Note all sources (used by main makefile srctar)
SOURCES		= $(SOURCES.c) $(HEADERS.c) $(SOURCES.odd)

# C source files:
SOURCES.c =	server.c terminal.c tload.c cofuns.c debug.c gem.c \
		linkio.c devices.c files.c tcp.c smlgen.c

# C header files:
HEADERS.c =	barthdr debugopt.h defines.h fundefs.h protocol.h \
		sccs.h server.h structs.h sml.h

GENHEADERS.c =	../defines.h ../protocol.h ../barthdr ../debugopt.h \
		../structs.h ../fundefs.h ../server.h

# Any other source files:
SOURCES.odd	= makefile

HOSTCFLAGS := -I$(HSRC)/$(COMPONENT)

# The generic server bits

SERVOBJ	= server.o files.o devices.o tcp.o tload.o cofuns.o \
		  debug.o terminal.o linkio.o

# all unix like hosts use sunlocal.o

ifneq (,$(findstring $(HHOST), SUN4 SUN3 SUN386 SM90 UNIX386 R140 I486V4 RS6000 HP9000))
   SERVOBJ := $(SERVOBJ) sunlocal.o
endif

# link objects

ifneq (,$(findstring $(HHOST), SUN4 SUN3))
  LINKOBJS := linklib.o \
	     niche.o \
	     b011.o \
	     b014.o \
	     b016.o \
	     telmat.o \
	     volvox.o \
	     hunt.o \
	     vc40.o \
	     vy86pid.o
# Sun I/O Servers built by Perihelion and Parsytec include support
# for the Parsytec boards.
  ifeq ( $(HLICENSEE), PARSYTEC)
        LINKOBJS := $(LINKOBJS) parsy.o par_link.o
        HOSTCFLAGS := $(HOSTCFLAGS) -DPARSY -DHELIOS_IOSERVER
  endif
  ifneq (,$(findstring $(HLICENSEE), PERIHELION))
        LINKOBJS := $(LINKOBJS) parsy.o par_link.o
        HOSTCFLAGS := $(HOSTCFLAGS) -DPARSY -DHELIOS_IOSERVER
  endif
endif

ifeq ($(HHOST),SUN3)
  LINKOBJS := $(LINKOBJS) cpswap.o
endif

ifneq (,$(findstring $(HHOST), SM90 UNIX386 R140 I486V4 RS6000 HP9000))
  LINKOBJS := linklib.o 
endif

ifeq ( $(HHOST), SUN386)
  LINKOBJS := linklib.o kpar.o
endif

ifeq ( $(HHOST), SCOUNIX)
  LINKOBJS := linklib.o kparix.o
endif

ifeq ( $(HHOST), I486V4)
  LINKOBJS := linklib.o hepc2.o
endif

# hydra

HYDRAOBJ   = hydra.o

# hydra monitor

HYDRAMON   = hydramon.o
	

#-----------------------------------------------------------------------------
# Other generic component variables:

# Set any flags required



#-----------------------------------------------------------------------------
# Generic targets and their dependencies:

iopsrcnames:
	@echo $(foreach FILE, $(SOURCES), $(COMPONENT)/$(FILE)) >> $(HSRC)/backedupsrcs

#-----------------------------------------------------------------------------
endif #InProcMake	# This line must be at EOF
