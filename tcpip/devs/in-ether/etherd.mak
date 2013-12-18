
HELIOS	= /helios/lib
INCLUDE	= /helios/include
LIB	= /helios/lib
AMPPDIR	= /helios/include/ampp

CC	= cc
ASM	= asm
CP	= cp
RM	= rm
AMPP	= ampp

LIBS	= $(HELIOS)/kernel.def $(HELIOS)/syslib.def\
		$(LIB)/util.def $(LIB)/posix.def

DEBUG	= # -DB407_DEBUG
TCPFIX	= # -DTCP_SETINFO

HFILES	= b407dev.h b407.h

.SUFFIXES: .o .s .c

.c.o:
		$(CC) $(DEBUG) $(TCPFIX) -ps1 -pf0 -r -j$(INCLUDE)/ $*.c -s $*.s
		$(ASM) -p $*.s -o $*.o

ether.d:	b407dev.o devs.p modend.p b407.o

		-$(ASM) -v -o ether.d devs.p b407dev.o b407.o modend.p $(LIBS)
		-$(CP) ether.d $(LIB)

b407dev.o:	b407dev.c $(HFILES)

b407.o:		b407.c $(HFILES)

devs.p:		devs.a

		$(AMPP) -o devs.out -i$(AMPPDIR)/ $(AMPPDIR)/basic.m devs.a
		$(ASM) -p -o devs.p devs.out

modend.p:	modend.a

		$(AMPP) -o modend.out -i$(AMPPDIR)/ $(AMPPDIR)/basic.m modend.a
		$(ASM) -p -o modend.p modend.out

