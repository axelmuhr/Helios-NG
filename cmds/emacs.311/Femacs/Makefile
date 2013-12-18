CFLAGS=		-O2 -pipe -w -DSUNOS

OFILES=		basic.o bind.o buffer.o char.o crypt.o display.o eval.o \
		exec.o file.o fileio.o folding.o input.o isearch.o line.o \
		lock.o main.o \
		mouse.o random.o region.o search.o tcap.o unix.o window.o word.o

CFILES=		basic.c bind.c buffer.c char.c crypt.c display.c eval.c \
		exec.c file.c fileio.c folding.c input.c isearch.c line.c \
		lock.c main.c \
		mouse.c random.c region.c search.c tcap.c unix.c window.c word.c

HFILES=		estruct.h edef.h efolding.h efunc.h epath.h\
		ebind.h evar.h etype.h

emacs:		$(OFILES)
		$(CC) -Bstatic $(CFLAGS) $(OFILES) -ltermcap -o uue

$(OFILES):	$(HFILES)
