ASM	=	asm
CAT	=	cat
DATE	=	date
ECHO	=	echo
EDIT	=	emacs
FGREP	=	fgrep
GMAKE	=	gmake
POPD	=	popd
PUSHD	=	pushd
RM	=	rm

TMP	=	/ram
TLIB	=	$(TMP)/lib
TASM	=	$(TMP)/asm
TBIN	=	$(TMP)/bin
TOBJ	=	$(TMP)/obj

LIBS	=	$(TLIB)/kernel.def $(TLIB)/syslib.def $(TLIB)/util.def
ERRORS	=	$(TMP)/errors.tmp
FILTER	=	/j/fgrep/makefilt.fgr
PPERRORS=	$(TMP)/pperrors
MAKETIME=	$(TMP)/maketime

ASMOPTS	=	-s100000 -v

UOBJS	=	$(TOBJ)/error.o \
		$(TOBJ)/prime.o \
		$(TOBJ)/proccnt.o \
		$(TOBJ)/procname.o \
		$(TOBJ)/fstring.o

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

$(TBIN)/fs : $(FSOBJS) $(UOBJS) $(TLIB)/s0.o $(LIBS)
	$(ECHO) "Linking" && \
	$(ASM) $(ASMOPTS) -f $(TLIB)/s0.o $(LIBS) $(FSOBJS) $(UOBJS) -o $(TBIN)/fs >>& $(ERRORS)

prepare:
	$(DATE) > $(MAKETIME); \
	$(RM) -f $(ERRORS); \
	$(GMAKE) -srf cpsysinc.mk all && \
	$(GMAKE) -srf cpuinc.mk all && \
	$(GMAKE) -srf cpfsinc.mk all && \
	$(GMAKE) -srf cpsyslib.mk all && \
	$(GMAKE) -srf computil.mk all && \
	$(GMAKE) -srf compfs.mk all
.PHONY : prepare

postpareA:
	$(DATE) >> $(MAKETIME); \
	$(PUSHD) $(TMP) > /null; \
	$(FGREP) -vif $(FILTER) $(ERRORS) > errors; \
	$(EDIT) errors; \
	$(POPD)

postpare11:
	$(GMAKE) -srf cpback.mk all11

postpare12:
	$(GMAKE) -srf cpback.mk all12

postpareB:
	$(CAT) $(MAKETIME); \
	$(RM) $(MAKETIME)

.PHONY : postpareA postpare11 postpare12 postpareB

fs11:	prepare $(TBIN)/fs postpareA postpare11 postpareB

fs12:	prepare $(TBIN)/fs postpareA postpare12 postpareB

.PHONY : fs11 fs12

clean:
	$(RM) -f $(TOBJ)/*.o
	$(RM) -f $(TBIN)/fs
.PHONY : clean
