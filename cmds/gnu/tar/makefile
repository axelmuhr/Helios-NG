#-----------------------------------------------------------------------------
# Helios generic make system - GENERIC COMPONENT MAKEFILE -
#-----------------------------------------------------------------------------
# Helios generic component make rules for: GNU tar
#
# File: /HSRC/cmds/gnu/tar/makefile
#
# These are the host/processor independent rules and variables for this
# component of Helios. It is included by the processor specific makefile
# That may add to some of its functionality. You should only alter this
# makefile by using "ifneq ($(HPROC),YYY)" clauses for a specific processor.
#
# SccsId: %W% %G%
# RcsId: $Id: makefile,v 1.7 1994/03/25 11:43:27 nickc Exp $
#		(C) Copyright 1990 Perihelion Software
#
# WARNING: assumes you are using GNUmake.
#-----------------------------------------------------------------------------

ifndef InProcMake
# This part of the makefile is run in the components generic source directory

# The variables HPROC, HHOST, HSRC, and HPROD should already be set by 
# your make script or environment, if not then get local defaults from
# $(HSRC)/makeinc/DFLT.mak. Note that the following pathname cannot use $(HSRC).
ifndef HPROC
include ../makeinc/DFLT.mak
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
COMPONENT = cmds/gnu/tar

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
HVPATH := $(RSRC)/$(COMPONENT)/$(HPROC):$(HSRC)/$(COMPONENT):$(RSRC)/$(COMPONENT)
endif

vpath %.h $(HVPATH)
vpath %.a $(HVPATH)
vpath %.c $(HVPATH)

#-----------------------------------------------------------------------------
# Generic component make rules and variables added below here

# Note all final generic targets here
TARGETS = tar

default : $(TARGETS) Xdefault

# The powers that be have decided to place tar in
#	/hprod/C40_public_domain_disk/tar
# instead of the normal place for the C40 object

ifeq ($(HPROC),C40)
install	: $(TARGETS) Xinstall
	$(CP) $(TARGETS) /hprod/C40_public_domain_disk/tar
	(cd .. ; cp $(SOURCES.odd) /hprod/C40_public_domain_disk/tar)
	/bin/rm -f /hprod/C40_public_domain_disk/tar/makefile
	$(TOUCH) install
else
install	: $(TARGETS) Xinstall
	$(CP) $(TARGETS) $(HPROD)/local/bin
	(cd .. ; cp $(SOURCES.odd) $(HPROD)/local/bin)
	$(TOUCH) install
endif

clean	: Xclean
	-$(RM)	$(OBJECTS) $(TARGETS) $(XOBJECTS) $(XTARGETS) mkout \
		core ../mkout install installdef Xinstall


#-----------------------------------------------------------------------------
# for archive, backup and cleaning purposes,
# note all generic SOURCE and OBJECT files:

# Note all sources
SOURCES		= $(SOURCES.c) $(SOURCES.a) \
		  $(HEADERS.c) $(HEADERS.m) \
		  $(SOURCES.odd)

OBJECTS	= tar.o create.o extract.o buffer.o getoldop.o list.o \
	names.o diffarch.o port.o wildmat.o getopt.o

# C source files
SOURCES.c	= tar.c create.c extract.c buffer.c getoldop.c list.c \
	 	  names.c diffarch.c port.c wildmat.c getopt.c

# AMPP source files
SOURCES.a	=

# C header files
HEADERS.c	= tar.h port.h open3.h tarpriv.h

# AMPP macro files
HEADERS.m	=

# Any other source files
SOURCES.odd	= makefile tar.1 tar.2 tar.3 tar.5 tar1.tex tar5.tex \
		  readme todo

#-----------------------------------------------------------------------------
# Other generic component variables:

defines = -DPOSIX -DNONAMES -DHELIOS -D_BSD \
	-DDEFBLOCKING=20 -DDEF_AR_FILE=\"/tape\"

# Set any flags required
ifeq ($(HPROC),ARM)
  NCFLAGS := $(NCFLAGS) $(defines) -w
else
  NCFLAGS := $(NCFLAGS) $(defines) -wvdf
endif

LINKFLAGS := $(LINKFLAGS) -v -s20000

#-----------------------------------------------------------------------------

tar: $(OBJECTS)
	$(LINK) $(LINKFLAGS) -o tar $(LIB)/cstart.o $(OBJECTS) -l$(LIB)/bsd.lib

#-----------------------------------------------------------------------------
endif #InProcMake	# This line must be at EOF

