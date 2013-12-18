TMP	=	/ram
TOBJ	=	$(TMP)/obj
TBIN	=	$(TMP)/bin
B11	=	/e/helios.117/bin
OBJ11	=	/e/develop/obj.11
BIN11	=	/e/develop/bin.11
OBJ12	=	/e/develop/obj.12
BIN12	=	/e/develop/bin.12

CP	=	$(B11)/cp_p -p

SOBJS	=	OBJDIR/error.o \
		OBJDIR/prime.o \
		OBJDIR/proccnt.o \
		OBJDIR/procname.o \
		OBJDIR/fstring.o \
		OBJDIR/alloc.o \
		OBJDIR/bio.o \
		OBJDIR/deal_fs.o \
		OBJDIR/dev.o \
		OBJDIR/fserr.o \
		OBJDIR/fserver.o \
		OBJDIR/fservlib.o \
		OBJDIR/fsyscall.o \
		OBJDIR/inode.o \
		OBJDIR/partchck.o \
		OBJDIR/tserver.o \
		OBJDIR/checker.o \
		OBJDIR/concheck.o \
		OBJDIR/condups.o \
		OBJDIR/dircheck.o \
		OBJDIR/misc_chk.o \
		OBJDIR/tidyup.o \
		OBJDIR/xtdcheck.o

SBIN	=	BINDIR/fs

$(OBJ11)/%.o	: $(TOBJ)/%.o
	$(CP) $< $@

$(BIN11)/%	: $(TBIN)/%
	$(CP) $< $@

$(OBJ12)/%.o	: $(TOBJ)/%.o
	$(CP) $< $@

$(BIN12)/%	: $(TBIN)/%
	$(CP) $< $@

all11	: $(subst OBJDIR, $(OBJ11), $(SOBJS)) $(subst BINDIR, $(BIN11), $(SBIN))
all12	: $(subst OBJDIR, $(OBJ12), $(SOBJS)) $(subst BINDIR, $(BIN12), $(SBIN))

.PHONY:	all11 all12

