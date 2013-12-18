
/* limits.h: ANSI draft (X3J11 Oct 86) library header, section 2.2.4.2 */
/* Copyright (C) A.C. Norman and A. Mycroft */
/* version 0.01 */

#ifndef __limits_h
#define __limits_h

#define CHAR_BIT 8
#define SCHAR_MIN (-128)
#define SCHAR_MAX 127
#define UCHAR_MAX 255
#define CHAR_MIN 0
#define CHAR_MAX 255

#define SHRT_MIN  (-0x8000)
#define SHRT_MAX  0x7fff
#define USHRT_MAX 65535U
#define INT_MIN   (~0x7fffffff)  /* -2147483648 and 0x80000000 are unsigned */
#define INT_MAX   0x7fffffff
#define UINT_MAX  0xffffffff
#define LONG_MIN  (~0x7fffffff)
#define LONG_MAX  0x7fffffff
#define ULONG_MAX 0xffffffffU

#endif

/* end of limits.h */
