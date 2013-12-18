#-----------------------------------------------------------------------------
# Helios generic make system - GENERIC COMPONENT MAKEFILE -
#-----------------------------------------------------------------------------
# Helios generic component make rules for: filesys/fs
#
# File: /HSRC/filesys/fs/makefile
#
# These are the host/processor independent rules and variables for this
# component of Helios. It is included by the processor specific makefile
# That may add to some of its functionality. You should only alter this
# makefile by using "ifneq ($(HPROC),YYY)" clauses for a specific processor.
#
# SccsId: %W% %G%
# RcsId: $Id: makefile,v 1.6 1991/10/01 09:42:54 martyn Exp $
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
COMPONENT = filesys/fs

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
TARGETS = fs

default : $(TARGETS) Xdefault

install	: $(TARGETS) Xinstall
	$(CP) $(TARGETS) $(HPROD)/lib
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

OBJECTS	= $(SOURCES.c:.c=.o) $(SOURCES.a:.a=.o)

# C source files
SOURCES.c	= bio.c alloc.c inode.c fsyscall.c deal_fs.c fserver.c \
                  fservlib.c dev.c tserver.c checker.c dircheck.c \
		  xtdcheck.c concheck.c condups.c tidyup.c misc_chk.c \
		  fstring.c

# AMPP source files
SOURCES.a	=

# C header files
HEADERS.c	= nfs.h check.h fservlib.h buf.h fs.h inode.h fstring.h

# AMPP macro files
HEADERS.m	=

# Any other source files
SOURCES.odd	= makefile rdevinfo.c


#-----------------------------------------------------------------------------
# Other generic component variables:

# Set any flags required
ifeq ($(HPROC),ARM)
  NCFLAGS := $(NCFLAGS)
else
  NCFLAGS := $(NCFLAGS) -ps1 -DBUFCSUM
endif

# Note any library def files to be linked with
LIBDEFS	:= $(LIB)/kernel.def $(LIB)/syslib.def $(LIB)/util.def

LINKFLAGS := $(LINKFLAGS) -s20000 -v -f 

#-----------------------------------------------------------------------------
# Generic targets and their dependencies:

# header dependencies

bio.o 		: nfs.h
inode.o 	: nfs.h
tserver.o 	: nfs.h
dev.o 		: nfs.h
fsyscall.o 	: nfs.h
deal_fs.o 	: nfs.h rdevinfo.c
fserver.o 	: nfs.h
alloc.o 	: nfs.h
fservlib.o 	: nfs.h

fstring.o	: fstring.h

checker.o 	: check.h 
dircheck.o 	: check.h
xtdcheck.o 	: check.h
concheck.o 	: check.h
condups.o 	: check.h 
tidyup.o 	: check.h 
misc_chk.o 	: check.h

nfs.h		: fservlib.h buf.h fs.h inode.h
check.h		: nfs.h


# The File Server itself

fs : $(OBJECTS)
	$(LINK) $(LINKFLAGS) $(LIB)/s0.o $(LIBDEFS) $(OBJECTS) -o fs


#-----------------------------------------------------------------------------
endif #InProcMake	# This line must be at EOF

