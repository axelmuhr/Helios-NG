#-----------------------------------------------------------------------------
# Helios generic make system - GENERIC COMPONENT MAKEFILE -
#-----------------------------------------------------------------------------
# Helios generic component make rules for: Public Domain Programs
#
# File: /HSRC/cmds/public/makefile
#
# These are the host/processor independent rules and variables for this
# component of Helios. It is included by the processor specific makefile
# That may add to some of its functionality. You should only alter this
# makefile by using "ifneq ($(HPROC),YYY)" clauses for a specific processor.
#
# SccsId: %W% %G%
# RcsId: $Id: makefile,v 1.5 1993/08/13 08:08:13 nickc Exp $
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
COMPONENT = cmds/public

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
TARGETS = clock spreadsh cookie yacc-1.4	# ed mille rcs sed stevie

default: Xdefault
	$(foreach PD, $(TARGETS), $(MAKE) -C ../$(PD)/$(HPROC); )

install: Xinstall $(TARGETS)
	$(foreach PD, $(TARGETS), $(MAKE) -C ../$(PD)/$(HPROC) install; )
	$(TOUCH) install

clean: Xclean
	-$(RM) mkout core install ../mkout
	$(foreach PD, $(TARGETS), $(MAKE) -C ../$(PD)/$(HPROC) clean; )


#-----------------------------------------------------------------------------
# for archive, backup and cleaning purposes,
# note all generic SOURCE and OBJECT files that are not in individual server
# directories.

# Note all sources
SOURCES		= makefile


#-----------------------------------------------------------------------------
# Individual tools in their own directories go here:

yacc-1.4:;	$(MAKE) -C ../yacc-1.4/$(HPROC) 
clock :;	$(MAKE) -C ../clock/$(HPROC) 
spreadsh :;	$(MAKE) -C ../spreadsh/$(HPROC) 
rcs :;		$(MAKE) -C ../rcs/$(HPROC) 
stevie :;	$(MAKE) -C ../stevie/$(HPROC) 
ed :;		$(MAKE) -C ../ed/$(HPROC) 
sed :;		$(MAKE) -C ../sed/$(HPROC) 
mille :;	$(MAKE) -C ../mille/$(HPROC) 
cookie :;	$(MAKE) -C ../cookie/$(HPROC) 


#-----------------------------------------------------------------------------

endif #InProcMake	# This line must be at EOF
