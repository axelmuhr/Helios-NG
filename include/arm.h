/*
 * File:	arm.h
 * Subsystem:	Helios/ARM implementation
 * Author:	P.A.Beskeen
 * Date:	Sept '92
 *
 * Description: ARM specific Helios manifests
 *
 *
 * RcsId: $Id: arm.h,v 1.3 1993/07/27 13:59:18 paul Exp $
 *
 * (C) Copyright 1991, 1992 Perihelion Software Ltd.
 *     All RIghts Reserved.
 * 
 */


#ifndef __arm_h
#define __arm_h

#include <helios.h>


#if defined(__ARCHIMEDES) && !defined(__IOC)
/* ARM IOC chip used by Archimedes. */
# define __IOC
#endif


/* Interrupt vectors
 *
 * To make platform portability easier, up to 64 possible indirect interrupt
 * vectors are defined. How these are mapped to actual devices is
 * platform dependent. @@@ This may be optimised in future.
 *
 * Number of interrupt vectors
 */
# define InterruptVectors	64


/* __ARCHIMEDES
 * The system interrupt handler on Archimedes based systems decodes IRQ
 * and FIQ interrupts into their IOC and expansion card sources.
 * 30 = (2 (Unknown IRQ/FIQ) + 24 (IOC) + 4 (XCB))
 *
 * __IOC
 * IOC based systems decode IRQ and FIQ interrupts into their
 * IOC defined sources.
 * 26 = (2 (Unknown IRQ/FIQ) + 24 (IOC))
 *
 * __VY86PID
 * VLSI PID board has eight IRQ sources and eight FIQ sources
 * 18 = (2 (Unknown IRQ/FIQ) + 8 (FIQ) + 8 (IRQ))
 */


/* ARM interrupt vector numbers.
 *
 * These define in a platform specific fashion which interrupt vectors
 * correspond to which interrupt sources.
 *
 * On most systems the first two vectors are used to note unknown FIQ/IRQ
 * interrupt sources. On systems with no decoding, they simply correspond to
 * FIQ and IRQ.
 */
#define INTR_FIQ		0
#define INTR_IRQ		1

#if defined(__IOC)
/* Pseudo interrupt numbers for IOC based systems such as the Archimedes.   */
/* These numbers correspond to the bits held in the IOC request registers.  */
/*									    */
/* If special on Archimedes H/W:   Early Models		82C710/711 Models   */
/*				   ------------		-----------------   */
/* IRQA request register */
				/* printer busy		printer interrupt   */
#define INTR_IRQA_0	2	/* IOC: IL6				    */
				/* serial port ring	low battery warning */
#define INTR_IRQA_1	3	/* IOC: IL7				    */
				/* printer ack		floppy disk index   */
#define INTR_IRQA_2	4	/* IOC: IF (high to low edge on IF)	    */
				/* 		Vsync pulse		    */
#define INTR_IRQA_3	5	/* IOC: IR (low to high edge on IR)	    */
#define INTR_IRQA_4	6	/* IOC: power on reset			    */
#define INTR_IRQA_5	7	/* IOC: Timer 0				    */
#define INTR_IRQA_6	8	/* IOC: Timer 1				    */
				/*		FIQ downgrade		    */
#define INTR_IRQA_7	9	/* IOC: force interrupt			    */

/* IRQB request register */
				/*	expansion card FIQ downgrade	    */
#define INTR_IRQB_0	10	/* IOC: IL0				    */
				/* 	sound system buffer change	    */
#define INTR_IRQB_1	11	/* IOC: IL1				    */
				/* serial port ctrler	serial port 82C711  */
#define INTR_IRQB_2	12	/* IOC: IL2				    */
				/* hard disk ctrler	IDE hard disk	    */
#define INTR_IRQB_3	13	/* IOC: IL3				    */
				/* floppy disk change	floppy 82C711 intr. */
#define INTR_IRQB_4	14	/* IOC: IL4				    */
				/*  unknown expansion card interrupt source */
#define INTR_IRQB_5	15	/* IOC: IL5				    */
#define INTR_IRQB_6	16	/* IOC: KART Tx ready			    */
#define INTR_IRQB_7	17	/* IOC: KART Rx ready			    */


/* FIRQ request register */
#define INTR_FIQ_0	18	/* IOC: FH0				    */
#define INTR_FIQ_1	19	/* IOC: FH1				    */
#define INTR_FIQ_2	20	/* IOC: FL				    */
#define INTR_FIQ_3	21	/* IOC: C3				    */
#define INTR_FIQ_4	22	/* IOC: C4				    */
#define INTR_FIQ_5	23	/* IOC: C5				    */
#define INTR_FIQ_6	24	/* IOC: IL0				    */
#define INTR_FIQ_7	25	/* IOC: force interrupt			    */

#if defined(__ARCHIMEDES)
/* Additional pseudo vectors for IOC based Archimedes. */
/* Decodes of the Acorn Expansion Card Bus (xcb) interrupts.		    */
# define INTR_XCB_0	26	/* IOC: IL4 - XCB slot 0		    */
# define INTR_XCB_1	27	/* IOC: IL4 - XCB slot 1		    */
# define INTR_XCB_2	28	/* IOC: IL4 - XCB slot 2		    */
# define INTR_XCB_3	29	/* IOC: IL4 - XCB slot 3		    */
#endif

#elif defined(__VY86PID)
/* Pseudo interrupt vector number for the VLSI PID board.		*/
#define INTR_IRQ_SERIAL	2	/* Serial port (intercepted by Kernel)	*/ 
#define INTR_IRQ_TIMER	3	/* Timer (intercepted by Kernel)	*/
#define INTR_IRQ_PARA	4	/* Parallel port 			*/
#define INTR_IRQ_3	5	/* Expansion slot IRQ 3 		*/
#define INTR_IRQ_4	6	/* Expansion slot IRQ 4 		*/
#define INTR_IRQ_5	7	/* Expansion slot IRQ 5 		*/
#define INTR_IRQ_6	8	/* Expansion slot IRQ 6 		*/
#define INTR_IRQ_PANIC	9	/* PANIC button */

#endif /* __IOC/__VY86PID */

/* ARM Procedure Call Standard (PCS) manifests */

/* The number of arguments passed in registers as defined by the 'C40 PCS */

#define PCS_ARGREGS	4


/* The number of bytes held as a stack overflow area. The true end of the
 * stack is pointed to by the user stack end register + PCSSTACKGUARD
 */
#define PCS_STACKGUARD	(64 * sizeof(word))


/* Notes what type of user stack is used by the PCS */

#define PCS_FULLDECENDING



/* ARM specific structures: */




/* ARM specific macros: */

#ifdef __ARM6
# error "No ARM6 definitions"
#else

/* Processor status register mode bits. */

# define	NFlag		(1U << 31)	/* miNus */
# define	ZFlag		(1 << 30)	/* Zero */
# define	CFlag		(1 << 29)	/* Carry */
# define	VFlag		(1 << 28)	/* oVerflow */
# define	IRQDisable	(1 << 27)	/* processor IRQ disable */
# define	FIQDisable	(1 << 26)	/* processor FIQ disable */

# define	SVCMode		0x3
# define	UserMode	0x0
# define	FIQMode		0x1
# define	IRQMode		0x2
# define	ModeMask	0x3

# define	INTRMask	(IRQDisable | FIQDisable)
# define	PSRFlags	(NFlag | ZFlag | CFlag | VFlag | INTRMask | ModeMask)

/* Extract and update items in the ARM2/3 processors status register. */
# define	GET_ARM23PSR_PC(x)	((x)  & 0x03fffffc)
# define	SET_ARM23PSR_PC(psr, addr) \
				(((psr) & 0xfc000003) | ((addr) & 0x03fffffc))
# define	GET_ARM23PSR_MODE(psr)	((psr) & 3)

/* PSR flags */
# define	PC_M0	0x00000001
# define	PC_M1	0x00000002
# define	PC_M	(PC_M0|PC_M1)
# define	PC_N	0x80000000
# define	PC_Z	0x40000000
# define	PC_C	0x20000000
# define	PC_V	0x10000000
# define	PC_I	0x08000000
# define	PC_F	0x04000000
# define	PC_INT	(PC_I|PC_F)
#endif



/* ARM specific miscellaneous functions. */

/* Alter processor mode. */

void EnterSVCMode(void);
void EnterUserMode(void);

/* Call pseudo trap handler from interrupt handler || !UserMode */
word PseudoTrap(word, word, word, word trapnum);

/* Debug fns (also available on C40): */

/* Print stack backtrace. */

void backtrace( void );	


/* Puts name of function whose frame pointer is 'frame' into 'name', 
 * and returns the frame pointer of the parent of that function.
 */
int _backtrace( char * name, int frame );


/* Inline assembler support: _word() inserts the given opcode into
 * the code generated by the C compiler.
 */
/* int _word( word opcode );	 */


#if 0 /* @@@ NOT PLATFORM PORTABLE - REMOVE */

/* @@@ TMP DBG support */
# define _DBG(x) { volatile char *la = (char *)0x3342000; \
		while((*(la+0xc) & 1) == 0) ; \
		*(la + 4) = 0x80 | (x & 0xff); \
		}

# define _DBG2(x) { volatile char *la = (char *)0x3342000; \
		while((*(la+0xc) & 1) == 0) ; \
		*(la + 4) = x & 0xff; \
		}

# define _DBGB(y) { \
		int i, j = y;			\
			for (i=0 ;i < 8;i++) {	\
				_DBG(j & 0xf);	\
				j >>= 4;	\
			}			\
		}
#else
# define _DBG(x)
# define _DBG2(x)
# define _DBGB(y) 
#endif

#endif /* __arm_h */

/* end of arm.h */
