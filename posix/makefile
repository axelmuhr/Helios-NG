#-----------------------------------------------------------------------------
# Helios generic make system
#-----------------------------------------------------------------------------
# Helios generic component make rules for: Posix Libarary
#
# File: /HSRC/posix/makefile
#
# These are the host/processor independent rules and variables for this
# component of Helios. It is included by the processor specific makefile
# That may add to some of its functionality. You should only alter this
# makefile by using "ifneq ($(HPROC),YYY)" clauses for a specific processor.
#
# SccsId: %W% %G%
# RcsId: $Id: makefile,v 1.18 1993/12/08 17:33:14 nickc Exp $
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
COMPONENT = posix

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
HVPATH := $(RSRC)/$(COMPONENT)/$(HPROC):$(HVPATH):$(RSRC)/$(COMPONENT)
endif

vpath %.h $(HVPATH)
vpath %.c $(HVPATH)
vpath %.a $(HVPATH)

#-----------------------------------------------------------------------------
# Generic component make rules and variables added below here

# Note all final generic targets here
TARGETS = posix.def posix.i

default : $(TARGETS) Xdefault

install	: installdef $(TARGETS) Xinstall
ifeq ($(HLICENSEE),ABC)
	sysstrip posix.i -o$(LIB)/Posix
else
	$(CP) posix.i $(LIB)/Posix
endif
	$(TOUCH) install

installdef: $(LIB)/posix.def
	$(TOUCH) installdef

$(LIB)/posix.def : posix.def
ifeq (HELIOS,$(findstring HELIOS,$(HHOST))) #csh default on Helios hosts
	(cmp $^ $@ ; if ( $$status ) cp $^ $@ )
else
	@ if cmp $^ $@; then echo "$^ not changed"; else $(CP) $^ $@ ;fi
endif

clean	: Xclean
	-$(RM)	$(OBJECTS) $(TARGETS) $(XOBJECTS) $(XTARGETS) mkout \
		core ../mkout install installdef Xinstall $(SAOBJS)

#-----------------------------------------------------------------------------
# for archive, backup and cleaning purposes,
# note all generic SOURCE and OBJECT files:

# Note all sources
SOURCES		= $(SOURCES.c) $(SOURCES.a) \
		   $(HEADERS.c) $(HEADERS.m) \
		   $(SOURCES.odd)

#Full set of library object files
#OBJECTS	= $(SOURCES.c:.c=.p) $(SOURCES.a:.a=.p)
# Or if you are not a library:
#OBJECTS	= $(SOURCES.c:.c=.o) $(SOURCES.a:.a=.o)

# C source files
SOURCES.c	= dir.c env.c exec.c fileio.c pwd.c saposix.c scandb.c \
			signal.c socket.c termios.c

# AMPP source files
SOURCES.a	=  pstart.a posixasm.a modend.a saposasm.a

# C header files
HEADERS.c	= pposix.h

# AMPP macro files
HEADERS.m	=

# Any other source files
SOURCES.odd	= makefile


#-----------------------------------------------------------------------------
# Other generic component variables:

# Set any flags required. Possible flags are:
# 	-DSTACKEXTENSION	signal handling thread has small stack
#				N.B. THIS REQUIRES STACKCHECKING 

ifeq ($(HPROC),TRAN)
    NCFLAGS := $(NCFLAGS) -ps1 -pf0 -j$(POSIX)/include/,$(INCLUDE)/ -DPOSIX
else
  ifeq ($(HPROC),C40)
    NCFLAGS := $(NCFLAGS) -Zpw1 -j$(POSIX),$(INCLUDE) -DPOSIX -DSTACKEXTENSION
  else
    ifeq ($(HPROC),ARM)
# -Zt = tiny memory model.
      NCFLAGS := $(NCFLAGS) -j$(POSIX),$(INCLUDE) -DPOSIX -Zt
    else
      ifeq ($(HPROC),ARM6)
        NCFLAGS := $(NCFLAGS) -j$(POSIX),$(INCLUDE) -DPOSIX -Zt
      endif
    endif
  endif
endif

# Note any library def files to be linked with
LIBDEFS	:= $(LIB)/kernel.def $(LIB)/syslib.def $(LIB)/util.def


#-----------------------------------------------------------------------------
# Generic targets and their dependencies:

# uncomment if you have compiler or makedepend generated dependencies
#include depends

OBJECTS = pstart.p posixasm.p fileio.p dir.p pwd.p env.p exec.p signal.p \
	termios.p socket.p scandb.p modend.p

posix.i : $(OBJECTS) $(LIBDEFS)
	$(LINK.LIBRARY) -nPosix $^ -oposix.i


posix.def: pstart.a
	$(MAKE.DEF)


#-----------------------------------------------------------------------------
# Standalone library support

ifeq ($(HPROC),TRAN)

SAOBJS = pstart.p saposasm.p saposix.p modend.p

posix.sa : $(SAOBJS)
	$(ASM) -f -nPosix $(SAOBJS) \
	$(LIB)/kernel.def $(LIB)/syslib.def $(LIB)/util.def -oposix.sa
	$(CP) posix.sa $(LIB)
ifdef SYSDEB
	$(CP) posix.sa /helios/lib
endif
else
SAOBJS =
endif

#-----------------------------------------------------------------------------
endif #InProcMake	# This line must be at EOF

