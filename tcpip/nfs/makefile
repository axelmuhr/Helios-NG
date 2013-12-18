#-----------------------------------------------------------------------------
# Helios generic make system - GENERIC COMPONENT MAKEFILE -
#-----------------------------------------------------------------------------
# Helios generic component make rules for: tcpip/nfs
#
# File: /HSRC/tcpip/nfs/makefile
#
# These are the host/processor independent rules and variables for this
# component of Helios. It is included by the processor specific makefile
# That may add to some of its functionality. You should only alter this
# makefile by using "ifneq ($(HPROC),YYY)" clauses for a specific processor.
#
# SccsId: %W% %G%
# RcsId: $Id: makefile,v 1.16 1993/04/15 15:54:37 paul Exp $
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
COMPONENT = tcpip/nfs

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
vpath %.x $(HVPATH)

#-----------------------------------------------------------------------------
# Generic component make rules and variables added below here

# Note all final generic targets here
# For now, the system cannot be built under Helios because rpcgen and
# sed are not part of the host utilities

ifeq ($(HHOST),HELIOSTRAN)
 TARGETS =
else
 TARGETS = nfsx hmount
endif

default : $(TARGETS) Xdefault

install	: installdef $(TARGETS) Xinstall
ifneq ($(HHOST),HELIOSTRAN)
	$(CP) nfsx $(LIB)/nfs
	$(CP) hmount $(BIN)
endif
	$(TOUCH) install

installdef:	# If this is a library, enter all .def files here (to install)
#	$(CP) $^ $(LIB)
#	$(TOUCH) installdef

clean	: Xclean
	-$(RM)	$(OBJECTS) $(TARGETS) $(XOBJECTS) $(XTARGETS) mkout \
		core ../mkout install installdef Xinstall \
		$(SOURCES.RPC) $(HEADERS.RPC) hmount.o


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
OBJECTS	= $(SOURCES.c:.c=.o) $(SOURCES.a:.a=.o) $(SOURCES.RPC:.c=.o)

# C source files
SOURCES.c	= server.c mount.c dir_server.c file_server.c

# AMPP source files
SOURCES.a	=

# C header files
HEADERS.c	= nfs.h

# AMPP macro files
HEADERS.m	=

# SUN RPC protocol files
SOURCES.x	= nfs_prot.x mount.x
SOURCES.RPC	= nfs_prot_clnt.c nfs_prot_xdr.c mount_clnt.c mount_xdr.c
HEADERS.RPC	= nfs_prot.h mount.h

# Any other source files
SOURCES.odd	= makefile hmount.c


#-----------------------------------------------------------------------------
# Other generic component variables:

NCFLAGS := $(NCFLAGS) -D_BSD

# Set any flags required
ifeq ($(HPROC),ARM)
  NCFLAGS := $(NCFLAGS) -i. -wd
else
  ifeq ($(HPROC),C40)
    NCFLAGS := $(NCFLAGS) -i. -wd
  else
    ifdef SYSDEB
      NCFLAGS := $(NCFLAGS) -ps1
    endif
    NCFLAGS := -wasdfv -ecpz $(NCFLAGS) 
  endif
endif

# Note any library def files to be linked with
LIBDEFS	:=

LINKFLAGS := $(LINKFLAGS)

#-----------------------------------------------------------------------------
# Generic targets and their dependencies:

# uncomment if you have compiler or makedepend generated dependencies
#include depends

$(SOURCES.c) : $(HEADERS.RPC) nfs.h
$(SOURCES.RPC) : $(HEADERS.RPC)
$(HEADERS.RPC) $(SOURCES.RPC) : $(SOURCES.x)

nfsx : $(OBJECTS)
	$(LINK.CPROGRAM) -nnfs -l$(LIB)/rpclib.lib -l$(LIB)/bsd.lib -l$(LIB)/helios.lib

%_clnt.c : %.x
ifeq ($(HPROC),TRAN)
	rpcgen -l -o $@ $*.x
else
	co -u $@
endif

%_xdr.c : %.x
ifeq ($(HPROC),TRAN)
	rpcgen -c -o $@ $*.x
else
	co -u $@
endif

%.h : %.x
ifeq ($(HPROC),TRAN)
	rpcgen -h $*.x | sed "/WFLUSH = 99,/s/,//; /NFFIFO = 8,/s/,//" >$@
else
	co -u $@
endif


hmount : hmount.o mount_clnt.o mount_xdr.o
	$(LINK.CPROGRAM) -l$(LIB)/rpclib.lib -l$(LIB)/bsd.lib


#-----------------------------------------------------------------------------
endif #InProcMake	# This line must be at EOF
