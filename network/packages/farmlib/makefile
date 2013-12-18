#-----------------------------------------------------------------------------
# Helios make rules for: network/packages/farmlib
#
# File: /HSRC/network/packages/farmlib/makefile
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
COMPONENT = network/packages/farmlib

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

# Note all final generic targets here
# Mandelbrot needs X
TARGETS = farm.lib pi pi2 pi3 farmtest testaux # mandel

EXPORTS = pi.c pi.h pi2.c pi2calc.c pi2.h pi3.c pi3calc.c pi3.h mandel.h \
	mandel.c mandcalc.c mandelx.c readme

default : $(TARGETS) Xdefault

install	: installdef $(TARGETS) Xinstall
	$(CP) farm.lib	$(HPROD)/lib
	$(foreach NAM, $(EXPORTS), \
		$(RM) -f $(HPROD)/users/guest/examples/farmlib/$(NAM) ; \
		$(CP) $(HSRC)/$(COMPONENT)/$(NAM) $(HPROD)/users/guest/examples/farmlib ; \
		$(CHMOD) ug+w $(HPROD)/users/guest/examples/farmlib/$(NAM); )
	$(TOUCH) install

installdef:   # If this is a library, enter all .def files here (to install)
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
SOURCES.c	=	farmlib.c pi.c pi2.c pi2calc.c pi3.c pi3calc.c \
			mandel.c mandcalc.c mandelx.c farmtest.c testaux.c \
			testaux2.c

# AMPP source files
SOURCES.a	=

# C header files
HEADERS.c	= pi.h pi2.h pi3.h mandel.h

# AMPP macro files
HEADERS.m	=

# Any other source files
SOURCES.odd	= makefile readme


#-----------------------------------------------------------------------------
# Other generic component variables:

CCFLAGS := -I$(HSRC)/$(COMPONENT)

LIBS := -l$(LIB)/helios.lib -l$(LIB)/c.lib -l$(LIB)/RmLib.def -lfarm.lib

#-----------------------------------------------------------------------------
# Generic targets and their dependencies:

farm.lib : farmlib.c
	$(CC) $(CCFLAGS) -c $< -o $@

#mandel : mandel.o mandcalc.o mandelx.o farm.lib
#	$(CC) $(CCFLAGS) -o $@ mandel.o mandcalc.o mandelx.o -L. -lfarm \
#		-lRmLib -lX -lX11 -lbsd -s5000
#
#mandel.o : mandel.c
#	$(CC) -c $(CCFLAGS) -o$@ $<
#
#mandcalc.o : mandcalc.c
#	$(CC) -c $(CCFLAGS) -o$@ $<
#
#mandelx.o : mandelx.c
#	$(CC) -c $(CCFLAGS) -o$@ $<

pi : pi.o farm.lib
	$(CC) $(CCFLAGS) -o$@ pi.o -L. -lfarm -lRmLib

pi.o : pi.c pi.h
	$(CC) -c -o$@ $(CCFLAGS) $<

pi2 : pi2.o pi2calc.o farm.lib
	$(CC) $(CCFLAGS) -o$@ pi2.o pi2calc.o -L. -lfarm -lRmLib

pi2.o : pi2.c pi2.h 
	$(CC) -c -o$@ $(CCFLAGS) $<

pi2calc.o : pi2calc.c pi2.h
	$(CC) -c -o$@ $(CCFLAGS) $<

pi3 : pi3.o pi3calc.o farm.lib
	$(CC) $(CCFLAGS) -o$@ pi3.o pi3calc.o -L. -lfarm -lRmLib

pi3.o : pi3.c pi3.h
	$(CC) -c -o$@ $(CCFLAGS) $<

pi3calc.o : pi3calc.c pi3.h
	$(CC) -c -o$@ $(CCFLAGS) $<

farmtest : farmtest.o
	$(LINK) $(LINKFLAGS) -o$@ -n$@ -s4000 -h4000 $(CSTART) $^ $(LIBS)

testaux : testaux.o testaux2.o farm.lib
	$(LINK) $(LINKFLAGS) -o$@ -n$@ -s4000 -h4000 $(CSTART) $^ $(LIBS)
#-----------------------------------------------------------------------------
endif #InProcMake	# This line must be at EOF


