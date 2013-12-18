
/* ctype.c: ANSI draft (X3J11 Oct 86) library code, section 4.3 */
/* Copyright (C) A.C. Norman and A. Mycroft */
/* Version 2 */

#include <ctype.h>

/* IL (illegal) and CC (control char) are 0, but enhance readability.  */

#define IL 0
#define CC 0
#define _UX (__U+__X)
#define _LX (__L+__X)

/* The compiler MUST arrange that the array eof gets put into memory
   just before the array _ctype.
   4 copies for both byte sexes and to fill out a 32 bit word.
*/

static unsigned char eof[4] = { IL, IL, IL, IL };

unsigned char _ctype[256] = {
#if 'A' == 193       /* ebcdic -- this test relies on NorCroft CC */
     CC, CC, CC, CC, IL,__S, IL, CC, IL, IL, IL,__S,__S,__S, CC, CC,
     CC, CC, CC, CC, IL, IL, CC, IL, CC, CC, IL, IL, CC, CC, CC, CC,
     IL, IL, IL, IL, IL,__S, CC, CC, IL, IL, IL, IL, IL, CC, CC, CC,
     IL, IL, CC, IL, IL, IL, IL, CC, IL, IL, IL, IL, CC, CC, IL, IL,
__B+__S, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL,__P,__P,__P,__P,__P,
    __P, IL, IL, IL, IL, IL, IL, IL, IL, IL,__P,__P,__P,__P,__P,__P,
    __P,__P, IL, IL, IL, IL, IL, IL, IL, IL, IL,__P,__P,__P,__P,__P,
     IL,__P, IL, IL, IL, IL, IL, IL, IL,__P,__P,__P,__P,__P,__P,__P,
     IL,_LX,_LX,_LX,_LX,_LX,_LX,__L,__L,__L, IL, IL, IL, IL, IL, IL,
     IL,__L,__L,__L,__L,__L,__L,__L,__L,__L, IL, IL, IL, IL, IL, IL,
     IL, IL,__L,__L,__L,__L,__L,__L,__L,__L, IL, IL, IL,__P, IL, IL,
     IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL,__P, IL, IL,
    __P,_UX,_UX,_UX,_UX,_UX,_UX,__U,__U,__U, IL, IL, IL, IL, IL, IL,
    __P,__U,__U,__U,__U,__U,__U,__U,__U,__U, IL, IL, IL, IL, IL, IL,
    __P, IL,__U,__U,__U,__U,__U,__U,__U,__U, IL, IL, IL, IL, IL, IL,
    __N,__N,__N,__N,__N,__N,__N,__N,__N,__N, IL, IL, IL, IL, IL, IL
#else                /* ascii  */
        CC,                     /*   nul        */
        CC,                     /*   \001       */
        CC,                     /*   \002       */
        CC,                     /*   \003       */
        CC,                     /*   \004       */
        CC,                     /*   \005       */
        CC,                     /*   \006       */
        CC,                     /*   bell       */
        CC,                     /*   backspace  */
        CC+__S,                 /*   tab        */
        CC+__S,                 /*   newline    */
        CC+__S,                 /*   vtab       */
        CC+__S,                 /*   formfeed   */
        CC+__S,                 /*   return     */
        CC,                     /*   \016       */
        CC,                     /*   \017       */
        CC,                     /*   \020       */
        CC,                     /*   \021       */
        CC,                     /*   \022       */
        CC,                     /*   \023       */
        CC,                     /*   \024       */
        CC,                     /*   \025       */
        CC,                     /*   \026       */
        CC,                     /*   \027       */
        CC,                     /*   \030       */
        CC,                     /*   \031       */
        CC,                     /*   \032       */
        CC,                     /*   \033       */
        CC,                     /*   \034       */
        CC,                     /*   \035       */
        CC,                     /*   \036       */
        CC,                     /*   \037       */
        __B+__S,                /*   space      */
        __P,                    /*   !          */
        __P,                    /*   "          */
        __P,                    /*   #          */
        __P,                    /*   $          */
        __P,                    /*   %          */
        __P,                    /*   &          */
        __P,                    /*   '          */
        __P,                    /*   (          */
        __P,                    /*   )          */
        __P,                    /*   *          */
        __P,                    /*   +          */
        __P,                    /*   ,          */
        __P,                    /*   -          */
        __P,                    /*   .          */
        __P,                    /*   /          */
        __N,                    /*   0          */
        __N,                    /*   1          */
        __N,                    /*   2          */
        __N,                    /*   3          */
        __N,                    /*   4          */
        __N,                    /*   5          */
        __N,                    /*   6          */
        __N,                    /*   7          */
        __N,                    /*   8          */
        __N,                    /*   9          */
        __P,                    /*   :          */
        __P,                    /*   ;          */
        __P,                    /*   <          */
        __P,                    /*   =          */
        __P,                    /*   >          */
        __P,                    /*   ?          */
        __P,                    /*   @          */
        __U+__X,                /*   A          */
        __U+__X,                /*   B          */
        __U+__X,                /*   C          */
        __U+__X,                /*   D          */
        __U+__X,                /*   E          */
        __U+__X,                /*   F          */
        __U,                    /*   G          */
        __U,                    /*   H          */
        __U,                    /*   I          */
        __U,                    /*   J          */
        __U,                    /*   K          */
        __U,                    /*   L          */
        __U,                    /*   M          */
        __U,                    /*   N          */
        __U,                    /*   O          */
        __U,                    /*   P          */
        __U,                    /*   Q          */
        __U,                    /*   R          */
        __U,                    /*   S          */
        __U,                    /*   T          */
        __U,                    /*   U          */
        __U,                    /*   V          */
        __U,                    /*   W          */
        __U,                    /*   X          */
        __U,                    /*   Y          */
        __U,                    /*   Z          */
        __P,                    /*   [          */
        __P,                    /*   \          */
        __P,                    /*   ]          */
        __P,                    /*   ^          */
        __P+__C,                /*   _          */
        __P,                    /*   `          */
        __L+__X,                /*   a          */
        __L+__X,                /*   b          */
        __L+__X,                /*   c          */
        __L+__X,                /*   d          */
        __L+__X,                /*   e          */
        __L+__X,                /*   f          */
        __L,                    /*   g          */
        __L,                    /*   h          */
        __L,                    /*   i          */
        __L,                    /*   j          */
        __L,                    /*   k          */
        __L,                    /*   l          */
        __L,                    /*   m          */
        __L,                    /*   n          */
        __L,                    /*   o          */
        __L,                    /*   p          */
        __L,                    /*   q          */
        __L,                    /*   r          */
        __L,                    /*   s          */
        __L,                    /*   t          */
        __L,                    /*   u          */
        __L,                    /*   v          */
        __L,                    /*   w          */
        __L,                    /*   x          */
        __L,                    /*   y          */
        __L,                    /*   z          */
        __P,                    /*   {          */
        __P,                    /*   |          */
        __P,                    /*   }          */
        __P,                    /*   ~          */
        CC                      /*   \177       */
/* and hence 128 more 0's (= IL's) */
#endif
};

/* Certain library facilities defined by macros can be used even if the
   corresponding header file has not been included, if the macro is #undef'ed
   or if not followed by a '('.  This means that they must exist
   as ordinary functions even if they are usually expanded as macros.
   Hence the following (note that this relies that <ctype.h> defines
   all the RHS's as macros to avoid defining fns as infinite loops):
*/

int (isalnum)(int c) { return isalnum(c); }
int (isalpha)(int c) { return isalpha(c); }
int (iscntrl)(int c) { return iscntrl(c); }
int (isdigit)(int c) { return isdigit(c); }
int (isgraph)(int c) { return isgraph(c); }
int (islower)(int c) { return islower(c); }
int (isprint)(int c) { return isprint(c); }
int (ispunct)(int c) { return ispunct(c); }
int (isspace)(int c) { return isspace(c); }
int (isupper)(int c) { return isupper(c); }
int (isxdigit)(int c) { return isxdigit(c); }

int tolower(int c) { return (isupper(c) ? c + ('a' - 'A') : c); }
int toupper(int c) { return (islower(c) ? c + ('A' - 'a') : c); }

/* End of ctype.c */
