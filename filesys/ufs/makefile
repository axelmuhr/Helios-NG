#-----------------------------------------------------------------------------
# Helios generic make system - GENERIC COMPONENT MAKEFILE -
#-----------------------------------------------------------------------------
# Helios generic component make rules for: filesys/ufs
#
# File: /HSRC/filesys/ufs/makefile
#
# These are the host/processor independent rules and variables for this
# component of Helios. It is included by the processor specific makefile
# That may add to some of its functionality. You should only alter this
# makefile by using "ifneq ($(HPROC),YYY)" clauses for a specific processor.
#
# SccsId: %W% %G%
# RcsId: $Id: makefile,v 1.3 1992/10/16 11:13:19 al Exp $
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
COMPONENT = filesys/ufs

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
TARGETS = ufs

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
SOURCES.c	= ufs_alloc.c ufs_bmap.c ufs_disksubr.c ufs_inode.c \
                  ufs_lockf.c ufs_lookup.c ufs_quota.c ufs_subr.c ufs_tables.c \
		  ufs_vfsops.c ufs_vnops.c vfs_bio.c vfs_cache.c vfs_lookup.c \
		  vfs_subr.c vfs_syscalls.c vfs_vnops.c dead_vnops.c \
		  spec_vnops.c sys_generic.c kern_descrip.c \
                  machine.c sys.c unix.c kmalloc.c kern_subr.c conf.c mem.c \
                  hd.c devinfo.c server.c dispatch.c services.c syscall.c \
		  fileserv.c dirserv.c private.c

# AMPP source files
SOURCES.a	=

# C header files
HEADERS.c	= dinode.h dir.h dispatch.h fs.h inode.h lockf.h \
		  private.h quota.h ufsmount.h

# AMPP macro files
HEADERS.m	=

# Any other source files
SOURCES.odd	= makefile


#-----------------------------------------------------------------------------
# Other generic component variables:

# Ensure local directories searched 1st by placing at beginning (last searched 1st)
UFS := $(HSRC)/filesys/ufs
INCLUDES := $(UFS)/include/sys/,$(UFS)/include/,$(UFS)/,$(INCLUDE)/,$(INCLUDE)/sys/,../
#INCLUDES := $(INCLUDE)/sys/,$(INCLUDE)/,../,$(UFS)/include/sys/,$(UFS)/include/,$(UFS)/

# Set any flags required
ifeq ($(HPROC),ARM)
  NCFLAGS := $(NCFLAGS) -I$(INCLUDES)
else
ifeq ($(HPROC),TRAN)
  NCFLAGS := -wadfpsz $(NCFLAGS) $(DEFINES) -i$(INCLUDES) -j$(INCLUDES) -D_BSD -D_BSD_MACROS -DKERNEL -Dvax
#  NCFLAGS := $(DEFINES) -i$(INCLUDES) -j$(INCLUDES) -D_BSD -D_BSD_MACROS -DKMEMSTATS -DKERNEL -Dvax -DSHOWCALLS -DDEBUG -DNAMEI_DIAGNOSTIC -DMONITOR
#  NCFLAGS := $(DEFINES) -i$(INCLUDES) -j$(INCLUDES) -D_BSD -D_BSD_MACROS -DKMEMSTATS -DKERNEL -Dvax -DDEBUG -DSHOWCALLS
else
ifeq ($(HPROC),C40)
  NCFLAGS := -Zpc1 -wanpvdf $(NCFLAGS) $(DEFINES) -i$(INCLUDES) -j$(INCLUDES) -D_BSD -D_BSD_MACROS -DKERNEL -Dvax
else
  error "UNKNOWN PROCESSOR"
endif
endif
endif


# Note any library def files to be linked with
LIBDEFS	:= $(LIB)/kernel.def $(LIB)/syslib.def $(LIB)/util.def

ifeq ($(HPROC),C40)
LINKFLAGS := $(LINKFLAGS) -s20000
else
LINKFLAGS := $(LINKFLAGS) -s20000 -v -f 
endif

#-----------------------------------------------------------------------------
# Generic targets and their dependencies:

# header dependencies



# The File Server itself

ufs : $(OBJECTS)
	$(LINK) $(LINKFLAGS) $(LIB)/s0.o $(LIBDEFS) $(OBJECTS) $(POBJECTS) \
                -l$(LIB)/servlib.def -o ufs -h4000
#	$(LINK) $(LINKFLAGS) $(LIB)/s0.o $(LIBDEFS) $(OBJECTS) $(POBJECTS) \
#                -l$(LIB)/helios.lib -o ufs


#-----------------------------------------------------------------------------
endif #InProcMake	# This line must be at EOF

