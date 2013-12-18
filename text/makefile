#-----------------------------------------------------------------------------
# Helios generic make system - GENERIC COMPONENT MAKEFILE -
#-----------------------------------------------------------------------------
# Helios generic component make rules for: Common Helios system text files
#
# File: /HSRC/text/makefile
#
# These are the host/processor independent rules and variables for this
# component of Helios. It is included by the processor specific makefile
# That may add to some of its functionality. You should only alter this
# makefile by using "ifneq ($(HPROC),YYY)" clauses for a specific processor.
#
# RcsId: $Id: makefile,v 1.35 1993/11/17 16:06:09 nickc Exp $
#		(C) Copyright 1990 Perihelion Software
#
# WARNING: assumes you are using GNUmake.
#
# $Header: /hsrc/text/RCS/makefile,v 1.35 1993/11/17 16:06:09 nickc Exp $
#
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
COMPONENT = text

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
VPATH := $(HSRC)/$(COMPONENT)

ifdef RSRC
 HVPATH := $(RSRC)/$(COMPONENT)/$(HPROC):$(HSRC)/$(COMPONENT):$(RSRC)/$(COMPONENT)
 VPATH := $(RSRC)/$(COMPONENT)/$(HPROC):$(HSRC)/$(COMPONENT):$(RSRC)/$(COMPONENT)
endif

vpath %.h $(HVPATH)
vpath %.a $(HVPATH)
vpath %.c $(HVPATH)


#-----------------------------------------------------------------------------
# Generic component make rules and variables added below here

# Note all final generic targets here
TARGETS = cshrc2 login2 shutlogin2 cpfiles initrc2 local etcfiles2 \
	  hfree

default :
	# do nothing

install	: $(TARGETS) Xinstall
	$(TOUCH) install

hfree: hfree.c hfree.exe
	$(RM) -f $(HPROD)/hfree.exe
	$(CP) $(HSRC)/$(COMPONENT)/hfree.exe $(HPROD)


cshrc2: cshrc
	$(RM) -f $(HPROD)/users/guest/.cshrc
	$(RM) -f $(HPROD)/users/root/.cshrc
	$(CP) $< $(HPROD)/users/guest/.cshrc
	$(CP) $< $(HPROD)/users/root/.cshrc
	$(CHMOD) ug+w $(HPROD)/users/root/.cshrc $(HPROD)/users/guest/.cshrc
	$(RM) -f $(HPROD)/users/guest/cshrc
	$(RM) -f $(HPROD)/users/root/cshrc
	$(CP) $< $(HPROD)/users/guest/cshrc
	$(CP) $< $(HPROD)/users/root/cshrc
	$(CHMOD) ug+w $(HPROD)/users/root/cshrc $(HPROD)/users/guest/cshrc
	$(TOUCH) cshrc2

login2: login
	$(RM) -f $(HPROD)/users/guest/.login
	$(RM) -f $(HPROD)/users/root/.login
	$(CP) $< $(HPROD)/users/guest/.login
	$(CP) $< $(HPROD)/users/root/.login
	$(CHMOD) ug+w $(HPROD)/users/root/.login $(HPROD)/users/guest/.login
	$(RM) -f $(HPROD)/users/guest/login
	$(RM) -f $(HPROD)/users/root/login
	$(CP) $< $(HPROD)/users/guest/login
	$(CP) $< $(HPROD)/users/root/login
	$(CHMOD) ug+w $(HPROD)/users/root/login $(HPROD)/users/guest/login
	$(TOUCH) login2

shutlogin2: shutlog
	$(RM) -f $(HPROD)/users/shutdown/.login
	$(RM) -f $(HPROD)/users/shutdown/.cshrc
	$(CP) $< $(HPROD)/users/shutdown/.login
	$(TOUCH) $(HPROD)/users/shutdown/.cshrc	
	$(CHMOD) ug+w $(HPROD)/users/shutdown/.login
	$(RM) -f $(HPROD)/users/shutdown/login
	$(RM) -f $(HPROD)/users/shutdown/cshrc
	$(CP) $< $(HPROD)/users/shutdown/login
	$(TOUCH) $(HPROD)/users/shutdown/cshrc	
	$(CHMOD) ug+w $(HPROD)/users/shutdown/login
	$(TOUCH) shutlogin2

initrc2: initrc
	$(RM) -f $(HPROD)/etc/initrc
	$(CP) $< $(HPROD)/etc
	$(CHMOD) ug+w $(HPROD)/etc/initrc
	$(TOUCH) initrc2


etcfiles := initrc.tin nsrc passwd
etcsources := $(foreach x,$(etcfiles),../$x)
etctargets := $(foreach x,$(etcfiles),$(HPROD)/etc/$x)

cpfiles: $(etcfiles)
	$(RM) -f $(etctargets)
	$(CP) $(etcsources) $(HPROD)/etc
	$(CHMOD) ug+w $(etctargets)
	$(TOUCH) cpfiles

etcfiles2: default.rm default.map motd
	$(RM) -f $(HPROD)/etc/default.rm $(HPROD)/etc/default.map $(HPROD)/etc/motd
	$(CP) $^ $(HPROD)/etc
	$(CHMOD) ug+w $(HPROD)/etc/default.rm $(HPROD)/etc/default.map $(HPROD)/etc/motd
	$(TOUCH) etcfiles2

local: readme.gnu licence.gnu
	cat ../readme.gnu > $(HPROD)/local/bin/README
	cat ../licence.gnu > $(HPROD)/local/bin/LICENCE
	$(TOUCH) local

clean	: Xclean
	$(RM) install $(TARGETS) mkout ../mkout core


#-----------------------------------------------------------------------------
# for archive, backup and cleaning purposes,
# note all generic SOURCE and OBJECT files:

# Note all sources
SOURCES		= $(SOURCES.c) $(SOURCES.a) \
		  $(HEADERS.c) $(HEADERS.m) \
		  $(SOURCES.odd)

OBJECTS	= $(SOURCES.c:.c=.o) $(SOURCES.a:.a=.o)

# C source files
SOURCES.c	=

# AMPP source files
SOURCES.a	=

# C header files
HEADERS.c	=

# AMPP macro files
HEADERS.m	=

# Any other source files
SOURCES.odd	= ampp.ins c_co.ins cshrc default.map default.rm ethe.ins \
		  exam.ins file.ins help.ins hfree.c hfree.exe initrc \
		  initrc.tin login \
		  makefile motd nsrc passwd publ.ins readme.121 readme.122 \
		  shutlog sour.ins upgr.ins readme.gnu licence.gnu

#-----------------------------------------------------------------------------
# Other generic component variables:

# Set any flags required
ifeq ($(HPROC),ARM)
  NCFLAGS := $(NCFLAGS)
else
  NCFLAGS := $(NCFLAGS)
endif

# Note any library def files to be linked with
LIBDEFS	:=

LINKFLAGS := $(LINKFLAGS)

#-----------------------------------------------------------------------------
# Generic targets and their dependencies:


#-----------------------------------------------------------------------------
endif #InProcMake	# This line must be at EOF

