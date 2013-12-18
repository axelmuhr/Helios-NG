#
# Makefile for the Atari ST - Sozobon C Compiler
#

CFLAGS = -O

.c.o:
	$(CC) -c $(CFLAGS) $<
	ar rv vi.lib $*.o

MACH =	tos.o

OBJ =	alloc.o \
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
	undo.o \
	version.o \
	$(MACH)

all : stevie.ttp

stevie.ttp : $(OBJ)
	$(CC) vi.lib -o stevie.ttp

clean :
	$(RM) $(OBJ) vi.lib
