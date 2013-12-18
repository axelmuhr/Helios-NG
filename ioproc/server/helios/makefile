#
# makefile for helios I/O Server for Helios
#

# miscellaneous macros
sys      = /helios/include
host	 = HELIOS
header   = barthdr
mycc     = c -c -D$(host)

# these files compile for all the hardware
servobjs = server.o files.o devices.o tload.o cofuns.o \
	debug.o terminal.o linkio.o helios/hlocal.o

default : helios.h server 

server: $(servobjs)
	c -o server $(servobjs) -lRmLib

tload.o: tload.c $(header)
	$(mycc) tload.c

server.o: server.c $(header) sccs.h fundefs.h \
	server.h defines.h
	$(mycc) server.c

files.o:       files.c   $(header)
	$(mycc) files.c

devices.o: devices.c $(header)
	$(mycc) devices.c

cofuns.o:     cofuns.c $(header)
	$(mycc) cofuns.c

debug.o : debug.c $(header)
	$(mycc) debug.c

terminal.o: terminal.c $(header)
	$(mycc) terminal.c

linkio.o : linkio.c $(header)
	$(mycc) linkio.c

helios/hlocal.o : helios/hlocal.c $(header)
	$(mycc) helios/hlocal.c

#
# The order of the header files is important. First I incorporate all the
# C header files. Then comes Defines.h to specify the environment in which the
# server is meant to run, which must come before any of the remaining headers.
# Barthdr declares some miscellaneous odds and ends. Next come all the helios
# header files from directory servinc. Finally the server's private header
# files, including the local one. structs.h declares the server's data
# structures and macros, fundefs declares the functions shared between modules,
# and server.h declares the shared variables.
#
helios.h: barthdr defines.h structs.h fundefs.h server.h debugopt.h 
	cat $(sys)/stdio.h $(sys)/ctype.h $(sys)/setjmp.h \
	$(sys)/time.h $(sys)/errno.h $(sys)/signal.h $(sys)/fcntl.h \
	$(sys)/unistd.h $(sys)/sys/stat.h $(sys)/dirent.h $(sys)/pwd.h \
	$(sys)/string.h $(sys)/stdlib.h $(sys)/stdarg.h \
	helios/hlocal.h \
	defines.h barthdr protocol.h \
	structs.h fundefs.h server.h debugopt.h \
	> helios.h

