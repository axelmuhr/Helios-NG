#-----------------------------------------------------------------------------
# Helios generic make system
#-----------------------------------------------------------------------------
# Helios generic component make rules for: KERNEL
#
# File: /HSRC/kernel/makefile
#
# These are the host/processor independent rules and variables for this
# component of Helios. It is included by the processor specific makefile
# That may add to some of its functionality. You should only alter this
# makefile by using "ifneq ($(HPROC),YYY)" clauses for a specific processor.
#
# SccsId: %W% %G%
# RcsId: $Id: makefile,v 1.39 1993/10/04 15:07:19 paul Exp $
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
COMPONENT = kernel

# Manditory includes
include $(HSRC)/makeinc/$(HHOST).mak
include $(HSRC)/makeinc/$(HPROC).mak

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
vpath %.c $(HVPATH)
vpath %.a $(HVPATH)


#-----------------------------------------------------------------------------
# Generic component make rules and variables added below here

ifeq ($(HPROC),TRAN)
  BOOTER =  nboot
else
  ifeq ($(HPROC),C40)
    BOOTER = c40boot
  endif
endif

# Note all final generic targets here
ifdef BOOTER
  TARGETS	= kernel.i kernel.def $(BOOTER).i
else
  TARGETS	= kernel.i kernel.def
endif

ifeq ($(HPROC),C40)
  TARGETS := $(TARGETS) kernel.sml
endif

default : $(TARGETS) Xdefault


install	: installdef $(TARGETS) Xinstall
ifeq ($(HLICENCEE),ABC)
	sysstrip kernel.i -o$(LIB)/kernel
endif
ifdef BOOTER
	$(CP) $(BOOTER).i $(LIB)
endif
	$(TOUCH) install

ifeq ($(HPROC),ARM)
installdef: $(LIB)/kernel.def \
	    $(INCLUDE)/ARM/trapdefs.h $(INCLUDE)/ampp/ARM/trapdefs.m
	$(TOUCH) installdef
else
installdef: $(LIB)/kernel.def
	$(TOUCH) installdef
endif

$(LIB)/kernel.def : kernel.def
ifeq (HELIOS,$(findstring HELIOS,$(HHOST)))
	(cmp $< $@ ; if ( $$status ) $(CP) $< $@ )
else
	@ if cmp $< $@; then echo "$< not changed"; else $(CP) $< $@; fi
endif


clean	: Xclean
	-$(RM)	$(OBJECTS) $(TARGETS) $(XOBJECTS) $(XTARGETS) \
		mkout core ../mkout install installdef Xinstall


#-----------------------------------------------------------------------------
# for archive, backup and cleaning purposes,
# note all generic SOURCE and OBJECT files:

SOURCES	=	$(SOURCES.c) $(SOURCES.a) $(HEADERS.c) $(HEADERS.m) $(SOURCES.odd)

#Full set of library object files
OBJECTS	= 	$(SOURCES.c:.c=.p) $(SOURCES.a:.a=.p)

# C source files
SOURCES.c =	event.c getmsg1.c kill1.c kstart.c link1.c \
		linkmsg1.c memory1.c port1.c putmsg1.c queue1.c \
		sem1.c task.c sastubs.c sml.c smlstub.c \
		gexec.c gslice.c glinkio.c gdebug.c nccsupport.c smlgen.c

# AMPP source files
SOURCES.a =	kend.a kmodule.a

# C header files
HEADERS.c =	kernel.h mcdep.h gexec.h sml.h

# AMPP macro files
HEADERS.m =     gexec.m


SOURCES.odd = makefile

#-----------------------------------------------------------------------------
# Generic targets and their dependencies:

# Kernel options include:
#	-dLATENCYTEST 1 (C40 debugging only)

ifdef SYSDEB
  AMPPFLAGS := $(AMPPFLAGS) -dSYSDEB 1
endif

# Turn off stack check and function header generation.
# Define particular kernel options to include:
#	-DCODES = fixed error code array - used by tranny to reduce kernel size
#	-DFLOWCONTROL = flow control protocol on the links
#	-DKERNELDEBUG(1/2/3) - kernel debug KDebug() messaging options (!__TRAN)
#	-DLATENCYTEST (C40 debugging only) Get Max Dispatch latency figures

ifeq ($(HPROC),ARM)
#	-Zt = tiny memory model.
  NCFLAGS := $(NCFLAGS) -Zps1 -I$(NUCLEUS)/ARM -DKERNELDEBUG3 -DFLOWCONTROL -Zt
  AMPPFLAGS := $(AMPPFLAGS)
else
  ifeq ($(HPROC),C40)
# -Fa to report used-before-set
# -Ff to disable function header generation
# -Fh to report globals not in headers
# -Fo to report old style K&R functions
# -Fp to report converting pointers to integers
# -Fv to report unused variables
# -Zpw1 when using static data areas < 64k
# -Zpl1 when using few modules < 256
# -Zpc1 to check all memory accesses (can only be use in some kernel modules)
# -Zpm1 to enable backtraces
# -Zpn1 to disable peephole optimisations
# -Zps1 to disable stack checking
   ifdef SYSDEB
     NCFLAGS := $(NCFLAGS) -DKERNELDEBUG3
     AMPPFLAGS := $(AMPPFLAGS) -dSYSDEB 1
   endif
   NCFLAGS :=  -I$(HSRC)/$(COMPONENT) $(NCFLAGS) -Zpw1 -Zps1 -DFLOWCONTROL #-DLATENCYTEST
   AMPPFLAGS := $(AMPPFLAGS) #-dLATENCYTEST 1
  else
    ifeq ($(HPROC),TRAN)
       NCFLAGS := $(NCFLAGS) -t4 -ps1 -pf0 -DTRANSPUTER -DCODES -DFLOWCONTROL
    #  NCFLAGS := $(NCFLAGS) -t4 -ps1 -pf0 -DTESTER
    endif
  endif
endif

#-----------------------------------------------------------------------------
# Normal Helios Kernel
#
# The list of objects must always be terminated with "kend.p"

  STDKERNEL.OBJECTS = 	kmodule.p kstart.p queue1.p sem1.p port1.p \
			putmsg1.p getmsg1.p kill1.p link1.p linkmsg1.p \
			memory1.p task.p event.p

ifeq ($(HPROC),ARM)
   # Generic executive objects.
   GEXEC.OBJS =		gexec.p gslice.p glinkio.p nccsupport.p gdebug.p

   # ARM specific objects.
   ARMEXEC.OBJS =	armnccsupp.p armexec.p armextra.p \
			trapinit.p trapstubs.p armcexec.p


   # Platform specific objects.
ifeq ($(HPLATFORM),VY86PID)
   # VLSI VY86PID ARM600 development card target
   # Last three objects are derived from traphand.a armintr.a and armslice.a.
   PLAT.OBJS =		hw_pida.p hw_pidc.p hw_serialc.p hw_seriala.p \
			pid_traphand.p pid_slice.p pid_intr.p
else
   # Acorn Archimedies 3/400 series
   # Last three objects are derived from armintr.a and armslice.a.
   PLAT.OBJS =		hw_archa.p hw_archc.p hw_glapc.p hw_glapa.p \
			arch_traphand.p arch_slice.p arch_intr.p
endif

   KERNEL.OBJECTS = 	$(STDKERNEL.OBJECTS) $(GEXEC.OBJS) $(ARMEXEC.OBJS) \
			$(PLAT.OBJS) kend.p

   $(STDKERNEL.OBJS) $(GEXEC.OBJS): kernel.h mcdep.h
   $(GEXEC.OBJS) :	gexec.h
   $(ARMEXEC.OBJS) :	armexec.m armexec.h
else
  ifeq ($(HPROC),C40)
    KERNEL.OBJECTS = 	$(STDKERNEL.OBJECTS) smlstub.p gexec.p gslice.p glinkio.p \
			nccsupport.p c40nccsupp.p c40exec.p \
			c40intr.p c40slice.p gdebug.p c40linkio.p \
			c40dma.p stack_a.p stack_c.p c40specials.p kend.p

    SMLKERNEL.OBJECTS = $(STDKERNEL.OBJECTS) sml.p gexec.p gslice.p glinkio.p \
			nccsupport.p c40nccsupp.p c40exec.p \
			c40intr.p c40slice.p gdebug.p c40linkio.p \
			c40dma.p stack_a.p stack_c.p c40specials.p kend.p

    $(KERNEL.OBJECTS): kernel.h mcdep.h
  else
    ifeq ($(HPROC),TRAN)
      KERNEL.OBJECTS = 	$(STDKERNEL.OBJECTS) romsupp.p tranasm.p \
			kend.p

      $(KERNEL.OBJECTS): kernel.h mcdep.h
    endif
  endif
endif


kstart.p: $(INCLUDE)/root.h		# depends on sizeof(RootStruct)

#ifeq ($(HPROC),TRAN)
#  kernel.d kernel.i : $(KERNEL.OBJECTS)
#	$(LINK.LIBRARY) -nKernel -dkernel.d $^ -okernel.i
#else
  kernel.i : $(KERNEL.OBJECTS)
	$(LINK.LIBRARY) -nKernel $^ -o$@
#endif

ifeq ($(HPROC),C40)

$(SMLKERNEL.OBJECTS): kernel.h mcdep.h

  kernel.sml : $(SMLKERNEL.OBJECTS)
	$(LINK.LIBRARY) -nKernel $^ -o$@

endif

ifeq ($(HPROC),ARM)
arch_traphand.p: traphand.a
	$(AMPP) $(AMPPFLAGS) $(MACRO)/basic.m traphand.a > $(TEMP)/$*.s
	$(ASM) $(ASMFLAGS) -o$*.p $(TEMP)/$*.s

arch_slice.p: armslice.a
	$(AMPP) $(AMPPFLAGS) $(MACRO)/basic.m armslice.a > $(TEMP)/$*.s
	$(ASM) $(ASMFLAGS) -o$*.p $(TEMP)/$*.s

arch_intr.p: armintr.a
	$(AMPP) $(AMPPFLAGS) $(MACRO)/basic.m armintr.a > $(TEMP)/$*.s
	$(ASM) $(ASMFLAGS) -o$*.p $(TEMP)/$*.s

pid_traphand.p: traphand.a
	$(AMPP) $(AMPPFLAGS) $(MACRO)/basic.m traphand.a > $(TEMP)/$*.s
	$(ASM) $(ASMFLAGS) -o$*.p $(TEMP)/$*.s

pid_slice.p: armslice.a
	$(AMPP) $(AMPPFLAGS) $(MACRO)/basic.m armslice.a > $(TEMP)/$*.s
	$(ASM) $(ASMFLAGS) -o$*.p $(TEMP)/$*.s

pid_intr.p: armintr.a
	$(AMPP) $(AMPPFLAGS) $(MACRO)/basic.m armintr.a > $(TEMP)/$*.s
	$(ASM) $(ASMFLAGS) -o$*.p $(TEMP)/$*.s

endif

#-----------------------------------------------------------------------------
# Kernel Debug Map
#

ifeq ($(HPROC),C40)
# nucleus load point + nucleus header + kernel image header
/tmp/kdebugmap: $(KERNEL.OBJECTS)
#	$(OBJDUMP) +12587265 +36 -d $(KERNEL.OBJECTS) > /tmp/kdebugmap &
	$(OBJDUMP) +36 -d $(KERNEL.OBJECTS) > /tmp/kdebugmap &
endif

ifeq ($(HPROC),ARM)
# nucleus load point + nucleus header + kernel image header
/tmp/kdebugmap.arm: $(KERNEL.OBJECTS)
	$(OBJDUMP) -d $(KERNEL.OBJECTS) > /tmp/kdebugmap.arm &
endif

#-----------------------------------------------------------------------------
# Stand Alone Kernel
#

SA.OBJECTS 	= 	kmodule.p kstart.p sastubs.p queue1.p sem1.p \
			memory1.p task.p tranasm.p kend.p

kernel.sa	: $(SA.OBJECTS)
		$(LINK.LIBRARY) -nKernel $^ -o$@
		$(CP) $@ $(LIB)
ifdef SYSDEB
		$(CP) $@ /helios/lib
endif

#-----------------------------------------------------------------------------
# Definitions file
#

ifeq ($(HPROC),ARM)	# generate trap definition and init files automatically

# automatically produce the trap number definitions for C and AMPP
trapdefs.h:  kmodule.a
	$(AMPP) -dmake.trapCdefs 1 $(AMPPFLAGS) $(MACRO)/basic.m $< > $@

trapdefs.m:  kmodule.a
	$(AMPP) -dmake.trapAdefs 1 $(AMPPFLAGS) $(MACRO)/basic.m $< > $@

# automatically produce the trap table initialisation
trapinit.p:  kmodule.a armexec.m
	$(AMPP) -dmake.traps 1 $(AMPPFLAGS) $(MACRO)/basic.m ../kmodule.a > trapinit.a
	$(AMPP) $(AMPPFLAGS) $(MACRO)/basic.m trapinit.a > $(TEMP)/trapinit.s
	$(ASM) $(ASMFLAGS) -o$@ $(TEMP)/trapinit.s

# automatically produce the trap stubs (temporary fix)
trapstubs.p:  kmodule.a trapdefs.m
	$(AMPP) -dmake.trapstubs 1 $(AMPPFLAGS) $(MACRO)/basic.m ../kmodule.a > trapstubs.a
	$(AMPP) $(AMPPFLAGS) $(MACRO)/basic.m trapstubs.a > $(TEMP)/trapstubs.s
	$(ASM) $(ASMFLAGS) -o$@ $(TEMP)/trapstubs.s
endif


# produce the shared library definition file
kernel.def: kmodule.a
	$(MAKE.DEF)


# *WARNING* kernel.d is made automatically in Transputer version.
# In ARM/C40 and other processor versions it is kept up to date with kmodule.a
# by hand. Each processor therfore has a different version held in its subdir.
#kernel.p	: kernel.d

#kernel.def	: xkernel.def # Causes GNU Make core dump!

#xkernel.def	: kernel.p
#		$(CP) $< $@


#-----------------------------------------------------------------------------
# Transputer bootstrap (should not be here)
#

ifeq ($(HPROC),TRAN)
  $(BOOTER).i		: $(BOOTER).p
		$(LINK) -N $< -o $@
else
  $(BOOTER).i	: $(BOOTER).p
		$(LINK) $< -o $@
endif


ifeq ($(HPROC),TRAN)
  testtask.i : testtask.p
	$(ASM) $(LIB)/sstart.o testtask.x -o$@

  testtask.x : testtask.c
	$(CC) -t4 -ps1 -pf0 -dTESTER -j,/helios/include/ $*.c -s$*.x

  exec.x : exec.c
	$(CC) -r -t4 -ps1 -pf0 -dTESTER -j,/helios/include/ $*.c -s$*.x
endif


ifeq ($(HPROC),C40)

sml.p	: sml.c sml.h smlgen.c

endif

endif #InProcMake



# end of kernel makefile
