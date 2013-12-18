#@@@ Note that this is not a helios generic HOSTUTIL makefile
# But it should co-exist with the system happily as it defines all the
# required targets.
#-----------------------------------------------------------------------------
# makefile for helios 3.80 generic unix server for the ARM
#
# PAB June 90
#
# SccsId: @(#)makefile	1.1 16/6/90
#-----------------------------------------------------------------------------


ifndef HHOSTBIN
	HHOSTBIN = /usr/local/bin
endif

InProcMake = TRUE

include ../makefile

sys	 = /usr/include
s        = ..
sa       = .
sun      = ../sun
header   = helios.h

mycc     = cc -c -O -I$(s),$(sa) -DARMBSD=1

#-----------------------------------------------------------------------------

XTARGETS = aserver hydra hydramon

Xdefault: $(XTARGETS)

Xinstall:
	$(CP) $(XTARGETS) $(HHOSTBIN)
	$(TOUCH) Xinstall

Xclean:


#-----------------------------------------------------------------------------
aserver: $(sa)/helios.h $(sa)/server.o $(sa)/files.o $(sa)/devices.o \
	        $(sa)/tload.o $(sa)/cofuns.o $(sa)/debug.o $(sa)/terminal.o \
		$(sa)/colib.o $(sa)/unixlocal.o  $(sa)/linklib.o \
	 	$(sa)/linkio.o $(sa)/gnome.o $(sa)/tcp.o
	cc -ltermcap -o aserver $(sa)/server.o $(sa)/files.o $(sa)/devices.o \
			$(sa)/tload.o $(sa)/cofuns.o $(sa)/debug.o \
			$(sa)/terminal.o $(sa)/linkio.o \
			$(sa)/colib.o $(sa)/unixlocal.o $(sa)/linklib.o \
			$(sa)/gnome.o $(sa)/tcp.o

#-----------------------------------------------------------------------------
hydra: $(sa)/hydra.o $(sa)/linklib.o $(sa)/gnome.o
	cc -o hydra $(sa)/hydra.o $(sa)/linklib.o $(sa)/gnome.o

#-----------------------------------------------------------------------------
hydramon: $(sa)/hydramon.o
	cc -o hydramon $(sa)/hydramon.o

#-----------------------------------------------------------------------------

$(sa)/tload.o: $(s)/tload.c $(header)
	$(mycc) $(s)/tload.c

$(sa)/server.o: $(s)/server.c $(header) $(s)/sccs.h $(s)/fundefs.h \
	$(s)/server.h $(s)/defines.h
	$(mycc) $(s)/server.c

$(sa)/files.o: $(s)/files.c $(header)
	$(mycc) $(s)/files.c

$(sa)/devices.o: $(s)/devices.c $(header)
	$(mycc) $(s)/devices.c

$(sa)/cofuns.o: $(s)/cofuns.c $(header)
	$(mycc) $(s)/cofuns.c

$(sa)/debug.o : $(s)/debug.c $(header)
	$(mycc) $(s)/debug.c

$(sa)/terminal.o : $(s)/terminal.c $(header)
	$(mycc) $(s)/terminal.c

$(sa)/linkio.o : $(s)/linkio.c $(header)
	$(mycc) $(s)/linkio.c

$(sa)/tcp.o : $(s)/tcp.c $(header)
	$(mycc) $(s)/tcp.c

$(sa)/unixlocal.o : $(sun)/sunlocal.c $(header)
	$(mycc) -o $(sa)/unixlocal.o $(sun)/sunlocal.c

$(sa)/linklib.o : $(sun)/linklib.c $(header)
	$(mycc) -o $(sa)/linklib.o $(sun)/linklib.c

$(sa)/gnome.o : gnome.c $(header)
	$(mycc) -o $(sa)/gnome.o $(sa)/gnome.c

$(sa)/colib.o : $(sun)/colib.c $(header)
	$(mycc) -o $(sa)/colib.o -DSIGNALS $(sun)/colib.c

$(sa)/hydra.o : $(sun)/hydra.c $(header)
	$(mycc) -o $(sa)/hydra.o $(sun)/hydra.c

$(sa)/hydramon.o : $(sun)/hydramon.c $(header)
	$(mycc) -o $(sa)/hydramon.o $(sun)/hydramon.c


#-----------------------------------------------------------------------------
# The order of the header files is important. First I incorporate all the
# C header files. Then comes Defines.h to specify the environment in which the
# server is meant to run, which must come before any of the remaining headers.
# Barthdr declares some miscellaneous odds and ends. Next come all the helios
# header files from directory servinc. Finally the server's private header
# files, including the local one. structs.h declares the server's data
# structures and macros, fundefs declares the functions shared between modules,
# and server.h declares the shared variables.

sysheads = $(sys)/stdio.h $(sys)/ctype.h $(sys)/setjmp.h \
	   $(sys)/sgtty.h $(sys)/signal.h $(sys)/fcntl.h $(sys)/time.h \
	   $(sys)/sys/time.h $(sys)/sys/types.h $(sys)/dirent.h \
	   $(sys)/sys/stat.h $(sys)/sys/vfs.h $(sys)/sys/file.h \
	   $(sys)/errno.h \
	   $(sys)/sys/socket.h $(sys)/sys/un.h $(sys)/netinet/in.h \
	   $(sys)/netdb.h
#sysheads = $(sys)/stdio.h $(sys)/ctype.h $(sys)/setjmp.h \
#	   $(sys)/sgtty.h $(sys)/sys/signal.h $(sys)/sys/fcntl.h $(sys)/sys/time.h \
#	   $(sys)/sys/types.h $(sys)/sys/dir.h \
#	   $(sys)/sys/stat.h $(sys)/sys/vfs.h $(sys)/sys/file.h \
#	   $(sys)/sys/errno.h \
#	   $(sys)/sys/socket.h $(sys)/sys/un.h $(sys)/netinet/in.h \
#	   $(sys)/netdb.h


servheads = $(sun)/sunlocal.h $(sa)/armlocal.h \
	$(s)/defines.h $(s)/barthdr $(s)/protocol.h $(s)/structs.h \
	$(s)/fundefs.h $(s)/server.h $(s)/debugopt.h

$(sa)/helios.h: $(sysheads) $(servheads)
	cat $(sysheads) $(servheads) > $(sa)/helios.h

