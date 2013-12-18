/* $Header$ */
/* $Source$ */
/************************************************************************/ 
/* hercules.h - Definitions for Hercules internal registers		*/
/*									*/
/* Copyright 1991 Active Book Company Ltd., Cambridge, England		*/
/*									*/
/* Author: Brian Knight, April 1991					*/
/************************************************************************/

/*
 * $Log$
 */

#ifdef __HELIOSARM

#ifndef  __HERCULES_H_
#define  __HERCULES_H_

/************************************************************************/
/* Names of registers and fields are the same as those used in the	*/
/* document "Hercules: The AB1 System Processor" q.v.			*/
/************************************************************************/

/************************************************************************/
/* Configuration and Mode Programming					*/
/************************************************************************/

#define CON_MAP		0x00400000	/* System mapping mode register	*/
#define CON_MDE		       0x1	/* Mode sub-type (0 or 1)	*/
#define CON_OSM		       0x2	/* 0: User mode, 1: OS mode	*/
#define CON_USR		       0x4	/* 0: Physical map, 1: user map */

#define CON_CLK		0x00420000	/* System configuration	reg	*/

/************************************************************************/
/* DMA									*/
/************************************************************************/

#define LCD_BAS0	0x00300000	/* LCD screen base 0		*/
#define LCD_PTR0	0x00300004	/* LCD screen pointer 0		*/
#define LCD_BAS1	0x00300008	/* LCD screen base 1		*/
#define LCD_PTR1	0x0030000C	/* LCD screen pointer 1		*/
#define DMA_CHN1_SRC0	0x00300010	/* DMA chan 1 buffer 0 source	*/
#define DMA_CHN1_DST0	0x00300014	/* DMA chan 1 buffer 0 dest	*/
#define DMA_CHN1_SRC1	0x00300018	/* DMA chan 1 buffer 1 source	*/
#define DMA_CHN1_DST1	0x0030001C	/* DMA chan 1 buffer 1 dest	*/
#define DMA_CHN2_SRC0	0x00300020	/* DMA chan 2 buffer 0 source	*/
#define DMA_CHN2_DST0	0x00300024	/* DMA chan 2 buffer 0 dest	*/
#define DMA_CHN2_SRC1	0x00300028	/* DMA chan 2 buffer 1 source	*/
#define DMA_CHN2_DST1	0x0030002C	/* DMA chan 2 buffer 1 dest	*/
#define DMA_CHN3_SRC0	0x00300030	/* DMA chan 3 buffer 0 source	*/
#define DMA_CHN3_DST0	0x00300034	/* DMA chan 3 buffer 0 dest	*/
#define DMA_CHN3_SRC1	0x00300038	/* DMA chan 3 buffer 1 source	*/
#define DMA_CHN3_DST1	0x0030003C	/* DMA chan 3 buffer 1 dest	*/

/* Fields in DMA source registers (DMA_CHNx_SRCy)			*/

#define DMA_WDT		0x00000003	/* Transfer width field		*/
#define DMA_WDT_8	0x00000001	/* 8 bits			*/
#define DMA_WDT_16	0x00000002	/* 16 bits			*/
#define DMA_WDT_32	0x00000003	/* 32 bits			*/

#define DMA_DIN		0x00000004	/* Incr enable for dest pointer	*/
#define DMA_SIN		0x00000008	/* Incr enable for source ptr	*/
#define DMA_TIN		0x00000010	/* Incr enable for xfer count	*/

#define DMA_PRA		0xFFFFFFC0	/* Physical read address ptr	*/
#define DMA_PRA_SHIFT		 6	/* Shift to reach DMA_PRA field	*/

/* Fields in DMA destination registers (DMA_CHNx_DSTy)			*/

#define DMA_TFR8	0x0000003F	/* Count for 8-bit xfer (1-64)	*/
#define DMA_ADR8	0xFFFFFFC0	/* A[25:0] for 8-bit transfer	*/
#define DMA_ADR8_SHIFT	         6	/* Shift of byte address	*/

#define DMA_TFR16	0x0000007F	/* Count for 16-bit xfer (1-128)*/
#define DMA_ADR16	0xFFFFFF80	/* A[25:1] for 16-bit transfer	*/
#define DMA_ADR16_SHIFT	         6	/* Shift of byte address	*/

#define DMA_TFR32	0x000000FF	/* Count for 32-bit xfer (1-256)*/
#define DMA_ADR32	0xFFFFFF00	/* A[25:2] for 32-bit transfer	*/
#define DMA_ADR32_SHIFT	         6	/* Shift of byte address	*/

/* DMA routing register							*/

#define DMA_CON		0x004C0000	/* DMA routing register (write)	*/

#define DMA_CHAN1_SEL	     0x007	/* Chan 1 source select field	*/
#define DMA_CHAN1_OFF	     0x000	/* Off				*/
#define DMA_CHAN1_CODRX	     0x001	/* CODEC receive		*/
#define DMA_CHAN1_MLRX	     0x002	/* Microlink receive		*/
#define DMA_CHAN1_MLTX	     0x003	/* Microlink transmit		*/
#define DMA_CHAN1_EXTA	     0x004	/* External request A		*/
#define DMA_CHAN1_EXTB	     0x005	/* External request B		*/
#define DMA_CHAN1_EXTC	     0x006	/* External request C		*/
#define DMA_CHAN1_EXTD	     0x007	/* External request D		*/

#define DMA_CHAN2_SEL	     0x038	/* Chan 2 source select field	*/
#define DMA_CHAN2_OFF	     0x000	/* Off				*/
#define DMA_CHAN2_CODTX	     0x008	/* CODEC transmit		*/
#define DMA_CHAN2_MLRX	     0x010	/* Microlink receive		*/
#define DMA_CHAN2_MLTX	     0x018	/* Microlink transmit		*/
#define DMA_CHAN2_EXTA	     0x020	/* External request A		*/
#define DMA_CHAN2_EXTB	     0x028	/* External request B		*/
#define DMA_CHAN2_EXTC	     0x030	/* External request C		*/
#define DMA_CHAN2_EXTD	     0x038	/* External request D		*/

#define DMA_CHAN3_SEL	     0x1C0	/* Chan 3 source select field	*/
#define DMA_CHAN3_OFF	     0x000	/* Off				*/
#define DMA_CHAN3_TEST	     0x040	/* DMA test			*/
#define DMA_CHAN3_MLRX	     0x080	/* Microlink receive		*/
#define DMA_CHAN3_MLTX	     0x0C0	/* Microlink transmit		*/
#define DMA_CHAN3_EXTA	     0x100	/* External request A		*/
#define DMA_CHAN3_EXTB	     0x140	/* External request B		*/
#define DMA_CHAN3_EXTC	     0x180	/* External request C		*/
#define DMA_CHAN3_EXTD	     0x1C0	/* External request D		*/

/* DMA status register							*/

#define DMA_STA		0x004C0000	/* DMA status register (read)	*/

#define DMA_CHAN1_BUF	     0x001	/* Chan 1 current buffer number	*/
#define DMA_CHAN1_INT	     0x002	/* Chan 1 empty buf int pending	*/
#define DMA_CHAN1_ERR	     0x004	/* Chan 1 overrun error		*/

#define DMA_CHAN2_BUF	     0x008	/* Chan 2 current buffer number	*/
#define DMA_CHAN2_INT	     0x010	/* Chan 2 empty buf int pending	*/
#define DMA_CHAN2_ERR	     0x020	/* Chan 2 overrun error		*/

#define DMA_CHAN3_BUF	     0x040	/* Chan 3 current buffer number	*/
#define DMA_CHAN3_INT	     0x080	/* Chan 3 empty buf int pending	*/
#define DMA_CHAN3_ERR	     0x100	/* Chan 3 overrun error		*/

#define DMA_CHAN1_TST	     0x200	/* Chan 1 test status		*/
#define DMA_CHAN2_TST	     0x400	/* Chan 2 test status		*/
#define DMA_CHAN3_TST	     0x800	/* Chan 3 test status		*/

/************************************************************************/
/* Interrupt subsystem							*/
/************************************************************************/

/* Register addresses */

#define INT_STA		0x00480000	/* Int status reg (read only)	*/
#define INT_TST		0x00480000	/* Int test reg (write only)	*/
#define FIQ_REQ		0x00480004	/* FIQ request reg (read only)	*/
#define FIQ_MSK		0x00480004	/* FIQ mask reg (write only)	*/
#define IRQ_REQ		0x00480008	/* IRQ request reg (read only)	*/
#define IRQ_MSK		0x00480008	/* IRQ mask reg (write only)	*/

/* Interrupt source bit positions */

#define INT_CRX		    0x0001	/* CODEC rx data latch full	*/
#define INT_CTX		    0x0002	/* CODEC tx data latch empty	*/
#define INT_MRX		    0x0004	/* Microlink rx data latch full	*/
#define INT_MTX		    0x0008	/* Microlink tx data latch empty*/
#define INT_EXA		    0x0010	/* External request A		*/
#define INT_EXB		    0x0020	/* External request B		*/
#define INT_EXC		    0x0040	/* External request C		*/
#define INT_EXD		    0x0080	/* External request D		*/
#define INT_TIM		    0x0100	/* Timer			*/
#define INT_LCD		    0x0200	/* LCD vertical sync		*/
#define INT_MBK		    0x0400	/* Microlink received break	*/
#define INT_DB1		    0x0800	/* DMA channel 1 buffer service	*/
#define INT_DB2		    0x1000	/* DMA channel 2 buffer service	*/
#define INT_DB3		    0x2000	/* DMA channel 3 buffer service	*/
#define INT_POR		    0x4000	/* Power On Reset flag		*/

/* Interrupt mask registers (FIQ_MSK and IRQ_MSK) */

#define FIQ_SET		    0x8000	/* 1: set given low order bits,	 */
#define IRQ_SET		    0x8000	/* 0: clear given low order bits */
					
/* Interrupt request registers (FIQ_REQ and IRQ_REQ) */

#define FIQ_ACT		    0x8000	/* Set if any FIQ source active	*/
#define IRQ_ACT		    0x8000	/* Set if any IRQ source active	*/


/************************************************************************/
/* Microlink								*/
/************************************************************************/

#define MLI_RXD		0x005C0000	/* Receive data reg (read)	*/
#define MLI_TXD		0x005C0000	/* Transmit data reg (write)	*/

/* Status register */

#define MLI_STA		0x005C0004	/* Status register (read)	*/

#define MLI_ENF		      0x01	/* Link enabled			*/
#define MLI_BIF		      0x02	/* Break interrupt pending	*/
#define MLI_RIF		      0x04	/* Receive interrupt pending	*/
#define MLI_FRE		      0x08	/* Frame error int pending	*/
#define MLI_TIF		      0x10	/* Transmit interrupt pending	*/
#define MLI_TGD		      0x20	/* Transmitting data packet	*/
#define MLI_TGA		      0x40	/* Transmitting ack packet	*/
#define MLI_TGB		      0x80	/* Transmitting break packet	*/

/* Control register */

#define MLI_CON		0x005C0004	/* Control register (write)	*/

#define MLI_ENA		      0x01	/* Enable receiver and transmitter */
#define MLI_ICP		      0x02	/* 1: internal clock, 0: ext clock */
#define MLI_TXB		      0x80	/* Send break packet. (Note that   */
					/* this bit clears automatically.) */

/************************************************************************/

#endif /* __HERCULES_H_ */
#endif /* __HELIOSARM */

/* End of hercules.h */
