#
# Makefile for the Helios miniserver on an IBM PC or compatible
#

include  = /I. /Ic:\include
MFLAGS   = $(include) /AS /c /Gs
BOARD	 = /DGEMINI
#BOARD	 = /DPARSY
#BOARD	 = /DB008

.c.obj :
	cl $(MFLAGS) $(BOARD) /Fo$*.obj $*.c

.asm.obj :
	MASM $*,$*.obj,NUL,NUL

default : rr_mserv.exe pa_mserv.exe b8_mserv.exe hstart.exe
	echo OK

rr_mserv.exe : miniserv.obj rr_asm.obj
	link rr_asm.obj+miniserv.obj,rr_mserv.exe,NUL, !
	c:\mslib\SLIBCE.LIB /ST:3000
	copy rr_mserv.exe \bin

pa_mserv.exe : miniserv.obj pa_asm.obj
	link pa_asm.obj+miniserv.obj,pa_mserv.exe,NUL, !
	c:\mslib\SLIBCE.LIB /ST:3000

b8_mserv.exe : miniserv.obj b8_asm.obj
	link b8_asm.obj+miniserv.obj,b8_mserv.exe,NUL, !
	c:\mslib\SLIBCE.LIB /ST:3000


miniserv.obj : miniserv.c

rr_asm.obj : miniasm.cpp
	cl /P /DGEMINI miniasm.cpp
	MASM miniasm.i,rr_asm.obj,NUL,NUL

pa_asm.obj : miniasm.cpp
	cl /P /DPARSY miniasm.cpp
	MASM miniasm.i,pa_asm.obj,NUL,NUL

b8_asm.obj : miniasm.cpp
	cl /P /DB008 miniasm.cpp
	MASM miniasm.i,b8_asm.obj,NUL,NUL


hstart.exe : hstart.obj hstartas.obj
	link hstart.obj+hstartas.obj,hstart.exe,NUL,c:\mslib\slibce.lib /ST:3000 /CO

hstart.obj : hstart.c
hstartas.obj : hstartas.asm

showints.exe : showints.obj hstartas.obj
	link showints.obj+hstartas.obj,showints.exe,NUL,c:\mslib\slibce.lib /ST:3000 /CO

showints.obj : showints.c

systest.exe : systest.obj sysasm.obj
	link systest.obj+sysasm.obj,systest.exe,NUL,!
	c:\mslib\SLIBCE.LIB /ST:3000 /CO
