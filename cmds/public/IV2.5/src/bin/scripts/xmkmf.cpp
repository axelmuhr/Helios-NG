/**/#!/bin/sh
/**/# 
/**/# generate a Makefile from an Imakefile outside of the source tree
/**/# 

if [ -f Makefile ]; then 
    rm Makefile.bak
    mv Makefile Makefile.bak
fi
imake CONFIGDIRSPEC -DUseInstalled $*
