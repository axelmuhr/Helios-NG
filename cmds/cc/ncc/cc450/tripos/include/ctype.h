
/* ctype.h: ANSI draft (X3J11 Oct 86) library header, section 4.3 */
/* Copyright (C) A.C. Norman and A. Mycroft */
/* version 0.01 */

#ifndef __ctype_h
#define __ctype_h

/* N.B. - keep in step with <ctype.c> */

#define __S 1            /* whitespace           */
#define __P 2            /* punctuation          */
#define __B 4            /* blank                */
#define __L 8            /* lower case letter    */
#define __U 16           /* upper case letter    */
#define __N 32           /* (decimal) digit      */
#define __C 64           /* underscore           */
#define __X 128          /* A-F and a-f          */
extern unsigned char _ctype[];

#    define isalnum(c) (_ctype[c] & (__U+__L+__N))
extern int (isalnum)(int c);
#    define isalpha(c) (_ctype[c] & (__U+__L))
extern int (isalpha)(int c);
#    define iscntrl(c) (!(_ctype[c] & (__P+__B+__L+__U+__N)))
extern int (iscntrl)(int c);
#    define isdigit(c) (_ctype[c] & __N)
extern int (isdigit)(int c);
#    define isgraph(c) (_ctype[c] & (__L+__U+__N+__P))
extern int (isgraph)(int c);
#    define islower(c) (_ctype[c] & __L)
extern int (islower)(int c);
#    define isprint(c) (_ctype[c] & (__L+__U+__N+__P+__B))
extern int (isprint)(int c);
#    define ispunct(c) (_ctype[c] & __P)
extern int (ispunct)(int c);
#    define isspace(c) (_ctype[c] & __S)
extern int (isspace)(int c);
#    define isupper(c) (_ctype[c] & __U)
extern int (isupper)(int c);
#    define isxdigit(c) (_ctype[c] & (__N+__X))
extern int (isxdigit)(int c);

extern int tolower(int c);
extern int toupper(int c);

#endif

/* end of ctype.h */
