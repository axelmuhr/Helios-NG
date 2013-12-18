#-----------------------------------------------------------------------------
# Helios make rules for: network/packages/hwtests1, a simple hardware test suite
#
# File: /HSRC/network/packages/hwtests1/makefile
#
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
COMPONENT = network/packages/hwtests1

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

vpath %.h $(HVPATH)
vpath %.a $(HVPATH)
vpath %.c $(HVPATH)

#-----------------------------------------------------------------------------
# Generic component make rules and variables added below here

CCFLAGS := 

# Note all final generic targets here
TARGETS = hwtest

default : $(TARGETS) Xdefault

install	: installdef $(TARGETS) Xinstall
#	-mkdir $(HPROD)/local/tests
#	-mkdir $(HPROD)/local/tests/hwtests1
#	cat $(HSRC)/$(COMPONENT)/hwtest.c > $(HPROD)/local/tests/hwtests1/hwtest.c
#	cat $(HSRC)/$(COMPONENT)/hwtest.tex > $(HPROD)/local/tests/hwtests1/hwtest.tex
	$(TOUCH) install

installdef:	# If this is a library, enter all .def files here (to install)
	$(TOUCH) installdef

clean	: Xclean
	-$(RM)	$(OBJECTS) $(TARGETS) $(XOBJECTS) $(XTARGETS) mkout \
		core ../mkout install installdef Xinstall


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
SOURCES.c	=	hwtest.c

# AMPP source files
SOURCES.a	=

# C header files
HEADERS.c	=	

# AMPP macro files
HEADERS.m	=

# Any other source files
SOURCES.odd	= makefile hwtest.tex


#-----------------------------------------------------------------------------
# Other generic component variables:

# Note any library def files to be linked with
LIBDEFS	:= -l$(LIB)/helios.lib -l$(LIB)/c.lib -l$(LIB)/RmLib.def

LINKFLAGS := $(LINKFLAGS)

#-----------------------------------------------------------------------------
# Generic targets and their dependencies:

hwtest: hwtest.c
	$(CC) $(CCFLAGS) -c $< -o hwtest.o
	$(LINK) $(LINKFLAGS) -o $@ $(LIB)/c0.o hwtest.o $(LIBDEFS)

#-----------------------------------------------------------------------------
endif #InProcMake	# This line must be at EOF

