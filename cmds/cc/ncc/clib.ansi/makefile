CC = ncc
SPARCCC = ../cc350/ccsparc/ncc
CFLAGS = -J. -D__unix_lib

SOURCES = ctype.c error.c fpprintf.c locale.c math.c printf.c \
	scanf.c sort.c stdlib.c string.c stdio.c clippersys.c alloc.c \
	sbrk.c unix-sbrk.c raise.c jpff-signal.c time.c getenv.c system.c \
	remove.c checks.c
# xmath.c

OBJ =  ctype.o error.o fpprintf.o locale.o math.o printf.o \
	scanf.o sort.o stdlib.o string.o stdio.o alloc.o clippersys.o \
	clipperprof.o syscall.o sbrk.o unix-sbrk.o raise.o jpff-signal.o \
	time.o getenv.o system.o vfork.o remove.o checks.o setjmp.o
# time.o xmath.o

ASM =  ctype.s error.s fpprintf.s locale.s math.s printf.s \
	scanf.s sort.s stdlib.s string.s stdio.s alloc.s sparcsys.s \
	unix-sbrk.s raise.s jpff-signal.s \
	time.s getenv.s system.s remove.s checks.s
# time.s xmath.s

XSRCS =	getuid.c isatty.c
XOBJS = getuid.o isatty.o

libAnsi.a:	$(OBJ) 
	ar crv libAnsi.a $(OBJ)
	ranlib libAnsi.a

libUnix.a:	$(XOBJS)
	ar crv libUnix.a $(XOBJS)
	ranlib libUnix.a

.c.o:	hostsys.h
	${CC} ${CFLAGS} -c $*.c

#### For SPARC ###########################################
ctype.s:	${SPARCCC} ctype.c
	${SPARCCC} ${CFLAGS} -D__sparc -S ctype.c

error.s:	${SPARCCC} error.c
	${SPARCCC} ${CFLAGS} -D__sparc -S error.c

fpprintf.s:	${SPARCCC} fpprintf.c
	${SPARCCC} ${CFLAGS} -D__sparc -S fpprintf.c

locale.s:	${SPARCCC} locale.c
	${SPARCCC} ${CFLAGS} -D__sparc -S locale.c

math.s:	${SPARCCC} math.c
	${SPARCCC} ${CFLAGS} -D__sparc -S math.c

printf.s:	${SPARCCC} printf.c
	${SPARCCC} ${CFLAGS} -D__sparc -S printf.c

scanf.s:	${SPARCCC} scanf.c
	${SPARCCC} ${CFLAGS} -D__sparc -S scanf.c

sort.s:	${SPARCCC} sort.c
	${SPARCCC} ${CFLAGS} -D__sparc -S sort.c

stdlib.s:	${SPARCCC} stdlib.c
	${SPARCCC} ${CFLAGS} -D__sparc -S stdlib.c

string.s:	${SPARCCC} string.c
	${SPARCCC} ${CFLAGS} -D__sparc -S string.c

stdio.s:	${SPARCCC} stdio.c
	${SPARCCC} ${CFLAGS} -D__sparc -S stdio.c

alloc.s:	${SPARCCC} alloc.c
	${SPARCCC} ${CFLAGS} -D__sparc -S alloc.c

sparcsys.s:	${SPARCCC} sparcsys.c
	${SPARCCC} ${CFLAGS} -D__sparc -S sparcsys.c

sbrk.s:	${SPARCCC} sbrk.c
	${SPARCCC} ${CFLAGS} -D__sparc -S sbrk.c

unix-sbrk.s:	${SPARCCC} unix-sbrk.c
	${SPARCCC} ${CFLAGS} -D__sparc -S unix-sbrk.c

raise.s:	${SPARCCC} raise.c
	${SPARCCC} ${CFLAGS} -D__sparc -S raise.c

jpff-signal.s:	${SPARCCC} jpff-signal.c
	${SPARCCC} ${CFLAGS} -D__sparc -S jpff-signal.c

time.s:	${SPARCCC} time.c
	${SPARCCC} ${CFLAGS} -D__sparc -S time.c

getenv.s:	${SPARCCC} getenv.c
	${SPARCCC} ${CFLAGS} -D__sparc -S getenv.c

system.s:	${SPARCCC} system.c
	${SPARCCC} ${CFLAGS} -D__sparc -S system.c

remove.s:	${SPARCCC} remove.c
	${SPARCCC} ${CFLAGS} -D__sparc -S remove.c

checks.s:	${SPARCCC} checks.c
	${SPARCCC} ${CFLAGS} -D__sparc -S checks.c


assem:	${ASM}
	echo done

install:	libAnsi.a Ncrt0.o
	cp libAnsi.a /usr/local/lib/libAnsi.a
	ranlib /usr/local/lib/libAnsi.a
	cp Ncrt0.o /lib/Ncrt0.o

#          /***************************************************/
#          /*                                                 */
#          /*    Define UN*Xmake dependencies                 */
#          /*                                                 */
#          /***************************************************/

depend:
	  -rm makedep
	  for i in ${SOURCES}; do \
	      ${CC} ${OPTS} -M ${CFLAGS} $$i | \
	      awk ' { if ($$1 != prev) \
		  { if (rec != "") print rec; rec = $$0; prev = $$1; } \
		  else { if (length(rec $$2) > 78) { print rec; rec = $$0; } \
		  else rec = rec " " $$2 } } \
		  END { print rec } ' >> makedep; done
	  cp Makefile makefile
	  echo '# DO NOT DELETE THIS LINE' >> makefile
	  cat makedep >> makefile
	  echo '# DEPENDENCIES MUST END AT END OF FILE' >> makefile
	  echo '# IF YOU PUT STUFF HERE IT WILL GO AWAY' >> makefile
	  echo '# see make depend above' >> makefile

# DO NOT DELETE THIS LINE
ctype.o:	ctype.c ctype.h
error.o:	error.c hostsys.h /users/jpff/ncc.include/syscall.h ./stdio.h ./math.h
error.o:	./stdio.h ./stdlib.h ./stddef.h ./signal.h ./assert.h ./string.h
error.o:	./errno.h
fpprintf.o:	fpprintf.c hostsys.h /users/jpff/ncc.include/syscall.h ./stdio.h
fpprintf.o:	./stdio.h ./stdarg.h ./string.h ./math.h
locale.o:	locale.c ./locale.h ./stddef.h ./stdio.h ./time.h ./string.h
locale.o:	./stdlib.h
math.o:	math.c hostsys.h /users/jpff/ncc.include/syscall.h ./stdio.h ./limits.h
math.o:	./errno.h ./math.h
printf.o:	printf.c hostsys.h /users/jpff/ncc.include/syscall.h ./stdio.h
printf.o:	stdio.h ctype.h stdarg.h string.h math.h
scanf.o:	scanf.c hostsys.h /users/jpff/ncc.include/syscall.h ./stdio.h
scanf.o:	./stdio.h ./stdlib.h ./math.h ./errno.h ./string.h ./ctype.h
scanf.o:	./stddef.h ./stdarg.h ./limits.h
sort.o:	sort.c ./stddef.h ./stdlib.h
stdlib.o:	stdlib.c hostsys.h /users/jpff/ncc.include/syscall.h ./stdio.h
stdlib.o:	./stdlib.h ./signal.h
string.o:	string.c hostsys.h /users/jpff/ncc.include/syscall.h ./stdio.h
string.o:	./stddef.h ./string.h
stdio.o:	stdio.c hostsys.h /users/jpff/ncc.include/syscall.h ./stdio.h
stdio.o:	./stdio.h ./string.h ./stddef.h ./stdlib.h ./errno.h
clippersys.o:	clippersys.c hostsys.h /users/jpff/ncc.include/syscall.h
clippersys.o:	./stdio.h ./stdio.h ./stdlib.h ./ctype.h ./string.h ./time.h
clippersys.o:	/users/jpff/ncc.include/sys/times.h
alloc.o:	alloc.c hostsys.h /users/jpff/ncc.include/syscall.h ./stdio.h
alloc.o:	./string.h
sbrk.o:	sbrk.c hostsys.h /users/jpff/ncc.include/syscall.h ./stdio.h
unix-sbrk.o:	unix-sbrk.c
raise.o:	raise.c ./syscall.h hostsys.h /users/jpff/ncc.include/syscall.h
raise.o:	./stdio.h
jpff-signal.o:	jpff-signal.c ./signal.h hostsys.h
jpff-signal.o:	/users/jpff/ncc.include/syscall.h ./stdio.h
time.o:	time.c ./time.h ./stdio.h ./string.h
getenv.o:	getenv.c
system.o:	system.c syscall.h hostsys.h /users/jpff/ncc.include/syscall.h
system.o:	./stdio.h ./signal.h
remove.o:	remove.c hostsys.h /users/jpff/ncc.include/syscall.h ./stdio.h
remove.o:	./stdio.h ./stdlib.h
checks.o:	checks.c ./stdio.h ./stdlib.h hostsys.h
checks.o:	/users/jpff/ncc.include/syscall.h ./stdio.h
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
