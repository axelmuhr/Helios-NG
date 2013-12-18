.SUFFIXES:
.SUFFIXES: .o .s .c 


s        = .
inc	 = c:/include
TMP      = .

include  = -I. -I$(inc)
header   = barthdr
mycc     = cc -c $(include) -DST -VPEEP

default : server.prg
	echo OK

# the Atari IO server
server.prg: helios.h server.o files.o devices.o terminal.o tload.o \
		cofuns.o debug.o \
		st/stlocal.o st/stasm.o 
	cc -o server.prg cofuns.o server.o files.o devices.o terminal.o \
			tload.o debug.o \
		        st/stlocal.o st/stasm.o

tload.o: tload.c $(header)
	$(mycc) tload.c

server.o: server.c $(header) sccs.h fundefs.h \
	server.h defines.h
	$(mycc) server.c

files.o:       files.c   $(header)
	$(mycc) files.c

devices.o: devices.c $(header)
	$(mycc) devices.c

terminal.o: terminal.c $(header)
	$(mycc) terminal.c

cofuns.o:     cofuns.c $(header)
	$(mycc) cofuns.c

debug.o : debug.c $(header)
	$(mycc) debug.c

st/stasm.o : st/stasm.s $(header)
	as -o st/stasm.o st/stasm.s

st/stlocal.o : st/stlocal.c $(header)
	$(mycc) st/stlocal.c
	mv stlocal.o $@

#
# The order of the header files is important. First I incorporate all the
# C header files. Then comes Defines.h to specify the environment in which the
# server is meant to run, which must come before any of the remaining headers.
# Barthdr declares some miscellaneous odds and ends. Next comes the helios
# protocol header. Finally the server's private header
# files, including the local one. structs.h declares the server's data
# structures and macros, fundefs declares the functions shared between modules,
# and server.h declares the shared variables.
#
helios.h: barthdr defines.h server.h fundefs.h protocol.h structs.h \
		debugopt.h st/stlocal.h
	cat $(inc)/stdio.h $(inc)/osbind.h $(inc)/errno.h \
	$(inc)/time.h $(inc)/ctype.h $(inc)/bios.h $(inc)/setjmp.h \
	defines.h barthdr protocol.h \
	structs.h fundefs.h server.h debugopt.h \
        st/stlocal.h > helios.h
