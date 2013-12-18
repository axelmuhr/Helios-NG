#-----------------------------------------------------------------------------
# Helios generic make system - GENERIC COMPONENT MAKEFILE -
#-----------------------------------------------------------------------------
# Helios generic component make rules for: sa
#
# File: /HSRC/sa/makefile
#
# These are the host/processor independent rules and variables for this
# component of Helios. It is included by the processor specific makefile
# That may add to some of its functionality. You should only alter this
# makefile by using "ifneq ($(HPROC),YYY)" clauses for a specific processor.
#
# SccsId: %W% %G%
# RcsId: $Id: makefile,v 1.6 1992/10/16 13:29:11 nick Exp $
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
COMPONENT = sa

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

vpath %.a	$(HVPATH)
vpath %.h	$(HVPATH)
vpath %.c	$(HVPATH)

#-----------------------------------------------------------------------------
# Generic component make rules and variables added below here

# Note all final generic targets here
TARGETS = sainit sa0.o salib samath.t4 samath.t8 sasup.o sarun salink

default : $(TARGETS) Xdefault

install	: installdef $(TARGETS) Xinstall
	$(CP) salink sarun $(BIN)
	$(CP) sainit sa0.o salib samath.t4 samath.t8 sasup.o $(LIB)
ifdef SYSDEB
	$(CP) salink sarun /helios/bin
	$(CP) sainit sa0.o salib samath.t4 samath.t8 sasup.o /helios/lib
endif
	-$(MKDIR) $(HPROD)/users/guest/examples/sa
	-$(RM) $(HPROD)/users/guest/examples/sa/*
	$(CP) ../explore.c ../worm.c ../worm.h ../sarun.c  $(HPROD)/users/guest/examples/sa
	$(CP) ../makefile.hel $(HPROD)/users/guest/examples/sa/makefile
	$(TOUCH) install

installdef:	# If this is a library, enter all .def files here (to install)
#	$(CP) $^ $(LIB)
#	$(TOUCH) installdef

clean	: Xclean
	-$(RM)	$(OBJECTS) $(TARGETS) $(XOBJECTS) $(XTARGETS) mkout \
		core ../mkout install installdef Xinstall \
		*.p *.o4 *.o8


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
SOURCES.c	= acccode.c alloc.c boot.c cstring.c iodebug.c \
		  math.c misc.c queue.c sainit.c salink.c sarcmd.c sarun.c \
		  sasup.c sem.c test.c thread.c trace.c \
		  explore.c worm.c    # hello.c 

# AMPP source files
SOURCES.a	= accel.a alt.a hipri.a mctype.a saasm.a saentry.a sastart.a \
		  setjmp.a string.a t4sim.a t8sim.a kend.a

# C header files
HEADERS.c	= worm.h
                  # chanio.h linkio.h salib.h sysinfo.h thread.h trace.h \

# AMPP macro files
HEADERS.m	=

# Any other source files
SOURCES.odd	= makefile makefile.hel


#-----------------------------------------------------------------------------
# Other generic component variables:

# Set any flags required

NCFLAGS		:= -ps1 -pf0 $(NCFLAGS) -DTRANSPUTER

ifndef RSRC
NCFLAGS		:= $(NCFLAGS) -i,../,$(INCLUDE)/,$(HSRC)/kernel/ -j$(INCLUDE)/
else
NCFLAGS		:= $(NCFLAGS) -i,$(RSRC)/$(COMPONENT)/$(HPROC),../,$(RSRC)/$(COMPONENT)/,$(INCLUDE)/,$(RSRC)/kernel/,$(HSRC)/kernel/ -j$(INCLUDE)/
endif




# Note any library def files to be linked with
LIBDEFS	:=

LINKFLAGS := $(LINKFLAGS) -v -f

#-----------------------------------------------------------------------------
# Generic targets and their dependencies:

sainit:	saentry.p sainit.p kend.p
	$(LINK) $(LINKFLAGS) -o $@ $^

LIBFILES = thread.o trace.o alloc.o string.p cstring.o boot.o saasm.p \
		sem.o queue.o acccode.o iodebug.o

salib : $(LIBFILES)
	$(ASM) -p $(LIBFILES) -o $@

samath.t4 : math.o4 t4sim.p
	$(ASM) -p math.o4 t4sim.p -o samath.t4

samath.t8 : math.o8 t8sim.p
	$(ASM) -p math.o8 t8sim.p -o samath.t8

salink : salink.o
	$(LINK) $(LINKFLAGS) $(LIB)/c0.o $^ -o $@ -l$(LIB)/c.lib -l$(LIB)/helios.lib

sarun : sarun.o sasup.o
	$(LINK) $(LINKFLAGS) $(LIB)/c0.o $^ -o $@ -l$(LIB)/c.lib -l$(LIB)/helios.lib

sa0.o	: sastart.o
	$(CP) $^ $@

# uncomment if you have compiler or makedepend generated dependencies
#include depends






#-----------------------------------------------------------------------------
endif #InProcMake	# This line must be at EOF

