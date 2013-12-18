# Makefile for GNU C compiler.
#   Copyright (C) 1987, 1988 Free Software Foundation, Inc.

#This file is part of GNU CC.

#GNU CC is free software; you can redistribute it and/or modify
#it under the terms of the GNU General Public License as published by
#the Free Software Foundation; either version 1, or (at your option)
#any later version.

#GNU CC is distributed in the hope that it will be useful,
#but WITHOUT ANY WARRANTY; without even the implied warranty of
#MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#GNU General Public License for more details.

#You should have received a copy of the GNU General Public License
#along with GNU CC; see the file COPYING.  If not, write to
#the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.


# Variables you should change for certain systems:

# These are what you would need on HPUX:
# CFLAGS = -Wc,-Ns2000 -Wc,-Ne700 -Wc,-Np300
# If you are using the GNU assembler and linker on HPUX,
# add -I../hp-include to CFLAGS.
# -g is desirable in CFLAGS, but a compiler bug in HPUX version 5
# bites whenever tree.def, rtl.def or machmode.def is included
# (ie., on every source file).
# CCLIBFLAGS = -Wc,-Ns2000 -Wc,-Ne700
# For CCLIBFLAGS you might want to specify the switch that
# forces only 68000 instructions to be used.

# If you are making gcc for the first time, and if you are compiling it with
# a non-gcc compiler, and if your system doesn't have a working alloca() in any
# of the standard libraries (as is true for HP/UX or Genix),
# then get alloca.c from GNU Emacs and un-comment the following line:
# Varun 5/10 18:04
ALLOCA = alloca.o

# If your system has alloca() in /lib/libPW.a, un-comment the following line:
# CLIB= -lPW

# If your system's malloc() routine fails for any reason (as it does on
# certain versions of Genix), try getting the files
# malloc.c and getpagesize.h from GNU Emacs and un-comment the following line:
# MALLOC = malloc.o

# If you are running GCC on an Apollo (SR10.x),
# go into a Berkeley environment and use this:
# CFLAGS = -g -A nansi -A cpu,3000 -A runtype,bsd4.3 -A systype,any -DSHORT_ENUM_BUG
# (Says vasta@apollo.com.)


# Variables that exist for you to override.

# varun 1l 5/10, 16:33
#CFLAGS = -g $(XCFLAGS)
CFLAGS= -Bstatic -O2
CC = cc
BISON = bison
BISONFLAGS = -v
AR = ar
SHELL = /bin/sh
# on sysV, define this as cp.
INSTALL = install -c

# Compiler to use for compiling gnulib.
# OLDCC should not be the GNU C compiler.
OLDCC = cc

# CFLAGS for use with OLDCC, for compiling gnulib.
CCLIBFLAGS=

# Directory where sources are, from where we are.
srcdir = .
# Directory in which to put the executable for the command `gcc'
bindir = $(prefix)/giga/bin
# Directory in which to put the subprograms used by the compiler.
libdir = $(prefix)/giga/lib
# Directory in which to put man pages.
mandir = $(prefix)/usr/man/man1
# Number to put in man-page filename.
manext = 1

# Additional system libraries to link with.
CLIB=

# Change this to a null string if obstacks are installed in the
# system library.
OBSTACK=obstack.o

# Dependency on obstack, alloca, malloc or whatever library facilities
# are not installed in the system libraries.
LIBDEPS= $(OBSTACK) $(ALLOCA) $(MALLOC)

# How to link with both our special library facilities
# and the system's installed libraries.
LIBS = $(OBSTACK) $(ALLOCA) $(MALLOC) $(CLIB)

DIR = ../gcc

# End of variables for you to override.

# Always use -I$(srcdir)/config when compiling.
.c.o:
	$(CC) -c $(CFLAGS) $(CPPFLAGS) -I$(srcdir)/config $<

# Language-specific object files for C.
C_OBJS = c-parse.tab.o c-decl.o c-typeck.o c-convert.o

# Language-specific object files for C++.
# (These are not yet released.)
CPLUS_OBJS = cplus-parse.o cplus-decl.o cplus-typeck.o \
   cplus-cvt.o cplus-search.o cplus-lex.o \
   cplus-class.o cplus-init.o cplus-method.o

# Language-independent object files.
OBJS = toplev.o version.o tree.o print-tree.o stor-layout.o fold-const.o \
 rtl.o expr.o stmt.o expmed.o explow.o optabs.o varasm.o \
 symout.o dbxout.o sdbout.o emit-rtl.o insn-emit.o \
 integrate.o jump.o cse.o loop.o flow.o stupid.o combine.o \
 regclass.o local-alloc.o global-alloc.o reload.o reload1.o caller-save.o \
 insn-peep.o final.o recog.o insn-recog.o insn-extract.o insn-output.o

# Files to be copied away after each stage in building.
STAGE_GCC=gcc cc1 cpp cccp
STAGESTUFF = *.o insn-flags.h insn-config.h insn-codes.h \
 insn-output.c insn-recog.c insn-emit.c insn-extract.c insn-peep.c \
 genemit genoutput genrecog genextract genflags gencodes genconfig genpeep \
 # cc1plus

# Members of gnulib.
LIBFUNCS = _eprintf _builtin_new _builtin_New _builtin_del \
   _umulsi3 _mulsi3 _udivsi3 _divsi3 _umodsi3 _modsi3 \
   _lshrsi3 _lshlsi3 _ashrsi3 _ashlsi3 _cmpdi2 _ucmpdi2 \
   _divdf3 _muldf3 _negdf2 _adddf3 _subdf3 _cmpdf2 \
   _fixunsdfsi _fixunsdfdi _fixdfsi _fixdfdi \
   _floatsidf _floatdidf _truncdfsf2 _extendsfdf2 \
   _addsf3 _negsf2 _subsf3 _cmpsf2 _mulsf3 _divsf3 _varargs

# Library members defined in gnulib2.c.
LIB2FUNCS = _adddi3 _subdi3 _muldi3 _divdi3 _moddi3 _udivdi3 _umoddi3 _negdi2 \
    _div_internal

# Header files that are made available to programs compiled with gcc.
USER_H = stddef.h stdarg.h assert.h varargs.h va-*.h limits.h

# If you want to recompile everything, just do rm *.o.
# CONFIG_H = config.h tm.h
CONFIG_H =
RTL_H = rtl.h rtl.def machmode.def
TREE_H = tree.h real.h tree.def machmode.def
CPLUS_TREE_H = $(TREE_H) cplus-tree.h c-tree.h

validate: compile
	cd ../bin; Run-gcc 1

run: validate

compile: gnulib gcc cc1 cpp # cc1plus gnulib2

# gnulib2 is commented out in 1.35 since it's too late for real testing.
# Do `make gnulib2' explicitly if you want `long long' support.
lang-c: gnulib gcc cc1 cpp # gnulib2
# lang-cplus: gnulib gcc cc1plus cpp

doc: cpp.info gplus.info gcc.info

compilations: ${OBJS}

gcc: gcc.o version.o $(LIBDEPS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o gccnew gcc.o version.o $(LIBS)
# Go via `gccnew' to avoid `file busy' if $(CC) is `gcc'.
	mv gccnew gcc

gcc.o: gcc.c $(CONFIG_H)
	$(CC) $(CFLAGS) -I$(srcdir)/config \
  -DSTANDARD_STARTFILE_PREFIX=\"$(libdir)/\" \
  -DSTANDARD_EXEC_PREFIX=\"$(libdir)/gcc-\" -c gcc.c

cc1: $(C_OBJS) $(OBJS) $(LIBDEPS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o cc1 $(C_OBJS) $(OBJS) $(LIBS)

cc1plus: $(CPLUS_OBJS) $(OBJS) $(LIBDEPS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o cc1plus $(CPLUS_OBJS) $(OBJS) $(LIBS)

#Library of arithmetic subroutines
# Don't compile this with gcc!
# (That would cause most arithmetic functions to call themselves.)
gnulib: gnulib.c
	-mkdir libtemp
	cd libtemp; \
	rm -f gnulib; \
	for name in $(LIBFUNCS); \
	do \
	  echo $${name}; \
	  rm -f $${name}.c; \
	  cp ../gnulib.c $${name}.c; \
	  $(OLDCC) $(CCLIBFLAGS) -O -I.. -I../config -c -DL$${name} $${name}.c; \
	  $(AR) qc gnulib $${name}.o; \
	done
	mv libtemp/gnulib .
	rm -rf libtemp
	if [ -f /usr/bin/ranlib ] ; then  ranlib gnulib ;fi
# On HPUX, if you are working with the GNU assembler and linker,
# the previous line must be replaced with the following two lines.
# No change is needed here if you are using the HPUX assembler and linker.
#	mv gnulib gnulib-hp
#	../hp-bin/hpxt gnulib-hp gnulib

gnulib2: stamp-gnulib2 ;
stamp-gnulib2: gnulib2.c
	-mkdir libtemp
	cd libtemp; \
	cp ../gnulib .; \
	for name in $(LIB2FUNCS); \
	do \
	  echo $${name}; \
	  rm -f $${name}.c; \
	  cp ../gnulib2.c $${name}.c; \
	  ../gcc -B../ -fstrength-reduce -O -I.. -I../config -c -DL$${name} $${name}.c; \
	  $(AR) qc gnulib $${name}.o; \
	done
	mv libtemp/gnulib .
	rm -rf libtemp
	if [ -f /usr/bin/ranlib ] ; then  ranlib gnulib ;fi
	touch stamp-gnulib2
# On HPUX, this might require some change; no one has tried it.

# C language specific files.

c-parse.tab.o : c-parse.tab.c $(CONFIG_H) $(TREE_H) c-parse.h c-tree.h
#c-parse.tab.c : c-parse.y
#	$(BISON) $(BISONFLAGS) c-parse.y

c-decl.o : c-decl.c $(CONFIG_H) $(TREE_H) c-tree.h c-parse.h flags.h
c-typeck.o : c-typeck.c $(CONFIG_H) $(TREE_H) c-tree.h flags.h
c-convert.o : c-convert.c $(CONFIG_H) $(TREE_H)

# C++ language specific files.

cplus-parse.o : cplus-parse.c $(CONFIG_H) $(CPLUS_TREE_H) flags.h
	$(CC) -c $(CFLAGS) -I$(srcdir)/config \
  -DPARSE_OUTPUT=\"$(PWD)/cplus-parse.output\" cplus-parse.c

#cplus-parse.h cplus-parse.c : cplus-parse.y
#	@echo expect 49 shift/reduce conflicts and 4 reduce/reduce conflicts
#	$(BISON) $(BISONFLAGS) -d -o cplus-parse.c cplus-parse.y

cplus-lex.o : cplus-lex.c $(CONFIG_H) $(CPLUS_TREE_H) cplus-parse.h
cplus-decl.o : cplus-decl.c $(CONFIG_H) $(CPLUS_TREE_H) flags.h
cplus-typeck.o : cplus-typeck.c $(CONFIG_H) $(CPLUS_TREE_H) flags.h
cplus-class.o : cplus-class.c $(CONFIG_H) $(CPLUS_TREE_H)
cplus-init.o : cplus-init.c $(CONFIG_H) $(CPLUS_TREE_H)
cplus-method.o : cplus-method.c $(CONFIG_H) $(CPLUS_TREE_H)
cplus-cvt.o : cplus-cvt.c $(CONFIG_H) $(CPLUS_TREE_H)
cplus-search.o : cplus-search.c $(CONFIG_H) $(CPLUS_TREE_H)
new-method.o : new-method.c $(CONFIG_H) $(CPLUS_TREE_H)

# Language-independent files.

tree.o : tree.c $(CONFIG_H) $(TREE_H) flags.h
print-tree.o : print-tree.c $(CONFIG_H) $(TREE_H)
stor-layout.o : stor-layout.c $(CONFIG_H) $(TREE_H)
fold-const.o : fold-const.c $(CONFIG_H) $(TREE_H)
toplev.o : toplev.c $(CONFIG_H) $(TREE_H) flags.h

rtl.o : rtl.c $(CONFIG_H) $(RTL_H)

varasm.o : varasm.c $(CONFIG_H) $(TREE_H) $(RTL_H) flags.h expr.h \
   insn-codes.h hard-reg-set.h
stmt.o : stmt.c $(CONFIG_H) $(RTL_H) $(TREE_H) flags.h  \
   insn-flags.h expr.h insn-config.h regs.h insn-codes.h
expr.o : expr.c $(CONFIG_H) $(RTL_H) $(TREE_H) flags.h  \
   insn-flags.h insn-codes.h expr.h insn-config.h recog.h
expmed.o : expmed.c $(CONFIG_H) $(RTL_H) $(TREE_H) flags.h  \
   insn-flags.h insn-codes.h expr.h insn-config.h recog.h
explow.o : explow.c $(CONFIG_H) $(RTL_H) $(TREE_H) flags.h expr.h insn-codes.h
optabs.o : optabs.c $(CONFIG_H) $(RTL_H) $(TREE_H) flags.h  \
   insn-flags.h insn-codes.h expr.h insn-config.h recog.h
symout.o : symout.c $(CONFIG_H) $(TREE_H) $(RTL_H) symseg.h gdbfiles.h
dbxout.o : dbxout.c $(CONFIG_H) $(TREE_H) $(RTL_H) flags.h
sdbout.o : sdbout.c $(CONFIG_H) $(TREE_H) $(RTL_H) c-tree.h

emit-rtl.o : emit-rtl.c $(CONFIG_H) $(RTL_H) regs.h insn-config.h real.h

integrate.o : integrate.c $(CONFIG_H) $(RTL_H) $(TREE_H) flags.h expr.h \
   insn-flags.h insn-codes.h

jump.o : jump.c $(CONFIG_H) $(RTL_H) flags.h regs.h
stupid.o : stupid.c $(CONFIG_H) $(RTL_H) regs.h hard-reg-set.h

cse.o : cse.c $(CONFIG_H) $(RTL_H) regs.h hard-reg-set.h flags.h real.h
loop.o : loop.c $(CONFIG_H) $(RTL_H) insn-config.h insn-codes.h \
   regs.h recog.h flags.h expr.h
flow.o : flow.c $(CONFIG_H) $(RTL_H) basic-block.h regs.h hard-reg-set.h
combine.o : combine.c $(CONFIG_H) $(RTL_H) flags.h  \
   insn-config.h regs.h basic-block.h recog.h
regclass.o : regclass.c $(CONFIG_H) $(RTL_H) flags.h regs.h \
   insn-config.h recog.h hard-reg-set.h
local-alloc.o : local-alloc.c $(CONFIG_H) $(RTL_H) basic-block.h regs.h \
   insn-config.h recog.h hard-reg-set.h
global-alloc.o : global-alloc.c $(CONFIG_H) $(RTL_H) flags.h  \
   basic-block.h regs.h hard-reg-set.h insn-config.h

reload.o : reload.c $(CONFIG_H) $(RTL_H)  \
   reload.h recog.h hard-reg-set.h insn-config.h regs.h
reload1.o : reload1.c $(CONFIG_H) $(RTL_H) flags.h  \
   reload.h regs.h hard-reg-set.h insn-config.h basic-block.h
caller-save.o : caller-save.c $(CONFIG_H) $(RTL_H) flags.h \
   reload.h regs.h hard-reg-set.h insn-config.h basic-block.h recog.h
final.o : final.c $(CONFIG_H) $(RTL_H) regs.h recog.h conditions.h gdbfiles.h \
   insn-config.h real.h
recog.o : recog.c $(CONFIG_H) $(RTL_H)  \
   regs.h recog.h hard-reg-set.h insn-config.h

# Normally this target is not used; but it is used if you
# define ALLOCA=alloca.o.  In that case, you must get a suitable alloca.c
# from the GNU Emacs distribution.
# Note some machines won't allow $(CC) without -S on this source file.
alloca.o:	alloca.c
	$(CC) $(CFLAGS) -S alloca.c
	as alloca.s -o alloca.o

# Now the source files that are generated from the machine description.

.PRECIOUS: insn-config.h insn-flags.h insn-codes.h \
  insn-emit.c insn-recog.c insn-extract.c insn-output.c insn-peep.c

# The following pair of rules has this effect:
# genconfig is run only if the md has changed since genconfig was last run;
# but the file insn-config.h is touched only when its contents actually change.

# Each of the other insn-* files is handled by a similar pair of rules.

insn-config.h: stamp-config.h ;
stamp-config.h : md genconfig
	./genconfig md > tinsn-config.h
	./move-if-change tinsn-config.h insn-config.h
	touch stamp-config.h

insn-flags.h: stamp-flags.h ;
stamp-flags.h : md genflags
	./genflags md > tinsn-flags.h
	./move-if-change tinsn-flags.h insn-flags.h
	touch stamp-flags.h

insn-codes.h: stamp-codes.h ;
stamp-codes.h : md gencodes
	./gencodes md > tinsn-codes.h
	./move-if-change tinsn-codes.h insn-codes.h
	touch stamp-codes.h

insn-emit.o : insn-emit.c $(CONFIG_H) $(RTL_H) expr.h insn-config.h real.h
	$(CC) $(CFLAGS) -I$(srcdir)/config -c insn-emit.c

insn-emit.c: stamp-emit.c ;
stamp-emit.c : md genemit
	./genemit md > tinsn-emit.c
	./move-if-change tinsn-emit.c insn-emit.c
	touch stamp-emit.c

insn-recog.o : insn-recog.c $(CONFIG_H) $(RTL_H) insn-config.h
	$(CC) $(CFLAGS) -I$(srcdir)/config -c insn-recog.c

insn-recog.c: stamp-recog.c ;
stamp-recog.c : md genrecog
	./genrecog md > tinsn-recog.c
	./move-if-change tinsn-recog.c insn-recog.c
	touch stamp-recog.c

insn-extract.o : insn-extract.c $(RTL_H)
	$(CC) $(CFLAGS) -I$(srcdir)/config -c insn-extract.c

insn-extract.c: stmp-extract.c ;
stmp-extract.c : md genextract
	./genextract md > tinsn-extrct.c
	./move-if-change tinsn-extrct.c insn-extract.c
	touch stmp-extract.c

insn-peep.o : insn-peep.c $(CONFIG_H) $(RTL_H) regs.h real.h
	$(CC) $(CFLAGS) -I$(srcdir)/config -c insn-peep.c

insn-peep.c: stamp-peep.c ;
stamp-peep.c : md genpeep
	./genpeep md > tinsn-peep.c
	./move-if-change tinsn-peep.c insn-peep.c
	touch stamp-peep.c

insn-output.o : insn-output.c $(CONFIG_H) $(RTL_H) regs.h real.h conditions.h \
    hard-reg-set.h insn-config.h insn-flags.h output.h aux-output.c
	$(CC) $(CFLAGS) -I$(srcdir)/config -c insn-output.c

insn-output.c: stamp-output.c ;
stamp-output.c : md genoutput
	./genoutput md > tinsn-output.c
	./move-if-change tinsn-output.c insn-output.c
	touch stamp-output.c

# Now the programs that generate those files.

genconfig : genconfig.o rtl.o $(LIBDEPS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o genconfig genconfig.o rtl.o $(LIBS)

genconfig.o : genconfig.c $(RTL_H)

genflags : genflags.o rtl.o $(LIBDEPS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o genflags genflags.o rtl.o $(LIBS)

genflags.o : genflags.c $(RTL_H)

gencodes : gencodes.o rtl.o $(LIBDEPS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o gencodes gencodes.o rtl.o $(LIBS)

gencodes.o : gencodes.c $(RTL_H)

genemit : genemit.o rtl.o $(LIBDEPS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o genemit genemit.o rtl.o $(LIBS)

genemit.o : genemit.c $(RTL_H)

genrecog : genrecog.o rtl.o $(LIBDEPS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o genrecog genrecog.o rtl.o $(LIBS)

genrecog.o : genrecog.c $(RTL_H)

genextract : genextract.o rtl.o $(LIBDEPS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o genextract genextract.o rtl.o $(LIBS)

genextract.o : genextract.c $(RTL_H)

genpeep : genpeep.o rtl.o $(LIBDEPS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o genpeep genpeep.o rtl.o $(LIBS)

genpeep.o : genpeep.c $(RTL_H)

genoutput : genoutput.o rtl.o $(LIBDEPS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o genoutput genoutput.o rtl.o $(LIBS)

genoutput.o : genoutput.c $(RTL_H)

# Making the preprocessor
cpp: cccp
	-rm -f cpp
	ln cccp cpp
cccp: cccp.o cexp.o version.o $(LIBDEPS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o cccp cccp.o cexp.o version.o $(LIBS)
cexp.o: cexp.c
#cexp.c: cexp.y
#	$(BISON) cexp.y
#	mv cexp.tab.c cexp.c
cccp.o: cccp.c
	$(CC) $(CFLAGS) -I$(srcdir)/config \
          -DGCC_INCLUDE_DIR=\"$(libdir)/gcc-include\" \
          -DGPLUSPLUS_INCLUDE_DIR=\"$(libdir)/g++-include\" -c cccp.c

cpp.info: cpp.texinfo
	makeinfo $<

gplus.info: gplus.texinfo
	makeinfo $<

gcc.info: gcc.texinfo
	makeinfo $<

# gnulib is not deleted because deleting it would be inconvenient
# for most uses of this target.
clean:
	-rm -f $(STAGESTUFF)
	-rm -f stamp-*.[ch]
	-rm -f *.s *.s[0-9] *.co *.greg *.lreg *.combine *.flow *.cse *.jump *.rtl *.tree *.loop
	-rm -f core

clobber: clean
	-rm -f $(STAGE_GCC)

# Like clean but also delete the links made to configure gcc.
cleanlinks: clean
	-rm -f tm.h aux-output.c config.h md config.status

# Get rid of every file that's generated from some other file (except INSTALL).
realclean: cleanlinks
	-rm -f cpp.aux cpp.cps cpp.fns cpp.info cpp.kys cpp.pgs cpp.tps cpp.vrs
#	-rm -f cplus-parse.tab.c cplus-parse.output
	-rm -f c-parse.tab.c c-parse.output
	-rm -f gnulib cexp.c TAGS 
	-rm -f cpp.info* cpp.?? cpp.??s cpp.log cpp.toc cpp.*aux
	-rm -f gcc.info* gcc.?? gcc.??s gcc.log gcc.toc gcc.*aux
	-rm -f gplus.info* gplus.?? gplus.??s gplus.log gplus.toc gplus.*aux
	-rm -f *.dvi

# Copy the files into directories where they will be run.
install: #all
	-mkdir $(libdir)
	-if [ -f cc1 ] ; then $(INSTALL) cc1 $(libdir)/gcc-cc1 ;fi
	-if [ -f cc1plus ] ; then $(INSTALL) cc1plus $(libdir)/gcc-cc1plus ;fi
	$(INSTALL) gnulib $(libdir)/gcc-gnulib
	-if [ -f /usr/bin/ranlib ] ; then (cd $(libdir); ranlib gcc-gnulib) ;fi
	$(INSTALL) cpp $(libdir)/gcc-cpp
	$(INSTALL) gcc $(bindir)
	-mkdir $(libdir)/gcc-include
	chmod ugo+rx $(libdir)/gcc-include
	for file in $(USER_H); do $(INSTALL) $${file} $(libdir)/gcc-include; done
	$(INSTALL) gcc.1 $(mandir)/gcc.$(manext)

# do make -f ../gcc/Makefile maketest DIR=../gcc
# in the intended test directory to make it a suitable test directory.
maketest:
	ln -s $(DIR)/*.[chy] .
	ln -s $(DIR)/*.def .
	ln -s $(DIR)/*.md .
	-rm -f =*
	ln -s $(DIR)/.gdbinit .
	-ln -s $(DIR)/bison.simple .
	ln -s $(DIR)/config.gcc .
	ln -s $(DIR)/move-if-change .
	if [ -f Makefile ] ; then false; else ln -s $(DIR)/Makefile . ; fi
	-rm tm.h aux-output.c config.h md
	make clean
# You must then run config.gcc to set up for compilation.

bootstrap: all force
	$(MAKE) stage1
	$(MAKE) CC="stage1/gcc -Bstage1/" CFLAGS="-O $(CFLAGS)"
	$(MAKE) stage2
	$(MAKE) CC="stage2/gcc -Bstage2/" CFLAGS="-O $(CFLAGS)"

bootstrap2: force
	$(MAKE) CC="stage1/gcc -Bstage1/" CFLAGS="-O $(CFLAGS)"
	$(MAKE) stage2
	$(MAKE) CC="stage2/gcc -Bstage2/" CFLAGS="-O $(CFLAGS)"

bootstrap3: force
	$(MAKE) CC="stage2/gcc -Bstage2/" CFLAGS="-O $(CFLAGS)"

# Copy the object files from a particular stage into a subdirectory.
stage1: force
	-mkdir stage1
	-mv $(STAGESTUFF) $(STAGE_GCC) stage1
	-rm -f stage1/gnulib
	-ln gnulib stage1 || (cp gnulib stage1 && ranlib stage1/gnulib)

stage2: force
	-mkdir stage2
	-mv $(STAGESTUFF) $(STAGE_GCC) stage2
	-rm -f stage2/gnulib
	-ln gnulib stage2 || (cp gnulib stage2 && ranlib stage2/gnulib)

stage3: force
	-mkdir stage3
	-mv $(STAGESTUFF) $(STAGE_GCC) stage3
	-rm -f stage3/gnulib
	-ln gnulib stage3 || (cp gnulib stage3 && ranlib stage3/gnulib)

TAGS: force
	mkdir temp
	-mv c-parse.tab.c cplus-parse.c cplus-parse.h cexp.c temp
	etags *.y *.h *.c
	mv temp/* .
	rmdir temp

#In GNU Make, ignore whether `stage*' exists.
.PHONY: stage1 stage2 stage3 clean realclean TAGS bootstrap

force:
