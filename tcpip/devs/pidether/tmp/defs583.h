/*--- 83c583 registers ---*/
#define MSR	0x00		/* memory select register */
#define ICR	0x01		/* interface configuration register */
#define IAR	0x02		/* io address register */
#define BIO	0x03		/* bios ROM address register */
#define IRR	0x04		/* interrupt request register */
#define GP1	0x05		/* general purpose register 1 */
#define IOD	0x06		/* io data latch */
#define GP2	0x07		/* general purpose register 2 */
#define LAR	0x08		/* LAN address register, init = 0x00	*/
#define LAR2	0x09		/*			 init = 0x00	*/
#define LAR3	0x0A		/*			 init = 0x0C	*/
#define LAR4	0x0B		/*			 init = ??	*/
#define LAR5	0x0C		/*			 init = ??	*/
#define LAR6	0x0D		/*			 init = ??	*/
#define LAR7	0x0E		/*			 init = 0x02	*/
#define LAR8	0x0F		/* LAN address register, init = ??	*/

/********************* Register Bit Definitions **************************/
/* MSR definitions */
#define RST	0x80		/* 1 => reset */
#define MENB	0x40		/* 1 => memory enable */
#define SA18	0x20		/* Memory enable bits	*/
#define	SA17	0x10		/*	telling where shared	*/
#define	SA16	0x08		/*	mem is to start.	*/
#define SA15	0x04		/*	Assume SA19 = 1		*/
#define SA14	0x02		/*				*/
#define	SA13	0x01		/*				*/

/* ICR definitions */
#define	STR	0x80		/* Non-volatile EEPROM store	*/
#define	RCL	0x40		/* Recall I/O Address from EEPROM */
#define	RX7	0x20		/* Recall all but I/O and LAN address */
#define RLA	0x10		/* Recall LAN Address	*/
#define	MSZ	0x08		/* Shared Memory Size	*/
#define	DMAE	0x04		/* DMA Enable	*/
#define	IOPE	0x02		/* I/O Port Enable */
#define WTS	0x01		/* Word Transfer Select */

/* IAR definitions */
#define	IA15	0x80		/* I/O Address Bits	*/
/*	.		*/
/*	.		*/
/*	.		*/
#define	IA5	0x01		/*			*/

/* BIO definitions */
#define	RS1	0x80		/* BIOS size bit 1 */
#define	RS0	0x40		/* BIOS size bit 0 */
#define	BA18	0x20		/* BIOS ROM Memory Address Bits */
#define	BA17	0x10		/*				*/
#define	BA16	0x08		/*				*/
#define	BA15	0x04		/*				*/
#define BA14	0x02		/* BIOS ROM Memory Address Bits */
#define	WINT	0x01		/* W8003 interrupt	*/

/* IRR definitions */
#define	IEN	0x80		/* Interrupt Enable	*/
#define	IRB	0x40		/* Interrupt request bit 1	*/
#define	IRA	0x20		/* Interrupt request bit 0	*/
#define	AMD	0x10		/* Alternate mode	*/
#define AINT	0x08		/* Alternate interrupt	*/
#define BW1	0x04		/* BIOS Wait State Control bit 1	*/
#define BW0	0x02		/* BIOS Wait State Control bit 0	*/
#define OWS	0x01		/* Zero Wait State Enable	*/

/* GP1 definitions */

/* IOD definitions */

/* GP2 definitions */

/* LARx definitions */

