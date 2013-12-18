#
# makefile for helios 3.10x server for SCO Unix
#

# start with the makefile configuration
# 1) specify the host and target hardware
host     = SCOUNIX

# 2) the host controls which link I/O modules are needed
linkobjs = sun/linklib.o sun/kparix.o

# 3) SCOUNIX uses its own coroutine library, based on code originally
# developed by Bleistein
colibobjs = scounix/colib.o scounix/coalloc.o
colib_link = scounix/colib.o scounix/coalloc.o -ltermcap -lsocket

# miscellaneous macros
sys      = /usr/include
header   = barthdr
mycc     = cc -c -D$(host) 

# these files compile for all the hardware
servobjs = server.o files.o devices.o tload.o cofuns.o \
	debug.o terminal.o linkio.o tcp.o sun/sunlocal.o sun/kparix.o # x.o

default : helios.h server hydra hydramon

server: $(servobjs) $(linkobjs) $(colibobjs)
	cc -o server $(servobjs) $(linkobjs) $(colib_link)

hydra : sun/hydra.o $(linkobjs)
	cc -o hydra sun/hydra.o $(linkobjs) -lsocket

hydramon : sun/hydramon.o
	cc -o hydramon sun/hydramon.o -lsocket

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

tcp.o: tcp.c $(header)
	$(mycc) tcp.c

linkio.o : linkio.c $(header)
	$(mycc) linkio.c

scounix/colib.o : scounix/colib.c $(header)
	$(mycc) scounix/colib.c
	mv colib.o $@

scounix/coalloc.o : scounix/coalloc.c $(header)
	$(mycc) scounix/coalloc.c
	mv coalloc.o $@

sun/sunlocal.o : sun/sunlocal.c $(header)
	$(mycc) sun/sunlocal.c
	mv sunlocal.o $@

sun/linklib.o : sun/linklib.c $(header) 
	$(mycc) sun/linklib.c
	mv linklib.o $@

sun/hydra.o : sun/hydra.c $(header)
	$(mycc) sun/hydra.c
	mv hydra.o $@

sun/hydramon.o : sun/hydramon.c $(header)
	$(mycc) sun/hydramon.c
	mv hydramon.o $@

sun/kparix.o : sun/kparix.c $(header)
	$(mycc) sun/kparix.c
	mv kparix.o $@

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
		sun/sunlocal.h scounix/scounix.h
	cat $(sys)/stdio.h $(sys)/sys/types.h $(sys)/ctype.h $(sys)/setjmp.h \
	$(sys)/stdarg.h $(sys)/sys/time.h $(sys)/errno.h $(sys)/sys/socket.h \
	$(sys)/sys/times.h $(sys)/signal.h $(sys)/fcntl.h \
	$(sys)/unistd.h $(sys)/sys/stat.h $(sys)/sys/param.h \
	$(sys)/sys/file.h $(sys)/memory.h \
	$(sys)/netinet/in.h $(sys)/netdb.h $(sys)/sys/statfs.h \
	$(sys)/dirent.h  $(sys)/pwd.h $(sys)/sys/wait.h $(sys)/termios.h \
	$(sys)/limits.h sun/sunlocal.h scounix/scounix.h scounix/un.h \
	defines.h barthdr protocol.h \
	structs.h fundefs.h server.h debugopt.h \
	> helios.h


