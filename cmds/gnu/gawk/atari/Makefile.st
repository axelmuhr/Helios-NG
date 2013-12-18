# Makefile for GNU Awk - ST version
#
# This is a subset of the full Makefile cut down for Atari ST
# gcc compiler is assumed
# It is known to work with gulam shell.
# It may need some customization depending on your setup!
# Replace with it 'Makefile' from the source directory.
#
# You need sed.ttp for an automatic creation of config.h file!
# Check gulam script mkconf.g in atari directory.
# In a pinch you may create one by checking config/atari file and
# editing config.h-dist by hand.
#
# Copyright (C) 1986, 1988, 1989 the Free Software Foundation, Inc.
#
# This file is part of GAWK, the GNU implementation of the
# AWK Progamming Language.
#
# GAWK is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 1, or (at your option)
# any later version.
#
# GAWK is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with GAWK; see the file COPYING.  If not, write to
# the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.

# User tunable macros

DESTDIR=
BINDIR=	c:\bin
MANDIR=	c:\man\manl
MANEXT=	l
RM=rm -f

# CFLAGS: options to the C compiler
#
#	-O	optimize
#	-mshort use integers which are 16-bits wide (a "correct" size on ST)
#
# Set LIBS to any libraries that are machine specific


# Comment out the next line if you don't have gcc.
# Also choose just one of -g and -O.
CC=		 gcc
# if cross-compiler
#CC=		 cgcc
LIBS = -lpml16

OPTIMIZE=	-O -fstrength-reduce -fcombine-regs -fomit-frame-pointer
#OPTIMIZE=	-O
AFLAGS  =	-G -x -mshort # -Wall
DEBUG=		#-DMALLOCDEBUG #-DMEMDEBUG #-DDEBUG #-DFUNC_TRACE #-DMPROF
WARN=		#-W -Wunused -Wimplicit -Wreturn-type -Wcomment	# for gcc only

# Parser to use on grammar -- if you don't have bison use the first one
#PARSER = yacc
PARSER = byacc
#PARSER = bison -y

#
# With the exception of names of parser output files
# need to customize this file below this point.
#

FLAGS=  $(DEBUG) $(AFLAGS)
CFLAGS= $(FLAGS) $(LINKSTATIC) $(PROFILE) $(OPTIMIZE) $(WARN)

# object files
AWKOBJS = main.o eval.o builtin.o msg.o iop.o io.o field.o \
 array.o node.o missing.o re.o version.o

# basename of parser output
#POUTPUT = awk.tab
POUTPUT = awk_tab

ALLOBJS = $(AWKOBJS) $(POUTPUT).o

# GNUOBJS
#	GNU stuff that gawk uses as library routines.
GNUOBJS= regex.o dfa.o $(ALLOCA)

# source and documentation files
SRC =	main.c eval.c builtin.c msg.c \
	iop.c io.c field.c array.c node.c missing.c re.c version.c

ALLSRC= $(SRC) $(POUTPUT).c
#ALLSRC= $(SRC) awk_tab.c

AWKSRC= awk.h awk.y $(ALLSRC) patchlevel.h protos.h config.h-dist

GNUSRC = alloca.c alloca.s dfa.c dfa.h regex.c regex.h

COPIES = missing/getopt.c missing/system.c missing/tzset.c \
	missing/memcmp.c missing/memcpy.c missing/memset.c \
	missing/random.c missing/strcase.c missing/strchr.c \
	missing/strerror.c missing/strtod.c missing/vprintf.c \
	missing/strftime.c missing/strftime.3 missing/strtol.c

SUPPORT = support/texindex.c support/texinfo.tex

DOCS= gawk.1 gawk.texinfo

#INFOFILES= gawk-info gawk-info-1 gawk-info-2 gawk-info-3 gawk-info-4 \
#	   gawk-info-5 gawk-info-6 gawk.aux gawk.cp gawk.cps gawk.fn \
#	   gawk.fns gawk.ky gawk.kys gawk.pg gawk.pgs gawk.toc \
#	   gawk.tp gawk.tps gawk.vr gawk.vrs

ALLDOC= gawk.dvi $(INFOFILES)

# rules to build gawk
gawk: $(ALLOBJS) $(GNUOBJS) $(REOBJS)
	$(CC) -o gawk.ttp $(CFLAGS) $(ALLOBJS) $(GNUOBJS) $(REOBJS) $(LIBS)
	toglclr gawk.ttp

$(AWKOBJS): awk.h config.h

dfa.o:	awk.h config.h dfa.h
regex.o:	awk.h config.h regex.h
main.o: patchlevel.h

$(POUTPUT).o: awk.h $(POUTPUT).c

$(POUTPUT).c: awk.y awk.h
	$(PARSER) awk.y
	sed '/^extern char .malloc(), .realloc();$$/d' ytab.c > $(POTPUT).c
	rm ytab.c

config.h: config.h-dist
	pushd atari
	mkconf.g
	popd

# One of these rules should have already been selected by running mkconf.

##MAKE_ALLOCA_S## alloca.o: alloca.s
##MAKE_ALLOCA_S## 	/lib/cpp < alloca.s | sed '/^#/d' > t.s
##MAKE_ALLOCA_S## 	as t.s -o alloca.o
##MAKE_ALLOCA_S## 	rm t.s
##MAKE_ALLOCA_C## alloca.o: alloca.c
install: gawk
	cp gawk.ttp $(DESTDIR)$(BINDIR)
	cp gawk.1 $(DESTDIR)$(MANDIR)/gawk.$(MANEXT)

clean:
	$(RM) gawk.ttp *.o core awk.output $(POUTPUT).c gmon.out make.out \
	y.output

clobber: clean
	$(RM) $(ALLDOC) gawk.log

gawk.dvi: gawk.texinfo
	tex gawk.texinfo ; texindex gawk.??
	tex gawk.texinfo ; texindex gawk.??
	tex gawk.texinfo

#$(INFOFILES): gawk.texinfo
#	makeinfo gawk.texinfo

# Many tests in test directory depend on having Bourne shell around.
# Before using this target edit test Makefile and test scripts adapting
# them to your local conditions.
test:	gawk
	pushd test
	make
	popd
