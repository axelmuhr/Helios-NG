/* $Header: cppFix.h,v 1.1 90/01/13 20:14:59 charles Locked $ */
/* $Source: /server/usr/users/charles/world/drawp/RCS/ext/cppFix.h,v $ */
/* $State: Release_0_1_1_EXPERIMENTAL $*/

/*-----------------------------------------------------------------------*/
/*                                                             cppFix.h  */
/*-----------------------------------------------------------------------*/

/* This is a (temporary,) file to fix the bug in the ANSI headers  */
/*   (stdio.h,) whereby it assumes that in #if's the precompiler   */
/*   takes macros to be 0 if undefined. This causes annoying       */
/*   warnings                                                      */


#ifndef _cppFix_h
#define _cppFix_h
#define vax   0
#define u3b   0
#define u3b2  0
#define u3b5  0
#define mc68k 0
#endif
