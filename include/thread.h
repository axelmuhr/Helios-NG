/* thread.h:	Stand-Alone C header			*/
/* $Id: thread.h,v 1.2 1992/10/16 13:49:55 nick Exp $ */

#include <helios.h>

extern void thread_create(void *stack, word pri, VoidFnPtr fn, word nargs, ... );

extern void thread_stop(void);


/* These pragmas are VERY IMPORTANT, they switch off the vector stack	*/
/* and stack checking.							*/
/* This is necessary if using thread_create() since it is very low	*/
/* level and does not build a full C environment. Use Fork() get a	*/
/* full C environment where the vector stack and stack checking work.	*/

#pragma -f0		/* switch off vector stack			*/
#pragma -s1		/* switch off stack checking			*/
