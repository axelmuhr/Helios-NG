#-----------------------------------------------------------------------------
# Helios generic make system - GENERIC COMPONENT MAKEFILE -
#-----------------------------------------------------------------------------
# Helios generic component make rules for: tcpip/devs/in-ether
#
# File: /HSRC/tcpip/devs/in-ether/makefile
#
# These are the host/processor independent rules and variables for this
# component of Helios. It is included by the processor specific makefile
# That may add to some of its functionality. You should only alter this
# makefile by using "ifneq ($(HPROC),YYY)" clauses for a specific processor.
#
# SccsId: %W% %G%
# RcsId: $Id: makefile,v 1.10 1992/08/17 11:20:04 paul Exp $
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
COMPONENT = tcpip/devs/in-ether

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
TARGETS = in-ether.d b407.b2h #in-ether0.d in-ether1.d in-ether2.d in-ether3.d 


default : $(TARGETS) Xdefault

install	:  $(TARGETS) Xinstall
	$(CP) $(TARGETS) $(LIB)
	$(TOUCH) install

clean	: Xclean
	-$(RM)	$(OBJECTS) in-ether.d $(XOBJECTS) $(XTARGETS) mkout \
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
SOURCES.c	= b407.c b407dev.c

# AMPP source files
SOURCES.a	= devs.a modend.a

# C header files
HEADERS.c	= b407.h b407dev.h

# AMPP macro files
HEADERS.m	= device.m

# Any other source files
SOURCES.odd	= makefile etherd.mak b407.b2h


#-----------------------------------------------------------------------------
# Other generic component variables:

# Set any flags required
ifeq ($(HPROC),ARM)
  NCFLAGS := $(NCFLAGS)
else
  ifeq ($(HPROC),C40)
    NCFLAGS := $(NCFLAGS) -Ff -zps1 -Zr
  else
    NCFLAGS := $(NCFLAGS) -ps1 -pf0 -r
  endif
endif

# Note any library def files to be linked with
LIBDEFS	:= $(LIB)/kernel.def $(LIB)/syslib.def $(LIB)/util.def $(LIB)/posix.def

ifeq ($(HPROC),C40)
  LINKFLAGS := $(LINKFLAGS)
else
  LINKFLAGS := $(LINKFLAGS) -v -f
endif

#-----------------------------------------------------------------------------
# Generic targets and their dependencies:

# uncomment if you have compiler or makedepend generated dependencies
#include depends

ifdef never
b407dev0.o : b407dev.c
	$(NC) -DB407_LINK0 $(NCFLAGS) $(DEFINES) $< -s$(TEMP)/$*.s
	$(ASM) $(ASMFLAGS) -o $@ $(TEMP)/$*.s
	-$(RMTEMP) $(TEMP)/$*.s

b407dev1.o : b407dev.c
	$(NC) -DB407_LINK1 $(NCFLAGS) $(DEFINES) $< -s$(TEMP)/$*.s
	$(ASM) $(ASMFLAGS) -o $@ $(TEMP)/$*.s
	-$(RMTEMP) $(TEMP)/$*.s

b407dev2.o : b407dev.c
	$(NC) -DB407_LINK2 $(NCFLAGS) $(DEFINES) $< -s$(TEMP)/$*.s
	$(ASM) $(ASMFLAGS) -o $@ $(TEMP)/$*.s
	-$(RMTEMP) $(TEMP)/$*.s

b407dev3.o : b407dev.c
	$(NC) -DB407_LINK3 $(NCFLAGS) $(DEFINES) $< -s$(TEMP)/$*.s
	$(ASM) $(ASMFLAGS) -o $@ $(TEMP)/$*.s
	-$(RMTEMP) $(TEMP)/$*.s
endif

in-ether.d :	devs.p b407dev.o b407.o modend.p
	$(LINK) $(LINKFLAGS) -n in-ether.d -o $@ $^ $(LIBDEFS)

ifdef never
in-ether0.d :	devs.p b407dev0.o b407.o modend.p
	$(LINK) $(LINKFLAGS) -n in-ether0.d -o $@ $^ $(LIBDEFS)

in-ether1.d :	devs.p b407dev1.o b407.o modend.p
	$(LINK) $(LINKFLAGS) -n in-ether1.d -o $@ $^ $(LIBDEFS)

in-ether2.d :	devs.p b407dev2.o b407.o modend.p
	$(LINK) $(LINKFLAGS) -n in-ether2.d -o $@ $^ $(LIBDEFS)

in-ether3.d :	devs.p b407dev3.o b407.o modend.p
	$(LINK) $(LINKFLAGS) -n in-ether3.d -o $@ $^ $(LIBDEFS)
endif
b407.b2h :


#-----------------------------------------------------------------------------
endif #InProcMake	# This line must be at EOF

