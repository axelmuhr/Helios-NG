#-----------------------------------------------------------------------------
# Helios generic make system - HOST SPECIFIC MAKEFILE -
#-----------------------------------------------------------------------------
# SUN4 Host system specific *DEFAULT* make rules.
# 
# File: /HSRC/makeinc/SUN.mak
#
# This file contains definitions of variables and rules which are
# common to all Helios makefiles, or which need customizing to 
# a particular host. You may tailor to a particular processor via:
# ifeq($(HPROC),XXX) directives. This will allow you for instance, to select
# a particular compiler on this host to create XXX processor binaries.
#
# SccsId: %W% %G%
# RcsId: $Id: SUN3.mak,v 1.1 1993/01/28 15:55:01 bart Exp $
#		(C) Copyright 1990 Perihelion Software
#
# WARNING: assumes you are using GNUmake.
#-----------------------------------------------------------------------------
# Host system directories:

ifndef HHOSTBIN
  HHOSTBIN	= /usr/local/bin#	# Where to place host utilities e.g. IO Server
endif

TEMP		= /tmp#		# dir for temporary files (/ram, /fifo, etc)


#-----------------------------------------------------------------------------
# Host system commands:

# For optional commands that do not exist, simply replace with dummy:
# e.g. DISASS = -@echo#

# Native host compiler (used to generate host utilities)
 HOSTCC = cc
#HOSTCC = gcc

# Native host compiler flags
 HOSTCFLAGS := $(HOSTCFLAGS) -pipe -O1 -DHOSTISBIGENDIAN -D$(HHOST) \
	-D__$(HPROC) -D__HELIOS$(HPROC)
#HOSTCFLAGS := $(HOSTCFLAGS) -ansi -pipe -O -DHOSTISBIGENDIAN -D$(HHOST) \
#	-D__$(HPROC) -D__HELIOS$(HPROC) -Dsparc 

# Cross C Compiler
ifeq ($(HPROC),TRAN)
  NC		= nc#			# Transputer C compil. on native Helios
else
  ifeq ($(HPROC),ARM)
    NC		= nccarm#			# ARM C compiler on Native Helios
  else
    ifeq ($(HPROC),I860)
      NC	= ncci860#		# i860 C compiler on Native Helios
    endif
  endif
endif

# Cross Compiler Driver, accepting Unix cc syntax
ifeq ($(HPROC),TRAN)
  CC		= c#			# Separate compiler driver
else
  ifeq ($(HPROC),ARM)
    CC		= nccarm#			# Built-in compiler driver
  else
    ifeq ($(HPROC),I860)
      CC	= ncci860#		# Built-in compiler driver ?
    else
      ifeq ($(HPROC),C40)
        CC      = ccc40#
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
        LINK = ldc40#
      endif
    endif
  endif
endif

# Name of Cross assembler
ifeq ($(HPROC),TRAN)
  ASM = asm#
else
  ifeq ($(HPROC),ARM)
#    ASM = hobjasm#
    ASM = as#
  else
    ifeq ($(HPROC),I860)
      ASM = i860asm#
    else
      ifeq ($(HPROC),C40)
        ASM = asc40#
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
    endif
  endif
endif

TCP	 = cp#			# text file copy
RMTEMP	 = rm#			# remove temporary files
CP	 = cp#			# binary file copy
OBJED	 = objed#		# object editor
AMPP	 = ampp#			# macro preprocessor
RM	 = rm -f#			# file remover
MKDIR	 = mkdir#		# directory creator
CHMOD	 = chmod#		# chmod (if applicable)
SYSBUILD = sysbuild#		# nucleus image builder
TOUCH	 = touch#		# update file modification time

#BACKUP	 = backup -t#		# backup utility
#UPLOAD	 = upload#		# upload utility

CPP  = /lib/cpp#		# stand-alone C pre-processor

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
# *OEM source distribution directory*
ifndef OEMDIR
OEMDIR		= /giga/HeliosRoot/Helios/oemdir#
endif

# same for BACKUPDIR
# *BACKUP system directory*
ifndef BACKUPDIR
BACKUPDIR	= /giga/HeliosRoot/Helios/backup#
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

