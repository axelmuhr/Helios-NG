/* 
  Modified from 
   Revision 1.2.1.2  90/09/17  15:57:13  meissner

  Chris Hadley 1.11.90
*/

#ifndef _STDDEF_H
#define _STDDEF_H

#ifndef _PTRDIFF_T
#define _PTRDIFF_T
/* Signed type of difference of two pointers.  */
typedef long int ptrdiff_t;
#endif

/* Unsigned type of `sizeof' something.  */

#ifndef _SIZE_T		/* in case <sys/types.h> has defined it. */
#ifndef _SIZE_T_	/* Ultrix 4.00 */
#define _SIZE_T
#define _SIZE_T_
typedef unsigned size_t;
#endif /* _SIZE_T_ */
#endif /* _SIZE_T */

#ifndef _WCHAR_T
#define _WCHAR_T
typedef int wchar_t;
#endif

/* A null pointer constant.  */
#ifdef NULL
#undef NULL		/* in case <stdio.h> has defined it. */
#endif
#define NULL ((void *)0)

/* Offset of member MEMBER in a struct of type TYPE.  */
#define offsetof(TYPE, MEMBER) ((size_t) &(((TYPE *)0)->MEMBER))

#endif /* _STDDEF_H */
