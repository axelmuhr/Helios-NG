#-----------------------------------------------------------------------------
# Helios generic make system
#-----------------------------------------------------------------------------
# Helios generic component make rules for: Fault Library
#
# File: /HSRC/fault/makefile
#
# These are the host/processor independent rules and variables for this
# component of Helios. It is included by the processor specific makefile
# That may add to some of its functionality. You should only alter this
# makefile by using "ifneq ($(HPROC),YYY)" clauses for a specific processor.
#
# SccsId: %W% %G%
# RcsId: $Id: makefile,v 1.21 1994/06/07 12:34:56 nickc Exp $
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
	@ $(MAKE) -C $(HPROC)

.DEFAULT:
	@ $(MAKE) -C $(HPROC) $@

else
#-----------------------------------------------------------------------------
# This part of the makefile is always run in a processor specific subdirectory.

# Manditory variables for a component:
# name of this dir relative to $(HSRC)
COMPONENT = fault

# Manditory includes
include $(HSRC)/makeinc/$(HHOST).mak	# Default host system variables
include $(HSRC)/makeinc/$(HPROC).mak	# Default processor rules and vars.

# Manditory phoney targets
.PHONY: default clean Xdefault Xclean
# "default" - should make this component in its proc subdir
# "install" - copy the new component to $(HPROD) production directory
# "clean"   - remove all intermediate and executable files from proc subdir
# Phoney targets prefixed by X are for adding processor specific targets.
#
# Processor specific sources are held in the processor specific subdir.,
# generic ones in the directory above it.
HVPATH := $(HSRC)/$(COMPONENT)

ifdef RSRC
  HVPATH := $(RSRC)/$(COMPONENT)/$(HPROC):$(HSRC)/$(COMPONENT):$(RSRC)/$(COMPONENT)
endif

vpath %.h $(HVPATH)
vpath %.c $(HVPATH)
vpath %.a $(HVPATH)
vpath %.fdb $(HVPATH)

#-----------------------------------------------------------------------------
# Generic component make rules and variables added below here

# Note all final generic targets here
TARGETS = fault.i fault.def faults

default : $(TARGETS) Xdefault

install: installdef $(TARGETS) Xinstall
ifeq ($(HLICENSEE),ABC)
	sysstrip fault.i -o$(LIB)/Fault
else
	$(CP) fault.i $(LIB)/Fault
endif
	$(CP) faults $(ETC)
ifeq (HELIOS,$(findstring HELIOS,$(HHOST))) #csh default on Helios hosts
	(cmp codes.h $(INCLUDE)/codes.h ; if ( $$status ) $(TCP) codes.h $(INCLUDE) )
	(cmp errno.h $(INCLUDE)/errno.h ; if ( $$status ) $(TCP) errno.h $(INCLUDE) )
	(cmp signal.h $(INCLUDE)/signal.h ; if ( $$status ) $(TCP) signal.h $(INCLUDE) )
else
	@ if cmp codes.h $(INCLUDE)/codes.h; then echo "codes.h not changed"; else $(TCP) codes.h $(INCLUDE) ;fi
	@ if cmp errno.h $(INCLUDE)/errno.h; then echo "errno.h not changed"; else $(TCP) errno.h $(INCLUDE) ;fi
	@ if cmp signal.h $(INCLUDE)/signal.h; then echo "signal.h not changed"; else $(TCP) signal.h $(INCLUDE) ;fi
endif
	$(TOUCH) install

installdef: $(LIB)/fault.def
	$(TOUCH) installdef

$(LIB)/fault.def : fault.def
ifeq (HELIOS,$(findstring HELIOS,$(HHOST))) #csh default on Helios hosts
	(cmp $^ $@ ; if ( $$status ) cp $^ $@ )
else
	@ if cmp $^ $@; then echo "$^ not changed"; else $(CP) $^ $@ ;fi
endif

clean	: Xclean
	-$(RM)	$(OBJECTS) $(TARGETS) $(XOBJECTS) $(XTARGETS) mkout \
		core ../mkout codes.h errno.h signal.h install \
		Xinstall installdef fault.tar

#-----------------------------------------------------------------------------
# for archive, backup and cleaning purposes,
# note all generic SOURCE and OBJECT files:

# Note all sources
SOURCES		= $(SOURCES.c) $(SOURCES.a) \
		  $(HEADERS.c) $(HEADERS.m) \
		  $(SOURCES.odd)

# C source files
SOURCES.c	= fault.c ftest.c

# AMPP source files
SOURCES.a	= fstart.a modend.a

# C header files
HEADERS.c	=

# AMPP macro files
HEADERS.m	=

# Any other source files
SOURCES.odd	= makefile master.fdb


#-----------------------------------------------------------------------------
# Other generic component variables:

# Set any flags required
# -I. to pick up latest codes.h
ifeq ($(HPROC),ARM)
  NCFLAGS := $(NCFLAGS) -Zps1 -I.
else
  ifeq ($(HPROC),C40)
    NCFLAGS := $(NCFLAGS) -Zps1 -I.
  else
    NCFLAGS := $(NCFLAGS) -ps1
  endif
endif

# Note any library def files to be linked with
LIBDEFS	=	$(LIB)/kernel.def $(LIB)/syslib.def \
		$(LIB)/util.def


#-----------------------------------------------------------------------------
# Generic targets and their dependencies:

# uncomment if you have compiler or makedepend generated dependencies
#include depends


fault.c: faults

OBJECTS = fstart.p fault.p modend.p

fault.i: $(OBJECTS) $(LIBDEFS)
	$(LINK.LIBRARY) -o$@ -nFault -s1000 -h1000 $^

fault.def: fstart.a
	$(MAKE.DEF)


#-----------------------------------------------------------------------------
# Create fault lookup files:
# etc/faults, include/errno.h, include/codes.h, include/signal.h

codes.h errno.h signal.h faults : master.fdb
	$(HHOSTBIN)/squash < $< > faults
	$(HHOSTBIN)/gencodes $< > $(NULL)


#-----------------------------------------------------------------------------
# Fault library test utilities:

ifneq ($(HPROC),ARM)
ftest : ftest.o
	 $(LINK) $(LIB)/c0.o ftest.o -l $(LIB)/libc -o ftest
endif


#-----------------------------------------------------------------------------
endif #InProcMake	# This line must be at EOF

