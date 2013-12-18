#ifndef __GNUC__
/* Use the system's macros with the system's compiler.  */
#include <varargs.h>
#else
#ifdef __spur__
#include "va-spur.h"
#else
#ifdef __mips__
#include "va-mips.h"
#else

/* These macros implement traditional (non-ANSI) varargs
   for GNU C.  */

#define va_alist  __builtin_va_alist
#define va_dcl    int __builtin_va_alist;
#define va_list   char *

#ifdef __sparc__
#define va_start(AP) 						\
 (__builtin_saveregs (),					\
  AP = ((void *) &__builtin_va_alist))
#else
#define va_start(AP)  AP=(char *) &__builtin_va_alist
#endif
#define va_end(AP)

#define __va_rounded_size(TYPE)  \
  (((sizeof (TYPE) + sizeof (int) - 1) / sizeof (int)) * sizeof (int))

#define va_arg(AP, TYPE)						\
 (AP += __va_rounded_size (TYPE),					\
  *((TYPE *) (AP - __va_rounded_size (TYPE))))

#endif /* not mips */
#endif /* not spur */
#endif /* __GNUC__ */
