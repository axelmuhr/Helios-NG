#
# Makefile for UNIX (System V)
#

LDFLAGS=
CFLAGS= -DBSD -DSTRCSPN

H=	ascii.h \
	env.h \
	keymap.h \
	ops.h \
	param.h \
	regexp.h \
	regmagic.h \
	stevie.h \
	term.h

MACH=	unix.o

OBJ=	alloc.o \
	cmdline.o \
	edit.o \
	enveval.o \
	fileio.o \
	help.o \
	hexchars.o \
	linefunc.o \
	main.o \
	mark.o \
	misccmds.o \
	normal.o \
	ops.o \
	param.o \
	ptrfunc.o \
	regexp.o \
	regsub.o \
	screen.o \
	search.o \
	sentence.o \
	tagcmd.o \
	term.o \
	undo.o \
	version.o

SRC=	$(OBJ:.o=.c) $(MACH:.o=.c)

all : stevie stevie.doc

stevie : $(OBJ) $(MACH)
	$(CC) $(LDFLAGS) $(OBJ) $(MACH) -o stevie -ltermcap

lint :
	lint $(SRC)

tags :
	ctags $(SRC) $(H)

stevie.doc : stevie.mm
	nroff -rB1 -Tlp -mm stevie.mm > stevie.doc

print :
	@pr $(H) $(SRC)

cflow :
	cflow $(SRC) > cflow.for
	cflow -r $(SRC) > cflow.rev

clean :
	rm $(OBJ) $(MACH)
