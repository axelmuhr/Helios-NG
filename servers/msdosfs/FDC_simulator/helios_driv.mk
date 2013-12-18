# Makefile for  -- Dos server with extern dos driver.
#		-- Telmat device drivers
# HELIOS Version 1.1B

.SUFFIXES:
.SUFFIXES: .z .o .c .x .s .t .a

HELIOS = /helios
LIB = $(HELIOS)/lib
INC = $(HELIOS)/include
MACRO = /helios/ampp/include
CC  = cc
ASM = asm
AMPP = ampp
TMPDIR = .

CFLAGS = -pg0 -ps1
# pour debug : flag DEBUG (par exemple) pour #ifdef ...
#CFLAGS = -pg0 -ps1 -dDEBUG
FNCFLAGS =     -j$(INC)/ $(CFLAGS)
RNCFLAGS =  -r -j$(INC)/ $(CFLAGS)


# .c.x : definit le passage de <file>.c a <file>.x
.c.x:
	$(CC) $(RNCFLAGS) $*.c -s$*.x 

.c.s:
	$(CC) $(FNCFLAGS) $*.c -s$*.s

.c.o:  
	$(CC) $(FNCFLAGS) $*.c -s$(TMPDIR)/$*.x
	$(ASM) -p $(TMPDIR)/$*.x -o$*.o
	rm $(TMPDIR)/$*.x

.a.o:
	$(AMPP) -i$(MINC)/ $(MINC)/basic.m $*.a | $(ASM) -p -o$*.o

.a.x:
	$(AMPP) -o$*.x -i$(MINC)/ $(MINC)/basic.m $*.a

.x.o:
	$(ASM) -p $*.x -o$*.o

.o.z:
	$(ASM) -f $(LIB)/cstart.o $*.o $(GEN)/errep.o  -o$*.z
	objed -i $*.z -n$* 
	cp $*.z $(HELIOS)/bin/$*


all: dos.d dos

dos.d : dos.o driver0.o driver1.o 
	asm dos.o driver0.o driver1.o $(LIB)/modend.o $(LIB) -o dos.d
	cp dos.d $(LIB)
	
dos.o : dos.a
	ampp -i$(INC)/ $(INC)/basic.m dos.a | asm -p -o dos.o

dos : servdos.s file_access.s path.s
	asm -v -f -h10000  $(LIB)/cstart.o \
	servdos.s file_access.s path.s \
	$(LIB)/servlib.def \
	-o dos
#	cp dos $(LIB)

servdos.s : servdos.c servdos.h  general.h
	$(CC) $(FNCFLAGS) servdos.c -sservdos.s

driver0.s : driver0.c driver0.h general.h
	$(CC) $(RNCFLAGS) driver0.c -sdriver0.s

driver1.s : driver1.c driver1.h general.h
	$(CC) $(RNCFLAGS) driver1.c -sdriver1.s

file_access.s : file_access.c file_access.h general.h
	$(CC) $(FNCFLAGS) file_access.c -sfile_access.s


path.s : path.c path.h general.h
	$(CC) $(FNCFLAGS) path.c -spath.s

