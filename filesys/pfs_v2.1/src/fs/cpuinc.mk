TCP	=	tcp

SOURCE	=	/j
UTIL	=	$(SOURCE)/util
UINC	=	$(UTIL)
TMP	=	/ram
TINC	=	$(TMP)/include

TINCS	=	$(TINC)/debug.h \
		$(TINC)/error.h \
		$(TINC)/file.h \
		$(TINC)/fio.h \
		$(TINC)/fstring.h \
		$(TINC)/misc.h \
		$(TINC)/prime.h \
		$(TINC)/proccnt.h \
		$(TINC)/procname.h


$(TINC)/%.h	:	$(UINC)/%.h
	$(TCP) $< $@

all	:	$(TINCS)
