ASM	=	asm
CC	=	cc
ECHO	=	echo
PP	=	pp
RM	=	rm
TCP	=	tcp

SOURCE	=	/j
PFS	=	$(SOURCE)/pfs
FS	=	$(PFS)/fs
TMP	=	/ram
TINC	=	$(TMP)/include
TC    	=	$(TMP)/c
TASM	=	$(TMP)/asm
TOBJ	=	$(TMP)/obj

#CFLAGS	=	-ps1
#CFLAGS	=	-fh -fm -fv
PPFLAGS	=	-dRELEASE=0 -dTEST=0 -dCHCKFRMT=0 -dIN_NUCLEUS=1 -d__HELIOS=1 -d__TRAN=1 -d__HELIOSTRAN=1
ERRORS	=	$(TMP)/errors.tmp

#	$(TCP) $(FS)/$*.c $(TC)/$*.c && \
#	$(PP) -v -i$(TINC) $(PPFLAGS) $(TC)/$*.c -o $(TC)/preprocd.c >>& $(ERRORS) && \
#	$(CC) $(CFLAGS) $(TC)/$preprocd.c -s $(TASM)/$*.s >>& $(ERRORS) && \

$(TOBJ)/%.o : $(FS)/%.c
	$(ECHO) Compiling $*.c && \
	$(PP) -v -i$(TINC) $(PPFLAGS) $(FS)/$*.c -o $(TC)/$*.c >>& $(ERRORS) && \
	$(CC) $(CFLAGS) $(TC)/$*.c -s $(TASM)/$*.s >>& $(ERRORS) && \
	$(ASM) -p $(TASM)/$*.s -o $(TOBJ)/$*.o && \
	$(RM) -f $(TASM)/$*.s $(TC)/$*.c

FSOBJS	=	$(TOBJ)/alloc.o \
		$(TOBJ)/bio.o \
		$(TOBJ)/deal_fs.o \
		$(TOBJ)/dev.o \
		$(TOBJ)/fserr.o \
		$(TOBJ)/fserver.o \
		$(TOBJ)/fservlib.o \
		$(TOBJ)/fsyscall.o \
		$(TOBJ)/inode.o \
		$(TOBJ)/partchck.o \
		$(TOBJ)/tserver.o \
		$(TOBJ)/checker.o \
		$(TOBJ)/concheck.o \
		$(TOBJ)/condups.o \
		$(TOBJ)/dircheck.o \
		$(TOBJ)/misc_chk.o \
		$(TOBJ)/tidyup.o \
		$(TOBJ)/xtdcheck.o

$(FS)/deal_fs.c	:	$(TINC)/error.h \
			$(TINC)/misc.h \
			$(TINC)/prime.h \
			$(TINC)/nfs.h \
			$(TINC)/root.h \
			$(TINC)/rdevinfo.c

$(FS)/checker.c	:	$(TINC)/error.h \
			$(TINC)/check.h \
			$(TINC)/fserr.h \
			$(TINC)/helios.h \
			$(TINC)/stdio.h \
			$(TINC)/stdlib.h \
			$(TINC)/syslib.h

$(FS)/bio.c	:	$(TINC)/error.h \
			$(TINC)/proccnt.h \
			$(TINC)/fserr.h \
			$(TINC)/nfs.h \
			$(TINC)/module.h

$(FS)/rdevinfo.c:	$(TINC)/error.h \
			$(TINC)/limits.h \
			$(TINC)/module.h \
			$(TINC)/string.h \
			$(TINC)/misc.h \
			$(TINC)/fserr.h

$(FS)/condups.c	:	$(TINC)/error.h \
			$(TINC)/check.h

$(FS)/partchck.c:	$(TINC)/device.h \
			$(TINC)/helios.h \
			$(TINC)/error.h \
			$(TINC)/fserr.h \
			$(TINC)/misc.h \
			$(TINC)/nfs.h

$(FS)/concheck.c:	$(TINC)/error.h \
			$(TINC)/check.h

$(FS)/fserver.c	:	$(TINC)/root.h \
			$(TINC)/error.h \
			$(TINC)/proccnt.h \
			$(TINC)/fserr.h \
			$(TINC)/nfs.h \
			$(TINC)/partchck.h

$(FS)/tidyup.c	:	$(TINC)/error.h \
			$(TINC)/check.h \
			$(TINC)/fserr.h

$(FS)/fsyscall.c:	$(TINC)/error.h \
			$(TINC)/proccnt.h \
			$(TINC)/fserr.h \
			$(TINC)/nfs.h

$(FS)/dircheck.c:	$(TINC)/error.h \
			$(TINC)/check.h

$(FS)/fserr.c	:	$(TINC)/stdio.h \
			$(TINC)/stdlib.h \
			$(TINC)/string.h \
			$(TINC)/error.h \
			$(TINC)/fstring.h \
			$(TINC)/fserr.h

$(FS)/fservlib.c:	$(TINC)/error.h \
			$(TINC)/proccnt.h \
			$(TINC)/nfs.h

$(FS)/tserver.c	:	$(TINC)/root.h \
			$(TINC)/error.h \
			$(TINC)/fserr.h \
			$(TINC)/nfs.h

$(FS)/misc_chk.c:       $(TINC)/error.h \
			$(TINC)/check.h \
			$(TINC)/fserr.h \
			$(TINC)/stdarg.h

$(FS)/inode.c	:	$(TINC)/error.h \
			$(TINC)/proccnt.h \
			$(TINC)/fstring.h \
			$(TINC)/inode.h \
			$(TINC)/nfs.h

$(FS)/xtdcheck.c:	$(TINC)/error.h \
			$(TINC)/check.h

$(FS)/alloc.c	:	$(TINC)/error.h \
			$(TINC)/proccnt.h \
			$(TINC)/fserr.h \
			$(TINC)/nfs.h

$(FS)/dev.c	:	$(TINC)/error.h \
			$(TINC)/fserr.h \
			$(TINC)/nfs.h \
			$(TINC)/device.h

all	: $(FSOBJS)
