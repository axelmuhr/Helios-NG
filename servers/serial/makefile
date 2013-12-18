#-----------------------------------------------------------------------------
# Helios generic make system - GENERIC COMPONENT MAKEFILE -
#-----------------------------------------------------------------------------
# Helios generic component make rules for: Serial server and driver
#
# File: /HSRC/servers/serial/makefile
#
# These are the host/processor independent rules and variables for this
# component of Helios. It is included by the processor specific makefile
# That may add to some of its functionality. You should only alter this
# makefile by using "ifneq ($(HPROC),YYY)" clauses for a specific processor.
#
# SccsId: %W% %G%
# RcsId: $Id: makefile,v 1.3 1991/05/31 14:59:23 paul Exp $
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
COMPONENT = servers/serial

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
VPATH := $(HSRC)/$(COMPONENT):$(HSRC)/servers/common/$(HPROC)

#-----------------------------------------------------------------------------
# Generic component make rules and variables added below here

# Note all final generic targets here
TARGETS = serial serialdev.dev serialtest

default : $(TARGETS) Xdefault

install	: $(TARGETS) Xinstall
	$(CP) serial serialtest $(BIN)
	$(CP) serialdev.dev $(LIB)
	$(TOUCH) install

clean : Xclean
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
SOURCES.c	= serial.c serialdev.c serialtest.c

# HOBJASM source files
SOURCES.s	= serialprefix.s

# C header files
HEADERS.c	= 

# AMPP macro files
HEADERS.m	=

# Any other source files
SOURCES.odd	= makefile


#-----------------------------------------------------------------------------
# Other generic component variables:

# Set any flags required
NCFLAGS := $(NCFLAGS)

LINKFLAGS := $(LINKFLAGS)

# Library def files to be linked with
DEFS     = ${LIB}/cstart.o ${LIB}/servlib.def ${LIB}/abclib.def

LIBS     = ${LIB}/kernel.def ${LIB}/syslib.def ${LIB}/util.def \
		${LIB}/abclib.def

COMMON    := ../../common/$(HPROC)#	# common objects
COMMONS   := ../../common#		# common sources

%.s : %.a	# remove standard ampp/as rule

#-----------------------------------------------------------------------------
# Generic targets and their dependencies:

${COMMON}/modend.o:	${COMMONS}/modend.s
			make -C $(COMMON) modend.o

#------------------------------------------------------------------------------

serialdev.dev: serialprefix.o serialdev.o ${COMMON}/modend.o
	${LINK} $^ ${LIBS} -o$@

serialdev.o: serialdev.c ${INCLUDE}/dev/serialdev.h ${INCLUDE}/dev/result.h 
ifeq ($(HPROC),ARM)
	${NC} ${NCFLAGS} -j${HSRC}/kernel -Zps1 -Zr -c $<
else
endif

ASMHDRS =	$(HINCLUDE)/listopts.s  $(HINCLUDE)/arm.s \
		$(HINCLUDE)/basic.s     $(HINCLUDE)/structs.s \
		$(HINCLUDE)/module.s    $(HINCLUDE)/queue.s \
		$(HINCLUDE)/device.s

serialprefix.o: serialprefix.s $(ASMHDRS)
ifeq ($(HPROC),ARM)
	${HOBJASM} $< $@ -hdr ${HINCLUDE}
else
endif

#------------------------------------------------------------------------------

serial: serial.o ${INCLUDE}/dev/serialdev.h ${INCLUDE}/dev/result.h
	${LINK} -o$@ -nserial -s10000 ${DEFS} $@.o

#------------------------------------------------------------------------------
 
serialtest: serialtest.o ${INCLUDE}/dev/serialdev.h ${INCLUDE}/dev/result.h
	${LINK} -o$@ -s10000 ${LIB}/cstart.o ${LIB}/fault.def $@.o


#-----------------------------------------------------------------------------
endif #InProcMake	# This line must be at EOF

