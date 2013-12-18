
/* stdarg.h: ANSI draft (X3J11 Oct 86) library header, section 4.8 */
/* Copyright (C) A.C. Norman and A. Mycroft */
/* version 0.01 */

#ifndef __stdarg_h
#define __stdarg_h

/* va_list is an array type so that when an object of that type is       */
/* passed as an argument it gets passed by reference.                    */

#ifndef __va_list_defined
typedef char *va_list[1];       /* see <stdio.h> */
#define __va_list_defined
#endif

#define va_start(ap,parmn) ((void)(*(ap) = (char *)&(parmn) + sizeof(parmn)))

/* care is taken in va_arg so that illegal things like va_arg(ap,char)
   which may seem natural but are illegal are patched up.  Note that
   va_arg(ap,float) is wrong but cannot be patched up at the C macro level. */
#define va_arg(ap,type) (sizeof(type) < sizeof(int) ? \
   (type)*(int *)((*(ap)+=sizeof(int))-sizeof(int)) : \
   *(type *)((*(ap)+=sizeof(type))-sizeof(type)))

/* the next macro is careful to avoid compiler warning messages */
/* and uses a -ve address to ensure address trap                */
#define va_end(ap) ((void)(*(ap) = (char *)-256))

#endif

/* end of stdarg.h */
