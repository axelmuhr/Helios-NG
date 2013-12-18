#pragma force_top_level
#pragma include_only_once

/* ctype.h: ANSI 'C' (X3J11 Oct 88) library header, section 4.3 */
/* Copyright (C) Codemist Ltd. */
/* version 0.02 */

/*
 * ctype.h declares several functions useful for testing and mapping
 * characters. In all cases the argument is an int, the value of which shall
 * be representable as an unsigned char or shall equal the value of the
 * macro EOF. If the argument has any other value, the behaviour is undefined.
 */

#ifndef __ctype_h
#define __ctype_h

/* N.B. - keep in step with <ctype.c> */

#define __S 1            /* whitespace           */
#define __P 2            /* punctuation          */
#define __B 4            /* blank                */
#define __L 8            /* lower case letter    */
#define __U 16           /* upper case letter    */
#define __N 32           /* (decimal) digit      */
#define __C 64           /* control chars        */
#define __X 128          /* A-F and a-f          */
extern unsigned char __ctype[];

#define isalnum(c) (__ctype[c] & (__U+__L+__N))
extern int (isalnum)(int c);
    /* non-0 iff c is alphabetic or numeric */

#define isalpha(c) (__ctype[c] & (__U+__L))
extern int (isalpha)(int c);
    /* non-0 iff c is alphabetic */

#define iscntrl(c) (__ctype[c] & __C)
extern int (iscntrl)(int c);
    /* non-0 iff c is a control character - in the ASCII locale */
    /*       this means (c < ' ') || (c > '~')                  */

#define isdigit(c) (__ctype[c] & __N)
extern int (isdigit)(int c);
    /* non-0 iff c is a decimal digit */

#define isgraph(c) (__ctype[c] & (__L+__U+__N+__P))
extern int (isgraph)(int c);
    /* non-0 iff c is any printing character other than ' ' */

#define islower(c) (__ctype[c] & __L)
extern int (islower)(int c);
    /* non-0 iff c is a lower-case letter */

#define isprint(c) (__ctype[c] & (__L+__U+__N+__P+__B))
extern int (isprint)(int c);
    /* non-0 iff c is a printing character - in the ASCII locale */
    /*       this means 0x20 (space) -> 0x7E (tilde) */

#define ispunct(c) (__ctype[c] & __P)
extern int (ispunct)(int c);
    /* non-0 iff c is a non-space, non-alpha-numeric, printing character */

#define isspace(c) (__ctype[c] & __S)
extern int (isspace)(int c);
    /* non-0 iff c is a white-space char: ' ', '\f', '\n', '\r', '\t', '\v'. */

#define isupper(c) (__ctype[c] & __U)
extern int (isupper)(int c);
    /* non-0 iff c is an upper-case letter */

#define isxdigit(c) (__ctype[c] & (__N+__X))
extern int (isxdigit)(int c);
    /* non-0 iff c is a digit, in 'a'..'f', or in 'A'..'F' */

extern int tolower(int c);
    /* if c is an upper-case letter then return the corresponding */
    /* lower-case letter, otherwise return c.                     */

extern int toupper(int c);
    /* if c is an lower-case letter then return the corresponding */
    /* upper-case letter, otherwise return c.                     */

#endif

/* end of ctype.h */
