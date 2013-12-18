
/* locale.h: ANSI draft (X3J11 Oct 86) library header, section 4.3 */
/* Copyright (C) A. Mycroft and A.C. Norman */
/* version 0.01 */

#ifndef __locale_h
#define __locale_h

#define LC_COLLATE  1
#define LC_CTYPE    2
#define LC_NUMERIC  4
#define LC_TIME     8
#define LC_ALL     15

extern char *setlocale(int category, const char *locale);

#endif

/* end of locale.h */
