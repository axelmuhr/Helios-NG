#
# Makefile for the Machine Independant Section of the Norcroft compiler
#
# Copyright (c) 1992 Perihelion Software Ltd
# All Rights Reserved
#
# Author: N Clifton
# RCS Id: $Id: makefile,v 1.4 1994/01/11 11:38:24 nickc Exp $
#

# object files

OBJS =  $(DEST)/aetree.o   \
	$(DEST)/bind.o     \
	$(DEST)/builtin.o  \
	$(DEST)/cg.o       \
	$(DEST)/codebuf.o  \
	$(DEST)/compiler.o \
	$(DEST)/cse.o      \
	$(DEST)/csescan.o  \
	$(DEST)/driver.o   \
	$(DEST)/flowgraf.o \
	$(DEST)/fname.o    \
	$(DEST)/ieeeflt.o  \
	$(DEST)/jopprint.o \
	$(DEST)/misc.o     \
	$(DEST)/regalloc.o \
	$(DEST)/regsets.o  \
	$(DEST)/store.o    \
	$(DEST)/version.o

#targets

default:
	@ make `uname`

HP-UX:
	@ cd .. ; makehp

SunOS:
	@ cd .. ; makesun

AIX:
	@ cd .. ; makers6000

install:
	@ make install-`uname`

install-HP-UX:
	@ cd ..; makehp install

install-SunOS:
	@ cd ..; makesun install

install-AIX:
	@ cd ..; makers6000 install


all: $(OBJS)
	
clean:
	rm -f *.o
	rcsclean
	co -u -q RCS/*

# rules:

$(DEST)/aetree.o: 	 aetree.c
	$(CC) $(CFLAGS)  $(DEFINES) -c aetree.c
	$(MV) aetree.o   $@

$(DEST)/bind.o: 	 bind.c
	$(CC) $(CFLAGS)  $(DEFINES) bind.c -c
	$(MV) bind.o     $@

$(DEST)/builtin.o: 	 builtin.c
	$(CC) $(CFLAGS)  $(DEFINES) builtin.c -c
	$(MV) builtin.o  $@

$(DEST)/cg.o: 	         cg.c
	$(CC) $(CFLAGS)  $(DEFINES) cg.c -c 
	$(MV) cg.o       $@

$(DEST)/codebuf.o: 	 codebuf.c
	$(CC) $(CFLAGS)  $(DEFINES) codebuf.c -c
	$(MV) codebuf.o  $@

$(DEST)/compiler.o: 	 compiler.c
	$(CC) $(CFLAGS)  $(DEFINES) compiler.c -c
	$(MV) compiler.o $@

$(DEST)/cse.o: 	cse.c
	$(CC) $(CFLAGS)  $(DEFINES) cse.c -c
	$(MV) cse.o      $@

$(DEST)/csescan.o: 	 csescan.c
	$(CC) $(CFLAGS)  $(DEFINES) csescan.c -c
	$(MV) csescan.o  $@

$(DEST)/driver.o: 	 driver.c
	$(CC) $(CFLAGS)  $(DEFINES) driver.c -c
	$(MV) driver.o   $@

$(DEST)/flowgraf.o: 	 flowgraf.c
	$(CC) $(CFLAGS)  $(DEFINES) flowgraf.c -c
	$(MV) flowgraf.o $@

$(DEST)/fname.o: 	 fname.c
	$(CC) $(CFLAGS)  $(DEFINES) fname.c -c
	$(MV) fname.o    $@

$(DEST)/ieeeflt.o: 	 ieeeflt.c
	$(CC) $(CFLAGS)  $(DEFINES) ieeeflt.c -c
	$(MV) ieeeflt.o  $@

$(DEST)/jopprint.o: 	 jopprint.c
	$(CC) $(CFLAGS)  $(DEFINES) jopprint.c -c
	$(MV) jopprint.o $@

$(DEST)/misc.o: 	 misc.c
	$(CC) $(CFLAGS)  $(DEFINES) misc.c -c
	$(MV) misc.o     $@

$(DEST)/regalloc.o: 	 regalloc.c
	$(CC) $(CFLAGS)  $(DEFINES) regalloc.c -c
	$(MV) regalloc.o $@

$(DEST)/regsets.o: 	 regsets.c
	$(CC) $(CFLAGS)  $(DEFINES) regsets.c -c
	$(MV) regsets.o  $@

$(DEST)/store.o: 	 store.c
	$(CC) $(CFLAGS)  $(DEFINES) store.c -c
	$(MV) store.o    $@

$(DEST)/version.o: 	 version.c	always-null-target
	$(CC) $(CFLAGS)  $(DEFINES) version.c -c -DRELEASE_VSN=\"5\"
	$(MV) version.o  $@

always-null-target:
