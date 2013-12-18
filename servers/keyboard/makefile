#-----------------------------------------------------------------------------
# Helios generic make system - GENERIC COMPONENT MAKEFILE -
#-----------------------------------------------------------------------------
# Helios generic component make rules for: standalone keyboard server
#
# File: /HSRC/servers/keyboard/makefile
#
# These are the host/processor independent rules and variables for this
# component of Helios. It is included by the processor specific makefile
# That may add to some of its functionality. You should only alter this
# makefile by using "ifneq ($(HPROC),YYY)" clauses for a specific processor.
#
# RcsId: $Id: makefile,v 1.5 1994/06/07 12:40:39 nickc Exp $
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
 include ../../makeinc/DFLT.mak
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
COMPONENT = servers/keyboard

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
vpath %.a $(HVPATH)
vpath %.c $(HVPATH)

#-----------------------------------------------------------------------------
# Extra server compilation rules:

.SUFFIXES: .si .dl .sa

# Servers incorporated into the system image
ifeq ($(HPROC),TRAN)
%.si:%.c
	$(NC) $(NCFLAGS) -DINSYSTEMIMAGE $< -s$(TEMP)/$*.s
	$(ASM) $(ASMFLAGS) -o $*.si $(TEMP)/$*.s
	$(RMTEMP) $(TEMP)/$*.s
else
%.si:%.c
	@ Echo Compiling $< ...
	@ $(NC) -o$*.si -c $(NCFLAGS) -DINSYSTEMIMAGE $<
endif

# Demand loaded servers
ifeq ($(HPROC),TRAN)
%.dl:%.c
	$(NC) $(NCFLAGS) -DDEMANDLOADED $< -s$(TEMP)/$*.s
	$(ASM) $(ASMFLAGS) -o $*.dl $(TEMP)/$*.s
	$(RMTEMP) $(TEMP)/$*.s
else
%.dl:%.c
	@ Echo Compiling $< ...
	@ $(NC) -o$*.dl -c $(NCFLAGS) -DDEMANDLOADED $<
endif

# Stand alone servers
ifeq ($(HPROC),TRAN)
%.sa:%.c
	$(NC) $(NCFLAGS) -DSTANDALONE $< -s$(TEMP)/$*.s
	$(ASM) $(ASMFLAGS) -o $*.sa $(TEMP)/$*.s
	$(RMTEMP) $(TEMP)/$*.s
else
%.sa:%.c
	@ Echo Compiling $< ...
	@ $(NC) -o$*.sa -c $(NCFLAGS) -DSTANDALONE $<
endif

#-----------------------------------------------------------------------------
# Generic component make rules and variables added below here

# Note all final generic targets here

ifeq ($(HPLATFORM), ARCHIMEDES)
 TARGETS = keyboard.i keyboard.cmd keytest
endif

default : $(TARGETS) Xdefault

install	: $(TARGETS) Xinstall
ifeq ($(HPLATFORM), ARCHIMEDES)
	$(CP) keyboard.i $(LIB)/keyboard
	$(CP) keyboard.cmd $(BIN)/private
endif
	$(CP) keytest $(BIN)/private
	$(TOUCH) install

clean	: Xclean
	-$(RM)	$(OBJECTS) $(TARGETS) $(XOBJECTS) $(XTARGETS) mkout \
		core ../mkout install Xinstall *.sa


#-----------------------------------------------------------------------------
# for archive, backup and cleaning purposes,
# note all generic SOURCE and OBJECT files:

# Note all sources
SOURCES		= $(SOURCES.c) $(SOURCES.a) \
		  $(HEADERS.c) $(HEADERS.m) \
		  $(SOURCES.odd)

OBJECTS	= $(SOURCES.c:.c=.o) $(SOURCES.a:.a=.o)

# C source files
SOURCES.c	= keyboard.c keytest.c

# AMPP source files
SOURCES.a	=

# C header files
HEADERS.c	=

# AMPP macro files
HEADERS.m	=

# Any other source files
SOURCES.odd	= makefile README template.akm


#-----------------------------------------------------------------------------
# Other generic component variables:

# Set any flags required

# Note any library def files to be linked with
LIBDEFS	:=

LINKFLAGS := $(LINKFLAGS)

#-----------------------------------------------------------------------------
# heap size set to < 2*buffer size so buffers are allocated from system &
# returned when not used.
#
# *.si = Server is incorporated into system image 
# *.dl = Demand loaded server - Name table entry already exists
# *.sa = Stand alone servers (run from shell)
#
# Generic targets and their dependencies:

keyboard.i: $(LIB)/sstart.o keyboard.si
	@ Echo Linking $@ ...
	@ $(LINK) $(LINKFLAGS) -o$@ -nKeyboard $^

keyboard.cmd: $(LIB)/sstart.o keyboard.sa
	@ Echo Linking $@ ...
	@ $(LINK) $(LINKFLAGS) -o$@ -nKeyboard $^

keytest: $(CSTART) keytest.o
	@ Echo Linking $@ ...
	@ $(LINK) $(LINKFLAGS) -o$@ -n$@ $^


#-----------------------------------------------------------------------------
endif #InProcMake	# This line must be at EOF
