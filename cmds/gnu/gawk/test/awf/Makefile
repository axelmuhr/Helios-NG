AWFLIB = /usr/lib/awf	# beware, awf itself knows this
BIN = /usr/bin
MAN = /usr/man/man1
CP = common dev.dumb mac.man mac.ms pass1 pass2.base pass2.man pass2.ms pass3
DTR = README COPYRIGHT Makefile awf awf.1 awf.1.out common dev.dumb mac.man \
	mac.ms pass1 pass2.base pass2.man pass2.ms pass3
# System V brain damage
SHELL = /bin/sh

r:	awf.1
	@chmod +x awf
	@AWFLIB=. sh awf -man awf.1 >tmp
	cmp tmp awf.1.out
	@rm tmp

install:
	-if test ! -d $(AWFLIB) ; then mkdir $(AWFLIB) ; fi
	cp $(CP) $(AWFLIB)
	cp awf $(BIN)
	cp awf.1 $(MAN)

rr:	r testm tests.Z tests.out.Z
	AWFLIB=. ./awf -man testm >tmp
	cmp tmp testm.out
	rm tmp
	uncompress <tests.Z | AWFLIB=. awf -ms >tmp
	uncompress <tests.out.Z | cmp - tmp
	rm tmp

dtr:	$(DTR)
	makedtr $(DTR) >$@

tar:	$(DTR)
	tar cvf awf.tar $(DTR)
	compress -v awf.tar

clean:
	rm -f tmp tests tests.out dtr awf.tar awf.tar.Z j*
