ASM	=	asm
CC	=	cc
ECHO	=	echo
PP	=	pp
RM	=	rm
TCP	=	tcp

SOURCE	=	/j
UTIL	=	$(SOURCE)/util
TMP	=	/ram
TINC	=	$(TMP)/include
TC    	=	$(TMP)/c
TASM	=	$(TMP)/asm
TOBJ	=	$(TMP)/obj

#CFLAGS	=	-ps1
#CFLAGS	=	-fh -fm -fv
PPFLAGS	=	-dRELEASE=1 -dTEST=0 -dCHCKFRMT=0 -dIN_NUCLEUS=1 -d__HELIOS=1 -d__TRAN=1 -d__HELIOSTRAN=1
ERRORS	=	$(TMP)/errors.tmp

#	$(TCP) $(UTIL)/$*.c $(TC)/$*.c && \
#	$(PP) -v -i$(TINC) $(PPFLAGS) $(TC)/$*.c -o $(TC)/preprocd.c >>& $(ERRORS) && \
#	$(CC) $(CFLAGS) $(TC)/preprocd.c -s $(TASM)/$*.s >>& $(ERRORS) && \

$(TOBJ)/%.o : $(UTIL)/%.c
	$(ECHO) Compiling $*.c && \
	$(PP) -v -i$(TINC) $(PPFLAGS) $(UTIL)/$*.c -o $(TC)/$*.c >>& $(ERRORS) && \
	$(CC) $(CFLAGS) $(TC)/$*.c -s $(TASM)/$*.s >>& $(ERRORS) && \
	$(ASM) -p $(TASM)/$*.s -o $(TOBJ)/$*.o && \
	$(RM) -f $(TASM)/$*.s $(TC)/$*.c

UOBJS	=	$(TOBJ)/error.o \
		$(TOBJ)/prime.o \
		$(TOBJ)/proccnt.o \
		$(TOBJ)/procname.o \
		$(TOBJ)/fstring.o

$(UTIL)/error.c	:	$(TINC)/codes.h \
			$(TINC)/helios.h \
			$(TINC)/sem.h \
			$(TINC)/stdarg.h \
			$(TINC)/string.h \
			$(TINC)/stdio.h \
			$(TINC)/syslib.h \
			$(TINC)/time.h \
			$(TINC)/error.h \
			$(TINC)/misc.h \
			$(TINC)/procname.h \
			$(TINC)/stdio.h

$(UTIL)/prime.c:	$(TINC)/helios.h \
			$(TINC)/misc.h

$(UTIL)/procname.c:	$(TINC)/helios.h \
			$(TINC)/module.h \
			$(TINC)/procname.h

$(UTIL)/fstring.c:	$(TINC)/helios.h \
			$(TINC)/string.h \
			$(TINC)/error.h \
			$(TINC)/fstring.h

$(UTIL)/proccnt.c:	$(TINC)/nonansi.h \
			$(TINC)/sem.h \
			$(TINC)/misc.h \
			$(TINC)/error.h \
			$(TINC)/proccnt.h

all	:	$(UOBJS)
