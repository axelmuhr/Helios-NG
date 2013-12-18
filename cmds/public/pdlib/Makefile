#	
#
makefile:	makehead makebody
	rm -f makefile
	cat makehead makebody > makefile
	egrep '#include[ 	][ 	]*"' *.[chyl] | \
		sed -e 's/^\([^\.]*\)\.[cyl]:.*"\(.*\)"/\1.o:	\2/' \
			-e 's/^\([^\.]*\).h:.*"\(.*\)"/\1.h:	\2/' \
			>> makefile
