#-----------------------------------------------------------------------------
# Helios make rules for: demos/mswin
#
# File: /HSRC/demos/mswin/makefile
#
#-----------------------------------------------------------------------------

MV = mv

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
COMPONENT = demos/mswin

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

DEMO = $(HPROD)/users/guest/examples/mswin

TARGETS = arctest.tst bitmap.tst bkcolor.tst bktest.tst devcaps.tst exttest.tst \
	flood.tst linecol.tst lines.tst metafill.tst metaline.tst \
	metalook.tst mousetst.tst pallight.tst paltest.tst pixtest.tst plyfll.tst \
	polyfill.tst prnline.tst sonolook.tst texttest.tst 

EXPORTS = $(SOURCES.c) makefile.hel runall

default : $(TARGETS) Xdefault

install	: installdef $(TARGETS) Xinstall
	-$(MKDIR) $(DEMO)
	$(foreach NAM, $(EXPORTS), \
		$(RM) $(DEMO)/$(NAM) ; \
		$(CP) $(HSRC)/$(COMPONENT)/$(NAM) $(DEMO) ; \
		$(CHMOD) ug+w $(DEMO)/$(NAM); )
	$(MV) $(DEMO)/makefile.hel $(DEMO)/makefile
	$(CP) $(HSRC)/ioproc/mswin/lib/mslights.c $(DEMO)
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
SOURCES.c	= arctest.c bitmap.c bkcolor.c bktest.c devcaps.c exttest.c flood.c \
		linecol.c lines.c metafill.c metaline.c metalook.c \
		mousetst.c pallight.c paltest.c pixtest.c \
		plyfll.c polyfill.c prnline.c sonolook.c texttest.c

# AMPP source files
SOURCES.a	=

# C header files
HEADERS.c	=

# AMPP macro files
HEADERS.m	=

# Any other source files
SOURCES.odd	= makefile makefile.hel runall login

#-----------------------------------------------------------------------------
# Generic targets and their dependencies:

# uncomment if you have compiler or makedepend generated dependencies
#include depends

# Turn off stack check and function header generation.
ifeq ($(HPROC),ARM)
  NCFLAGS := $(NCFLAGS) -Zps1
else
  ifeq ($(HPROC),C40)
    NCFLAGS := $(NCFLAGS)
  else
    ifdef SYSDEB
      NCFLAGS := $(NCFLAGS) -ps1
    endif
    NCFLAGS := $(NCFLAGS) -wd
  endif
endif

# Note any library def files to be linked with
LIBDEFS	:=

LINKFLAGS := $(LINKFLAGS)

#-----------------------------------------------------------------------------
# Generic targets and their dependencies:

# uncomment if you have compiler or makedepend generated dependencies
#include depends

arctest.tst : arctest.o $(LIB)/graph.lib
	$(CC) -o$@ arctest.o -l$(LIB)/graph.lib

bitmap.tst : bitmap.o $(LIB)/graph.lib
	$(CC) -o$@ bitmap.o -l$(LIB)/graph.lib

bkcolor.tst : bkcolor.o $(LIB)/graph.lib
	$(CC) -o$@ bkcolor.o -l$(LIB)/graph.lib

bktest.tst : bktest.o $(LIB)/graph.lib
	$(CC) -o$@ bktest.o -l$(LIB)/graph.lib

devcaps.tst : devcaps.o $(LIB)/graph.lib
	$(CC) -o$@ devcaps.o -l$(LIB)/graph.lib

exttest.tst : exttest.o $(LIB)/graph.lib
	$(CC) -o$@ exttest.o -l$(LIB)/graph.lib

flood.tst : flood.o $(LIB)/graph.lib
	$(CC) -o$@ flood.o -l$(LIB)/graph.lib

linecol.tst : linecol.o $(LIB)/graph.lib
	$(CC) -o$@ linecol.o -l$(LIB)/graph.lib

lines.tst : lines.o $(LIB)/graph.lib
	$(CC) -o$@ lines.o -l$(LIB)/graph.lib

metafill.tst : metafill.o $(LIB)/graph.lib
	$(CC) -o$@ metafill.o -l$(LIB)/graph.lib

metaline.tst : metaline.o $(LIB)/graph.lib
	$(CC) -o$@ metaline.o -l$(LIB)/graph.lib

metalook.tst : metalook.o $(LIB)/graph.lib
	$(CC) -o$@ metalook.o -l$(LIB)/graph.lib

mousetst.tst : mousetst.o $(LIB)/graph.lib
	$(CC) -o$@ mousetst.o -l$(LIB)/graph.lib

pallight.tst : pallight.o $(LIB)/graph.lib
	$(CC) -o$@ pallight.o -l$(LIB)/graph.lib

paltest.tst : paltest.o $(LIB)/graph.lib
	$(CC) -o$@ paltest.o -l$(LIB)/graph.lib

pixtest.tst : pixtest.o $(LIB)/graph.lib
	$(CC) -o$@ pixtest.o -l$(LIB)/graph.lib

plyfll.tst : plyfll.o $(LIB)/graph.lib
	$(CC) -o$@ plyfll.o -l$(LIB)/graph.lib

polyfill.tst : polyfill.o $(LIB)/graph.lib
	$(CC) -o$@ polyfill.o -l$(LIB)/graph.lib

prnline.tst : prnline.o $(LIB)/graph.lib
	$(CC) -o$@ prnline.o -l$(LIB)/graph.lib

sonolook.tst : sonolook.o $(LIB)/graph.lib
	$(CC) -o$@ sonolook.o -l$(LIB)/graph.lib

texttest.tst : texttest.o $(LIB)/graph.lib
	$(CC) -o$@ texttest.o -l$(LIB)/graph.lib

#-----------------------------------------------------------------------------
endif #InProcMake	# This line must be at EOF
