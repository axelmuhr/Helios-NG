#
# Makefile for the Machine Dependent Back End to the Norcroft compiler
#
# Copyright (c) 1992 Perihelion Software Ltd
# All Rights Reserved
#
# Author: N Clifton
# RCS Id: $Id: makefile,v 1.2 1994/03/29 10:33:19 nickc Exp $
#

# object files

OBJS = 	$(DEST)/gen.o     \
	$(DEST)/mcdep.o   \
	$(DEST)/asm.o     \
	$(DEST)/debug.o   \
	$(DEST)/peep.o    \
	$(DEST)/heliobj.o 

default:
	cd .. ; gmake all

all: $(OBJS)
	
install:
	cd ..; gmake install-RS6000

install-HP:
	cd ..; gmake install-HP

install-RS6000:
	cd ..; gmake install-RS6000

clean:
	rm -f *.o
	rcsclean
	co -u -q RCS/*

#dependencies

$(DEST)/gen.o: 		gen.c
	$(CC) $(CFLAGS) $(DEFINES) -c gen.c
	$(MV) gen.o     $(DEST)/gen.o

$(DEST)/mcdep.o: 	mcdep.c
	$(CC) $(CFLAGS) $(DEFINES) -c mcdep.c
	$(MV) mcdep.o   $(DEST)/mcdep.o

$(DEST)/asm.o: 		asm.c
	$(CC) $(CFLAGS) $(DEFINES) -c asm.c
	$(MV) asm.o     $(DEST)/asm.o

$(DEST)/debug.o: 	debug.c
	$(CC) $(CFLAGS) $(DEFINES) -c debug.c
	$(MV) debug.o   $(DEST)/debug.o

$(DEST)/peep.o: 	peep.c
	$(CC) $(CFLAGS) $(DEFINES) -c peep.c
	$(MV) peep.o    $(DEST)/peep.o

$(DEST)/heliobj.o: 	heliobj.c
	$(CC) $(CFLAGS) $(DEFINES) -c heliobj.c
	$(MV) heliobj.o $(DEST)/heliobj.o
