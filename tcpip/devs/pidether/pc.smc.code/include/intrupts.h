/******************************************************************************
*******************************************************************************
	A George Kalwitz Production, 1990
*******************************************************************************
******************************************************************************/

/****** Interrupt Controller Registers ******/

#define MR8259A		0x21		/* interrupt controller 1 */
#define IR8259A		0x20
#define MR8259B		0xA1		/* interrupt controller 2 */
#define IR8259B		0xA0

/****** 8259 REGISTER DEFINITIONS ******/
#define	EOI		0x20		/* end of interrupt */
#define	RIRR		0x0a		/* read interrupt request register */

