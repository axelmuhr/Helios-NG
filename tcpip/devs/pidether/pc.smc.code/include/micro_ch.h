/******************************************************************************
*******************************************************************************
	A George Kalwitz Production, 1990
*******************************************************************************
******************************************************************************/

/*--- 83c593 registers ---*/
#define MER	0x00		/* memory enable, reset register */
#define EEC	0x01		/* eeprom control register */
#define PID0	0x02		/* read only register with board ID */
#define PID1	0x03		/* read only register with board ID */
#define ICS	0x04		/* interrupt control and status register */
#define CCR	0x05		/* communication control register */
#define FEX	0x06		/* fifo entry-exit (16 bit)  */
#define GPR	0x07		/* general purpose register */

/*--- 83c594 registers ---*/

#define REVR	0x07		/* Revision register */

/*--- Micro channel definitions -----*/
#define	NUM_OF_CHANNELS	8		/* number of slots in machine */

#define	CPSR		0x96		/* channel position select register */
#define POS100		0x100		/* LSB ID POS register address */
#define POS101		0x101		/* MSB ID POS register address */
#define POS102		0x102		/* iobase POS register address */
#define POS103		0x103		/* RABbase POS register address */
#define POS104		0x104		/* BIOS ROM POS register address */
#define POS105		0x105		/* irq POS register address */
#define DISBIOS		0x02		/* disable BIOS ROM */
#define DISSETUP	0x00		/* disable set up,back to normal mode */
#define SETUP		0x08		/* select channel 1 */
#define IDMSB		0x6F		/* adapter id MSB */
#define IDLSB0		0xC0		/* adapter id LSB */
#define IDLSB1		0xC1		/* adapter id LSB */
#define IDLSB2		0xC2		/* adapter id LSB */
#define IDLSB3		0xC3		/* adapter id LSB */
#define IDLSB4		0xC4		/* adapter id LSB */
#define IDLSB5		0xC5		/* adapter id LSB */
#define IDLSB6		0xC6		/* adapter id LSB */

/* BISTRO ID BYTES */
#define	BISTROIDMSB	0xEF
#define	BISTROIDLSB	0xE5
#define	ALTBISTROIDLSB	0xD5

#define	CDEN		0x01
#define	PME		0x02

/* CCR definitions */
#define	EIL		0x04	/* enable interrupts */

