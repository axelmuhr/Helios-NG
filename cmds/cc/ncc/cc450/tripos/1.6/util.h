
/* C compiler file util.h: Copyright (C) A.C.Norman and A.Mycroft */

/* version 0.02 */

#ifndef _UTIL_LOADED

#define _UTIL_LOADED 1

#define isodigit(c) (!((c)-'0' & ~7))    /* tests for octal digit         */
#define intofdigit(c) ((c)-'0')          /* turns oct or dec digit to int */
#define pad_to_word(n) ((n)+3L & ~3L)
#define padsize(n,align) (-((-(n)) & (-(align))))
/* the next routine pads a string size to a whole number of words (and
   counts 1 for the terminating NUL, returning the size in bytes.         */
#define padstrlen(n) ((((int32)(n))+INTWIDTH) & ~(INTWIDTH-1L))

#endif

/* end of util.h */
