#
# Makefile for DOS
#
# Microsoft make is brain-dead, so please bear with me.
#

#
# Compact model lets us edit large files, but keep small model code
#
MODEL= /AC
CFLAGS = $(MODEL) /DDOS

MACH=	dos.obj

OBJ=	alloc.obj \
	main.obj \
	cmdline.obj \
	edit.obj \
	enveval.obj \
	fileio.obj \
	help.obj \
	hexchars.obj \
	linefunc.obj \
	mark.obj \
	misccmds.obj \
	normal.obj \
	ops.obj \
	param.obj \
	ptrfunc.obj \
	regexp.obj \
	regsub.obj \
	screen.obj \
	search.obj \
	sentence.obj \
	tagcmd.obj \
	undo.obj \
	version.obj \
	$(MACH)

all: stevie.exe

alloc.obj : alloc.c
	cl -c $(CFLAGS) alloc.c

cmdline.obj : cmdline.c
	cl -c $(CFLAGS) cmdline.c

edit.obj : edit.c
	cl -c $(CFLAGS) edit.c

enveval.obj : enveval.c
	cl -c $(CFLAGS) enveval.c

fileio.obj : fileio.c
	cl -c $(CFLAGS) fileio.c

help.obj : help.c
	cl -c $(CFLAGS) help.c

hexchars.obj : hexchars.c
	cl -c $(CFLAGS) hexchars.c

linefunc.obj : linefunc.c
	cl -c $(CFLAGS) linefunc.c

main.obj:	main.c
	cl -c $(CFLAGS) main.c

mark.obj : mark.c
	cl -c $(CFLAGS) mark.c

misccmds.obj : misccmds.c
	cl -c $(CFLAGS) misccmds.c

normal.obj : normal.c
	cl -c $(CFLAGS) normal.c

ops.obj : ops.c
	cl -c $(CFLAGS) ops.c

param.obj : param.c
	cl -c $(CFLAGS) param.c

ptrfunc.obj : ptrfunc.c
	cl -c $(CFLAGS) ptrfunc.c

regexp.obj : regexp.c
	cl -c $(CFLAGS) regexp.c

regsub.obj : regsub.c
	cl -c $(CFLAGS) regsub.c

screen.obj : screen.c
	cl -c $(CFLAGS) screen.c

search.obj : search.c
	cl -c $(CFLAGS) search.c

sentence.obj : sentence.c
	cl -c $(CFLAGS) sentence.c

tagcmd.obj : tagcmd.c
	cl -c $(CFLAGS) tagcmd.c

undo.obj : undo.c
	cl -c $(CFLAGS) undo.c

version.obj : version.c
	cl -c $(CFLAGS) version.c

dos.obj : dos.c
	cl -c $(CFLAGS) dos.c

stevie.exe : $(OBJ)
	cl $(MODEL) *.obj c:\pmsdk\lib\setargv.obj -o stevie.exe /F 6000 -link /NOE
