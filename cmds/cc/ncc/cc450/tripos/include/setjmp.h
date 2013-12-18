
/* setjmp.h: ANSI draft (X3J11 Oct 86) library header, section 4.6 */
/* Copyright (C) A.C. Norman and A. Mycroft */
/* This is the 68000 version by C.G. Selwyn */
/* version 0.02 */

/* Change Log
   v0.01 to v0.02 22nd January 1989 by S.K. Williams
      jmp_buf extended to make room for stack checking
      variables.
*/

#ifndef __setjmp_h
#define __setjmp_h

typedef int jmp_buf[16];

extern int setjmp(jmp_buf env);
extern void longjmp(jmp_buf env, int val);

#endif

/* end of setjmp.h */
