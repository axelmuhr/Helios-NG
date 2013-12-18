SHELL = /bin/sh

all:	msg awf pearls resub chem swaplns delete messages argarray longwrds \
	fstabplus compare arrayref

gawk:	fieldwidths ignorecase regtest posix manyfiles

bigtest:	all book lisp gawk

msg::
	@echo 'Any output from "cmp" is bad news!'
	@echo '(some differences in floating point values are to be expected)'

awf::
	@cd awf; AWK=../../gawk  make r

pearls::
	@cd pearls; ../../gawk -f pearls >tmp
	cd pearls; cmp good tmp; rm tmp

resub::
	@AWK=../gawk sh T.re >tmp
	cmp T.re.good tmp
	@AWK=../gawk sh T.sub >tmp
	cmp T.sub.good tmp

chem::
	@cd chem; AWK=../../gawk sh chem *.p >tmp
	cd chem; cmp good tmp; rm tmp

swaplns::
	@../gawk -f swaplns.awk data >tmp
	cmp swaplns.good tmp; rm tmp

delete::
	@../gawk -f delete.awk

messages::
	@../gawk -f messages.awk >out2 2>out3
	cmp out1.good out1; cmp out2.good out2; cmp out3.good out3
	@rm -f out1 out2 out3

argarray::
	@TEST=test ../gawk -f argarray.awk >tmp
	cmp argarray.good tmp
	@rm -f tmp

fstabplus::
	@echo '1		2' | ../gawk -f fstabplus >tmp
	cmp fstabplus.good tmp && rm -f tmp

longwrds::
	@../gawk -f longwrds.awk manpage | sort >tmp
	cmp longwrds.good tmp && rm -f tmp

book::
	@./book.run
	@echo Run diff -r on book.out vs. book.good
	@echo '(some differences in dates are to be expected)'

lisp::
	@cd lisp; ../../gawk -f walk *.w >tmp
	cd lisp && cmp good tmp && rm -f tmp

fieldwidths::
	@echo '123456789' | ../gawk -v FIELDWIDTHS="2 3 4" '{print $$2}' >tmp
	cmp fieldwidths.good tmp && rm -f tmp

ignorecase::
	@echo xYz | ../gawk -v IGNORECASE=1 '{sub(/y/, ""); print}' >tmp
	cmp ignorecase.good tmp && rm -f tmp

regtest::
	@echo 'some of the output from regtest is very system specific'
	regtest

posix::
	@echo '1:2,3 4' | ../gawk -f posix >tmp
	cmp posix.good tmp && rm -f tmp

manyfiles::
	@mkdir junk
	@../gawk 'BEGIN { for (i = 1; i <= 100; i++) print i, i}' >tmp
	@../gawk -f manyfiles.awk tmp tmp
	@echo -n "This number better be 1 ->"
	@wc -l junk/* | ../gawk '$$1 != 2' | wc -l
	@rm -rf junk tmp

compare::
	@../gawk -f compare.awk 0 1 compare.in >tmp
	cmp compare.good tmp && rm -f tmp

arrayref::
	@../gawk -f arrayref >tmp
	cmp arrayref.good tmp && rm -f tmp
