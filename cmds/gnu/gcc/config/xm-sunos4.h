/* Config file for running GCC on Sunos version 4.
   This file is good for either a Sun 3 or a Sun 4 machine.  */

#ifdef sparc 
#include "xm-sparc.h" 
#else 
#include "xm-m68k.h"
#endif

/* Provide required defaults for linker -e and -d switches.
   Also, it is hard to debug with shared libraries,
   so don't use them if going to debug.  */

#define LINK_SPEC "%{!e*:-e start} -dc -dp %{g:-Bstatic} %{static:-Bstatic}"
