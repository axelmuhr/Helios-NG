#-----------------------------------------------------------------------------
# Helios generic make system - GENERIC COMPONENT MAKEFILE -
#-----------------------------------------------------------------------------
# Helios generic component make rules for: HOBJASM ARM macro assembler
#
# File: /HSRC/cmds/hobjasm/makefile
#
# These are the host/processor independent rules and variables for this
# component of Helios. It is included by the processor specific makefile
# That may add to some of its functionality. You should only alter this
# makefile by using "ifneq ($(HPROC),YYY)" clauses for a specific processor.
#
# SccsId: %W% %G%
# RcsId: $Id$
#               (C) Copyright 1990 Perihelion Software
#
# WARNING: assumes you are using GNUmake.
#-----------------------------------------------------------------------------

ifndef InProcMake
# This part of the makefile is run in the components generic source directory

# The variables HPROC, HHOST, HSRC, and HPROD should already be set by 
# your make script or environment, if not then get local defaults from
# makeinc/DFLT.mak. Note that the following pathname cannot use $(HSRC).
ifndef HPROC
include ../../../makeinc/DFLT.mak
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
COMPONENT = cmds/hobjasm

# Manditory includes
include $(HSRC)/makeinc/$(HHOST).mak    # Default host system variables
include $(HSRC)/makeinc/$(HPROC).mak    # Default processor rules and vars.

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
VPATH := $(HSRC)/$(COMPONENT)

#-----------------------------------------------------------------------------
# Generic component make rules and variables added below here

# Note all final generic targets here
TARGETS = hobjasm

default : $(TARGETS) Xdefault

install : $(TARGETS) Xinstall
	$(CP) $(TARGETS) $(BIN)
	$(TOUCH) install

clean   : Xclean
	-$(RM)  $(OBJECTS) $(TARGETS) $(XOBJECTS) $(XTARGETS) mkout \
		core ../mkout install Xinstall

#-----------------------------------------------------------------------------
# for archive, backup and cleaning purposes,
# note all generic SOURCE and OBJECT files:

# Note all sources
SOURCES         = $(SOURCES.c) $(SOURCES.a) \
		  $(HEADERS.c) $(HEADERS.m) \
		  $(SOURCES.odd)

OBJECTS = $(SOURCES.c:.c=.o) $(SOURCES.a:.a=.o)

# C source files
SOURCES.c =     aasm.c apply.c applysub.c asm.c code.c conds.c \
		errors.c expr.c exstore.c extypes.c formatio.c \
		fpio.c getdir.c getline.c globvars.c initdir.c listing.c \
		literals.c llabel.c mactypes.c nametype.c occur.c \
		opcode.c osdepend.c p1dirs.c p1hand.c p2dirs.c \
		p2hand.c stats.c store.c stubs.c symbol.c \
		tables.c tokens.c vars.c iface.c

# AMPP source files
SOURCES.a       =

# C header files
HEADERS.c       = asm.h code.h conds.h constant.h errors.h \
		  exstore.h extypes.h formatio.h fpio.h getdir.h getline.h  \
		  globvars.h helios.h initdir.h iface.h listing.h \
		  llabel.h mactypes.h nametype.h asmvars.h opcode.h  \
		  osdepend.h p1dirs.h p1hand.h p2dirs.h \
		  stats.h store.h stubs.h symbol.h tables.h tokens.h  \
		  vars.h version.h apply.h applysub.h occur.h \
		  expr.h literals.h p2hand.h

# AMPP macro files
HEADERS.m       =

# Any other source files - remeber to add any new example sources here
SOURCES.odd     = makefile DOC README \
		  examples/libend.s examples/libhdr.s examples/library.s \
		  examples/libtest.c examples/makefile examples/minimum.s \
		  examples/sample.c examples/standard.s 

#-----------------------------------------------------------------------------
# Other generic component variables:

# Set any flags required
ifeq ($(HPROC),ARM)
  NCFLAGS := $(NCFLAGS) -D__NCCBUG -DDEBUG
else
  NCFLAGS := $(NCFLAGS)
endif

#-----------------------------------------------------------------------------
# Generic targets and their dependencies:

# Always recompile "aasm.o" to get the correct version information

hobjasm:        $(OBJECTS)
		$(NC) -c $(NCFLAGS) ../aasm.c
		$(LINK) -ohobjasm -nhobjasm -s8000 -h8000 $(CSTART) $(OBJECTS)

#-----------------------------------------------------------------------------
endif #InProcMake       # This line must be at EOF
