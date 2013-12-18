#-----------------------------------------------------------------------------
# Helios generic make system - GENERIC COMPONENT MAKEFILE -
#-----------------------------------------------------------------------------
# Helios generic component make rules for: General Commands
#
# File: /HSRC/cmds/private/makefile
#
# These are the host/processor independent rules and variables for this
# component of Helios. It is included by the processor specific makefile
# That may add to some of its functionality. You should only alter this
# makefile by using "ifneq ($(HPROC),YYY)" clauses for a specific processor.
#
# RcsId: $Id: makefile,v 1.7 1994/02/17 11:45:35 nickc Exp $
#		(C) Copyright 1990 - 1994 Perihelion Software
#
# WARNING: assumes you are using GNUmake.
#
#
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
COMPONENT = cmds/private

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
HVPATH := $(RSRC)/$(COMPONENT)/$(HPROC):$(HVPATH):$(RSRC)/$(COMPONENT)
endif

vpath %.h $(HVPATH)
vpath %.c $(HVPATH)
vpath %.a $(HVPATH)

#-----------------------------------------------------------------------------
# Generic component make rules and variables added below here

# Note all final generic targets here
TARGETS = $(RBINS) $(PBINS) $(SBINS)

# default & install are at end of this file due to problem with GNUMake
# See comments there...

#default: $(TARGETS) Xdefault
#
#install	: $(TARGETS) Xinstall
#	$(CP) $(RBINS) $(BIN)
#	$(CP) $(PBINS) $(BIN)/private
#	$(CP) $(SBINS) $(BIN)
#	$(TOUCH) install
# Stop no args cleaning the cmds!
newdefault: default


#-----------------------------------------------------------------------------
# for archive, backup and cleaning purposes,
# note all generic SOURCE and OBJECT files:

# AMPP source files
SOURCES.a	=

# C header files
HEADERS.c	=

# AMPP macro files
HEADERS.m	=

# Any other source files
SOURCES.odd	= makefile

# Private commands which we don't formally support or release
PSRCS := bounce.c b2.c dbg.c dbt.c dhry.c links.c pause.c ports.c ansitest.c \
        free.c disktest.c mtime.c trail.c enable.c szal.c timeio.c linkcat.c  \
	portcnt.c portown.c

# no network/CDL style commands for the ARM
ifneq ($(HPROC),ARM)
PSRCS := $(PSRCS) format.c memsearch.c
endif

#ifeq ($(HPROC),C40)
#PSRCS := $(PSRCS)
#endif
 
# C source files
SOURCES.c	= $(PSRCS)

# Transform to lists of object files
POBJS = $(PSRCS:.c=.o)

default: $(TARGETS) Xdefault

#transform to lists of binaries
PBINS = $(POBJS:.o=)

# Note all sources
SOURCES		= $(SOURCES.c) $(SOURCES.a) \
		   $(HEADERS.c) $(HEADERS.m) \
		   $(SOURCES.odd)

OBJECTS	= $(ROBJS) $(POBJS) $(SOBJS)

#-----------------------------------------------------------------------------
# Other generic component variables:

# Set any flags required
ifeq ($(HPROC),TRAN)
  NCFLAGS := $(NCFLAGS) -wadfsv -ecpz
else
  NCFLAGS := $(NCFLAGS)
endif

%: %.o
	$(LINK) $(LINKFLAGS) -o$@ -n$@ -s2000 -h4000 $(CSTART) $@.o

#-----------------------------------------------------------------------------
# Generic targets and their dependencies:

#$(TARGETS): $(CSTART)

# Compile regular and private binaries

$(POBJS):

$(PBINS): $(CSTART)

#------------------------------------------------------------------------
# Special cases

disktest : $(CSTART) disktest.o
	$(LINK) $(LINKFLAGS) -o$@ -n$@ -s8000 -h4000 $^

#-----------------------------------------------------------------------------
# Due to the use of ':=', rather than '=', these rules have to appear here.

default: $(TARGETS) Xdefault

install	: $(TARGETS) Xinstall
	@ test -d $(BIN)/private || mkdir -p $(BIN)/private
	$(CP) $(PBINS) $(BIN)/private
	$(TOUCH) install

clean	: Xclean
	-$(RM)	$(OBJECTS) $(TARGETS) $(XOBJECTS) $(XTARGETS) mkout \
		core ../mkout install installdef Xinstall

#-----------------------------------------------------------------------------
endif #InProcMake	# This line must be at EOF
