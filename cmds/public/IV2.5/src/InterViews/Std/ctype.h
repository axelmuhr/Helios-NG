/*
 * C++ interface to C library character type mappings
 */

#ifndef ctype_h

#include "//usr/include/ctype.h"

/* just in case standard header file didn't */
#ifndef ctype_h
#define ctype_h
#endif

#ifndef tolower
extern int tolower(int);
#endif

#ifndef toupper
extern int toupper(int);
#endif

#endif
