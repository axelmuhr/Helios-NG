#
# makefile for hello
#

.SUFFIXES:
.SUFFIXES: .o .s .a .c

.c.o:
	c -c $*.c -o $*.o

.a.o:
	ampp -o $*.s $*.a
	as -o $*.o $*.s


all : hello hello2

hello    : hello.c
	c hello.c -o hello

hello2:	hello2.a
	ampp -i /hsrc/include/ampp -o hello2.s /hsrc/include/ampp/basic.m \
		-d helios.C40 1 -d __SMT 1 hello2.a
	asc40 -o hello2.o hello2.s
	ldc40 -o hello2 /hprod/C40/lib/cstart.o hello2.o
