#-----------------------------------------------------------------------------
# Helios generic make system - GENERIC COMPONENT MAKEFILE -
#-----------------------------------------------------------------------------
# Helios generic component make rules for: filesys
#
# File: /HSRC/filesys/makefile
#
# These are the host/processor independent rules and variables for this
# component of Helios. It is included by the processor specific makefile
# That may add to some of its functionality. You should only alter this
# makefile by using "ifneq ($(HPROC),YYY)" clauses for a specific processor.
#
# SccsId: %W% %G%
# RcsId: $Id: makefile,v 1.7 1991/04/08 12:22:08 martyn Exp $
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

# Manditory variables for a component:
# name of this dir relative to $(HSRC)
COMPONENT = filesys

include $(HSRC)/makeinc/$(HHOST).mak

# Manditory phoney targets
.PHONY: default clean Xdefault Xclean fs cmds raw msc m212 he1000 b422
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

TARGETS = fs cmds raw msc m212 he1000 b422
.PHONY : $(TARGETS)


default : $(TARGETS) Xdefault

# @@@ This is probably a stupid place to copy these files
install	: $(TARGETS) localinstall 
	$(MAKE) -C ../fs/$(HPROC) install
	$(MAKE) -C ../cmds/$(HPROC) install
	$(MAKE) -C ../devs/raw/$(HPROC) install
	$(MAKE) -C ../devs/msc/$(HPROC) install
	$(MAKE) -C ../devs/m212/$(HPROC) install
	$(MAKE) -C ../devs/he1000/$(HPROC) install
	$(MAKE) -C ../devs/b422/$(HPROC) install
	$(TOUCH) install	

localinstall: ../devinfo.src ../msdos/makedisk.exe
	$(RM) -f $(HPROD)/etc/devinfo.src $(HPROD)/makedisk.exe
	$(CP) ../devinfo.src $(HPROD)/etc
	$(CP) ../msdos/makedisk.exe $(HPROD)
	$(TOUCH) localinstall

clean	: Xclean
	-$(RM)	$(OBJECTS) $(TARGETS) $(XOBJECTS) $(XTARGETS) mkout \
		core mkout install installdef Xinstall localinstall
	$(MAKE) -C ../fs/$(HPROC) clean
	$(MAKE) -C ../cmds/$(HPROC) clean
	$(MAKE) -C ../devs/raw/$(HPROC) clean
	$(MAKE) -C ../devs/msc/$(HPROC) clean
	$(MAKE) -C ../devs/m212/$(HPROC) clean
	$(MAKE) -C ../devs/he1000/$(HPROC) clean
	$(MAKE) -C ../devs/b422/$(HPROC) clean


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
OBJECTS	= $(SOURCES.c:.c=.o) $(SOURCES.a:.a=.o)

# C source files
SOURCES.c	=

# AMPP source files
SOURCES.a	=

# C header files
HEADERS.c	=

# AMPP macro files
HEADERS.m	=

# Any other source files
SOURCES.odd	= makefile devinfo.src


#-----------------------------------------------------------------------------
# Other generic component variables:

# Set any flags required
ifeq ($(HPROC),ARM)
  NCFLAGS := $(NCFLAGS)
else
  NCFLAGS := $(NCFLAGS)
endif

# Note any library def files to be linked with
LIBDEFS	:=

LINKFLAGS := $(LINKFLAGS)

#-----------------------------------------------------------------------------
# Generic targets and their dependencies:

# uncomment if you have compiler or makedepend generated dependencies
#include depends

fs	:;	$(MAKE) -C ../fs/$(HPROC)
cmds	:;	$(MAKE) -C ../cmds/$(HPROC)
raw	:;	$(MAKE) -C ../devs/raw/$(HPROC)
msc	:;	$(MAKE) -C ../devs/msc/$(HPROC)
m212	:;	$(MAKE) -C ../devs/m212/$(HPROC)
he1000	:;	$(MAKE) -C ../devs/he1000/$(HPROC)
b422	:;	$(MAKE) -C ../devs/b422/$(HPROC)

#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
endif #InProcMake	# This line must be at EOF

