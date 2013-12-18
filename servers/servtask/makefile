#-----------------------------------------------------------------------------
# Helios generic make system
#-----------------------------------------------------------------------------
# Helios generic component make rules for: server task
#
# File: /HSRC/servers/servtask/makefile
#
# These are the host/processor independent rules and variables for this
# component of Helios. It is included by the processor specific makefile
# That may add to some of its functionality. You should only alter this
# makefile by using "ifneq ($(HPROC),YYY)" clauses for a specific processor.
#
# SccsId: %W% %G%
# RcsId: $Id: makefile,v 1.7 1991/03/07 19:14:56 paul Exp $
#		(C) Copyright 1990 Perihelion Software
#
# WARNING: assumes you are using GNUmake.
#-----------------------------------------------------------------------------

ifndef InProcMake
# This part of the makefile is run in the components generic source directory

# The variables HPROC, HHOST, HSRC, and HPROD should already be set by 
# your make script or environment, if not then we get local defaults from
# ../makeinc/DFLT.mak. Note that the following pathname cannot use $(HSRC).
ifndef HPROC
include ../../makeinc/DFLT.mak
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
COMPONENT = servers/servtask

# Manditory includes
include $(HSRC)/makeinc/$(HHOST).mak	# Default host system variables
include $(HSRC)/makeinc/$(HPROC).mak	# Default processor rules and vars.

# Manditory phoney targets
.PHONY: default clean Xdefault Xclean syslib.i syslib.def
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
SSTART := $(LIB)/s0.o
CSTART := $(LIB)/c0.o

# Note all final generic targets here
ifeq ($(HPROC),TRAN)
TARGETS := slumber servtask.i
else
TARGETS :=
endif

default : $(TARGETS) Xdefault

install	: installdef $(TARGETS) Xinstall
ifeq ($(HPROC),TRAN)
	$(CP) servtask.i $(LIB)
	$(CP) slumber $(BIN)/slumber
endif
	$(TOUCH) install

installdef: 
	$(TOUCH) installdef

clean	: Xclean
	-$(RM) $(OBJECTS) $(TARGETS) $(XOBJECTS) $(XTARGETS) mkout \
		core ../mkout install installdef Xinstall

#-----------------------------------------------------------------------------
# for archive, backup and cleaning purposes,
# note all generic SOURCE and OBJECT files:

# Note all sources
SOURCES		= $(SOURCES.c) $(SOURCES.a) \
		   $(HEADERS.c) $(HEADERS.m) \
		   $(SOURCES.odd)

# Full set of object files - cheat a little - all possibles
OBJECTS		= $(SOURCES.c:.c=.p) $(SOURCES.a:.a=.p) \
		  $(SOURCES.c:.c=.o) $(SOURCES.a:.a=.o) \
		  $(SOURCES.c:.c=.si) $(SOURCES.c:.c=.sa) \
		  $(SOURCES.c:.c=.dl)

SOURCES.c 	= servtask.c window.c slumber.c	

# AMPP source files
SOURCES.a 	= 	

# C header files
HEADERS.c	= keymap.h version.h servtask.h window.h

# AMPP macro files
HEADERS.m	=

# Any other source files
SOURCES.odd	= makefile


#-----------------------------------------------------------------------------
# Other generic component variables:

# Other possibles are:
DEFINES	:= $(DEFINES) 

# Set any flags required
ifeq ($(HPROC),ARM)
  NCFLAGS := $(NCFLAGS) -Zps1
else
 ifeq ($(HPROC),TRAN)
  NCFLAGS := $(NCFLAGS) -ps1 -t4
  LINKFLAGS := $(LINKFLAGS) -f
 else
  NCFLAGS := $(NCFLAGS) -ps1
 endif
endif

# Note any library def files to be linked with

#-----------------------------------------------------------------------------
# Extra server compilation rules:

#-----------------------------------------------------------------------------
# Generic targets and their dependencies:

#-----------------------------------------------------------------------------
# A nucleus server:

servtask.i : $(SSTART) servtask.o window.o 
ifeq ($(HPROC),ARM)
	$(LINK) $(LINKFLAGS) -l$(LIB)/helios.lib -nServerTask -s4000 -h50000 $^ -o$@
else
	$(LINK) $(LINKFLAGS) -l$(LIB)/helios.lib -nServerTask -s2000 -h60000 $^ -o$@
endif


# Slumber Command:

slumber :	$(CSTART) slumber.o 
	$(LINK) $(LINKFLAGS) -l$(LIB)/helios.lib -l$(LIB)/c.lib -nslumber $^ -o$@


#-----------------------------------------------------------------------------
endif #InProcMake	# This line must be at EOF
