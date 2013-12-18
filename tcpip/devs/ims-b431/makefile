#-----------------------------------------------------------------------------
# Helios generic make system - GENERIC COMPONENT MAKEFILE -
#-----------------------------------------------------------------------------
# Helios generic component make rules for: tcpip/devs/ims-b431
#
# File: /HSRC/tcpip/devs/ims-b431/makefile
#
# These are the host/processor independent rules and variables for this
# component of Helios. It is included by the processor specific makefile
# That may add to some of its functionality. You should only alter this
# makefile by using "ifneq ($(HPROC),YYY)" clauses for a specific processor.
#
# SccsId: %W% %G%
# RcsId: $Id: makefile,v 1.4 1992/08/17 11:20:38 paul Exp $
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
COMPONENT = tcpip/devs/ims-b431

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
TARGETS = in-b431.d imsb431.b2h


default : $(TARGETS) Xdefault

install	:  $(TARGETS) Xinstall
	$(CP) $(TARGETS) $(LIB)
	$(TOUCH) install

clean	: Xclean
	-$(RM)	$(OBJECTS) in-b431.d $(XOBJECTS) $(XTARGETS) mkout \
		core ../mkout install installdef Xinstall *.o *.p


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
SOURCES.c	= b431io.c etherdev.c

# AMPP source files
SOURCES.a	= devs.a modend.a

# C header files
HEADERS.c	= b431drvr.h b431io.h

# AMPP macro files
HEADERS.m	=

# Any other source files
SOURCES.odd	= makefile imsb431.b2h devinfo.net read.me


#-----------------------------------------------------------------------------
# Other generic component variables:

# Set any flags required
ifeq ($(HPROC),ARM)
  NCFLAGS := $(NCFLAGS)
else
  NCFLAGS := $(NCFLAGS) -ps1 -pf0 -r
endif

# Note any library def files to be linked with
LIBDEFS	:= $(LIB)/kernel.def $(LIB)/syslib.def $(LIB)/util.def $(LIB)/posix.def

LINKFLAGS := $(LINKFLAGS) -v -f

#-----------------------------------------------------------------------------
# Generic targets and their dependencies:

# uncomment if you have compiler or makedepend generated dependencies
#include depends

b407dev0.o : b407dev.c
	$(NC) -DB407_LINK0 $(NCFLAGS) $(DEFINES) $< -s$(TEMP)/$*.s
	$(ASM) $(ASMFLAGS) -o $@ $(TEMP)/$*.s
	-$(RMTEMP) $(TEMP)/$*.s

in-b431.d :	devs.p etherdev.o b431io.o modend.p
	$(LINK) $(LINKFLAGS) -n in-b431.d -o $@ $^ $(LIBDEFS)

b407.b2h :


#-----------------------------------------------------------------------------
endif #InProcMake	# This line must be at EOF

