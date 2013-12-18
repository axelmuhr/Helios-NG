# Makefile for GNU DIFF
# Copyright (C) 1988, 1989 Free Software Foundation, Inc.

# $Header: /usr/perihelion/Helios/cmds/gnu/diff/RCS/makefile,v 1.1 90/08/28 12:39:55 james Exp $

# This file is part of GNU DIFF.

# GNU DIFF is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 1, or (at your option)
# any later version.
# 
# GNU DIFF is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with GNU DIFF; see the file COPYING.  If not, write to
# the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.  */


# You can compile this with ordinary cc as well,
# but gcc makes it faster.
# Also, gcc supports -O and -g together.
CC= c
CFLAGS = -DUSG -wvdf -DHELIOS
INSTALL = cp
DIFF3 = gdiff3
DIFF = gdiff

# On system V, enable these three lines:
# CFLAGS = -g -DUSG
# LIBS = -lPW
# INSTALL = cp
# (If you compile with GCC, you don't need to define LIBS.)
# And, depending on the names and contents of your header files,
# add either -DHAVE_NDIR or -DHAVE_DIRECT or both to CFLAGS.
# Add -DHAVE_NDIR to CFLAGS if your system used ndir.h instead of dirent.h
# Add -DHAVE_DIRECT to CFLAGS if your system uses 'struct direct' instead of
# 'struct dirent' (this is the case at least with one add-on ndir library).

bindir=/helios/local/bin
prefix=

# All source files
srcs=diff.c analyze.c io.c context.c ed.c normal.c ifdef.c util.c dir.c \
	diff.h regex.c regex.h limits.h diff3.c
# Object files for diff only.
objs=$(archpfx)diff.o $(archpfx)analyze.o $(archpfx)io.o $(archpfx)context.o \
     $(archpfx)ed.o $(archpfx)normal.o $(archpfx)util.o $(archpfx)dir.o \
     $(archpfx)regex.o $(archpfx)ifdef.o ../tar/getopt.o ../bison/alloca.o
tapefiles = $(srcs) README diagmeet.note Makefile COPYING ChangeLog

all: $(archpfx)diff $(archpfx)diff3

diff3objs = $(archpfx)diff3.o ../tar/getopt.o

$(archpfx)$(DIFF3): $(diff3objs)
	$(CC) -o $(archpfx)$(DIFF3) $(CFLAGS) $(LDFLAGS) $(diff3objs) $(LIBS)

$(archpfx)$(DIFF): $(objs)
	$(CC) -o $(archpfx)$(DIFF) $(CFLAGS) $(LDFLAGS) $(objs) $(LIBS)

$(objs): diff.h

$(archpfx)context.o $(archpfx)diff.o: regex.h

$(archpfx)diff3.o: diff3.c
	$(CC) -c $(CFLAGS) -DDIFF_PROGRAM=\"$(bindir)/diff\" diff3.c \
 $(OUTPUT_OPTION)

clean:
	rm -f *.o $(archpfx)$(DIFF) $(archpfx)$(DIFF3) diff.tar diff.tar.Z

install: install-diff install-diff3

install-diff: $(prefix)$(bindir)/$(DIFF)

$(prefix)$(bindir)/$(DIFF): $(archpfx)$(DIFF)
	$(INSTALL) $(archpfx)$(DIFF) $(prefix)$(bindir)/$(DIFF)

install-diff3: $(prefix)$(bindir)/$(DIFF3)

$(prefix)$(bindir)/$(DIFF3): $(archpfx)$(DIFF3)
	$(INSTALL) $(archpfx)$(DIFF3) $(prefix)$(bindir)/$(DIFF3)

diff.tar: $(tapefiles)
	mkdir tmp
	mkdir tmp/diff
	-ln $(tapefiles) tmp/diff
	for file in $(tapefiles); do \
		if [ ! -r tmp/diff/$$file ]; then cp $$file tmp/diff; fi \
	done
	cd tmp; tar cf ../diff.tar diff
	rm -rf tmp

diff.tar.Z: diff.tar
	compress < diff.tar > diff.tar.Z
