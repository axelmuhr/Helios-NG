#
# OASYS C++ requires all C++ files to have the extension .cxx.
# This script to convert an InterViews directory to OASYS suffixes
# was provided by Walter Milliken at BBN.
#
# You have to set the "oasify" environment variable to whereever
# oasify.sed lives.
#
for i in *.c
do
  mv $i `basename $i .c`.cxx
done

mv genMakefile genMakefile.old
sed -f $oasify/oasify.sed genMakefile.old >genMakefile
