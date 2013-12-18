#-----------------------------------------------------------------------------
# Helios generic make system
#-----------------------------------------------------------------------------
# Helios generic component make rules for: Utility Library
#
# File: /HSRC/util/makefile
#
# These are the host/processor independent rules and variables for this
# component of Helios. It is included by the processor specific makefile
# That may add to some of its functionality. You should only alter this
# makefile by using "ifneq ($(HPROC),YYY)" clauses for a specific processor.
#
#
# SccsId: %W% %G%
# RcsId: $Id: makefile,v 1.17 1993/08/06 12:37:24 bart Exp $
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
COMPONENT = util

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
vpath %.c $(HVPATH)
vpath %.a $(HVPATH)

#-----------------------------------------------------------------------------
# Generic component make rules and variables added below here

# Note all final generic targets here
TARGETS := util.i util.def

default : $(TARGETS) Xdefault


install	: installdef $(TARGETS) Xinstall
ifeq ($(HLICENSEE),ABC)		# depreciated function - delete me
	sysstrip util.i -o$(LIB)/util
else
	$(CP) util.i $(LIB)/util
endif
	$(TOUCH) install


installdef: $(LIB)/util.def
	$(TOUCH) installdef

$(LIB)/util.def : util.def
ifeq (HELIOS,$(findstring HELIOS,$(HHOST))) #csh default on Helios hosts
	(cmp $^ $@ ; if ( $$status ) cp $^ $@ )
else
	@ if cmp $^ $@; then echo "$^ not changed"; else $(CP) $^ $@ ;fi
endif


clean	: Xclean
	-$(RM)	$(OBJECTS) $(TARGETS) $(XOBJECTS) $(XTARGETS) *.s \
		mkout core ../mkout install installdef Xinstall 

#-----------------------------------------------------------------------------
# for archive, backup and cleaning purposes,
# note all generic SOURCE and OBJECT files:

# Note all sources
SOURCES		= $(SOURCES.c) $(SOURCES.a) \
		   $(HEADERS.c) $(HEADERS.m) \
		   $(SOURCES.odd)

#Full set of library object files
OBJECTS		= $(SOURCES.c:.c=.p) $(SOURCES.a:.a=.p)
# Or if you are not a library:
#OBJECTS	= $(SOURCES.c:.c=.o) $(SOURCES.a:.a=.o)

# C source files
SOURCES.c	=  misc.c

# AMPP source files
SOURCES.a	= utilstar.a utilasm.a modend.a

# C header files
HEADERS.c	=

# AMPP macro files
HEADERS.m	=

# Any other source files
SOURCES.odd	= makefile


#-----------------------------------------------------------------------------
# Other generic component variables:

# Set any flags required
# Possible flags are:
#	-DSTACKCHECK
#
ifeq ($(HPROC),TRAN)
    NCFLAGS := $(NCFLAGS) -ps1
endif
ifeq ($(HPROC),C40)
# -Zpw1 = little data < 64k static data optimisation
    NCFLAGS := $(NCFLAGS) -Zpw1 -DSTACKCHECK
endif
ifeq ($(HPROC),ARM)
# -Zt = tiny memory model
    NCFLAGS := $(NCFLAGS) -Zt
endif

# Note any library def files to be linked with
LIBDEFS	:= $(LIB)/kernel.def $(LIB)/syslib.def


#-----------------------------------------------------------------------------
# Generic targets and their dependencies:

# uncomment if you have compiler or makedepend generated dependencies
#include depends

ifeq ($(HPROC),C40)
 UTIL.OBJECTS = utilstar.p utilasm.p misc.p string.p c40string.p modend.p
else
 ifeq ($(HPROC),ARM)
  UTIL.OBJECTS = utilstar.p utilasm.p misc.p string.p pseudtrap.p modend.p
 else
  UTIL.OBJECTS = utilstar.p utilasm.p misc.p string.p modend.p
 endif
endif

util.i: $(UTIL.OBJECTS) $(LIBDEFS)
	$(LINK.LIBRARY) -nUtil $^ -o$@

util.def: utilstar.a
	$(MAKE.DEF)



endif #InProcMake	# This line must be at EOF
