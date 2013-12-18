#-----------------------------------------------------------------------------
# Helios generic make system - HOST SPECIFIC MAKEFILE -
#-----------------------------------------------------------------------------
# native *HELIOS* Host system specific *DEFAULT* make rules.
# 
# File: /HSRC/makeinc/HELIOSTRAN.mak
#
# This file contains definitions of variables and rules which are
# common to all Helios makefiles, or which need customizing to 
# a particular host. You may tailor to a particular processor via:
# ifeq($(HPROC),XXX) directives. This will allow you for instance, to select
# a particular compiler on this host to create XXX processor binaries.
#
# SccsId: %W% %G%
# RcsId: $Id: HELIOSC40.mak,v 1.2 1993/05/26 15:17:01 bart Exp $
#		(C) Copyright 1990 Perihelion Software
#
# WARNING: assumes you are using GNUmake.
#-----------------------------------------------------------------------------
# Host system directories:

ifndef HHOSTBIN
  HHOSTBIN = /helios/bin	# Where to place host utilities e.g. IO Server
endif

SCRATCHEXISTS := $(notdir $(strip $(wildcard /pds/.)))
ifeq ($(SCRATCHEXISTS),.)
     TEMP := /pds#
else
     TEMP :=.#
endif

RSRCEXISTS := $(notdir $(strip $(wildcard /hsrc/.)))
ifeq ($(RSRCEXISTS),.)
     RSRC := /hsrc
endif

#TEMP	= /pds#		# dir for temporary files (/ram, /fifo, etc)
#TEMP	= .#		# dir for temporary files (/ram, /fifo, etc)
NULL	= /null			# Bit bucket

#-----------------------------------------------------------------------------
# Host system commands:

# For optional commands that do not exist, simply replace with dummy:
# e.g. DISASS = -@echo#

# Native host compiler (used to generate host utilities)
HOSTCC = c

# Cross C Compiler
ifeq ($(HPROC),TRAN)
  NC		= cc#			# Transputer C compil. on native Helios
  CC		= c#			# Unix-style compiler
else
  ifeq ($(HPROC),ARM)
    NC		= ncc#			# ARM C compiler on Native Helios
    CC		= ncc#
  else
    ifeq ($(HPROC),I860)
      NC	= ncci860#		# i860 C compiler on Native Helios
    else
      ifeq ($(HPROC),C40)
        NC		= ncc#		# C40 C compiler on Native Helios
        CC		= c#
      endif
    endif
  endif
endif

# Name of Cross linker
ifeq ($(HPROC),TRAN)
  LINK	= asm#
else
  ifeq ($(HPROC),ARM)
    LINK = armlink#
  else
    ifeq ($(HPROC),I860)
      LINK = i860link#
    else
      ifeq ($(HPROC),C40)
        LINK = ld#
      endif
    endif
  endif
endif

# Name of Cross assembler
ifeq ($(HPROC),TRAN)
  ASM = asm#
else
  ifeq ($(HPROC),ARM)
    ASM = hobjasm#
  else
    ifeq ($(HPROC),I860)
      ASM = i860asm#
    else
      ifeq ($(HPROC),C40)
        ASM = as#
      endif
    endif
  endif
endif

# Name of Cross diassembler
ifeq ($(HPROC),TRAN)
  DISASS = -@echo#
else
  ifeq ($(HPROC),ARM)
    DISASS = armdis#
  else
    ifeq ($(HPROC),I860)
      DISASS = -@echo#
    else
      ifeq ($(HPROC),C40)
        DISASS = disas
      endif
    endif
  endif
endif

# Name of object dump program
ifeq ($(HPROC),TRAN)
  OBJDUMP = -@echo#
else
  ifeq ($(HPROC),ARM)
    OBJDUMP = objdump#
  else
    ifeq ($(HPROC),I860)
      OBJDUMP = objdump#
    else
      ifeq ($(HPROC),C40)
        OBJDUMP = objdump#
      endif
    endif
  endif
endif

TCP	= tcp#			# text file copy
RMTEMP	= rm#			# remove temporary files
CP	= cp#			# binary file copy
OBJED	= objed#		# object editor
AMPP	= ampp#			# macro preprocessor
RM	= rm#			# file remover
MKDIR   = mkdir#		# directory creator
CHMOD   = echo#
SYSBUILD = sysbuild#		# nucleus image builder
TOUCH	= touch#		# update file modification time
CPP	= ncc -E		# C preprocessor
YACC	= /helios/local/bin/yacc#

#BACKUP	= backup -t#		# backup utility
#UPLOAD	= upload#		# upload utility

LIBCPP	= /helios/lib/cpp#	# stand-alone C preprocessor

#-----------------------------------------------------------------------------
# Generic variables and rules for making Helios
#
# No changes should be necessary beyond this point
#
#-----------------------------------------------------------------------------
# Directories where things will be found...

INCLUDE	= /helios/include#	# standard include directory
NUCLEUS	= $(HSRC)/nucleus#	# nucleus source directory
KERNEL	= $(HSRC)/kernel#	# kernel source directory
UTIL	= $(HSRC)/util#		# util source directory
POSIX	= $(HSRC)/posix#	# posix source directory
CLIB	= $(HSRC)/cmds/cc/clib#	# C library source directory
FPLIB	= $(HSRC)/fplib#	# floating point library source directory
FAULT	= $(HSRC)/fault#	# fault library source directory
TCPIP	= $(HSRC)/tcpip#	# tcp/ip source directory
MACRO	= /helios/include/ampp#	# AMPP macro files
CMDS	= $(HSRC)/cmds#		# Commands directory

BIN	= $(HPROD)/bin#		# production binaries
LIB	= $(HPROD)/lib#		# production libraries
ETC	= $(HPROD)/etc#		# production etc directory
TMP	= $(HPROD)/tmp#		# production temp directory

CSTART  = $(LIB)/cstart.o#	# Standard C runtime init. object file


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


#------------------------
