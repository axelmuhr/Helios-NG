ASM	=	asm
CC	=	cc
ECHO	=	echo
PP	=	pp
RM	=	rm

TMP	=	/ram

TC	=	$(TMP)/c
TINC	=	$(TMP)/include
TASM	=	$(TMP)/asm
TLIB	=	$(TMP)/lib
TOBJ	=	$(TMP)/obj
TBIN 	=	$(TMP)/bin

HINC	=	/helios/include
UINC	=	/j/util
INCPATH =	$(TINC),$(HINC),$(UINC)

PPFLAGS	=	-dTEST=1 -dRELEASE=0 -dCHCKFRMT=0 -dIN_NUCLEUS=0 -d__HELIOS=1 -d__TRAN=1 -d__HELIOSTRAN=1
ERRORS	=	$(TMP)/errors

#LIBS	=	$(TLIB)

fstring.c	:	error.h fstring.h
fstring		:	$(TBIN)/fstring

$(TBIN)/fstring	: 	$(TOBJ)/fstring.o
	$(ECHO) "Linking" && \
	$(ASM) -s20000 -v -f \
	$(TOBJ)/fstring.o \
	$(TLIB)/cstart.o $(LIBS) -o $(TBIN)/fstring >>& $(ERRORS)

proccnt.c	:	misc.h error.h proccnt.h
proccnt		:	$(TBIN)/proccnt

$(TBIN)/proccnt	: 	$(TOBJ)/proccnt.o $(TOBJ)/error.o $(TOBJ)/procname.o
	$(ECHO) "Linking" && \
	$(ASM) -s20000 -v -f \
	$(TOBJ)/proccnt.o $(TOBJ)/error.o $(TOBJ)/procname.o \
	$(TLIB)/cstart.o $(LIBS) -o $(TBIN)/proccnt >>& $(ERRORS)

$(TOBJ)/%.o : %.c
	$(ECHO) Compiling $*.c && \
	$(PP) -v -i$(INCPATH) $(PPFLAGS) $*.c -o $(TC)/preprocd.c >>& $(ERRORS) && \
	$(CC) $(CFLAGS) $(TC)/preprocd.c -s $(TASM)/$*.s >>& $(ERRORS) && \
	$(ASM) -p $(TASM)/$*.s -o $(TOBJ)/$*.o && \
	$(RM) -f $(TASM)/$*.s

#$(FSOBJS) $(UOBJS) $(TMPLIB)/s0.o $(LIBS)
