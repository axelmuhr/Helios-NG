#-----------------------------------------------------------------------------
# Helios generic make system - GENERIC COMPONENT MAKEFILE -
#-----------------------------------------------------------------------------
# Helios generic component make rules for: filesys/devs/b422
#
# File: /HSRC/filesys/devs/b422/makefile
#
# These are the host/processor independent rules and variables for this
# component of Helios. It is included by the processor specific makefile
# That may add to some of its functionality. You should only alter this
# makefile by using "ifneq ($(HPROC),YYY)" clauses for a specific processor.
#
# SccsId: %W% %G%
# RcsId: $Id: makefile,v 1.7 1991/03/21 11:34:17 martyn Exp $
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
COMPONENT = filesys/devs/b422

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
VPATH := $(HSRC)/$(COMPONENT)

ifdef RSRC
VPATH := $(RSRC)/$(COMPONENT)/$(HPROC):$(HSRC)/$(COMPONENT):$(RSRC)/$(COMPONENT)
endif

#-----------------------------------------------------------------------------
# Generic component make rules and variables added below here

# Note all final generic targets here
TARGETS = b422.d b422.b2u

default : $(TARGETS) Xdefault

install	:  $(TARGETS) Xinstall installsources
	$(CP) $(TARGETS) $(HPROD)/lib
	$(TOUCH) install

clean	: Xclean
	-$(RM)	$(OBJECTS) b422.d $(XOBJECTS) $(XTARGETS) mkout \
		core ../mkout install installdef Xinstall


#-----------------------------------------------------------------------------
# for archive, backup and cleaning purposes,
# note all generic SOURCE and OBJECT files:

# Note all sources
SOURCES		= $(SOURCES.c) $(SOURCES.a) \
		  $(HEADERS.c) $(HEADERS.m) \
		  $(SOURCES.odd)

#Full set of library object files
#OBJECTS		= $(SOURCES.c:.c=.p) $(SOURCES.a:.a=.p)
# Or if you are not a library:
#OBJECTS	= $(SOURCES.c:.c=.o) $(SOURCES.a:.a=.o)

# C source files
SOURCES.c	= b422dev.c b422pcol.c b422xpt.c b422w10.c b422r10.c b422cap.c \
		  b422fmt.c b422tur.c b422reqs.c b422inq.c b422time.c b422r6.c \
		  b422w6.c b422msel.c b422msen.c

# AMPP source files
SOURCES.a	= discdevs.a modend.a

# C header files
HEADERS.c	= b422cons.h b422err.h b422fns.h b422pcol.h

# AMPP macro files
HEADERS.m	=

# Any other source files
SOURCES.odd	= makefile


#-----------------------------------------------------------------------------
# Other generic component variables:

# Set any flags required
ifeq ($(HPROC),ARM)
  NCFLAGS := $(NCFLAGS)
else
  NCFLAGS := $(NCFLAGS) -ps1 -pf0
endif

# Note any library def files to be linked with
LIBDEFS	:= $(LIB)/kernel.def $(LIB)/syslib.def $(LIB)/util.def

LINKFLAGS := $(LINKFLAGS) -v -f

#-----------------------------------------------------------------------------
# Generic targets and their dependencies:

# uncomment if you have compiler or makedepend generated dependencies
#include depends

b422.d : discdevs.p b422dev.p b422pcol.p b422xpt.p b422tur.p b422w10.p \
	 b422r10.p b422cap.p b422fmt.p b422reqs.p b422inq.p modend.p
	$(LINK) $(LINKFLAGS) -n b422.d -o $@ $^ $(LIBDEFS)

b422.b2u :

installsources : $(SOURCES.c) $(SOURCES.a) $(HEADERS.c)
	$(CP) $^ $(HPROD)/local/src/hfs/b422
	$(CP) example.mkf $(HPROD)/local/src/hfs/b422/makefile
	$(TOUCH) $@

#-----------------------------------------------------------------------------
endif #InProcMake	# This line must be at EOF

