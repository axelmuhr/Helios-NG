#
# Makefile for the C40 Back End to the Norcroft compiler
#
# Copyright (c) 1992-1993 Perihelion Software Ltd
# All Rights Reserved
#
# Author: N Clifton
# RCS Id: $Id: makefile,v 1.15 1993/07/27 10:16:22 nickc Exp $
#

# object files

OBJS = 	$(DEST)/gen.o     \
	$(DEST)/mcdep.o   \
	$(DEST)/asm.o     \
	$(DEST)/debug.o   \
	$(DEST)/peep.o    

default:
	cd .. ; gmake all

all: $(OBJS)
	
install:
	cd ..; gmake install

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
