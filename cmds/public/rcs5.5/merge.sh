: 'Output the merge command as a shell file.'

Id='$Id: merge.sh,v 5.3 90/11/01 05:03:32 eggert Exp $'

cat >a.sh <<'EOF' && chmod +x a.sh || exit
#!/bin/sh
export something
EOF
if
	(
		./a.sh &&
		if csh -c :
		then csh -c ./a.sh
		else :
		fi
	) 2>/dev/null
then
	echo '#!/bin/sh'
	echo '# merge - three-way file merge'
	echo "#	$Id"
else
	echo ': merge - three-way file merge'
	echo ": '$Id'"
fi
rm -f a.sh

cat <<EOF

DIFF=${DIFF?}
DIFF3=${DIFF3?}
EOF

cat <<'EOF'
PATH=/bin:/usr/bin
labels=0 p=w say=echo
while
	case $1 in
	-p)
		p='1,$p';;
	-q)
		say=:;;
	-L)
		case $labels in
		0) l1=$2 labels=1;;
		1) l3=$2 labels=2;;
		*) echo "merge: too many -L options"; exit 2
		esac
		case $# in
		1) ;;
		*) shift
		esac;;
	-*)
		echo "merge: $1: unknown option"; exit 2;;
	*)
		break
	esac
do shift
done

case $# in
3) ;;
*)
	echo >&2 'merge: usage: merge [-p] [-q] [-L label1 [-L label3]] file1 file2 file3'
	exit 2
esac

f1=$1 f2=$2 f3=$3
case $1 in +*) f1=./$1;; esac
case $2 in +*|-*) f2=./$2;; esac
case $3 in +*|-*) f3=./$3;; esac

case $labels in
0) l3=$3 l1=$1;;
1) l3=$3
esac

case $p in
w)
	if test ! -w "$f1"
	then
		echo >&2 "merge: $1 not writeable"
		exit 2
	fi
esac

status=2
temps=
trap '
	case $temps in
	?*) rm -f $temps || status=2
	esac
	exit $status
' 0
trap exit 1 2 3 13 15
umask 077

t=/tmp/d3t$$

EOF

case ${DIFF3_TYPE?} in
bin) sed 's/^[	 ]*//' <<'EOF'
	case $p in
	w) temps=$t;;
	*) t=
	esac

	$DIFF3 -am -L "$l1" -L "$l3" "$f1" "$f2" "$f3" >$t
	s=$?

	case $s in
	0) ;;
	1) $say >&2 "merge: overlaps during merge";;
	*) exit
	esac

	case $p in
	w) cp $t "$f1" || s=2
	esac

	status=$s
EOF
;;

*) sed 's/^[	 ]*//' <<'EOF'
	temps="/tmp/d3a$$ /tmp/d3b$$ $t"

	$DIFF "$f1" "$f3" >/tmp/d3a$$
	case $? in
	0|1) ;;
	*) exit
	esac

	$DIFF "$f2" "$f3" >/tmp/d3b$$
	case $? in
	0|1) ;;
	*) exit
	esac

	$DIFF3 -E /tmp/d3a$$ /tmp/d3b$$ "$f1" "$f2" "$f3" "$l1" "$l3" >$t
	s=$?

	case $s in
	0) ;;
	*) s=1; $say >&2 "merge: overlaps or other problems during merge"
	esac

	echo $p >>$t  &&  ed - "$f1" <$t   ||   s=2

	status=$s
EOF
esac
