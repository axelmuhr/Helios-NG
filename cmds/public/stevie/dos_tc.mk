#
# Makefile for MSDOS using Turbo C and any GOOD make program.
# (Like, forget Microsoft Make; it's a joke.  Use Turbo Make - comes with TC,
#  or NDMAKE - shareware, or PolyMake - from Polytron.  There are others...)
#

MODEL=	-mc
DEFS=	-DDOS -DTURBOC -DTAGSTACK
CFLAGS=	$(MODEL) $(DEFS)
CC=	tcc

PROGS=	alloc.c \
	cmdline.c \
	ctags.c \
	dos.c \
	edit.c \
	enveval.c \
	fileio.c \
	help.c \
	hexchars.c \
	linefunc.c \
	main.c \
	mark.c \
	minix.c \
	misccmds.c \
	normal.c \
	ops.c \
	os2.c \
	param.c \
	ptrfunc.c \
	regexp.c \
	regsub.c \
	screen.c \
	search.c \
	sentence.c \
        setenv.c \
	tagcmd.c \
	term.c \
	tos.c \
	undo.c \
	unix.c \
	version.c

HDRS=	ascii.h \
	env.h \
	keymap.h \
	ops.h \
	param.h \
	regexp.h \
	regmagic.h \
	stevie.h \
	term.h

MKFS=	dos_msc.mk \
	dos_tc.mk \
	minix.mk \
	os2.mk \
	tos.mk \
	unix.mk

MACH=	dos.obj

OBJ=	alloc.obj \
	cmdline.obj \
	edit.obj \
	enveval.obj. \
	fileio.obj \
	help.obj \
	hexchars.obj \
	linefunc.obj \
	main.obj \
	mark.obj \
	misccmds.obj \
	normal.obj \
	ops.obj \
	param.obj \
	ptrfunc.obj \
	screen.obj \
	search.obj \
	sentence.obj \
	tagcmd.obj \
	term.obj \
	undo.obj \
	version.obj

OTHER=	regexp.obj regsub.obj \tc\lib\wildargs.obj

.c.obj :
	$(CC) -c $(CFLAGS) $*

all : stevie.exe stevie.doc

stevie.exe : $(OBJ) $(MACH) $(OTHER)
	$(CC) -estevie $(OBJ) $(MACH) $(OTHER)

ctags.exe : ctags.c
	$(CC) ctags.c

setenv.exe : setenv.c
	$(CC) setenv.c

stevie.doc : stevie.mm
	nroff -rB1 -Tlp -mm stevie.mm > stevie.doc

clean :
	rm $(OBJ) $(MACH)

# Clean out the .OBJs that depend on whether BIOS is defined.
cleanbios :
        rm dos.obj screen.obj help.obj

# Specific header dependencies.

$(OBJ) : stevie.h env.h ascii.h keymap.h param.h term.h
$(MACH) : stevie.h env.h ascii.h keymap.h param.h term.h
linefunc.obj : ops.h
normal.obj : ops.h
ops.obj : ops.h
regexp.obj : regexp.h regmagic.h ops.h
regsub.obj : regexp.h regmagic.h
search.obj : regexp.h
sentence.obj : ops.h


zip : stevi369.zip
stevi369.zip : readme readme.dmt stevie.mm stevie.doc stevie.exe \
                ctags.exe setenv.exe source.zip
	pkzip -u stevi369 $?

source.zip : $(PROGS) $(HDRS) $(MKFS) porting.doc source.doc stevie.prj
	pkzip -u source $?

