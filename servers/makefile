#-----------------------------------------------------------------------------
# Helios generic make system - GENERIC COMPONENT MAKEFILE -
#-----------------------------------------------------------------------------
# Helios generic component make rules for: Servers
#
# File: /HSRC/servers/makefile
#
# These are the host/processor independent rules and variables for this
# component of Helios. It is included by the processor specific makefile
# That may add to some of its functionality. You should only alter this
# makefile by using "ifneq ($(HPROC),YYY)" clauses for a specific processor.
#
# SccsId: %W% %G%
# RcsId: $Id: makefile,v 1.16 1994/02/17 09:47:18 nickc Exp $
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

# Note ALL remote make targets here:
.PHONY: notarget ttyserv servtask codec serial fdc msdosfs window
notarget:
	@$(MAKE) -C $(HPROC)

.DEFAULT:
	@$(MAKE) -C $(HPROC) $@

else
#-----------------------------------------------------------------------------
# This part of the makefile is always run in a processor specific subdirectory.

# Manditory variables for a component:
# name of this dir relative to $(HSRC)
COMPONENT = servers

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

TARGETS = lock include ttyserv

ifeq ($(HLICENSEE),TELMAT)
TARGETS := $(TARGETS) msdosfs
endif

ifeq ($(HLICENSEE),ABC)
TARGETS := $(TARGETS) serial fdc codec keyboard window \
# @@@ to be added:	helios romdisk ramdisk
endif

ifeq ($(HPROC),TRAN)
TARGETS := $(TARGETS) servtask
endif


default : Xdefault
	$(foreach SERVER, $(TARGETS), $(MAKE) -C ../$(SERVER)/$(HPROC);)


install	: Xinstall $(TARGETS)
	test -d $(HPROD)/users/guest || mkdir -p $(HPROD)/users/guest
	test -d $(HPROD)/users/guest/examples || mkdir $(HPROD)/users/guest/examples
	test -d $(HPROD)/users/guest/examples/servers || mkdir $(HPROD)/users/guest/examples/servers
	$(foreach SERVER, $(TARGETS), $(MAKE) -C ../$(SERVER)/$(HPROC) install;)
	$(TOUCH) install


clean	: Xclean
	-$(RM) mkout core ../mkout install installdef Xinstall
	$(foreach SERVER, $(TARGETS), $(MAKE) -C ../$(SERVER)/$(HPROC) clean;)
	$(MAKE) -C ../common/$(HPROC) clean;


#-----------------------------------------------------------------------------
# for archive, backup and cleaning purposes,
# note all generic SOURCE and OBJECT files that are not in individual server
# directories.

# Note all sources
SOURCES		= $(SOURCES.c) $(SOURCES.a) \
		  $(HEADERS.c) $(HEADERS.m) \
		  $(SOURCES.odd)

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
SOURCES.odd	= makefile


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

# Start and completion messages:
inmessage:	; @echo "                                 [[[ Making the Helios/$(HPROC) servers and device drivers ]]]"
outmessage:	; @echo "                                 [[[ Helios/$(HPROC) servers and device drivers constructed ]]]"

#-----------------------------------------------------------------------------
# Individual targets go here:



#-----------------------------------------------------------------------------
# Individual servers in their own directories go here:

msdosfs:
	$(MAKE) -C ../msdosfs/$(HPROC)

lock:
	$(MAKE) -C ../lock/$(HPROC)

include:
	$(MAKE) -C ../include/$(HPROC)

ifeq ($(HPROC),ARM)
# Following servers are copyright (C) 1990, ABC:

serial:
	$(MAKE) -C ../serial/$(HPROC)

fdc:
	$(MAKE) -C ../fdc/$(HPROC)

codec:
	$(MAKE) -C ../codec/$(HPROC)
endif


keyboard:
	$(MAKE) -C ../keyboard/$(HPROC)


ttyserv:
	$(MAKE) -C ../ttyserv/$(HPROC)


ifeq ($(HPROC),TRAN)
servtask:
	$(MAKE) -C ../servtask/$(HPROC)
endif

ifeq ($(HPROC),ARM)
window:
	$(MAKE) -C ../window/$(HPROC)
endif

#-----------------------------------------------------------------------------
endif #InProcMake	# This line must be at EOF
