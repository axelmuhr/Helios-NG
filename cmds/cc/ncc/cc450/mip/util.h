#pragma force_top_level
#pragma include_only_once
/*
 * C compiler file util.h, version 0.02
 * Copyright (C) Codemist Ltd., 1987.
 */

/*
 * RCS $Revision: 1.1 $
 * Checkin $Date: 1993/07/14 14:07:18 $
 * Revising $Author: nickc $
 */

#ifndef _util_LOADED
#define _util_LOADED 1

#define isodigit(c) (!((c)-'0' & ~7))    /* tests for octal digit         */
#define intofdigit(c) ((c)-'0')          /* turns oct or dec digit to int */
#define pad_to_word(n) ((n)+3L & ~3L)
#define padsize(n,align) (-((-(n)) & (-(align))))
/* the next routine pads a string size to a whole number of words (and
   counts 1 for the terminating NUL, returning the size in bytes.         */
#define padstrlen(n) ((((int32)(n))+sizeof(int32)) & ~(sizeof(int32)-1L))

#endif

/* end of util.h */
