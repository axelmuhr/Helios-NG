: '@(#)sendbatch.sh	1.16	12/1/87'

cflags=
LIM=50000
MINDF=MINDISKFREE
MAXBATCH=MAXPERBATCH
SPOOLDISK=SPOOL_DISK
CMD='LIBDIR/batch BATCHDIR/$rmt $BLIM'
ECHO=
COMP=
C7=
DOIHAVE=
RNEWS=rnews

for rmt in $*
do
	case $rmt in
	-[bBC]*)	cflags="$cflags $rmt"; continue;;
	-s*)	LIM=`expr "$rmt" : '-s\(.*\)'`
		continue;;
	-c7) 	COMP='| LIBDIR/compress $cflags'
		C7='| LIBDIR/encode'
		ECHO='echo "#! c7unbatch"'
		continue;;
	-c)	COMP='| LIBDIR/compress $cflags'
		ECHO='echo "#! cunbatch"'
		continue;;
	-o*)	ECHO=`expr "$rmt" : '-o\(.*\)'`
		RNEWS='cunbatch'
		continue;;
	-i*)	DOIHAVE=`expr "$rmt" : '-i\(.*\)'`
		if test -z "$DOIHAVE"
		then
			DOIHAVE=`uuname -l`
		fi
		continue;;
	-m*)	MAXBATCH=`expr "$rmt" : '-m\(.*\)'`
		continue;;
	esac

	df=`df $SPOOLDISK | awk "\\$6 == \\"$SPOOLDISK\" {print \\$4}
		\\$1 == \\"$SPOOLDISK\\" {print \\$3}"`
	if test ! -z "$df" -a \( "$df" -lt $MINDF \)
	then
		echo not enough space on $SPOOLDISK: $df
		continue
	fi

	if test -s /tmp/uuq.output
	then
		q=`echo "$rmt" | sed 's/\(.......\).*/\1/'`
		q=`awk "\\$1 == \\"$q:\\" { print \\$4;exit}" </tmp/uuq.output`
		if test ! -z "$q" -a \( "$q" -gt $MAXBATCH \)
		then 
			echo $rmt already has $q bytes queued
			continue
		fi
	fi

	if test -n "$COMP"
	then
		BLIM=`expr $LIM \* 2`
	else
		BLIM=$LIM
	fi

	: make sure $? is zero
	sentbytes=0
	while test $? -eq 0 -a $sentbytes -le $MAXBATCH -a \
		\( \( $sentbytes -eq 0 -a -s BATCHDIR/$rmt \) -o \
		 -s BATCHDIR/$rmt.work -o  \
		\( -n "$DOIHAVE" -a -s BATCHDIR/$rmt.ihave \) \)
	do
		if test -n "$DOIHAVE" -a -s BATCHDIR/$rmt.ihave
		then
			mv BATCHDIR/$rmt.ihave BATCHDIR/$rmt.$$
			LIBDIR/inews -t "cmsg ihave $DOIHAVE" -n to.$rmt.ctl < \
				BATCHDIR/$rmt.$$
			rm BATCHDIR/$rmt.$$
					
		else
			(eval $ECHO; eval $CMD $COMP $C7) |
			if test -s BATCHDIR/$rmt.cmd
			then
				BATCHDIR/$rmt.cmd
			else
				uux - UUXFLAGS $rmt!$RNEWS
			fi
			sentbytes=`expr $sentbytes + $LIM`
		fi
	done
done
