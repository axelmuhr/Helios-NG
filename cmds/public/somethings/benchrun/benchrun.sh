#!/bin/sh
#	main shell for running 1 or more benchmarks, sending all
#	output to stdout, or each run into a results directory
if [ $# -lt 2 ]
then
	echo >&2 "usage: $0 target ident [ [ code ] directory1 ... ]
		target = compile 	compile only.
			all		run test, compare results, compile if needed.
			clean		clean up any leftover junk.
			clobber		restore to pristine state.
			run		run test only, compile if needed.
			compare		compare the results to reference results.
			save		save the results in results.ident
			validate		see all.
			anythingelse	if you have such
		ident  = company, or company.machine (same as in M.ident)
			 or can be "" to pick up the default makefiles
		code   = identification subcode (release, etc)
		directory1... = list of directories, 1/benchmark
			if omitted, does all of benchspec"
	echo >&2 "shell variables"
	echo >&2 "MACHINE: use for identification if ident == ''"
	echo >&2 "RESULTDIR:  if unset, all output to stdout"
	echo >&2 "\tif set, results go in \$RESULTDIR/\$bench/\$ident.code"
	exit 1
fi

target="$1"
case "$1" in
validate | all | compile | run | clean | clobber | compare | save) ;;
*)	echo >&2 "$0 warning: $1 nonstandard, attempting anyway";;
esac

#	set up Mfile= M.ident, or Makefile
#	set up ident = machine-type, $MACHINE, or unk(nown)
if [ "$2" != "" ]
then
	Mfile="M.$2"
	ident="$2"
else
		Mfile=Makefile
		ident=${MACHINE-unk}
fi

#	set up code as input code, or unk(nown)
if [ $# -le 2 ]
then
	code=unk
	shift;shift
elif [ $# -eq 3 ]
then
	code="$3"
	if [ ! -d benchspec ]
	then
		echo >&2 "$0: error, no benchspec subdirectory found"
		exit 1
	fi
	set benchspec/*
else
	code="$3"
	shift;shift;shift
fi


if [ "$RESULTDIR" != "" ]
then
	case $RESULTDIR in
	/* ) ;;
	* )	RESULTDIR=`pwd`/$RESULTDIR
	esac
	if [ ! -d $RESULTDIR ]
	then
		echo >&2 "$0: error: RESULTDIR = $RESULTDIR not directory"
		exit 1
	fi
fi

#	everything is set, $* = list of directories to do
if [ "$#" = "0" ] ; then
	dirlist="benchspec/*"
else
	dirlist="$*"
fi
here=`pwd`
for dir in $dirlist
do
	cd $here
	echo $dir
	(
	if [ ! -d $dir ]
	then
		echo >&2 "$0: error: $dir not found, skipped"
	else
		bench=`basename $dir`
		date=`date`
		cd $dir
		if [ -f $Mfile ]
		then
			Mfilename=$Mfile
		elif [ -f Makefile ]
		then
			Mfilename=Makefile
		elif [ -f makefile ]
		then
			Mfilename=makefile
		else
			echo >&2 "$0: error in `pwd`"
			echo >&2 "$0: error: no $Mfile, Makefile, or makefile"
			exit 1
		fi
		if [ "$RESULTDIR" != "" ]
		then
			mkdir $RESULTDIR/$bench 2>/dev/null
			resultfile=$RESULTDIR/$bench/$ident.$code
			eval >$resultfile 2>&1	# switch output
		fi
		#	finally, the real thing!
		echo "BENCHRUN:START $bench, $ident, $code $date"
		make -f $Mfilename $target
		echo "BENCHRUN:END $bench, $ident, $code, $date"
	fi
	)
done
