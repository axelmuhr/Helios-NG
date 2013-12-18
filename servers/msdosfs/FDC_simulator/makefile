# Makefile for  -- Dos server with extern dos driver.
#		-- Telmat device drivers
# HELIOS Version 1.1B

.SUFFIXES:
.SUFFIXES: .z .o .c .x .s .t .a

HELIOS = /helios
LIB = $(HELIOS)/lib
INC = $(HELIOS)/include
MINC = /helios/include/ampp
CC  = cc
ASM = asm
AMPP = ampp
TMPDIR = .
LIBDEFS = $(LIB)/kernel.def $(LIB)/syslib.def $(LIB)/util.def $(LIB)/posix.def

CFLAGS = -pf0 -ps1
# pour debug : flag DEBUG (par exemple) pour #ifdef ...
#CFLAGS = -pf0 -ps1 -dDEBUG
FNCFLAGS =     -j$(INC)/ $(CFLAGS)
RNCFLAGS =  -r -j$(INC)/ $(CFLAGS)


all: fdcdev.d

fdcdev.d : dos.o fdcdev.o
	asm -f dos.o fdcdev.o $(LIB)/modend.o $(LIBDEFS) -o fdcdev.d
	cp fdcdev.d $(LIB)
	
dos.o : dos.a
	ampp -dhelios.TRAN 1 -i$(MINC)/ $(MINC)/basic.m dos.a > dos.s
	asm -p dos.s -o dos.o

fdcdev.o : fdcdev.s
	asm -p fdcdev.s -o fdcdev.o

fdcdev.s : fdcdev.c
	$(CC) $(RNCFLAGS) fdcdev.c -sfdcdev.s
