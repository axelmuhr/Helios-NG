INSDIR = ../..
ALL = makemine printable unshar
makemine:	makemine.sh
	cp makemine.sh makemine
	chmod 755 makemine
printable:	printable.sh
	cp printable.sh printable
	chmod 755 printable
unshar: unshar.sh
install:	$(ALL)
	mv $(ALL) ${INSDIR}/bin
clean:
clobber:
	rm -f makemine printable
