# $Header: /hsrc/filesys/pfs_v2.1/src/msc_new/RCS/msc_new.mk,v 1.1 1992/07/13 16:20:44 craig Exp $
#
# makefile for HELIOS MSC Device driver V1.1 910314
#

##############################################################

ASM	=	asm
CC	=	cc
ECHO	=	echo
PP	=	pp
RM	=	rm
TCP	=	tcp

SOURCE	=	/j
PFS	=	$(SOURCE)/pfs
MSC	=	$(PFS)/msc_new
TMP	=	/ram
TINC	=	$(TMP)/include
TC    	=	$(TMP)/c
TASM	=	$(TMP)/asm
TOBJ	=	$(TMP)/obj
B12	=	/e/develop/bin.12
O12	=	/e/develop/obj.12

PPFLAGS	=	-dRELEASE=0 -dTEST=0 -dCHCKFRMT=0 -dIN_NUCLEUS=1 -d__HELIOS=1 -d__TRAN=1 -d__HELIOSTRAN=1
AMPPFLAGS=	-dhelios.TRAN 1
ERRORS	=	$(TMP)/errors.tmp

#############################################################################

.SUFFIXES:
.SUFFIXES: .z .o .p .c .a

# pattern rules :
%:: %,v
#  commands to execute (built-in):
#	test -f $@ || $(CO) $(COFLAGS) $< $@
	if ( ! -f $@ ) $(CO) $(COFLAGS) $< $@

%:: RCS/%,v
#  commands to execute (built-in):
#	test -f $@ || $(CO) $(COFLAGS) $< $@
	if ( ! -f $@ ) $(CO) $(COFLAGS) $< $@

#############################################################

MAKEFILE= msc_new.mk
RINC	= /ram/include
RLOC	= /ram/msc
TEMP	= /ram
HINC	= /helios/include
KINC	= /helios/include/ampp
LIB     = /helios/lib
BIN     = /helios/bin
#ASMFLAGS= -lhelios
#ASMFLAGS= -l/helios/lib/helios

#
#	Debugging constants
#
NONE	= 0
HR	= 0x1
INT	= 0x2
CMD	= 0x4
MSGOUT	= 0x8
MSGIN	= 0x10
RESET	= 0x20
TIDYUP	= 0x40
INIT	= 0x80
REGS	= 0x100
WDCMD	= 0x200
STAT	= 0x400
HACK	= 0x800
ABORT	= 0x1000
SCSI	= 0x2000
SENSE	= 0x4000
FMT	= 0x8000
ALL	= 0xFFFFFFFF
#DLEVEL	= DEBUG=$(ALL)
#DLEVEL	= DEBUG=$(INT)+$(MSGIN)+$(TIDYUP)+$(SENSE)
#DLEVEL	= DEBUG=$(HR)+$(INT)+$(MSGIN)+$(TIDYUP)
DLEVEL	= DEBUG=$(NONE)

HDRS	= msc.h mscaddr.h mscstruc.h scsiinfo.h rdevinfo.h
RHDRS	= $(RLOC)/msc.h $(RLOC)/mscaddr.h $(RLOC)/mscstruc.h \
	  $(RLOC)/scsiinfo.h $(RLOC)/rdevinfo.h

ASRCS	= mscasm.a mscdiscs.a
CSRCS	= msc.c device.c scsi.c rdscsi.c utility.c rdevinfo.c \
	  finddrv.c testdrv.c testinfo.c gsi.c

AOBJS	= mscasm.o mscdiscs.o
COBJS	= rdscsi.o rdevinfo.o finddrv.o testdrv.o testinfo.o gsi.o
DOBJS	= msc.o device.o scsi.o utility.o

DRVNAME	= msc21.dev
DRVPATH	= $(B12)/$(DRVNAME)

H12	= -d__HELIOS=1 -d__TRAN=1 -d__HELIOSTRAN=1

COPTS	= $(H12) -j$(RINC)/,$(HINC)/ -i$(RLOC)/,./ -dEVENTS -d$(DLEVEL)

ROPTS	= $(H12) -ps1 -pf0 -r -j$(RINC)/,$(HINC)/ -i$(RLOC)/,./ -d$(DLEVEL) -dEVENTS -dDriver -dRECONNECT

.c.p:
	cc $*.c $(COPTS) -s $*.p

.a.p:
	ampp $(AMPPFLAGS) -i$(KINC)/ $(KINC)/basic.m -o $*.p $*.a

.p.o:
	asm -p $*.p -o $*.o

.c.o:
	cc $*.c $(COPTS) -s $(TEMP)/$*.p >>& $(ERRORS) && \
	asm -p $(TEMP)/$*.p -o $*.o && \
	rm -f $(TEMP)/$*.p

.a.o:	
	( ampp $(AMPPFLAGS) -i$(KINC)/ $(KINC)/basic.m $*.a | asm -p -o $*.o ) >>& $(ERRORS)

.PHONY:	$(DRVNAME) all de gsi finddrv testdrv testinfo msc21.dev se get put

all:	$(DRVNAME) finddrv testdrv testinfo

de:	
	rm -f $(ERRORS)

se:
	-more $(ERRORS)

$(RLOC):	
	mkdir $(RLOC)

$(RHDRS):	$(RLOC)/%.h:	%.h $(RLOC)
	tcp $< $(RLOC)

$(COBJS):	$(HDRS) $(MAKEFILE)
$(DOBJS):	$(HDRS) $(MAKEFILE)
msc.o:		$(RHDRS)

finddrv:	$(B12)/finddrv

$(B12)/finddrv:	msc.o device.o rdscsi.o scsi.o utility.o mscasm.o rdevinfo.o finddrv.o
	asm -v -f $(LIB)/cstart.o $(ASMFLAGS) finddrv.o msc.o device.o rdscsi.o scsi.o utility.o mscasm.o rdevinfo.o\
	-o $(B12)/finddrv

testdrv:	$(B12)/testdrv

$(B12)/testdrv:	msc.o device.o rdscsi.o scsi.o utility.o mscasm.o rdevinfo.o testdrv.o
	asm -v -f $(LIB)/cstart.o $(ASMFLAGS) testdrv.o msc.o device.o rdscsi.o scsi.o utility.o mscasm.o rdevinfo.o\
	-o $(B12)/testdrv

msc.or:		$(RHDRS) rdscsi.c scsi.c device.c utility.c msc.c
	cc msc.c $(ROPTS) -s $(TEMP)/msc.pr >>& $(ERRORS) && \
	asm -p $(TEMP)/msc.pr -o msc.or && \
	rm -f $(TEMP)/msc.pr

$(DRVNAME):	$(DRVPATH)

$(DRVPATH):	mscdiscs.o msc.or
	asm -v -f -n mscdisc -o $(DRVPATH) mscdiscs.o msc.or $(LIB)/modend.p \
	$(LIB)/kernel.def $(LIB)/syslib.def $(LIB)/util.def >>& $(ERRORS)

#testinfo.o:	testinfo.c $(RHDRS)

testinfo:	$(B12)/testinfo

$(B12)/testinfo:	msc.o device.o rdscsi.o scsi.o utility.o mscasm.o rdevinfo.o testinfo.o
	asm -v -f $(LIB)/cstart.o testinfo.o msc.o device.o rdscsi.o scsi.o utility.o mscasm.o rdevinfo.o \
	$(LIB)/fault.def -o $(B12)/testinfo

gsi:		$(B12)/gsi

$(B12)/gsi:	gsi.o 
	asm -v -f $(LIB)/cstart.o gsi.o -o $(B12)/gsi

put:
ifdef	MSG
	ci -m'$(MSG)' $(MAKEFILE) $(HDRS) $(ASRCS) $(CSRCS)
else
	ci $(MAKEFILE) $(HDRS) $(ASRCS) $(CSRCS)
endif

get:
	co -l $(MAKEFILE) $(HDRS) $(ASRCS) $(CSRCS)
	touch $(MAKEFILE) $(HDRS) $(ASRCS) $(CSRCS)

touch:
	touch $(MAKEFILE) $(HDRS) $(ASRCS) $(CSRCS)

clean:
	rm -f $(AOBJS) $(COBJS) $(DOBJS) msc.or *~* $(ERRORS) fgreps
