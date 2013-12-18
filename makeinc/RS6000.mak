#-----------------------------------------------------------------------------
# Helios generic make system - HOST SPECIFIC MAKEFILE -
#-----------------------------------------------------------------------------
# IBM/RS6000 Host system specific *DEFAULT* make rules.
# 
# File: /HSRC/makeinc/RS6000.mak
#
# This file contains definitions of variables and rules which are
# common to all Helios makefiles, or which need customizing to 
# a particular host. You may tailor to a particular processor via:
# ifeq($(HPROC),XXX) directives. This will allow you for instance, to select
# a particular compiler on this host to create XXX processor binaries.
#
# RcsId: $Id: RS6000.mak,v 1.11 1994/02/03 09:48:44 tony Exp $
#		(c) Copyright 1994 Perihelion Software
#
# WARNING: assumes you are using GNUmake.
#-----------------------------------------------------------------------------
# Host system directories:

ifndef HHOSTBIN
  HHOSTBIN	= /usr/local/bin## Where to place host utilities e.g. IO Server
endif

#-----------------------------------------------------------------------------
# Host system commands:

# For optional commands that do not exist, simply replace with dummy:
# e.g. DISASS = -@echo#

# Native host compiler (used to generate host utilities)
HOSTCC = xlc

# Directory to place temporary files during a compilation.
# Essential for compiling with maketran (at least).
TEMP =	/tmp

# Native host compiler flags: ANSI mode, Split module table
# If -DHOSTISBIGENDIAN is not set littleendian is defaulted to.
# @@@ -O causes the disassembler to core dump!
# M68K version notes that target is bigendian with __BIGENDIAN.

ifeq ($(HPROC),M68K)
  HOSTCFLAGS := $(HOSTCFLAGS) -DHOSTISBIGENDIAN -D$(HHOST) \
		-D_POSIX_SOURCE -D__$(HPROC) -D__HELIOS$(HPROC) \
		-D__BIGENDIAN
else
  HOSTCFLAGS := $(HOSTCFLAGS)  -DHOSTISBIGENDIAN -D__SMT -D$(HHOST) \
		-D_POSIX_SOURCE -D__$(HPROC) -D__HELIOS$(HPROC)
endif

# Cross C Compiler
ifeq ($(HPROC),TRAN)
  NC		= $(HHOSTBIN)/nc#	# Transputer C compil. on native Helios
else
  ifeq ($(HPROC),ARM)
    NC	= $(HHOSTBIN)/ccarm.32#	# ARM C compiler (NOTE: APCS-3 variant (even on ARCHIMEDES))
#   NC	= $(NONDIST)/bin/RS6000/armcc150 -li -apcs 3/reentrant -D__ARM -D__HELIOS -D__HELIOSARM -D__AOF#
  else
    ifeq ($(HPROC),I860)
      NC	= $(HHOSTBIN)/cci860#	# i860 C compiler on Native Helios
    else
      ifeq ($(HPROC),C40)
        NC	= $(HHOSTBIN)/ccc40#	# 'C40 C compiler 
      else
        ifeq ($(HPROC),M68K)
          NC	= $(HHOSTBIN)/ccm68k#	# 68K C compiler
	endif
      endif
    endif
  endif
endif

# Cross Compiler Driver, accepting Unix cc syntax
ifeq ($(HPROC),TRAN)
  CC = $(HHOSTBIN)/c#			# Separate compiler driver
else
  ifeq ($(HPROC),ARM)
    CC = $(HHOSTBIN)/c -xarm#		# Separate compiler driver
#    CC = no#
  else
    ifeq ($(HPROC),I860)
      CC = $(HHOSTBIN)/cci860#		# @@@ Built-in compiler driver ?
    else
      ifeq ($(HPROC),C40)
        CC = $(HHOSTBIN)/c -xc40#	# Separate compiler driver
      else
        ifeq ($(HPROC),M68K)
          CC = $(HHOSTBIN)/c -xm68#	# Separate compiler driver
	endif
      endif
    endif
  endif
endif

# Name of Cross linker
ifeq ($(HPROC),TRAN)
  LINK	= $(HHOSTBIN)/asm#
else
  ifeq ($(HPROC),ARM)
    LINK = $(HHOSTBIN)/ldarm#
#    LINK = /hsrc/cmds/linker/RS6000/ldarm#
  else
    ifeq ($(HPROC),I860)
      LINK = $(HHOSTBIN)/ldi860#
    else
      ifeq ($(HPROC),C40)
        LINK = $(HHOSTBIN)/ldc40#
      else
        ifeq ($(HPROC),M68K)
          LINK = $(HHOSTBIN)/ldm68k#
        endif
      endif
    endif
  endif
endif

# Name of Cross assembler
ifeq ($(HPROC),TRAN)
  ASM = $(HHOSTBIN)/asm#
else
  ifeq ($(HPROC),ARM)
    ASM = $(HHOSTBIN)/asarm#
  else
    ifeq ($(HPROC),I860)
      ASM = $(HHOSTBIN)/asi860#
    else
      ifeq ($(HPROC),C40)
        ASM = $(HHOSTBIN)/asc40#
      else
        ifeq ($(HPROC),M68K)
          ASM = $(HHOSTBIN)/asm68k#
        endif
      endif
    endif
  endif
endif

# Name of Cross diassembler
ifeq ($(HPROC),TRAN)
  DISASS = -@echo#
else
  ifeq ($(HPROC),ARM)
    DISASS = $(HHOSTBIN)/disasarm#
  else
    ifeq ($(HPROC),I860)
      DISASS = -@echo#
    else
      ifeq ($(HPROC),C40)
        DISASS = $(HHOSTBIN)/disasc40#
      else
        ifeq ($(HPROC),M68K)
          DISASS = $(HHOSTBIN)/disasm68k#
	endif
      endif
    endif
  endif
endif

# Name of object dump program
ifeq ($(HPROC),TRAN)
  OBJDUMP = -@echo#
else
  ifeq ($(HPROC),ARM)
    OBJDUMP = $(HHOSTBIN)/objdumparm#
  else
    ifeq ($(HPROC),I860)
      OBJDUMP = $(HHOSTBIN)/objdumpi860#
    else
      ifeq ($(HPROC),C40)
        OBJDUMP = $(HHOSTBIN)/objdumpc40#
      else
        ifeq ($(HPROC),M68K)
          OBJDUMP = $(HHOSTBIN)/objdumpm68k#
        endif
      endif
    endif
  endif
endif

TCP	= /bin/cp#			# text file copy
RMTEMP	= /bin/rm#			# remove temporary files
CP	= /bin/cp#			# binary file copy
CAT	= /bin/cat#			# text file cat (create using default protect.)
OBJED	= $(HHOSTBIN)/objed#		# object editor
AMPP	= $(HHOSTBIN)/ampp#		# macro preprocessor
RM	= /bin/rm -f#			# file remover
MKDIR	= /bin/mkdir#			# directory creator
CHMOD	= /bin/chmod#			# chmod (if applicable)
SYSBUILD = $(HHOSTBIN)/sysbuild		# nucleus image builder
TOUCH	= /bin/touch#			# update file modification time
#CPP     = /usr/ccs/lib/cpp -P#		# stand-alone C pre-processor
CPP     = $(NC) -E#			# C pre-processor
YACC	= $(HHOSTBIN)/byacc#		# Our own berkley yacc implementation

#-----------------------------------------------------------------------------
# Generic variables and rules for making Helios
#
# No changes should be necessary beyond this point
#
#-----------------------------------------------------------------------------
# Directories where things will be found...

INCLUDE	= $(HSRC)/include#	# standard include directory
NUCLEUS	= $(HSRC)/nucleus#	# nucleus source directory
KERNEL	= $(HSRC)/kernel#	# kernel source directory
UTIL	= $(HSRC)/util#		# util source directory
POSIX	= $(HSRC)/posix#	# posix source directory
CLIB	= $(HSRC)/cmds/cc/clib#	# C library source directory
FPLIB	= $(HSRC)/fplib#	# floating point library source directory
FAULT	= $(HSRC)/fault#	# fault library source directory
TCPIP	= $(HSRC)/tcpip#	# tcp/ip source directory
MACRO	= $(INCLUDE)/ampp#	# AMPP macro files
HINCLUDE = $(HSRC)/include/ARM/armasm#	# ARM Ltd assembler include files
CMDS	= $(HSRC)/cmds#		# Commands directory
EXTINCLUDE = $(NONDIST)/include## Include files not part of the Helios source tree
EXTLIB = $(NONDIST)/lib/RS6000#	# Host/Target libraries not part of the Helios source tree

BIN	= $(HPROD)/bin#		# production binaries
LIB	= $(HPROD)/lib#		# production libraries
ETC	= $(HPROD)/etc#		# production etc directory
TMP	= $(HPROD)/tmp#		# production temp directory

CSTART  = $(LIB)/cstart.o#	# Standard C runtime init. object file

NULL	= /dev/null		# Bit bucket

#-----------------------------------------------------------------------------
# Following two variables are NOT USED at present
# OEMDIR should be set from command line or environment, if not make a
# suitable default here
# *OEM source distribution directory*
ifndef OEMDIR
OEMDIR		= /hsrc/oemdir#
endif

# same for BACKUPDIR
# *BACKUP system directory*
ifndef BACKUPDIR
BACKUPDIR	= /hsrc/backup#
endif


#-----------------------------------------------------------------------------
# Rule to make native objects.
# This will be overlayed if processor specific rules are included later

.SUFFIXES: .o .c

%.o: %.c
	$(HOSTCC) $(HOSTCFLAGS) -c $<

#-----------------------------------------------------------------------------
# Default rules

firstrule: default	# default in user makefile is always first rule


# Standard backup target

tar:
	cd ..; rm -f $(notdir $(COMPONENT)).tar ; tar chf $(notdir $(COMPONENT)).tar *

srcnames:
	@echo $(foreach FILE, $(XSOURCES), $(COMPONENT)/$(HPROC)/$(FILE)) \
	$(foreach FILE, $(SOURCES), $(COMPONENT)/$(FILE)) >> $(HSRC)/backedupsrcs

#-----------------------------------------------------------------------------
# disable RCS extraction

% :: %,v

% :: RCS/%,v


# End of RS6000.mak
