#-----------------------------------------------------------------------------
# Helios generic make system - GENERIC COMPONENT MAKEFILE -
#-----------------------------------------------------------------------------
# Helios generic component make rules for: GNU Programs
#
# File: /HSRC/cmds/gnu/makefile
#
# These are the host/processor independent rules and variables for this
# component of Helios. It is included by the processor specific makefile
# That may add to some of its functionality. You should only alter this
# makefile by using "ifneq ($(HPROC),YYY)" clauses for a specific processor.
#
# SccsId: %W% %G%
# RcsId: $Id: makefile,v 1.3 1993/08/08 14:20:56 paul Exp $
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

#.PHONY: notarget cmds
notarget:
	@$(MAKE) -C $(HPROC)

.DEFAULT:
	@$(MAKE) -C $(HPROC) $@

else
#-----------------------------------------------------------------------------
# This part of the makefile is always run in a processor specific subdirectory.

# Manditory variables for a component:
# name of this dir relative to $(HSRC)
COMPONENT = cmds/gnu

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
 HVPATH := $(RSRC)/$(COMPONENT)/$(HPROC):$(HSRC)/$(COMPONENT):$(RSRC)/$(COMPONENT)
endif

vpath %.h $(HVPATH)
vpath %.c $(HVPATH)
vpath %.a $(HVPATH)

#-----------------------------------------------------------------------------
# Generic component make rules and variables added below here

# Note all final generic targets here
TARGETS = gmake		# Get to compile with -D_BSD: gmake
	# @@@ TOTAL TODO: bison diff flex gawk gcc

default: Xdefault
	$(foreach PD, $(TARGETS), $(MAKE) -C ../$(PD)/$(HPROC))

install: Xinstall
	$(foreach PD, $(TARGETS), $(MAKE) -C ../$(PD)/$(HPROC) install;)
	# always call subdirs.

clean: Xclean
	-$(RM) mkout core install ../mkout
	$(foreach PD, $(TARGETS), $(MAKE) -C ../$(PD)/$(HPROC) clean)


#-----------------------------------------------------------------------------
# for archive, backup and cleaning purposes,
# note all generic SOURCE and OBJECT files that are not in individual server
# directories.

# Note all sources
SOURCES		= makefile


#-----------------------------------------------------------------------------

gmake :;	$(MAKE) -C ../gmake/$(HPROC) 
bison :;	$(MAKE) -C ../bison/$(HPROC) 
diff :;		$(MAKE) -C ../diff/$(HPROC) 
flex :;		$(MAKE) -C ../flex/$(HPROC) 
gawk :;		$(MAKE) -C ../gawk/$(HPROC) 
gcc :;		$(MAKE) -C ../gcc/$(HPROC) 


#-----------------------------------------------------------------------------

endif #InProcMake	# This line must be at EOF
