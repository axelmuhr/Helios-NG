#> makefile
# ncc SMT test programs
#------------------------------------------------------------------------------

# make link to appropriate ncc
CC = ./ncc
#non smt flags
CFLAGS = -c -j/helios/include
NOTE = non SMT code

# smt flags
SMTCFLAGS = -zs -c -j/helios/include
SMTNOTE = standard SMT code

# smt flags + resident libraries
SRCFLAGS = -zs -zr -c -j/helios/include
SRNOTE = SMT + resident library modes

#------------------------------------------------------------------------------

.SUFFIXES:
.SUFFIXES: .lst .smtlst .srlst .o .c

.c.lst:
	${CC} ${CFLAGS} -o $*.o $*.c
	echo "$(NOTE)" > $@
	cat $*.c >> $@
	echo >> $@
	objdump -d $*.o >> $@

.c.smtlst:
	${CC} ${SMTCFLAGS} -o $*.o $*.c
	echo "$(SMTNOTE)" > $@
	cat $*.c >> $@
	echo >> $@
	objdump -d $*.o >> $@

.c.srlst:
	${CC} ${SRCFLAGS} -o $*.o $*.c
	echo "$(SRNOTE)" > $@
	cat $*.c >> $@
	echo >> $@
	objdump -d $*.o >> $@

#------------------------------------------------------------------------------

#bug1.lst bug2.lst bug4.lst bug5.lst bug7.lst \

LISTS =	smt_test1.lst smt_test2.lst smt_test3.lst smt_test4.lst \
	smt_test5.lst smt_test6.lst smt_test7.lst smt_test8.lst \
	smt_test9.lst smt_test10.lst smt_test11.lst smt_test12.lst \
	smt_test13.lst


#bug1.smtlst bug2.smtlst bug4.smtlst bug5.smtlst bug7.smtlst \

SMTLISTS = smt_test1.smtlst smt_test2.smtlst smt_test3.smtlst smt_test4.smtlst \
	smt_test5.smtlst smt_test6.smtlst smt_test7.smtlst smt_test8.smtlst \
	smt_test9.smtlst smt_test10.smtlst smt_test11.smtlst \
	smt_test12.smtlst smt_test13.smtlst

# bug4.srlst - now gives error for static init with -Zr
# shouldn't with -Zl though.

#bug1.srlst bug2.srlst bug5.srlst bug7.srlst \

SRLISTS = smt_test1.srlst smt_test2.srlst smt_test3.srlst smt_test4.srlst \
	smt_test5.srlst smt_test6.srlst smt_test7.srlst smt_test8.srlst \
	smt_test9.srlst smt_test10.srlst smt_test11.srlst smt_test12.srlst \
	smt_test13.srlst


#------------------------------------------------------------------------------
.PHONEY: print all clean

all:	${SMTLISTS} ${LISTS} ${SRLISTS}

#------------------------------------------------------------------------------
print: all
	cpr *.smtlst *.lst *.srlst
#laserp -l *.smtlst *.lst *.srlst


clean:
	-rm ${SMTLISTS} ${LISTS} ${SRLISTS} *.o

	
#------------------------------------------------------------------------------

ascii:
	$(CC) -oascii2 -j/helios/include ascii.c

#> done
