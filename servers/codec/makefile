#-----------------------------------------------------------------------------
# Helios generic make system - GENERIC COMPONENT MAKEFILE -
#-----------------------------------------------------------------------------
# Helios generic component make rules for: codec device driver
#
# File: /HSRC/servers/codec/makefile
#
# These are the host/processor independent rules and variables for this
# component of Helios. It is included by the processor specific makefile
# That may add to some of its functionality. You should only alter this
# makefile by using "ifneq ($(HPROC),YYY)" clauses for a specific processor.
#
# SccsId: %W% %G%
# RcsId: $Id: makefile,v 1.5 1991/05/31 14:58:57 paul Exp $
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
COMPONENT = servers/codec

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
TARGETS = codec.dev codecHigh.o

default : $(TARGETS) Xdefault

install	: $(TARGETS) Xinstall
	$(CP) $(TARGETS) $(LIB)
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

#Full set of library object files
OBJECTS	= $(SOURCES.c:.c=.o) $(SOURCES.s:.s=.o)

# C source files
SOURCES.c	= codec.c codecHigh.c

# HOBJASM source files
SOURCES.s	= codecprefix.s

# C header files
HEADERS.c	= codec.h codecHigh.h

# AMPP macro files
HEADERS.m	=

# Any other source files
SOURCES.odd	= makefile notes README


#-----------------------------------------------------------------------------
# Other generic component variables:

# Set any flags required
ifeq ($(HPROC),ARM)
  NCFLAGS := $(NCFLAGS)
else
  NCFLAGS := $(NCFLAGS)
endif

# Note any library def files to be linked with
LIBDEFS	:= ${LIB}/kernel.def ${LIB}/syslib.def ${LIB}/util.def \
		${LIB}/abclib.def

LINKFLAGS := $(LINKFLAGS)

COMMON   := ../../common/$(HPROC)#		# common objects
COMMONS  := ../../common#			# common sources

%s : %a		# Cancel std rule AMPP->AS

#-----------------------------------------------------------------------------
# Generic targets and their dependencies:

$(COMMON)/trace.o: $(COMMONS)/trace.c
	${MAKE} -C $(COMMON) trace.o

$(COMMON)/modend.o: $(COMMONS)/modend.s
	${MAKE} -C $(COMMON) modend.o

#------------------------------------------------------------------------------

codec.dev: codecprefix.o codec.o ${COMMON}/trace.o ${COMMON}/modend.o
ifeq ($(HPROC),ARM)
	${LINK} $^ ${LIBDEFS} -o$@
else
endif

codec.o: codec.c codec.h ${INCLUDE}/dev/trace.h
ifeq ($(HPROC),ARM)
	${NC} -J${INCLUDE} -Zps1 -Zr -c $<
else
endif

ASMHDRS =	$(HINCLUDE)/listopts.s  $(HINCLUDE)/arm.s \
		$(HINCLUDE)/basic.s     $(HINCLUDE)/structs.s \
		$(HINCLUDE)/module.s    $(HINCLUDE)/queue.s \
		$(HINCLUDE)/device.s

codecprefix.o: codecprefix.s ${ASMHDRS}
ifeq ($(HPROC),ARM)
	${HOBJASM} $< $@ -hdr ${HINCLUDE}
else
endif

codecHigh.o: codecHigh.c codecHigh.h codec.h ${INCLUDE}/dev/result.h


#-----------------------------------------------------------------------------
endif #InProcMake	# This line must be at EOF

