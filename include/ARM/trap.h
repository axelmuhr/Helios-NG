/*
 * File:	trap.h
 * Subsystem:	Helios-ARM header files.
 * Author:	P.A.Beskeen
 * Date:	Oct '92
 *
 * Description: Helios-ARM traps (swi's). These are encoded as inline opcodes
 *		callable from C.
 *
 *		Include this file in preference to trapdefs.h.
 *
 *		*WARNING* must be kept in step with "../ampp/ARM/trap.m"
 *
 *
 * RcsId: $Id: trap.h,v 1.2 1993/08/08 19:37:41 paul Exp $
 *
 * (C) Copyright 1992 Perihelion Software Ltd.
 *     All Rights Reserved.
 * 
 */


#ifndef __trap_h
#define __trap_h


/* Define SWI fields */
#define SWI_OP		0xef000000	/* swi instruction */

#define SWI_OS_RISCOS	0x00000000	/* RISCOS is OS 0 */
#define SWI_OS_RISCIX	0x00100000	/* RISCiX is OS 1 */
#define SWI_OS_ARX	0x00200000	/* ARX is OS 2    */
#define SWI_OS_HELIOS	0x00300000	/* Helios is OS 3 */

#define SWI_OS_MASK	0x00f00000	/* Mask for OS bits */
#define SWI_TRAPMASK	0x000000ff	/* Helios system call mask <= 255 fns */
#define SWI_UNUSEDMASK	0x000fff00	/* @@@ Maybe use later for flags */


/* Define std bits to use whenever calling Helios traps */
#define TRAP_STDHELIOS	SWI_OS_HELIOS


#ifdef WHEN_WORD_IS_WORKING_IN_CCARM
/* Use _Trap(TRAP_xxx) from C to call an appropriate kernel trap function */
# define _Trap(x)	_word(SWI_OP | (x));
/* Pseudo trap is only callable from non User mode. */
# define _PseudoTrap(x) \
	/* @@@ need to insert correct assembler */
	_word(0 | (trapnum & ~TRAP_STDHELIOS), 	/* mov	tmp, 0 */ \
	_word(0),				/* mov	lr, pc */ \
	_word(0)				/* ldr	pc, (r0, -r0) */
#else
/* @@@ While _word()/_trap() aren't working in the C compiler, cause an error */
# include <assert.h>
# define _Trap(x)	assert(0)
# define _PseudoTrap(x) assert(0)
#endif


#include <ARM/trapdefs.h> /* include automatically produced trap number defs */


#endif /*__swi_h*/


/* end of trap.h */
