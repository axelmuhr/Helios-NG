#-----------------------------------------------------------------------------
# Helios generic make system - GENERIC COMPONENT MAKEFILE -
#-----------------------------------------------------------------------------
# Helios generic component make rules for: Helios Test Suite
#
# File: /HSRC/cmdtests/makefile
#
# These are the host/processor independent rules and variables for this
# component of Helios. It is included by the processor specific makefile
# That may add to some of its functionality. You should only alter this
# makefile by using "ifneq ($(HPROC),YYY)" clauses for a specific processor.
#
# SccsId: %W% %G%
# RcsId: $Id: makefile,v 1.6 1994/06/07 12:33:37 nickc Exp $
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
COMPONENT = cmds/cmdtests

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

INSTALLDIR = $(HPROD)/local/tests/cmdtests

# Note all final generic targets here
TARGETS = testsuite

default: $(TARGETS) Xdefault

install: $(TARGETS) Xinstall
	@ test -d $(INSTALLDIR) || $(MKDIR) -p $(INSTALLDIR)
	-$(RM) $(INSTALLDIR)/makefile
	$(CP) ../makefile.hel $(INSTALLDIR)/makefile
	$(CHMOD) gu+rw $(INSTALLDIR)/makefile
	-$(RM) $(INSTALLDIR)/testsuite.c
	$(CP) ../testsuite.c $(INSTALLDIR)
	$(CHMOD) gu+rw $(INSTALLDIR)/testsuite.c
	-@ $(foreach NAME, $(EXPORTS), \
		$(RM) $(INSTALLDIR)/$(NAME) ; \
		$(CP) ../$(NAME) $(INSTALLDIR) ;\
		$(CHMOD) gu+rw $(INSTALLDIR)/$(NAME) ;)
	$(CP) $(TARGETS) $(INSTALLDIR)
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

OBJECTS		= $(SOURCES.c:.c=.o) $(SOURCES.a:.a=.o)

# C source files
SOURCES.c	= testsuite.c

# C header files
HEADERS.c  	= 

# AMPP source files
SOURCES.a	=

# AMPP macro files
HEADERS.m	=

EXPORTS		= pi.cdl map.in sendto.in wall.in write.in worker.c control.c

# Any other source files
SOURCES.odd	= makefile makefile.hel $(EXPORTS)



#-----------------------------------------------------------------------------
# Other generic component variables:

# Set any flags required
  NCFLAGS := $(NCFLAGS)

# Note any library def files to be linked with
LIBDEFS	= 


#-----------------------------------------------------------------------------
# Generic targets and their dependencies:

testsuite: $(CSTART) testsuite.o
	$(LINK) $(LINKFLAGS) -o$@ -n$@ -s2000 -h8000 $^



#-----------------------------------------------------------------------------
endif #InProcMake	# This line must be at EOF
