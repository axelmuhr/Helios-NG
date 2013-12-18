TCP	=	tcp

SOURCE	=	/j
PFS	=	$(SOURCE)/pfs
FS	=	$(PFS)/fs
FSINC	=	$(FS)
TMP	=	/ram
TINC	=	$(TMP)/include

TINCS	=	$(TINC)/buf.h \
		$(TINC)/check.h \
		$(TINC)/fs.h \
		$(TINC)/fserr.h \
		$(TINC)/fservlib.h \
		$(TINC)/inode.h \
		$(TINC)/nfs.h \
		$(TINC)/partchck.h \
		$(TINC)/rdevinfo.c

$(TINC)/%.h	:	$(FSINC)/%.h
	$(TCP) $< $@

$(TINC)/%.c	:	$(FSINC)/%.c
	$(TCP) $< $@

all	:	$(TINCS)
