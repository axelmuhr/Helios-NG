#-----------------------------------------------------------------------------
# Helios generic make system - GENERIC COMPONENT MAKEFILE -
#-----------------------------------------------------------------------------
# Helios generic component make rules for: sys
#
# File: /HSRC/tcpip/sys/makefile
#
# These are the host/processor independent rules and variables for this
# component of Helios. It is included by the processor specific makefile
# That may add to some of its functionality. You should only alter this
# makefile by using "ifneq ($(HPROC),YYY)" clauses for a specific processor.
#
# SccsId: %W% %G%
# RcsId: $Id: makefile,v 1.8 1993/02/25 18:42:31 paul Exp $
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
COMPONENT = tcpip/sys

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
TARGETS = sockets

default : $(TARGETS) Xdefault

install	: $(TARGETS) Xinstall
	$(CP) $(TARGETS) ../../$(HPROC)
	$(TOUCH) install

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

#Full set of library object files
#OBJECTS		= $(SOURCES.c:.c=.p) $(SOURCES.a:.a=.p)
# Or if you are not a library:
OBJECTS	= $(SOURCES.c:.c=.o) $(SOURCES.a:.a=.o)

# C source files
SOURCES.c	= sys_socket.c uipc_domain.c \
		uipc_socket.c uipc_socket2.c uipc_syscalls.c

# AMPP source files
SOURCES.a	=

# C header files
HEADERS.c	=

# AMPP macro files
HEADERS.m	=

# Any other source files
SOURCES.odd	= makefile
		# makefile.olduipc_mbuf.c uipc_proto.c uipc_usrreq.c
		# C sources not used in helios version (original bsd versions)

#-----------------------------------------------------------------------------
# Other generic component variables:

INCLUDES := $(TCPIP)/h/,$(TCPIP)/include/sys/,$(INCLUDE)/sys/,$(TCPIP)/,$(INCLUDE)/,../

ifdef RSRC
INCLUDES := $(INCLUDES),$(subst $(TCPIP),$(subst $(HSRC),$(RSRC),$(TCPIP)),$(INCLUDES)),$(RSRC)/$(COMPONENT)/
endif

# Set any flags required
ifeq ($(HPROC),ARM)
  NCFLAGS := $(NCFLAGS) -wadfpv -I$(INCLUDES) -J$(INCLUDES) -D_BSD
else
  ifeq ($(HPROC),C40)
    NCFLAGS := $(NCFLAGS) -I$(INCLUDES) -J$(INCLUDES) -D_BSD -wadfpv -Zpl1
  else
    ifndef SYSDEB
      NCFLAGS := $(NCFLAGS) -ps1
    endif
    NCFLAGS := $(NCFLAGS) -wasdfv -ecpz -I$(INCLUDES) -J$(INCLUDES) -D_BSD
  endif
endif

LINKFLAGS := $(LINKFLAGS)

#-----------------------------------------------------------------------------
# Generic targets and their dependencies:


sockets	: $(OBJECTS)
ifeq ($(HPROC),TRAN)
	asm -p -o sockets $(OBJECTS)	
else
	cat $(OBJECTS) > sockets
endif


#-----------------------------------------------------------------------------
endif #InProcMake	# This line must be at EOF

