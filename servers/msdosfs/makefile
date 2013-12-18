#-----------------------------------------------------------------------------
# Helios generic make system - GENERIC COMPONENT MAKEFILE -
#-----------------------------------------------------------------------------
# Helios generic component make rules for:
#					Floppy disc controller driver
#					FDC Test program
# File: /HSRC/servers/msdosfs/makefile
#
# These are the host/processor independent rules and variables for this
# component of Helios. It is included by the processor specific makefile
# That may add to some of its functionality. You should only alter this
# makefile by using "ifneq ($(HPROC),YYY)" clauses for a specific processor.
#
# SccsId: %W% %G%
# RcsId: $Id: makefile,v 1.4 1991/04/02 16:29:49 martyn Exp $
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
COMPONENT = servers/msdosfs

# Manditory includes
include $(HSRC)/makeinc/$(HHOST).mak	# Default host system variables
include $(HSRC)/makeinc/$(HPROC).mak	# Default processor rules and vars.

# Manditory phoney targets
.PHONY: default clean Xdefault Xclean
# "default"    - should make this component in its proc subdir
# "install"    - copy the new component to $(HPROD) production directory
# "clean"      - remove all intermediate and executable files from proc subdir
# Phoney targets prefixed by X are for adding processor specific targets.
#
# Processor specific sources are held in the processor specific subdir.,
# generic ones in the directory above it.
VPATH := $(HSRC)/$(COMPONENT):$(HSRC)/servers/common/$(HPROC)

#-----------------------------------------------------------------------------
# Generic component make rules and variables added below here

# Note all final generic targets here
ifeq ($(HPROC),TRAN)
 TARGETS = dos msfloppy #dostest
else
 TARGETS = dos msfloppy #dostest
endif

default : $(TARGETS) Xdefault

install	: $(TARGETS) Xinstall
	$(CP) dos $(LIB)
	$(CP) msfloppy $(BIN)
	$(TOUCH) install

clean	: Xclean
	-$(RM)	$(OBJECTS) $(TARGETS) $(XOBJECTS) $(XTARGETS) mkout \
		core ../mkout install Xinstall


#-----------------------------------------------------------------------------
# for archive, backup and cleaning purposes,
# note all generic SOURCE and OBJECT files:

# Note all sources
SOURCES		= $(SOURCES.c) $(SOURCES.s) \
		  $(HEADERS.c) $(HEADERS.m) \
		  $(SOURCES.odd)

OBJECTS	= $(SOURCES.c:.c=.o) $(SOURCES.s:.s=.o)

# C source files
SOURCES.c	= msdosfs.c data.c dir.c fat.c misc.c path.c read.c write.c \
		  msfloppy.c

# HOBJASM source files
SOURCES.s	= 

# C header files
HEADERS.c	= msdosfs.h prototype.h

# AMPP macro files
HEADERS.m	=

# Any other source files
SOURCES.odd	= makefile


#-----------------------------------------------------------------------------
# Other generic component variables:

# Set any flags required
ifeq ($(HPROC),ARM)
  NCFLAGS := $(NCFLAGS) -Zps0 -DVERSION=\"1.0a\" -DDEVICE_DIRECTORY=\"/files/helios/lib/fdcdev.d\" #-Ddebug -DdebugE
  DEFS     := ${LIB}/cstart.o ${LIB}/fault.def ${LIB}/servlib.def \
           ${LIB}/abclib.def
else
  NCFLAGS := $(NCFLAGS)	-ps0 -DVERSION=\"1.0a\" -DDEVICE_DIRECTORY=\"/helios/lib/fdcdev.d\" #-Ddebug -DdebugE
  DEFS     := ${LIB}/cstart.o ${LIB}/fault.def ${LIB}/servlib.def
endif

# Note any library def files to be linked with
LINKFLAGS := $(LINKFLAGS)


COMMON   := ../../common/$(HPROC)#	# Common objects
COMMONS  := ../../common#		# Common sources

%s : %a		# Cancel std rul AMPP->AS


#------------------------------------------------------------------------------
# Generic targets and their dependencies:


${COMMON}/modend.o:	${COMMONS}/modend.s
			make -C $(COMMON) modend.o

#-----------------------------------------------------------------------------


#------------------------------------------------------------------------------
 
dos:	msdosfs.o data.o dir.o fat.o misc.o path.o read.o write.o
	${LINK} -o$@ ${DEFS} $^

msfloppy:	msfloppy.o
	${LINK} -o$@ -s50000 ${DEFS} $^

#dostest: dostest.o
#	${LINK} -o$@ -s50000 ${DEFS} $^

msdosfs.o: msdosfs.h prototype.h ${INCLUDE}/dev/fdcdev.h

data.o: msdosfs.h ${INCLUDE}/dev/fdcdev.h

dir.o: msdosfs.h prototype.h ${INCLUDE}/dev/fdcdev.h

fat.o: msdosfs.h prototype.h ${INCLUDE}/dev/fdcdev.h

misc.o: msdosfs.h prototype.h ${INCLUDE}/dev/fdcdev.h

path.o: msdosfs.h prototype.h ${INCLUDE}/dev/fdcdev.h

read.o: msdosfs.h prototype.h ${INCLUDE}/dev/fdcdev.h

write.o: msdosfs.h prototype.h ${INCLUDE}/dev/fdcdev.h

msfloppy.o: ${INCLUDE}/dev/fdcdev.h

#-----------------------------------------------------------------------------
endif #InProcMake	# This line must be at EOF

