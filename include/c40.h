/*
 * File:	c40.h
 * Subsystem:	Helios/C40 implementation
 * Author:	P.A.Beskeen
 * Date:	Nov '91
 *
 * Description: 'C40 specific Helios manifests
 *
 *
 * RcsId: $Id: c40.h,v 1.27 1993/08/06 15:01:03 nickc Exp $
 *
 * (C) Copyright 1991, 1992 Perihelion Software Ltd.
 *    All RIghts Reserved.
 * 
 */


#ifndef __c40_h
#define __c40_h

#include <helios.h>

/* Define 'C40 interrupt manifests */
/* NMI, IIOF0-3 and TINIT1 can be used */
#define	InterruptVectors	6

/* define 'vector' number passed via SetEvent */
#define INTR_NMI		0
#define INTR_IIOF0		1
#define INTR_IIOF1		2
#define INTR_IIOF2		3
#define INTR_IIOF3		4
#define INTR_TINT1		5


/* 'C40 Procedure Call Standard (PCS) manifests */

/* The number of arguments passed in registers as defined by the 'C40 PCS */
#define PCS_ARGREGS	4

/* The number of bytes held as a stack overflow area. The true end of the */
/* stack is pointed to by the user stack end register + PCSSTACKGUARD     */
#define PCS_STACKGUARD	(96 * sizeof(word))
/* It has been experimentally determined that 64 (the old value) is
 * insufficient, since some kernel routines (which do not stack check) need
 * more than 64 words of stack.  TaskInit() is an example.
 */

/* Notes what type of user stack is used by the PCS */
#define PCS_FULLDECENDING


/* 'C40 specific structures */

#ifdef ALLOCDMA

/* DMA request structure, held on Root.DMAReqQ */

typedef struct DMAReq
  {
    struct DMAReq *	next;
    word		endtime;
    SaveState *		state;
    word		link;		/* -1 if not a link DMA request */
    word		flags;		/* DMA_Tx / DMA_Rx denotes split mode */
    word		rc;
  }
DMAReq;

#define DMA_Tx	1		/* Requesting primary split mode channel */
#define DMA_Rx	2		/* Requesting auxillary split mode channel */

/* DMA Allocation structure, held on Root.DMAFreeQ */

typedef struct DMAFree
  {
    struct DMAFree *	next;
    word		DMAEng;
  }
DMAFree;

#endif /* ALLOCDMA */


/* This structure defines the contents of the TIM-40 IDROM. The IDROM */
/* characterises the C40 system Helios is running on. If the board has */
/* no built-in IDROM a pseudo one is constructed and sent by the I/O Server. */
/* For more information see the TIM-40 specification */

typedef struct IDROM
  {
    word	SIZE;		/* self inclusive size of this block */

    short	MAN_ID;		/* TIM-40 module manufacturers ID */
    byte	CPU_ID;		/* CPU type (00 = C40) */
    byte	CPU_CLK;	/* CPU cycle time (60ns = 59) */

    short	MODEL_NO;	/* TIM-40 module model number */
    byte	REV_LVL;	/* module revision level */
    byte	RESERVED;	/* currently unused (align to word boundary) */

    word	GBASE0;		/* address base of global bus strobe 0 */
    word	GBASE1;		/* address base of global bus strobe 1 */
    word	LBASE0;		/* address base of local bus strobe 0 */
    word	LBASE1;		/* address base of local bus strobe 1 */

				/* sizes are in words */
    word	GSIZE0;		/* size of memory on global bus strobe 0 */
    word	GSIZE1;		/* size of memory on global bus strobe 1 */
    word	LSIZE0;		/* size of memory on local bus strobe 0 */
    word	LSIZE1;		/* size of memory on local bus strobe a */

    word	FSIZE;		/* size of fast ram pool (inc. on-chip RAM) */

	/* Each of the following bytes contains two nibbles, one for */
	/* strobe 0 and one for strobe 1. The nibbles define how many cycles */
	/* it takes to read a word from that strobes associated memory. */
    byte	WAIT_G;		/* within page on global bus */
    byte	WAIT_L;		/* within page on local bus */
    byte	PWAIT_G;	/* outside page on global bus */
    byte	PWAIT_L;	/* outside page on local bus */

    word	TIMER0_PERIOD;	/* period time for 1ms interval on timer 0 */
    word	TIMER1_PERIOD;	/* period for DRAM refresh timer (optional) */
    short	TIMER0_CTRL;	/* contents set TCLK0 to access RAM not IDROM */
    short	TIMER1_CTRL;	/* sets up timer to refresh DRAM (optional) */

    word	GBCR;		/* global bus control register */
    word	LBCR;		/* local bus control register */

    word	AINIT_SIZE;	/* total size of auto-initialisation data */
  }
IDROM;

/* Initialisation Block conatins size, address to load to and data to load. */
/* B_SIZE of zero ends the IDROM load sequence.				    */

typedef struct IDROMBLOCK
  {
    word	B_SIZE;				/* size of data block */
    word	B_ADDR;				/* address to data @ */
    word	B_DATASTART[ 1 ];		/* data block */
  }
IDROMBLOCK;


typedef struct stack_chunk
  {
    struct stack_chunk * 	next;			/* link to next stack chunk	 	   */
    struct stack_chunk *	prev;			/* link to previous stack chunk		   */    
    unsigned long int		link_register;		/* return address before chunk was made    */
    unsigned long int		stack_end_pointer;	/* end of previous stack chunk		   */
    unsigned long int		stack_pointer;		/* stack pointer inside previous chunk     */
    unsigned long int		size;			/* word size of stack chunk without header */
  }
stack_chunk;


/* C40 specific functions */

/* Convert a Helios C byte address pointer to a 'C40 CPU word pointer. */
/* Note that this may loose some byte selection information. */
/* Same as _DataToFuncConvert(), but returns word address as a word type. */

word C40WordAddress(void *Cptr);


/* Convert a 'C40 CPU word pointer to a Helios C byte address pointer. */
/* Returns 0 if address cannot be converted. */
/* Same as _FuncToDataConvert(), but can be passed any pointer type. */

void *C40CAddress(MPtr Wordptr);


IDROM *		GetIDROM( void );	/* Return pointer to IDROM struct. */


/* C40 specific debug support */
int		_backtrace( char * name, int frame );
					/* Puts name of function whose frame */
					/* pointer is 'frame' into 'name',   */
					/* and returns the frame pointer of  */
					/* the parent of that function.      */

int		_stack_size( void );	/* Returns number of words remaining */
					/* in the current stack chunk.       */

void		JTAGHalt( void );	/* Causes an emulator halt. */


/* inline assembler support */
int		_word( word opcode );	/* Inserts an opcode into the code. */

#ifndef NOP
#define NOP()	_word( 0x0c800000 )	/* For example: a NOP instruction. */
#endif

/* Support for copying between C byte addresses and C40 word addresses */

/* Get word of data from a word address. */
word MP_GetWord(MPtr addr, word wordoffset);

/* Put a word of data to a word address. */
void MP_PutWord(MPtr addr, word wordoffset, word data);

/* Copy data from a word address to a byte address. */
void MP_GetData(void *dstbytepointer, MPtr addr, word wordoffset,
	word numwords);

/* Copy data from a byte address to a word address. */
void MP_PutData(MPtr addr, word wordoffset, void *srcbytepointer,
	word wordcount);


/* Shared memory interlocking support */

/* Lock shared memory and read an integer. */
word MP_ReadLock(MPtr lockaddr);

/* Lock shared memory and read a single precision FP number. */
float MP_ReadFPLock(MPtr lockaddr);

/* Write an integer and unlock shared memory. */
void MP_WriteUnlock(MPtr lockaddr, word data);

/* Write a single precison FP number and unlock shared memory. */
void MP_WriteFPUnlock(MPtr lockaddr, float data);

/* Signal a word addressed counting semaphore (spin lock) in shared memory. */
word MP_Signal(MPtr countaddr);

/* Wait on a word addressed counting semaphore (spin lock) in shared memory. */
word MP_BusyWait(MPtr countaddr);


#ifdef ALLOCDMA
word   		AllocDMA( word timeout );
void		FreeDMA(  word DMAEng );
#endif


#endif /* end of __C40_h */
