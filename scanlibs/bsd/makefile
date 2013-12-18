#-----------------------------------------------------------------------------
# Helios generic make system - GENERIC COMPONENT MAKEFILE -
#-----------------------------------------------------------------------------
# Helios generic component make rules for: bsd
#
# File: /HSRC/scanlibs/bsd/makefile
#
# These are the host/processor independent rules and variables for this
# component of Helios. It is included by the processor specific makefile
# That may add to some of its functionality. You should only alter this
# makefile by using "ifneq ($(HPROC),YYY)" clauses for a specific processor.
#
# RcsId: $Id: makefile,v 1.14 1994/05/12 11:34:01 nickc Exp $
#		(C) Copyright 1990 Perihelion Software
#	(C) Copyright 1994 Perihelion Distributed Software
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
COMPONENT = scanlibs/bsd

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
HVPATH := $(HSRC)/$(COMPONENT)

ifdef RSRC
HVPATH := $(HSRC)/$(COMPONENT)/$(HPROC):$(RSRC)/$(COMPONENT)/$(HPROC):$(HSRC)/$(COMPONENT):/$(RSRC)/$(COMPONENT)
endif

vpath %.h $(HVPATH)
vpath %.a $(HVPATH)
vpath %.c $(HVPATH)

#-----------------------------------------------------------------------------
# Generic component make rules and variables added below here

# Note all final generic targets here
TARGETS = bsd.lib

default : $(TARGETS) Xdefault

install	: $(TARGETS) Xinstall
	$(CP) $(TARGETS) $(LIB)
	$(TOUCH) install

clean	: Xclean
	-$(RM)	$(OBJECTS) $(TARGETS) $(XOBJECTS) $(XTARGETS) mkout \
		core ../mkout install Xinstall *.ghof fred


#-----------------------------------------------------------------------------
# for archive, backup and cleaning purposes,
# note all generic SOURCE and OBJECT files:

# Note all sources
SOURCES		= $(SOURCES.c) $(SOURCES.a) \
		  $(HEADERS.c) $(HEADERS.m) \
		  $(SOURCES.odd)

OBJECTS	= $(SOURCES.c:.c=.o) $(SOURCES.a:.a=.o)
LOBJECTS = $(patsubst %,-l%,$(OBJECTS))
GOBJECTS = $(OBJECTS:.o=.o.ghof)

# C source files
SOURCES.c	= getopt.c inetaddr.c inetnet.c inetntoa.c popen.c \
		  string.c signal.c errlist.c syslog.c misc.c fileio.c \
		  que.c ioctl.c inet.c alloca.c	rwv.c rexec.c rcmd.c \
		  ruserpass.c putenv.c

# AMPP source files
SOURCES.a	= 

# C header files
HEADERS.c	=

# AMPP macro files
HEADERS.m	=

# Any other source files
SOURCES.odd	= makefile spec needed mkerrlst.c


#-----------------------------------------------------------------------------
# Other generic component variables:

# Set any flags required
NCFLAGS := $(NCFLAGS) -DSTACK_DIRECTION=-1 -D_BSD

ifeq ($(HPROC),C40)
  NCFLAGS := $(NCFLAGS) -Zpl0		# prevent few_modules option, it stops the X library working
endif

#-----------------------------------------------------------------------------
# Generic targets and their dependencies:

# uncomment if you have compiler or makedepend generated dependencies
#include depends

bsd.lib: $(OBJECTS)
ifeq ($(HPROC),TRAN)
	$(LINK) $(LINKFLAGS) -p -o bsd.lib $(OBJECTS)
else
	@ echo Creating scanned library $@ ...
  ifeq ($(COMPILER_TYPE),ARMLtd)
	@ $(LINK) $(LOBJECTS) -o fred
	@ cat $(GOBJECTS) > $@
  else
	@ cat $^ > $@
  endif
endif
ifeq ($(HHOST),HELIOSTRAN)
	$(RM) /helios/lib/bsd.lib
	$(CP) bsd.lib /helios/lib/bsd.lib
endif

#-----------------------------------------------------------------------------
endif #InProcMake	# This line must be at EOF

