#
# Makefile for OS/2
#
# The make command with OS/2 is really stupid.
#

#
# Compact model lets us edit large files, but keep small model code
#
MODEL= /AC
CFLAGS = $(MODEL)

MACH=	os2.obj

OBJ=	alloc.obj \
	cmdline.obj \
	edit.obj \
	envevla.obj \
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
	undo.obj \
	version.obj \
	$(MACH)

alloc.obj : alloc.c
	cl -c $(CFLAGS) alloc.c

cmdline.obj : cmdline.c
	cl -c $(CFLAGS) cmdline.c

edit.obj : edit.c
	cl -c $(CFLAGS) edit.c

fileio.obj : fileio.c
	cl -c $(CFLAGS) fileio.c

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

help.obj : help.c
	cl -c $(CFLAGS) help.c

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

os2.obj : os2.c
	cl -c $(CFLAGS) os2.c

stevie.exe : $(OBJ)
	cl $(MODEL) *.obj \pmsdk\lib\setargv.obj -o stevie.exe /F 6000 -link /NOE
