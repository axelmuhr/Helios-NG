#
# makefile for Helios 3.8x I/O Server for Unix386
# port done by Hans-Wilhelm Rock, Bleistein-Rohde Systemtechnik GmbH
# Grueniger Weg 21, D-6301 Pohlheim 2, West Germany
# Fax : +49 6404 3640    Phone : +49 6404 61071
#

# start with the makefile configuration
host     = UNIX386
linkobjs = sun/linklib.o bleistein/linkif.o  

colibobjs = bleistein/colib.o bleistein/coalloc.o
colib_link = bleistein/colib.o bleistein/coalloc.o -ltermcap

# miscellaneous macros
sys      = /usr/include
header   = barthdr
mycc     = cc -c -pipe -O1 -D$(host)

# these files compile for all the hardware
servobjs = server.o files.o devices.o tload.o cofuns.o \
	debug.o terminal.o linkio.o sun/sunlocal.o 

default : helios.h server hydra hydramon

server: $(servobjs) $(linkobjs) $(colibobjs)
	cc -o server $(servobjs) $(linkobjs) $(colib_link)

hydra : sun/hydra.o $(linkobjs)
	cc -o hydra sun/hydra.o $(linkobjs)

hydramon : sun/hydramon.o
	cc -o hydramon sun/hydramon.o

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

bleistein/colib.o : bleistein/colib.c $(header)
	$(mycc) bleistein/colib.c
	mv colib.o $@

bleistein/coalloc.o : bleistein/coalloc.c $(header)
	$(mycc) bleistein/coalloc.c
	mv coalloc.o $@

sun/sunlocal.o : sun/sunlocal.c $(header)
	$(mycc) sun/sunlocal.c
	mv sunlocal.o $@

sun/linklib.o : sun/linklib.c $(header) 
	$(mycc) sun/linklib.c
	mv linklib.o $@

bleistein/linkif.o : bleistein/linkif.c $(header)
	$(mycc) bleistein/linkif.c
	mv linkif.o $@

sun/hydra.o : sun/hydra.c $(header)
	$(mycc) sun/hydra.c
	mv hydra.o $@

sun/hydramon.o : sun/hydramon.c $(header)
	$(mycc) sun/hydramon.c
	mv hydramon.o $@

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
		sun/sunlocal.h bleistein/unix386.h
	cat $(sys)/stdio.h $(sys)/sys/types.h $(sys)/ctype.h $(sys)/setjmp.h \
	$(sys)/sys/time.h $(sys)/errno.h $(sys)/sys/socket.h \
	$(sys)/sys/times.h $(sys)/signal.h $(sys)/fcntl.h \
	$(sys)/unistd.h $(sys)/sys/stat.h $(sys)/sys/param.h \
	$(sys)/sys/file.h $(sys)/memory.h $(sys)/sys/vfs.h \
	$(sys)/netinet/in.h $(sys)/netdb.h $(sys)/sys/un.h \
	$(sys)/dirent.h  $(sys)/termio.h \
	sun/sunlocal.h bleistein/unix386.h \
	defines.h barthdr protocol.h \
	structs.h fundefs.h server.h debugopt.h \
	> helios.h

