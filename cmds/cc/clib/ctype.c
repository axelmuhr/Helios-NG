/* ctype.c: ANSI draft (X3J11 Oct 86) library code, section 4.3 */
/* Copyright (C) A.C. Norman and A. Mycroft */
/* $Id: ctype.c,v 1.6 1992/09/16 11:26:47 paul Exp $ */

#include <ctype.h>
#include <stdlib.h>
#ifndef __C40
#include <stddef.h>	/* for NULL */
#endif

/* HIDDEN EXPORTs */
void _set_ctype_8859( int );

/* IL (illegal) is 0, but enhance readability.  */

#define IL 0

#ifdef POSIX

#ifdef __C40
extern unsigned char 	_ctype[ 256 ];
#else
extern unsigned char *	_ctype;
#endif

#else

/* The compiler MUST arrange that the array eof gets put into memory
   just before the array _ctype.
   4 copies for both byte sexes and to fill out a 32 bit word.
*/

static unsigned char eof[ 4 ] = { IL, IL, IL, IL };

unsigned char _ctype[ 256 ] =
  {
        __C,                    /*   nul        */
        __C,                    /*   \001       */
        __C,                    /*   \002       */
        __C,                    /*   \003       */
        
        __C,                    /*   \004       */
        __C,                    /*   \005       */
        __C,                    /*   \006       */
        __C,                    /*   bell       */
       
        __C,                    /*   backspace  */
        __C + __S,              /*   tab        */
        __C + __S,              /*   newline    */
        __C + __S,              /*   vtab       */
        __C + __S,              /*   formfeed   */
        __C + __S,              /*   return     */
        __C,                    /*   \016       */
        __C,                    /*   \017       */
        __C,                    /*   \020       */
        __C,                    /*   \021       */
        __C,                    /*   \022       */
        __C,                    /*   \023       */
        __C,                    /*   \024       */
        __C,                    /*   \025       */
        __C,                    /*   \026       */
        __C,                    /*   \027       */
        __C,                    /*   \030       */
        __C,                    /*   \031       */
        __C,                    /*   \032       */
        __C,                    /*   \033       */
        __C,                    /*   \034       */
        __C,                    /*   \035       */
        __C,                    /*   \036       */
        __C,                    /*   \037       */
        __B + __S,              /*   space      */
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
        __U + __X,              /*   A          */
        __U + __X,              /*   B          */
        __U + __X,              /*   C          */
        __U + __X,              /*   D          */
        __U + __X,              /*   E          */
        __U + __X,              /*   F          */
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
        __P,                    /*   _          */
        __P,                    /*   `          */
        __L + __X,              /*   a          */
        __L + __X,              /*   b          */
        __L + __X,              /*   c          */
        __L + __X,              /*   d          */
        __L + __X,              /*   e          */
        __L + __X,              /*   f          */
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
        __C                     /*   \177       */
/* and hence 128 more 0's (= IL's) */
};

#endif

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

/* AM: did the following function for setting IEEE-8859 ought to belong */
/* in setlocale.c?                                                      */

#ifdef __ARM /* Cannot cope with statics within functions */
  static unsigned char *	_ctype_copy = NULL;
  static unsigned char t[] = { __C, __P, __U, __L };
#endif

void
_set_ctype_8859( int yes )
{
  unsigned int j;               /* unsigned is cue to division */

#ifndef __C40
  /*
   * In order to improve efficiency, the _ctype is kept in
   * the code area of the C library.  In order to change it,
   * therefore, we must copy the table into data RAM first.
   */

#ifndef __ARM
  static unsigned char *	_ctype_copy = NULL;
#endif

  if (_ctype_copy == NULL)
    {
      int	i;

      /* remember 4 guard bytes before start of table */
      
      _ctype_copy = (unsigned char *)malloc( sizeof (_ctype) + 4 );

      if (_ctype_copy == NULL)
	return;

      for (i = -4; i < sizeof (_ctype); i++)
	{
	  _ctype_copy[ i ] = _ctype[ i ];	  
	}

      _ctype = _ctype_copy + 4;
    }    
#endif /* ! __C40 */
  
  if (yes)
    {
#ifndef __ARM
      static unsigned char t[] = { __C, __P, __U, __L };
#endif      

      for (j = 128;  j < 256; ++j)
	_ctype[ j ] = t[ (j - 128) / 32 ];
      
      _ctype[ 215 ] = __P;
      _ctype[ 247 ] = __P;
    }
  else
    {
      for (j = 128;  j < 256;  ++j)
	_ctype[ j ] = IL;
    }

  return;
  
} /* _set_ctype_8859 */

/* End of ctype.c */
