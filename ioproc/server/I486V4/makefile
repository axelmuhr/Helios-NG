#
# makefile for helios I/O server for SM90
#

helios	 = /helios/server
sys      = /usr/include
header   = barthdr
mycc     = cc -c -O -Di486V4

default : server hydra hydramon

server: helios.h server.o files.o devices.o tload.o tcp.o \
		cofuns.o debug.o terminal.o linkio.o \
		sun/sunlocal.o sun/linklib.o sun/colib.o sun/hepc2.o \
		sun/kparix.o
	cc -o server server.o files.o devices.o linkio.o tcp.o \
			tload.o cofuns.o debug.o terminal.o \
			sun/sunlocal.o sun/linklib.o sun/colib.o \
			sun/kparix.o sun/hepc2.o -ltermcap -lsocket -lnsl

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

tcp.o : tcp.c $(header)
	$(mycc) tcp.c

sun/colib.o : sun/colib.c $(header)
	$(mycc) sun/colib.c
	mv colib.o $@

sun/sunlocal.o : sun/sunlocal.c $(header)
	$(mycc) sun/sunlocal.c
	mv sunlocal.o $@

sun/linklib.o: sun/linklib.c $(header)
	$(mycc) sun/linklib.c
	mv linklib.o $@

sun/hepc2.o: sun/hepc2.c $(header)
	$(mycc) sun/hepc2.c
	mv hepc2.o $@

sun/kpar2.o: sun/kpar2.c $(header)
	$(mycc) sun/kpar2.c
	mv kpar2.o $@

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
helios.h: barthdr defines.h structs.h fundefs.h server.h debugopt.h \
	  sun/sunlocal.h
	cat $(sys)/stdio.h $(sys)/sys/types.h $(sys)/setjmp.h $(sys)/limits.h \
	$(sys)/sys/time.h $(sys)/errno.h $(sys)/sys/socket.h \
	$(sys)/sys/times.h $(sys)/signal.h $(sys)/fcntl.h \
	$(sys)/unistd.h $(sys)/sys/stat.h $(sys)/sys/param.h \
	$(sys)/sys/file.h $(sys)/memory.h $(sys)/sys/vfs.h \
	$(sys)/netinet/in.h $(sys)/netdb.h $(sys)/sys/un.h \
	$(sys)/dirent.h $(sys)/sys/termio.h $(sys)/pwd.h \
	sun/sunlocal.h defines.h barthdr protocol.h \
	structs.h fundefs.h server.h debugopt.h \
	> helios.h

hydra : sun/hydra.o sun/linklib.o
	cc -o hydra sun/hydra.o sun/linklib.o sun/hepc2.o sun/kparix.o \
		-lsocket -lnsl

hydramon : sun/hydramon.o
	cc -o hydramon sun/hydramon.o -lsocket -lnsl

sun/kparix.o : sun/kparix.c $(header)
	$(mycc) sun/kparix.c
	mv kparix.o $@

sun/hydra.o : sun/hydra.c $(header)
	$(mycc) sun/hydra.c
	mv hydra.o $@

sun/hydramon.o : sun/hydramon.c $(header)
	$(mycc) sun/hydramon.c
	mv hydramon.o $@

clean :
	rm -f helios.h *.o sun/*.o server hydra hydramon

