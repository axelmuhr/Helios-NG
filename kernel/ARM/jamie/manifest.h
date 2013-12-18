/*> manifest/h <*/
/*----------------------------------------------------------------------*/
/*									*/
/*				manifest.h				*/
/*				----------				*/
/*									*/
/* Useful Executive manifests.						*/
/*									*/
/*----------------------------------------------------------------------*/
/* These are the timer tick values used when incrementing the software
 * timers. **NOTE** The software timer should NEVER be incremented in
 * steps other than "TickSize". If it is, process scheduling errors
 * will start to appear at random intervals.
 */

#define TickChunk       (1000)	     /* # of micro-seconds in a milli-second */
#define TickSize        (0x00002700) /* # of micro-seconds in a centi-second */
                            /* should = (10 * TickChunk)    gives 16us error */

/* This should give around 3centi-seconds per process slice */
#define TicksPerSlice   (0x00007500) /* (30 * TickChunk)    gives 30us error */

/* The above values should only ever be used for process timing. The
 * errors are introduced to make the numbers easier to deal with (ie.
 * 8bit shifted ARM constants). A proper Real-Time-Clock (RTC) device
 * should be used for "USER" timing (probably to the resolution of a
 * second).
 */
/*----------------------------------------------------------------------*/
/* The lo-pri priority numbers are logically numbered from 1 (since 0 is
 * system hi-priority).
 */

#define log2_numpris	(3)	/* # of bits required to describe priority */
#define NumberPris	(1 << log2_numpris)

/*----------------------------------------------------------------------*/
/* ROM FileSystem locations */

/* This information should be tied into the "hardware" information from the
 * correct header file. (TO BE DONE)
 */

#define loc_internal		(0x00)	/* "internal" system ROM */
#define loc_CARD1		(0x01)	/* External (CARD slot 1) ROM */
#define loc_internalFlash	(0xFF)	/* "internal" FlashEPROM */

#define loc_limit	(loc_CARD1)	/* maximum # of available CARD slots */

/*---------------------------------------------------------------------------*/
/* Interrupt source (vector) numbers used by kernel event handling functions */

/* FIXME : THESE SHOULD BE HELD IN THE PARTICULAR MACHINE DESCRIPTION HEADER FILE */

#define VINT_CRX (0)	/* CODEC receive data latch full */
#define VINT_CTX (1)	/* CODEC transmit data latch empty */
#define VINT_MRX (2)	/* MicroLink receive data latch full */
#define VINT_MTX (3)	/* MicroLink transmit data latch empty */
#define VINT_EXA (4)	/* External request A */
#define VINT_EXB (5)	/* External request B */
#define VINT_EXC (6)	/* External request C */
#define VINT_EXD (7)	/* External request D */
#define VINT_TIM (8)	/* Timer interrupt */
#define VINT_LCD (9)	/* LCD vertical sync. */
#define VINT_MBK (10)	/* MicroLink receive BREAK condition */
#define VINT_DB1 (11)	/* DMA channel 1 buffer service */
#define VINT_DB2 (12)	/* DMA channel 2 buffer service */
#define VINT_DB3 (13)	/* DMA channel 3 buffer service */
#define VINT_POR (14)	/* Power On Reset (write to "CLOCK_regs" clears) */

/*-----------------------------------------------------------------------*/
/* User event vectors. Number of vectors (UserVectors) defined in root.h */

/* FIXME : THESE SHOULD BE HELD IN THE PARTICULAR MACHINE DESCRIPTION HEADER FILE */

#define UVec_Power	(0)	/* PowerManagement events */
#define UVec_MemLow	(1)	/* Memory events */
#define UVec_Card	(2)	/* CARD events */

/*---------------------------------------------------------------------------*/
/* Do NOT alter these values if at all possible. They are used explicitly in
 * the definition of the root structure. If they are changed every program
 * that references the root structure will NEED to be re-built.
 */

/* FIXME : THESE SHOULD BE HELD IN THE PARTICULAR MACHINE DESCRIPTION HEADER FILE */

#define InterruptVectors	(VINT_POR + 8)
#define UserVectors		(UVec_Card + 8)

/* We currently allocate 8 extension slots in each vector chain. This is
 * in-case we need to add new vector users, without having to re-compile
 * every root structure user.
 */

/*----------------------------------------------------------------------*/
/* User event handler arguments */

/* UVec_Power: */
#define	Power_Fail	(1)	/* =EMERGENCY= the power has run out	*/
#define	Power_Down	(2)	/* Processor is about to go to sleep	*/
                                /* user requested, or via timeout	*/
#define Power_Back	(3)	/* re-awake from quiescient state	*/

/* UVec_MemLow: */
#define MemLow_Cache	(1)	/* Throw away cache memory	*/
#define MemLow_Low	(2)	/* Memory is very low		*/
#define MemLow_Out	(3)	/* Memory space is exausted	*/

/* UVec_Card: */
#define Card_Insert	(1)	/* A card has been inserted	*/
#define Card_Extract	(2)	/* A card has been extracted	*/

/*----------------------------------------------------------------------*/
/*> EOF manifest.h <*/
