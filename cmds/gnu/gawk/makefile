# Makefile for GNU Awk.
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

# User tunable macros -- CHANGE THESE IN Makefile-dist RATHER THAN IN 
# Makefile, OR configure WILL OVERWRITE YOUR CHANGES

DESTDIR= /usr/local/bin
BINDIR=	/usr/local/bin
MANDIR=	/usr/local/man/manl
MANEXT=	l

# CFLAGS: options to the C compiler
#
#	-O	optimize
#	-g	include dbx/sdb info
#	-pg	include new (gmon) profiling info
#
#	The provided "configure" is used to turn a config file (samples in
#	the "config" directory into commands to edit config.h-dist into
#	a suitable config.h and to edit Makefile-dist into Makefile.
#	To port GAWK, create an appropriate config file using the ones in
#	the config directory as examples and using the comments in config.h-dist
#	as a guide.
#

CC=		 c

OPTIMIZE=	#-O -fstrength-reduce
PROFILE=	#-pg
DEBUG=		#-DMALLOCDEBUG #-DMEMDEBUG #-DDEBUG #-DFUNC_TRACE #-DMPROF
LINKSTATIC=	#-Bstatic
WARN=		-wA #-W -Wunused -Wimplicit -Wreturn-type -Wcomment	# for gcc only

# Parser to use on grammar - any one of the following will work
PARSER = yacc
#PARSER = byacc
#PARSER = bison -y

# Set LIBS to any libraries that are machine specific
LIBS = -lbsd # /usr/local/lib/alloca.o

# Cray 2 running Unicos 5.0.7
##MAKE_LIBNET## LIBS = -lnet

##MAKE_NeXT## FLAGS = -DGFMT_WORKAROUND

# Systems with alloca in /lib/libPW.a
##MAKE_ALLOCA_PW## LIBS = -lPW

# ALLOCA - only needed if you use bison
#	Set equal to alloca.o if your system is S5 and you don't have
#	alloca. Uncomment one of the rules below to make alloca.o from
#	either alloca.s or alloca.c.
#	This should have already been done automatically by configure.
#
#	Some systems have alloca in libPW.a, so LIBS=-lPW may work, too.
##MAKE_ALLOCA_C## ALLOCA= alloca.o
##MAKE_ALLOCA_S## ALLOCA= alloca.o

FLAGS=
##MAKE_RS6000## FLAGS = -qchars=signed

# HP/Apollo running cc version 6.7 or earlier
##MAKE_Apollo## FLAGS = -U__STDC__ -A run,sys5.3
##MAKE_Apollo## LIBS = -A sys,any

CFLAGS= $(FLAGS) $(DEBUG) $(LINKSTATIC) $(PROFILE) $(OPTIMIZE) $(WARN)

# object files
AWKOBJS = main.o eval.o builtin.o msg.o iop.o io.o field.o array.o \
	node.o version.o missing.o re.o

ALLOBJS = $(AWKOBJS) awk.tab.o

# GNUOBJS
#	GNU stuff that gawk uses as library routines.
GNUOBJS= regex.o dfa.o $(ALLOCA)

# source and documentation files
SRC =	main.c eval.c builtin.c msg.c version.c \
	iop.c io.c field.c array.c node.c missing.c re.c

ALLSRC= $(SRC) awk.tab.c

AWKSRC= awk.h awk.y $(ALLSRC) patchlevel.h protos.h config.h-dist

GNUSRC = alloca.c alloca.s dfa.c dfa.h regex.c regex.h

COPIES = missing/getopt.c missing/system.c missing/tzset.c \
	missing/memcmp.c missing/memcpy.c missing/memset.c \
	missing/random.c missing/strcase.c missing/strchr.c \
	missing/strerror.c missing/strtod.c missing/vprintf.c \
	missing/strftime.c missing/strftime.3 missing/strtol.c

SUPPORT = support/texindex.c support/texinfo.tex

DOCS= gawk.1 gawk.texinfo

INFOFILES= gawk-info gawk-info-1 gawk-info-2 gawk-info-3 gawk-info-4 \
	   gawk-info-5 gawk-info-6 gawk.aux gawk.cp gawk.cps gawk.fn \
	   gawk.fns gawk.ky gawk.kys gawk.pg gawk.pgs gawk.toc \
	   gawk.tp gawk.tps gawk.vr gawk.vrs

MISC =	CHANGES COPYING FUTURES Makefile-dist PROBLEMS README* PORTS mkconf \
	mungeconf configure ACKNOWLEDGMENT LIMITATIONS

OTHERS= pc/* atari/* vms/*

ALLDOC= gawk.dvi $(INFOFILES)

ALLFILES= $(AWKSRC) $(GNUSRC) $(COPIES) $(MISC) $(DOCS) $(ALLDOC) $(OTHERS) \
	  $(SUPPORT)

# Release of gawk.  There can be no leading or trailing white space here!
REL=2.13

# rules to build gawk
gawk: $(ALLOBJS) $(GNUOBJS) $(REOBJS)
	$(CC) -o gawk $(CFLAGS) $(ALLOBJS) $(GNUOBJS) $(REOBJS) -lm $(LIBS)

$(AWKOBJS): awk.h config.h

dfa.o:	awk.h config.h dfa.h

regex.o:	awk.h config.h regex.h

main.o: patchlevel.h

awk.tab.o: awk.h awk.tab.c

awk.tab.c: awk.y
	$(PARSER) -v awk.y
	sed '/^extern char .malloc(), .realloc();$$/d' y.tab.c >awk.tab.c
	rm y.tab.c

config.h: config.h-dist
	@echo You must provide a config.h!
	@echo Run \"./configure\" to build it for known systems
	@echo or copy config.h-dist to config.h and edit it.; exit 1

install: gawk
	install -s gawk $(DESTDIR)$(BINDIR)
	install -c gawk.1 $(DESTDIR)$(MANDIR)/gawk.$(MANEXT)

# ALLOCA: uncomment this if your system (notably System V boxen)
# does not have alloca in /lib/libc.a or /lib/libPW.a
#
# If your machine is not supported by the assembly version of alloca.s,
# use the C version which follows instead.  It uses the default rules to
# make alloca.o.
#
# One of these rules should have already been selected by running configure.


##MAKE_ALLOCA_S## alloca.o: alloca.s
##MAKE_ALLOCA_S## 	/lib/cpp < alloca.s | sed '/^#/d' > t.s
##MAKE_ALLOCA_S## 	as t.s -o alloca.o
##MAKE_ALLOCA_S## 	rm t.s

##MAKE_ALLOCA_C## alloca.o: alloca.c

# auxiliary rules for release maintenance
lint: $(ALLSRC)
	lint -hcbax $(FLAGS) $(ALLSRC)

xref:
	cxref -c $(FLAGS) $(ALLSRC) | grep -v '	/' >xref

clean:
	rm -f *.o core awk.output gmon.out make.out y.output

cleaner:	clean
	rm -f gawk awk.tab.c

clobber: clean
	rm -f $(ALLDOC) gawk.log

gawk.dvi: gawk.texinfo
	tex gawk.texinfo ; texindex gawk.??
	tex gawk.texinfo ; texindex gawk.??
	tex gawk.texinfo

$(INFOFILES): gawk.texinfo
	makeinfo gawk.texinfo

gawk-test-$(REL).tar.Z::
	-rm -f gawk-test-$(REL).tar.Z
	tar -cf - test | compress >gawk-test-$(REL).tar.Z

dist:	$(AWKSRC) $(GNUSRC) $(DOCS) $(MISC) $(COPIES) $(SUPPORT)
	configure msdos
	mv config.h pc
	-rm -rf gawk-$(REL) gawk-$(REL).*.tar.Z
	-mkdir gawk-$(REL)
	cp -p $(AWKSRC) $(GNUSRC) $(DOCS) $(MISC) gawk-$(REL)
	-mkdir gawk-$(REL)/missing
	cp -p $(COPIES) gawk-$(REL)/missing
	-mkdir gawk-$(REL)/atari
	cp -p  atari/* gawk-$(REL)/atari
	-mkdir gawk-$(REL)/pc
	cp -p pc/* gawk-$(REL)/pc
	-mkdir gawk-$(REL)/vms
	cp -p vms/* gawk-$(REL)/vms
	-mkdir gawk-$(REL)/config
	cp -p config/* gawk-$(REL)/config
	-mkdir gawk-$(REL)/support
	cp -p support/* gawk-$(REL)/support
	ln -s ../test gawk-$(REL)
	tar -cfh - gawk-$(REL) | compress > gawk-$(REL).`gawk '{print $$3}' patchlevel.h`.tar.Z

gawk-doc-$(REL).tar.Z: $(ALLDOC)
	-rm -rf gawk-doc-$(REL) gawk-doc-$(REL).tar.Z
	-mkdir gawk-doc-$(REL)
	cp -p $(INFOFILES) gawk.dvi gawk-doc-$(REL)
	nroff -man gawk.1 > gawk-doc-$(REL)/gawk.1.pr
	tar -cf - gawk-$(REL)-doc | compress > gawk-doc-$(REL).tar.Z

gawk-ps-$(REL).tar.Z: gawk-ps-$(REL).tar.Z
	-rm -rf gawk-ps-$(REL) gawk-ps-$(REL).tar.Z
	-mkdir gawk-ps-$(REL)
	dvips -o !cat gawk.dvi > gawk-ps-$(REL)/gawk.postscript
	pstroff -man gawk.1 > gawk-ps-$(REL)/gawk.1.ps
	tar -cf - gawk-ps-$(REL) | compress > gawk-ps-$(REL).tar.Z

release: gawk-src-$(REL).tar.Z gawk-doc-$(REL).tar.Z gawk-ps-$(REL).tar.Z \
	 gawk-test-$(REL).tar.Z

diff:
	for i in RCS/*; do rcsdiff -c -b $$i > `basename $$i ,v`.diff; done

test::
	make gawk
	cd test; make -k

bigtest::
	make gawk
	cd test; make -k bigtest
