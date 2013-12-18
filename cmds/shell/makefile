#-----------------------------------------------------------------------------
# Helios generic make system - GENERIC COMPONENT MAKEFILE -
#-----------------------------------------------------------------------------
# Helios generic component make rules for: Helios Shell
#
# File: /HSRC/cmds/shell/makefile
#
# These are the host/processor independent rules and variables for this
# component of Helios. It is included by the processor specific makefile
# That may add to some of its functionality. You should only alter this
# makefile by using "ifneq ($(HPROC),YYY)" clauses for a specific processor.
#
# SccsId: %W% %G%
# RcsId: $Id: makefile,v 1.10 1994/01/13 17:23:37 nickc Exp $
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

.PHONEY: notarget
notarget:
	@$(MAKE) -C $(HPROC)

.DEFAULT:
	@$(MAKE) -C $(HPROC) $@

else
#-----------------------------------------------------------------------------
# This part of the makefile is always run in a processor specific subdirectory.

# Manditory variables for a component:
# name of this dir relative to $(HSRC)
COMPONENT = cmds/shell

# Manditory includes
include $(HSRC)/makeinc/$(HHOST).mak	# Default host system variables
include $(HSRC)/makeinc/$(HPROC).mak	# Default processor rules and vars.

# Manditory phoney targets
.PHONEY: default clean Xdefault Xclean
# "default"    - should make this component in its proc subdir
# "install"    - copy the new component to $(HPROD) production directory
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
TARGETS = shell

default: $(TARGETS) Xdefault

install: $(TARGETS) Xinstall
	$(CP) $(TARGETS) $(BIN)
	$(TOUCH) install

clean: Xclean
	-$(RM)	$(OBJECTS) $(TARGETS) $(XOBJECTS) $(XTARGETS) mkout \
		core ../mkout install Xinstall


#-----------------------------------------------------------------------------
# for archive, backup and cleaning purposes,
# note all generic SOURCE and OBJECT files:

# Note all sources
SOURCES		= $(SOURCES.c) $(SOURCES.a) \
		  $(HEADERS.c) $(HEADERS.m) \
		  $(SOURCES.odd)

OBJECTS		= job.o list.o shell.o builtin.o parse.o line.o sub.o \
	          hash.o error.o extra.o data.o signal.o helios.o

# C source files
SOURCES.c	= amiga.c atari.c builtin.c cdl.c data.c dodebug.c error.c \
		extra.c hash.c helios.c job.c line.c list.c parse.c shell.c \
		signal.c sub.c unix.c

# C header files
HEADERS.c  	= shell.h prototype.h define.h typedef.h cdl.h cdlobj.h \
	          unix.h _helios.h atari.h amiga.h dir.h dodebug.h _errno.h \
		  stat.h types.h csh.h

# AMPP source files
SOURCES.a	=

# AMPP macro files
HEADERS.m	=

# Any other source files + .c files are for other versions of shell
SOURCES.odd	= makefile # dodebug.c atari.c amiga.c


#-----------------------------------------------------------------------------
# Other generic component variables:

# Set any flags required
#	-DHELIOS, -DPOSIX	compulsory
#	-DCDL			interpret ^^, <|, <> and similar CDL funnies
#				N.B. "set cdl" still sends commands to the TFM
ifeq ($(HPROC),ARM)
  NCFLAGS := $(NCFLAGS) -DHELIOS -DPOSIX
endif
ifeq ($(HPROC),C40)
  NCFLAGS := -Zps1 $(NCFLAGS) -DHELIOS -DPOSIX
endif
ifeq ($(HPROC),TRAN)
  NCFLAGS := $(NCFLAGS) -DHELIOS -DPOSIX -DCDL
endif

# Note any library def files to be linked with
LIBDEFS	= $(LIB)/fault.def


#-----------------------------------------------------------------------------
# Generic targets and their dependencies:

shell.o: shell.c dodebug.c cdl.c

$(OBJECTS): shell.h prototype.h define.h typedef.h cdl.h cdlobj.h _helios.h

shell: $(CSTART) $(LIBDEFS) $(OBJECTS)
	$(LINK) $(LINKFLAGS) -o$@ -n$@ -s20000 -h10000 $^



#-----------------------------------------------------------------------------
endif #InProcMake	# This line must be at EOF
