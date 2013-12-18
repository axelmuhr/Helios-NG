#-----------------------------------------------------------------------------
# Helios generic make system
#-----------------------------------------------------------------------------
# Helios generic component make rules for: KERNEL
#
# File: /HSRC/tcpip/tinybiff/makefile
#
# These are the host/processor independent rules and variables for this
# component of Helios. It is included by the processor specific makefile
# That may add to some of its functionality. You should only alter this
# makefile by using "ifneq ($(HPROC),YYY)" clauses for a specific processor.
#
# SccsId: %W% %G%
# RcsId: $Id: makefile,v 1.3 1993/03/10 09:29:08 nickc Exp $
#		(C) Copyright 1990 Perihelion Software
#
# WARNING: assumes you are using GNUmake.
#-----------------------------------------------------------------------------

ifndef InProcMake
# This part of the makefile is run in the components generic source directory

# The variables HPROC, HHOST, HSRC, and HPROD should already be set by 
# your make script or environment, if not then get local defaults from
# makeinc/DFLT.mak. Note that the following pathname cannot use $(HSRC).
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
COMPONENT = tcpip/tinybiff

# Manditory includes
include $(HSRC)/makeinc/$(HHOST).mak
include $(HSRC)/makeinc/$(HPROC).mak

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
vpath %.c $(HVPATH)
vpath %.a $(HVPATH)

#-----------------------------------------------------------------------------
# Generic component make rules and variables added below here

# Note all final generic targets here

MV = mv

DEMO = $(HPROD)/local/tcpip/tinybiff

EXPORTS = $(SOURCES.c) makefile.hel readme

TARGETS	= tinybiff

default : $(TARGETS) Xdefault

install	: installdef $(TARGETS) Xinstall
	$(CP) tinybiff $(HPROD)/local/lib
	-$(MKDIR) $(DEMO)
	$(foreach NAM, $(EXPORTS), \
		$(RM) $(DEMO)/$(NAM) ; \
		$(CP) $(HSRC)/$(COMPONENT)/$(NAM) $(DEMO) ; \
		$(CHMOD) ug+w $(DEMO)/$(NAM); )
	$(MV) $(DEMO)/makefile.hel $(DEMO)/makefile
	$(TOUCH) install

installdef:
	$(TOUCH) installdef

clean	: Xclean
	-$(RM)	$(OBJECTS) $(TARGETS) $(XOBJECTS) $(XTARGETS) \
		mkout core ../mkout install installdef Xinstall


#-----------------------------------------------------------------------------
# for archive, backup and cleaning purposes,
# note all generic SOURCE and OBJECT files:

SOURCES	=	$(SOURCES.c) $(SOURCES.a) $(HEADERS.c) $(HEADERS.m) \
		$(SOURCES.odd)

OBJECTS = 	$(SOURCES.c:.c=.o)

# C source files
SOURCES.c=	tinybiff.c

SOURCES.a =

# C header files
HEADERS.c =

# AMPP macro files
HEADERS.m =

SOURCES.odd = makefile makefile.hel readme

#-----------------------------------------------------------------------------
# Generic targets and their dependencies:

# uncomment if you have compiler or makedepend generated dependencies
#include depends

# Turn off stack check and function header generation.
ifeq ($(HPROC),ARM)
  NCFLAGS := $(NCFLAGS) -Zps1
else
#  NCFLAGS := $(NCFLAGS) -wd -D_BSD -DMEM_CHECK -DDEBUG
  ifeq ($(HPROC),C40)
    NCFLAGS := $(NCFLAGS) -Zps1 -D_BSD 
  else
    ifdef SYSDEB
      NCFLAGS := $(NCFLAGS) -ps1
    endif
    NCFLAGS := $(NCFLAGS) -wd -D_BSD
  endif
endif

# Note any library def files to be linked with
LIBDEFS	:=

LINKFLAGS := $(LINKFLAGS)

#-----------------------------------------------------------------------------
# Generic targets and their dependencies:

# uncomment if you have compiler or makedepend generated dependencies
#include depends

$(OBJECTS): $(HEADERS.c)

tinybiff: $(CSTART) $(LIBDEFS) $(OBJECTS)
	$(LINK) $(LINKFLAGS) -otinybiff $^ -l$(LIB)/bsd.lib

#-----------------------------------------------------------------------------
endif #InProcMake	# This line must be at EOF
