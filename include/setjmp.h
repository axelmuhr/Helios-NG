/* setjmp.h: ANSI draft (X3J11 Oct 86) library header, section 4.6 */
/* Copyright (C) A.C. Norman and A. Mycroft */
/* version 0.01 - SccsId: %W% %G% */
/* $Id: setjmp.h,v 1.3 1992/03/20 10:31:35 paul Exp $ */

#ifndef __setjmp_h
#define __setjmp_h

#ifdef __C40
# include <cpustate.h>
typedef int jmp_buf[sizeof(CPURegs) / sizeof(int)];

#elif defined(__ARM)
typedef int jmp_buf[28];

#elif defined(__TRAN)
typedef int jmp_buf[2];

#else
# error processor type not defined or jmp_buf not defined for this processor
#endif

extern int setjmp(jmp_buf env);
extern void longjmp(jmp_buf env, int val);

#endif

/* end of setjmp.h */
