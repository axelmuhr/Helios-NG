#
# Makefile for Atari ST Minix
#

LDFLAGS=
CFLAGS= -O

MACH=	minix.o

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

all : stevie

stevie : $(OBJ) $(MACH)
	$(CC) $(LDFLAGS) $(OBJ) $(MACH) -o stevie
	chmem =150000 stevie

clean :
	rm $(OBJ) $(MACH)
