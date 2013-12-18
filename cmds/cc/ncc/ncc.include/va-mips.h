/* ---------------------------------------- */
/*           VARARGS  for MIPS/GNU CC       */
/*                                          */
/*                                          */
/*                                          */
/*                                          */
/* ---------------------------------------- */

/*
 * HISTORY
 * $Log:	va-mips.h,v $
 * Revision 1.3.2.2  90/09/17  15:57:19  meissner
 * 	changes to build on Ultrix 4.0
 * 	[90/09/17  15:52:54  meissner]
 * 
 * Revision 1.3  90/09/13  12:57:57  devrcs
 * 	Remove lock
 * 	[90/03/28  17:26:58  meissner]
 * 
 * 	remove lock
 * 	[90/03/15  16:50:44  meissner]
 * 
 * 	Use same AP format as MIPS uses and stdarg.h uses
 * 	[90/02/21  13:05:50  meissner]
 * 
 * Revision 1.2  90/03/27  21:36:18  gm
 * 	whatever insulting thing
 * 
 */

/* These macros implement traditional (non-ANSI) varargs
   for GNU C.  */

#define va_alist  __builtin_va_alist
#define va_dcl    int __builtin_va_alist;

#ifndef _VA_LIST_
#define _VA_LIST_
#define va_list   char *
#endif

#define va_start(AP)  AP = (char *) &__builtin_va_alist
#define va_end(AP)

#ifdef lint	/* complains about constant in conditional context */
#define va_arg(list, mode) ((mode *)(list += sizeof(mode)))[-1]

#else		/* !lint */
#define va_arg(AP, mode) ((mode *)(AP = \
	(char *) (sizeof(mode) > 4 ? ((int)AP + 2*8 - 1) & -8 \
				   : ((int)AP + 2*4 - 1) & -4)))[-1]
#endif		/* lint */
