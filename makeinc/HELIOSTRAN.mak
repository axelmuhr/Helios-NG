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
# RcsId: $Id: HELIOSTRAN.mak,v 1.12 1992/09/23 16:39:50 martyn Exp $
#		(C) Copyright 1990 Perihelion Software
#
# WARNING: assumes you are using GNUmake.
#-----------------------------------------------------------------------------
# Host system directories:

ifndef HHOSTBIN
  HHOSTBIN = /helios/bin	# Where to place host utilities e.g. IO Server
endif

#TEMP	= /00/fifo#		# dir for temporary files (/ram, /fifo, etc)
TEMP	= /00/ram#		# (what TELMAT use)
NULL	= /null			# Bit bucket

#-----------------------------------------------------------------------------
# Host system commands:

# For optional commands that do not exist, simply replace with dummy:
# e.g. DISASS = -@echo#

# Native host compiler (used to generate host utilities)
HOSTCC = c

HOSTCFLAGS := $(HOSTCFLAGS) -D$(HHOST) -D__$(HPROC) -D__HELIOS$(HPROC)

# Cross C Compiler
ifeq ($(HPROC),TRAN)
  NC		= cc#			# Transputer C compil. on native Helios
else
  ifeq ($(HPROC),ARM)
    NC		= ncc#			# ARM C compiler on Native Helios
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

TCP	= tcp#			# text file copy
#RMTEMP	= @echo#rm#		# remove temporary files
RMTEMP	= rm#			#  (what TELMAT use)
CP	= cp#			# binary file copy
OBJED	= objed#		# object editor
AMPP	= ampp#			# macro preprocessor
RM	= rm -f#		# file remover
MKDIR	= mkdir#		# directory creator
CHMOD	= unset#		# chmod not applicable (unset does nowt)
SYSBUILD = sysbuild#		# nucleus image builder
TOUCH	= touch#		# update file modification time

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

INCLUDE	= $(HSRC)/include#	# standard include directory
NUCLEUS	= $(HSRC)/nucleus#	# nucleus source directory
KERNEL	= $(HSRC)/kernel#	# kernel source directory
UTIL	= $(HSRC)/util#		# util source directory
POSIX	= $(HSRC)/posix#	# posix source directory
CLIB	= $(HSRC)/cmds/cc/clib#	# C library source directory
FPLIB	= $(HSRC)/fplib#	# floating point library source directory
FAULT	= $(HSRC)/fault#	# fault library source directory
TCPIP	= $(HSRC)/tcpip#	# tcp/ip source directory
CMDS	= $(HSRC)/cmds#		# commands source directory
MACRO	= $(INCLUDE)/ampp#	# AMPP macro files

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


#-----------------------------------------------------------------------------
# Rule to make native objects.
# This will be overlayed if processor specific rules are included later

.SUFFIXES: .o .c

%.o: %.c
	$(HOSTCC) $(HOSTCFLAGS) -D$(HHOST) -c $<

#-----------------------------------------------------------------------------
# This ensures that just going makeXXX makes the default target

FirstTarget: default

#-----------------------------------------------------------------------------
# Standard backup target

tar:
	cd ..; -rm -f $(notdir $(COMPONENT)).tar ; tar cf $(notdir $(COMPONENT)).tar *

srcnames:
	@echo $(foreach FILE, $(XSOURCES), $(COMPONENT)/$(HPROC)/$(FILE)) \
	$(foreach FILE, $(SOURCES), $(COMPONENT)/$(FILE)) >> $(HSRC)/backedupsrcs

ifdef SYSDEB
#-----------------------------------------------------------------------------
# disable RCS extraction

% :: %,v

% :: RCS/%,v

endif


