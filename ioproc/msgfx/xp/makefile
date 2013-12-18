# REMOTE = remote 01

CC = $(REMOTE) cc
ASM = $(REMOTE) asm
LIB      = /helios/lib/
CSTART   = $(LIB)/cstart.o
FPFILES  = $(LIB)/fplib.def $(LIB)/fpclib.def
MSGBIND  = msgbind.o


.SUFFIXES: .c .o
.c.o:
	$(CC) $*.c -s $*.s -t8
	$(ASM) -p -o $*.o $*.s 
	rm $*.s

PLTFILES = msgplot.o $(MSGBIND)
msgplot : $(PLTFILES)
	$(ASM) $(CSTART) $(FPFILES) $(PLTFILES) -o $@

GRESETFILES = greset.o $(MSGBIND)
greset : $(GRESETFILES)
	$(ASM) $(CSTART) $(FPFILES) $(GRESETFILES) -o $@

FLOODFILES = flood.o $(MSGBIND)
flood : $(FLOODFILES)
	$(ASM) $(CSTART) $(FPFILES) $(FLOODFILES) -o $@

GTESTFILES = gtest.o $(MSGBIND)
gtest : $(GTESTFILES)
	$(ASM) $(CSTART) $(FPFILES) $(GTESTFILES) -o $@

all: msgplot gtest flood greset
