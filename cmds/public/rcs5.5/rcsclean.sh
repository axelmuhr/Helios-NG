#! /bin/sh
#
# rcsclean - remove working files that are copies of the latest RCS revision

#	$Id: rcsclean.sh,v 1.7 90/11/13 15:46:17 hammer Exp $

# This program removes working files which are copies of the latest
# revision on the default branch of the corresponding RCS files.
# For each file given, rcsclean performs a co operation for the latest
# revision on the default branch, and compares
# the result with the working file. If the two are identical,
# the working file is deleted.
#
# A typical application in a Makefile would be:
# clean:;       rm *.o; rcsclean *.c *.o
#
# Limitation: This program doesn't work if given the name of
# an RCS file rather than the name of the working file.

PATH=/usr/local/bin:/bin:/usr/bin:/usr/ucb:$PATH
export PATH

usage='rcsclean: usage: rcsclean file ...'

case $1 in
0) echo >&2 "$usage"; exit 2
esac

_='
'
IFS=$_

rcs=rcs
rcsdiff=rcsdiff

for i
do
	case $i in
	-*)
		case $i in
		-[qr]*) rcs=$rcs$_$i
		esac
		rcsdiff=$rcsdiff$_$i
		shift;;
	*) break
	esac
done

case $# in
0)
	files=
	for file in .* *
	do
		case $file in
		*,v | . | ..) ;;
		[-+]* | *$_*) echo >&2 "rcsclean: $file: strange file name"; exit 2;;
		*)
			case $file in
			'*' | '.*') [ -f "$file" ] || continue
			esac
			files=$files$_$file
		esac
	done
	case $files in
	?*) set $files
	esac;;
*)
	case $* in
	*$_*) echo >&2 'rcsclean: newline in arguments'; exit 2
	esac
esac

remove=
status=0

for i
do
	case $i in
	-*)
		case $i in
		-[qr]*) rcs=$rcs$_$i
		esac
		rcsdiff=$rcsdiff$_$i;;
	*,v)
		echo >&2 "rcsclean: $i: cannot handle RCS file name"; exit 2;;
	*)
		$rcsdiff -q $i >/dev/null 2>&1
		case $? in
		# Ignore rcsdiff trouble (usually files that are not under RCS).
		0) remove=$remove$_$i;;
		1)
			echo >&2 "rcsclean: $i: " || exit
			status=1
		esac
	esac
done

case $remove in
?*)
	unlock=`rlog -L -R -l${LOGNAME-$USER} $remove` &&
	case $unlock in
	?*) $rcs -u $unlock
	esac &&
	rm -f $remove || status=2
esac

exit $status
