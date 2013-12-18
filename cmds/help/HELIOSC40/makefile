#------------------------------------------------------------------------------
# Helios generic make system - NATIVE HOST MAKEFILE -
#-----------------------------------------------------------------------------
# SUN4 Host specific make rules to make NATIVE HOST version of HELP component.
#
# File: /HSRC/cmds/help/HELIOSTRAN/makefile
#
# This file contains ALL the definitions and rules that are
# required to make a NATIVE HOST version of this component.
# IT IS NOT COMPLIMENTARY TO THE GENERIC VERSION.
#
# This makefile is always executed within this host specific subdir.
# All host specific source files should be kept here.
# All intermediate, object and binary files should be made in here.
#
# RcsId: $Id: makefile,v 1.3 1994/03/17 12:24:45 nickc Exp $
#		(C) Copyright 1990 Perihelion Software
#
# WARNING: assumes you are using GNUmake.
#-----------------------------------------------------------------------------
# Manditory variables for a component:
# name of COMPONENT directory relative to $(HSRC)
COMPONENT = cmds/help

# Manditory includes
include $(HSRC)/makeinc/$(HHOST).mak	# Default host system variables

# Manditory phoney targets
.PHONY: default clean
# "default" - should make this component in its proc subdir
# "install" - copy the new component to $(HHOSTBIN) directory
# "clean"   - remove all intermediate and executable files from host subdir

VPATH := $(HSRC)/$(COMPONENT)

HHOSTLIB = $(HHOSTBIN)/../lib
#-----------------------------------------------------------------------------
# Note all final targets here

TARGETS = help dbbuild truncate dbdump dbprint database

default : $(TARGETS)

install	: help dbbuild dbdump truncate dbprint database
	$(CP) help dbbuild dbdump truncate dbprint $(HHOSTBIN)
	$(CP) $(HSRC)/$(COMPONENT)/$(HPROC)/help0.txt $(HHOSTLIB)
	$(CP) $(HSRC)/$(COMPONENT)/$(HPROC)/help0.dct $(HHOSTLIB)
	$(TOUCH) install

clean	:
	-$(RM) $(OBJECTS) $(TARGETS) mkout core ../mkout install\
	help.o dbbuild.o dbdump.o truncate.o dbprint.o
	-$(RM) help0.txt
	-$(RM) help0.dct

# Note all sources
SOURCES		= $(SOURCES.c) $(HEADERS.c) $(SOURCES.odd)

# Object files
OBJECTS         = big.o lrutils.o stem.o utils.o btree.o \
                  insert.o search.o storage.o lrucache.o \
                  seq.o delete.o lruhash.o split.o updutils.o
 
# C source files
SOURCES.c       = big.c help.c lrutils.c stem.c utils.c btree.c \
                  insert.c search.c storage.c dbbuild.c lrucache.c \
                  seq.c delete.c lruhash.c split.c updutils.c \
		  dbdump.c truncate.c dbprint.c
 
# C header files
HEADERS.c       = btree.h db.h endian.h lrucache.h
 
# Any other source files
SOURCES.odd     = makefile generic.txt c40.txt arm.txt transputer.txt \
		  vectlib.txt not_tran.txt stopword.lst dbmake

#-----------------------------------------------------------------------------
# Native Host targets and their dependencies:

NCFLAGS  := $(NCFLAGS) -i,../,/helios/include/ -j,../,/helios/include/ -D__HELIOS -D__C40 -D__HELIOSC40 -D_BSD

%.o : %.c
    $(NC) $(NCFLAGS) -c $<

dbprint : dbprint.o

dbdump : dbdump.o

truncate : truncate.o $(OBJECTS)
	c $< $(OBJECTS) -o truncate -lbsd

help : help.o $(OBJECTS)
	c $< $(OBJECTS) -o help -lbsd

dbbuild : dbbuild.o $(OBJECTS)
	c $< $(OBJECTS) -o dbbuild -lbsd

database:
ifeq ($(HPROC), TRAN)
	cat $(HSRC)/$(COMPONENT)/generic.txt \
	    $(HSRC)/$(COMPONENT)/transputer.txt \
	    $(HSRC)/$(COMPONENT)/vectlib.txt > alldoc
endif
ifeq ($(HPROC), ARM)
	cat $(HSRC)/$(COMPONENT)/generic.txt \
	    $(HSRC)/$(COMPONENT)/arm.txt \
	    $(HSRC)/$(COMPONENT)/not_tran.txt > alldoc
endif
ifeq ($(HPROC), C40)
	cat $(HSRC)/$(COMPONENT)/generic.txt \
	    $(HSRC)/$(COMPONENT)/c40.txt \
	    $(HSRC)/$(COMPONENT)/vectlib.txt \
	    $(HSRC)/$(COMPONENT)/not_tran.txt > alldoc
endif
	$(RM) $(HSRC)/$(COMPONENT)/$(HPROC)/help0.txt
	$(RM) $(HSRC)/$(COMPONENT)/$(HPROC)/help0.dct
	./dbmake
	$(RM) alldoc
#-----------------------------------------------------------------------------
