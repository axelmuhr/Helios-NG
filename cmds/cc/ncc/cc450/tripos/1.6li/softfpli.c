
/* C library for use with software floating point.                       */
/* Copyright A. Mycroft and A. C. Norman, 1987.                          */
/* AM wonders if this file is dead */

#define SOFTWARE_FLOATING_POINT

/* Floating point representation defined by IBMFLOAT or IEEE             */

#ifndef IBMFLOAT
#   ifndef IEEE
#      error neither IBMFLOAT nor IEEE defined - assuming IEEE
#      define IEEE
#   endif
#endif


#include "ansilib.c"
#include "softfp.c"


/* end of softfplib.c */
