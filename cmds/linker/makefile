#-----------------------------------------------------------------------------
# Helios generic make system - GENERIC COMPONENT MAKEFILE -
#-----------------------------------------------------------------------------
# Helios generic component make rules for: Generic Helios linker
#
# File: /HSRC/cmds/linker/makefile
#
# These are the host/processor independent rules and variables for this
# component of Helios. It is included by the processor specific makefile
# That may add to some of its functionality. You should only alter this
# makefile by using "ifneq ($(HPROC),YYY)" clauses for a specific processor.
#
# RcsId: $Id: makefile,v 1.18 1994/01/13 16:00:00 nickc Exp $
#		(C) Copyright 1990 - 1994 Perihelion Software
#
# WARNING: assumes you are using GNUmake.
#-----------------------------------------------------------------------------

ifndef InProcMake
# This part of the makefile is run in the components generic source directory

  # These variables should already be set by your make script or environment.
  ifndef HPROC
    HPROC = TRAN
  endif

  ifndef HHOST
    HHOST = SUN4
  endif

  ifndef HSRC
    HSRC = /usr/perihelion/Helios
  endif

  ifndef HPROD
    HPROD = $(HSRC)/slashhelios/$(HPROC)
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
COMPONENT = cmds/linker

# Manditory includes
include $(HSRC)/makeinc/$(HHOST).mak	# Default host system variables
include $(HSRC)/makeinc/$(HPROC).mak	# Default processor rules and vars.

# Manditory phoney targets
.PHONEY: default clean Xdefault Xclean
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
HVPATH := $(RSRC)/$(COMPONENT)/$(HPROC):$(HVPATH):$(RSRC)/$(COMPONENT)
endif

vpath %.h $(HVPATH)
vpath %.c $(HVPATH)
vpath %.a $(HVPATH)

#-----------------------------------------------------------------------------
# Generic component make rules and variables added below here

# Note all final generic targets here
ifeq ($(HPROC),ARM)
  TARGETS = ld imdump objdump disas
  NCFLAGS := $(NCFLAGS) -I$(EXTINCLUDE)
else
  ifeq ($(HPROC),C40)
    TARGETS = ld imdump objdump disas
  else
    ifeq ($(HPROC),M68K)
      TARGETS = ld imdump objdump disas
    else
      TARGETS = ld imdump objdump
    endif
  endif
endif

default : $(TARGETS) Xdefault

install	: $(TARGETS) Xinstall
	$(CP) $(TARGETS) $(BIN)
	$(TOUCH) install

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

OBJECTS		= sym.o objed.o module.o mem.o scancode.o link.o readfile.o \
		vm.o genimage.o imdump.o objdump.o stubs.o \
		armdisas.o ARMshape.o 68kdisas.o \
		c40disas.o c40objdis.o 

# C source files
SOURCES.c	= sym.c objed.c module.c mem.c scancode.c link.c readfile.c \
		vm.c genimage.c hash.c objdump.c imdump.c stubs.c \
		armdisas.c ARMshape.c 68kdisas.c \
		c40disas.c c40objdis.c converter.c 

# AMPP source files
SOURCES.a	=

# C header files
HEADERS.c	= link.h vm.h ARMshape.h c40target.h c40ops.h converter.h \
		endian.h

# AMPP macro files
HEADERS.m	=

# Any other source files - is the queue stuff still required?
SOURCES.odd	= makefile queue.c queue.nonansi.h


#-----------------------------------------------------------------------------
# Other generic component variables:

# Set any flags required
ifeq ($(HPROC),ARM)
  NCFLAGS := $(NCFLAGS)
else
  NCFLAGS := $(NCFLAGS)
endif

#-----------------------------------------------------------------------------
# Linker

LINK.OBJECTS	= sym.o objed.o module.o mem.o scancode.o link.o readfile.o \
		vm.o genimage.o stubs.o 

$(LINK.OBJECTS) : link.h vm.h 
vm.o : vm.h
link.o  : hash.c

# native linker
ld :  $(CSTART) $(LINK.OBJECTS)
	$(LINK) $(LINKFLAGS) -o$@ -n$@ -s2000 -h8000 $^

#-----------------------------------------------------------------------------
# Linker utility programs

ifeq ($(HPROC),ARM)	# should be moved to ARM directory
  # Object dump utility
  objdump : $(CSTART) objdump.o ARMshape.o
	$(LINK) $(LINKFLAGS) -o$@ -n$@ -s2000 -h4000 $^

  # ARM disassembler
  disas : $(CSTART) armdisas.o ARMshape.o
	$(LINK) $(LINKFLAGS) -o$@ -n$@ -s2000 -h4000 $^
else
  ifeq ($(HPROC),C40)	# should be moved to C40 directory
    # Object dump utility
    objdump : $(CSTART) objdump.o c40objdis.o
	$(LINK) $(LINKFLAGS) -o$@ -n$@ -s2000 -h4000 $^

    # 'C40 disassembler
    c40disas.o : c40target.h c40ops.h

    disas : $(CSTART) c40disas.o
	$(LINK) $(LINKFLAGS) -o$@ -n$@ -s2000 -h4000 $^
  else
    ifeq ($(HPROC),M68K)	# should be moved to M68K directory
      # Object dump utility
      objdump : $(CSTART) objdump.o 68kobjdis.o
	$(LINK) $(LINKFLAGS) -o$@ -n$@ -s2000 -h4000 $^

      # 68K disassembler
      68kdisas.o : 68ktarget.h 68kops.h

      disas : $(CSTART) 68kdisas.o
	$(LINK) $(LINKFLAGS) -o$@ -n$@ -s2000 -h4000 $^
    else
      # Object dump utility
      objdump : $(CSTART) objdump.o
	$(LINK) $(LINKFLAGS) -o$@ -n$@ -s2000 -h4000 $^
    endif
  endif
endif


# Image dump utility
imdump : $(CSTART) imdump.o
	$(LINK) $(LINKFLAGS) -o$@ -n$@ -s2000 -h4000 $^


#-----------------------------------------------------------------------------
endif #InProcMake	# This line must be at EOF
