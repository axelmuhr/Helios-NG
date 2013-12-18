/* varargs.h: UNIX compatible variadic argument handling	*/
/*								*/
/* NOTE: This file is NOT compatible with stdarg.h		*/
/* %W% %G% (C) Copyright 1990, Perihelion Software Ltd.		*/
/* RcsId: $Id: varargs.h,v 1.2 90/09/26 18:46:49 paul Exp $ */

#ifdef __stdarg_h
# error "Cannot use BSD and ANSI variable arg handling at the same time"
#endif

#ifndef __varargs_h
#define __varargs_h

#ifdef __HELIOSARM
/* N.B. <stdio.h> is required to declare vfprintf() without defining      */
/* va_list.  Clearly the type __va_list there must keep in step.          */
typedef char *va_list[1];       /* see <stdio.h> */
#else
# ifndef __va_list_defined
typedef char *va_list[1];       /* see <stdio.h> */
# define __va_list_defined
# endif
#endif

#define va_alist __va_alist, ...

#define va_dcl long __va_alist;

#ifdef __HELIOSARM

# define __alignof(type) \
   ((char *)&(((struct{char __member1; \
                       ___type type __member2;}*) 0)->__member2) - \
    (char *)0)
#define __alignuptotype(ptr,type) \
   ((char *)((int)(ptr) + (__alignof(type)-1) & ~(__alignof(type)-1)))

#define va_arg(ap,type) \
   (___assert((___typeof(___type type) & 0x481) == 0, \
              "Illegal type used with va_arg"), \
   *(___type type *)((*(ap)=__alignuptotype(*(ap),type)+sizeof(___type type))-\
                     sizeof(___type type)))

#else /* Transputer C version */

#define va_arg(ap,type) (sizeof(type) < sizeof(int) ? \
   (type)*(int *)((*(ap)+=sizeof(int))-sizeof(int)) : \
   *(type *)((*(ap)+=sizeof(type))-sizeof(type)))

#endif /*__HELIOSARM*/

#define va_start(ap) ((void)(*(ap) = (char *)&(__va_alist)))

#define va_end(ap) ((void)(*(ap) = (char *)-256))

#endif


/* end of varargs.h */
