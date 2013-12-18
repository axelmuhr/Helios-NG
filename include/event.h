/*------------------------------------------------------------------------
--                                                                      --
--                     H E L I O S   K E R N E L                        --
--                     -------------------------                        --
--                                                                      --
--             Copyright (C) 1987, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- event.h								--
--                                                                      --
--	Event channel support.						--
--                                                                      --
--	Author:  NHG 8/8/88						--
--                                                                      --
------------------------------------------------------------------------*/
/* SccsId: %W%	%G% Copyright (C) 1987, Perihelion Software Ltd.	*/
/* $Id: event.h,v 1.11 1993/07/27 13:59:18 paul Exp $ */

#ifndef __event_h
#define __event_h

#ifndef __helios_h
# include <helios.h>
#endif
#include <queue.h>

#ifdef __C40
# include <c40.h>
#endif
#ifdef __ARM
# include <ARM/trap.h>
#endif

typedef struct Event {
	struct Node	Node;		/* link in event chain		*/
#ifdef __TRAN
	word		Pri;		/* priority in chain		*/
	VoidFnPtr	Code;		/* event routine		*/
#else
	word		Vector;		/* interrupt vector		*/
	word		Pri;		/* priority in chain		*/
	WordFnPtr	Code;		/* event routine		*/
# ifdef __C40
	word		AddrBase;	/* C data address base		*/
# endif
#endif
	void		*Data;		/* data for this		*/
	word		*ModTab;	/* set by kernel		*/
} Event;


/* Kernel support routines */

word SetEvent(Event *event);
word RemEvent(Event *event);

#ifdef __ABC
 /* user event mechanism prototypes */
 word SetUserEvent(Event *event);
 word RemUserEvent(Event *event);
 void CauseUserEvent(word vector, word arg);
#endif

#ifndef __TRAN
 /* disable interrupts for the duration of this fn call */
 word AvoidEvents(WordFnPtr fn, ...);
#endif

#ifdef __C40
/* direct mainipulation of status and IIE registers via embedded machine code */
# define IntsOn()	_word(0x10752000)	/* or	ST_GIE, st	*/
# define IntsOff()	_word(0x03752000)	/* andn	ST_GIE, st	*/

/* System() should be used in preference to these functions */
# define ClockIntsOn()	_word(0x10770001)	/* or	IIE_ETINT0, st	*/
# define ClockIntsOff() _word(0x03770001)	/* andn IIE_ETINT0, st	*/

/* more optimal version of AvoidEvents() (fixed 3 parameters) */
# define _AvoidEvents(aefn, a,b,c)	IntsOff(); \
					((WordFnPtr)aefn)(a,b,c); \
					IntsOn()
#endif

#ifdef __ARM
# ifdef in_kernel
  /* These functions must *NOT* be used in non user modes */
  /* (+ it would be unecessary and inefficent) */

  /* User mode callable processor interrupt mask update */
  void IntsOn(void);
  void IntsOff(void);

  void ClockIntsOn(void);
  void ClockIntsOff(void);

  void EnableIRQ(void);
  void DisableIRQ(void);

  void EnableFIQ(void);
  void DisableFIQ(void);
# else
#  if 0	/* @@@ disabled until _word()/_trap() implemented */
#  define IntsOn()		_Trap(SWI_IntsOn)
#  define IntsOff()		_Trap(SWI_IntsOff)

#  define EnableIRQ(void)	_Trap(TRAP_EnableIRQ)
#  define DisableIRQ()		_Trap(TRAP_DisableIRQ)

#  define EnableFIQ()		_Trap(TRAP_EnableFIQ)
#  define DisableFIQ()		_Trap(TRAP_DisableFIQ)

   /* Turn the timeslicer/Helios clock interrupt off temporarily */
#  define ClockIntsOn()	_Trap(SWI_ClockIntsOn)
#  define ClockIntsOff()	_Trap(TRAP_ClockIntsOff)
#  endif /* 0 */
# endif /* in_kernel */
#endif /* __ARM */


#endif


/* -- End of event.h */
