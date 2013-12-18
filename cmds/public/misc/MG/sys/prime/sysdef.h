/*
 * Prime specific definitions for MicroGnuEmacs 2a
 */
#include <stdio.h>

#define	       PCC		       /* "[]" gets an error.		      */
#define	       KBLOCK  1024	       /* Kill grow.			      */
#define	       GOOD    0	       /* Good exit status.		      */
#define	       NO_RESIZE	       /* screen size is constant	      */
#define	       MAXPATH 256

/* typedefs for gnu version */
typedef int    RSIZE;	       /* Type for file/region sizes   */
typedef short  KCHAR;	       /* Type for internal keystrokes */

/*
 * Macros used by the buffer name making code.
 * Start at the end of the file name, scan to the left
 * until BDC1 (or BDC2, if defined) is reached. The buffer
 * name starts just to the right of that location, and
 * stops at end of string (or at the next BDC3 character,
 * if defined). BDC2 and BDC3 are mainly for VMS.
 */
#define	       BDC1    '>'

#define bcopy(from,to,len)	  if(1) {\
    register char *from_=from, *to_=to; register int len_=len;\
    while(len_--) *to_++ = *from_++;} else

#define MALLOCROUND(m) (m+=7, m&=~7)   /* round to 8 byte boundary */

char *gettermtype();	/* #define fails because of static storage */

#define unlink(f)    delete(f)
#define unlinkdir(f) delete(f)

#ifdef DO_METAKEY
#define METABIT 0400
#endif
