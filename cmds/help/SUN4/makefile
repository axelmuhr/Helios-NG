#------------------------------------------------------------------------------
# Helios generic make system - NATIVE HOST MAKEFILE -
#-----------------------------------------------------------------------------
# SUN4 Host specific make rules to make NATIVE HOST version of HELP component.
#
# File: /HSRC/cmds/help/SUN4/makefile
#
# This file contains ALL the definitions and rules that are
# required to make a NATIVE HOST version of this component.
# IT IS NOT COMPLIMENTARY TO THE GENERIC VERSION.
#
# This makefile is always executed within this host specific subdir.
# All host specific source files should be kept here.
# All intermediate, object and binary files should be made in here.
#
# RcsId: $Id: makefile,v 1.8 1993/08/13 10:47:27 nickc Exp nickc $
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
	$(CP) help0.txt $(HHOSTLIB)
	$(CP) help0.dct $(HHOSTLIB)
	$(TOUCH) install

clean	:
	-$(RM) $(OBJECTS) $(TARGETS) mkout core ../mkout install\
	help.o dbbuild.o dbdump.o truncate.o dbprint.o

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

HOSTCFLAGS  := $(HOSTCFLAGS) -w -I/usr/include -I. -I$(INCLUDE) -DBYTE_ORDER=BIG_ENDIAN

LOCALHOSTCFLAGS  := -pipe -ansi -O -DHOSTISBIGENDIAN -DSUN4 -D__TRAN -D__HELIOSTRAN -w -I/usr/include -I. -I$(INCLUDE) -DBYTE_ORDER=BIG_ENDIAN

dbprint : dbprint.c
	gcc $< $(LOCALHOSTCFLAGS) -Bstatic -o dbprint

dbdump : dbdump.c
	gcc $< $(LOCALHOSTCFLAGS) -Bstatic -o dbdump

truncate : truncate.c $(OBJECTS)
	gcc $< $(OBJECTS) $(LOCALHOSTCFLAGS) -Bstatic -o truncate

help : help.c $(OBJECTS)
	gcc $< $(OBJECTS) $(LOCALHOSTCFLAGS) -Bdynamic -o help

dbbuild : dbbuild.c $(OBJECTS)
	gcc $< $(OBJECTS) $(LOCALHOSTCFLAGS) -Bstatic -o dbbuild

database:
	$(RM) $(HSRC)/$(COMPONENT)/TRAN/help0.txt
	$(RM) $(HSRC)/$(COMPONENT)/TRAN/help0.dct
	$(RM) $(HSRC)/$(COMPONENT)/C40/help0.txt
	$(RM) $(HSRC)/$(COMPONENT)/C40/help0.dct
	$(RM) $(HSRC)/$(COMPONENT)/ARM/help0.txt
	$(RM) $(HSRC)/$(COMPONENT)/ARM/help0.dct
	$(RM) ./help0.txt
	$(RM) ./help0.dct
	cat $(HSRC)/$(COMPONENT)/generic.txt \
	    $(HSRC)/$(COMPONENT)/transputer.txt \
	    $(HSRC)/$(COMPONENT)/vectlib.txt > alldoc
	./dbmake $(HSRC)/$(COMPONENT)/TRAN alldoc
	cat $(HSRC)/$(COMPONENT)/generic.txt \
	    $(HSRC)/$(COMPONENT)/arm.txt \
	    $(HSRC)/$(COMPONENT)/not_tran.txt > alldoc
	./dbmake $(HSRC)/$(COMPONENT)/ARM alldoc
	cat $(HSRC)/$(COMPONENT)/generic.txt \
	    $(HSRC)/$(COMPONENT)/c40.txt \
	    $(HSRC)/$(COMPONENT)/vectlib.txt \
	    $(HSRC)/$(COMPONENT)/not_tran.txt > alldoc
	./dbmake $(HSRC)/$(COMPONENT)/C40 alldoc
	cat $(HSRC)/$(COMPONENT)/generic.txt \
	    $(HSRC)/$(COMPONENT)/transputer.txt \
	    $(HSRC)/$(COMPONENT)/arm.txt \
	    $(HSRC)/$(COMPONENT)/c40.txt \
	    $(HSRC)/$(COMPONENT)/vectlib.txt \
	    $(HSRC)/$(COMPONENT)/not_tran.txt > alldoc
	./dbmake . alldoc
	$(RM) alldoc
#-----------------------------------------------------------------------------
