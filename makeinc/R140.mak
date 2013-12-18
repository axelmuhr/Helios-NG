#-----------------------------------------------------------------------------
# Helios generic make system - HOST SPECIFIC MAKEFILE -
#-----------------------------------------------------------------------------
# Acorn R140 Host system specific *DEFAULT* make rules.
# 
# File: /HSRC/makeinc/R140.mak
#
# This file contains definitions of variables and rules which are
# common to all Helios makefiles, or which need customizing to 
# a particular host. You may tailor to a particular processor via:
# ifeq($(HPROC),XXX) directives. This will allow you for instance, to select
# a particular compiler on this host to create XXX processor binaries.
#
# SccsId: %W% %G%
# RcsId: $Id: R140.mak,v 1.21 1993/07/26 17:24:31 paul Exp $
#		(C) Copyright 1990 Perihelion Software
#
# WARNING: assumes you are using GNUmake.
#-----------------------------------------------------------------------------
# Host system directories:

ifndef HHOSTBIN
  HHOSTBIN	= /usr/local/bin## Where to place host utilities e.g. IO Server
endif

TEMP		= /tmp#		# dir for temporary files (/ram, /fifo, etc)


#-----------------------------------------------------------------------------
# Host system commands:

# For optional commands that do not exist, simply replace with dummy:
# e.g. DISASS = -@echo#

# Native host compiler (used to generate host utilities)
HOSTCC = cc

# defaults to host is littleendian unless -DHOSTISBIGENDIAN is set
HOSTCFLAGS := $(HOSTCFLAGS) -D__SMT -D$(HHOST) -D__$(HPROC) -D__HELIOS$(HPROC)



# Cross C Compiler
ifeq ($(HPROC),TRAN)
  NC		= $(HHOSTBIN)/nc#	# Transputer C compil. on native Helios
else
  ifeq ($(HPROC),ARM)
    NC		= $(HHOSTBIN)/ccarm#	# ARM C compiler on Native Helios
  else
    ifeq ($(HPROC),I860)
      NC	= $(HHOSTBIN)/cci860#	# i860 C compiler on Native Helios
    else
      ifeq ($(HPROC),C40)
        NC	= $(HHOSTBIN)/ccc40#	# 'C40 C compiler on Native Helios
      endif
    endif
  endif
endif

# Cross Compiler Driver, accepting Unix cc syntax
ifeq ($(HPROC),TRAN)
  CC = $(HHOSTBIN)/c#			# Separate compiler driver
else
  ifeq ($(HPROC),ARM)
    CC = $(HHOSTBIN)/ccarm#		# Built-in compiler driver
  else
    ifeq ($(HPROC),I860)
      CC = $(HHOSTBIN)/cci860#		# @@@ Built-in compiler driver ?
    else
      ifeq ($(HPROC),C40)
        CC = $(HHOSTBIN)/c#		# Separate compiler driver
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
  else
    ifeq ($(HPROC),I860)
      LINK = $(HHOSTBIN)/ldi860#
    else
      ifeq ($(HPROC),C40)
      LINK = $(HHOSTBIN)/ldc40#
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
CPP	= cc -E -C -ansi		# standalone C pre-processor
#CPP	= /usr/lib/cpp -E -P -C		# standalone C pre-processor
#(HP) CPP     = /bin/cc -E -C -P -Aa	# stand-alone C pre-processor



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
PATCHLIB = $(HSRC)/patchlib#	# patch library source directory
FAULT	= $(HSRC)/fault#	# fault library source directory
TCPIP	= $(HSRC)/tcpip#	# tcp/ip source directory
MACRO	= $(HSRC)/include/ampp#	# AMPP macro files
HINCLUDE = $(HSRC)/include/abcARM/asm#	# hobjasm include files
CMDS	= $(HSRC)/cmds#		# Commands directory

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
ifndef OEMDIR
OEMDIR		= /a/helios#		# OEM source distribution directory
endif

# same for BACKUPDIR
ifndef BACKUPDIR
BACKUPDIR	= /c/helios#		# BACKUP system directory
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

tardir:
	cd ..; rm -f $(notdir $(COMPONENT)).tar ; tar chf $(notdir $(COMPONENT)).tar *

srcnames:
	@echo $(foreach FILE, $(XSOURCES), $(COMPONENT)/$(HPROC)/$(FILE)) \
	$(foreach FILE, $(SOURCES), $(COMPONENT)/$(FILE)) >> $(HSRC)/backedupsrcs

ifdef SYSDEB
#-----------------------------------------------------------------------------
# disable RCS extraction

% :: %,v

% :: RCS/%,v

endif

