/^CC/i\
.SUFFIXES:	.cxx\
.cxx.o:\
\	${CC} -c ${CFLAGS} $*.cxx\

/^CC/s/CC$/ccxx !q/
/^LD/s/CC$/ccxx !q/
/\.c[^x]/s/\.c/\.cxx/g
/\.c$/s/\.c$/\.cxx/
/-I\/usr\/include\/CC/s:-I/usr/include/CC::
