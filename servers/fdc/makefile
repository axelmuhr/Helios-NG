#-----------------------------------------------------------------------------
# Helios generic make system - GENERIC COMPONENT MAKEFILE -
#-----------------------------------------------------------------------------
# Helios generic component make rules for:
#					Floppy disc controller driver
#					FDC Test program
# File: /HSRC/servers/fdc/makefile
#
# These are the host/processor independent rules and variables for this
# component of Helios. It is included by the processor specific makefile
# That may add to some of its functionality. You should only alter this
# makefile by using "ifneq ($(HPROC),YYY)" clauses for a specific processor.
#
# SccsId: %W% %G%
# RcsId: $Id: makefile,v 1.3 1991/05/31 15:01:44 paul Exp $
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
COMPONENT = servers/fdc

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
TARGETS = fdcdev.d fdctest

default : $(TARGETS) Xdefault

install	: $(TARGETS) Xinstall
	$(CP) fdctest $(BIN)
	$(CP) fdcdev.d $(LIB)
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
SOURCES.c	= fdcdev.c fdctest.c

# HOBJASM source files
SOURCES.s	= fdcfiq.s fdcprefix.s

# C header files
HEADERS.c	=

# AMPP macro files
HEADERS.m	=

# Any other source files
SOURCES.odd	= makefile


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

DEFS     := ${LIB}/cstart.o ${LIB}/fault.def ${LIB}/servlib.def \
		${LIB}/abclib.def

COMMON   := ../../common/$(HPROC)#	# Common objects
COMMONS  := ../../common#		# Common sources

%s : %a		# Cancel std rul AMPP->AS


#------------------------------------------------------------------------------
# Generic targets and their dependencies:


${COMMON}/modend.o:	${COMMONS}/modend.s
			make -C $(COMMON) modend.o

#-----------------------------------------------------------------------------

fdcdev.d: fdcprefix.o fdcfiq.o fdcdev.o ${COMMON}/modend.o
	${LINK} $^ ${LIBDEFS} -o$@

fdcdev.o: fdcdev.c ${INCLUDE}/dev/fdcdev.h
ifeq ($(HPROC),ARM)
	${NC} -j$(HSRC)/include/ -Zps0 -Zr -c $<
else
endif

ASMHDRS =	$(HINCLUDE)/listopts.s  $(HINCLUDE)/arm.s \
		$(HINCLUDE)/basic.s     $(HINCLUDE)/structs.s \
		$(HINCLUDE)/module.s    $(HINCLUDE)/queue.s \
		$(HINCLUDE)/device.s    $(HINCLUDE)/exmacros.s \
		$(HINCLUDE)/SWIinfo.s

fdcprefix.o: fdcprefix.s $(ASMHDRS)
ifeq ($(HPROC),ARM)
	${HOBJASM} $< $@ -hdr ${HINCLUDE}
else
endif

fdcfiq.o: fdcfiq.s $(ASMHDRS)
ifeq ($(HPROC),ARM)
	${HOBJASM} $< $@ -hdr ${HINCLUDE} -library
else
endif

#------------------------------------------------------------------------------
 
fdctest: fdctest.o ${INCLUDE}/dev/fdcdev.h
	${LINK} -o$@ -s50000 ${DEFS} $@.o


#-----------------------------------------------------------------------------
endif #InProcMake	# This line must be at EOF

