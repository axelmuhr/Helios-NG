#! /bin/csh -f
#
# Build g++ assuming we are in the top-level directory.
#

setenv TOP iv
setenv makingCC true

#
# Check to make sure we know what machine we're targetting.
#
if (! $?ivPlatform) then
    setenv ivPlatform `/bin/sh $TOP/src/bin/scripts/cpu.sh $TOP/config/InterViews`
endif

set gmachine = unknown
switch ($ivPlatform)
    case SUN4:
	set gmachine = sun4-os4
	breaksw
    case SUN3:
	set gmachine = sun3-os4
	breaksw
    case VAX:
	set gmachine = vax
	breaksw
    default:
	echo "Don't know how to build g++ on" $ivPlatform
	exit 1
	breaksw
endsw
set d = `pwd`
cd g++
make machine=$gmachine

set bindir = $d/iv/bin/$ivPlatform
set libdir = $d/iv/lib/$ivPlatform
foreach i ($bindir $libdir)
    if (! -d $i) then
	echo "mkdir $i"
	mkdir $i
	chmod g+w $i
    endif
end
mv g++-$gmachine/c++ $bindir/c++
mv g++-$gmachine/ld++ $bindir/ld++
mv g++-$gmachine/crt0+.o $libdir/crt0+.o
mv g++-$gmachine/gnulib $libdir/libgnulib.a
