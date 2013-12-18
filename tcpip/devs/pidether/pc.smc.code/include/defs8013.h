/******************************************************************************
*******************************************************************************
	A George Kalwitz Production, 1989
*******************************************************************************
******************************************************************************/

/*****************************************************************************

this module contains definitions specific to the WD8013 family of cards

******************************************************************************/

/**** 83c583 registers ****/
#define	BSR		0x01		/* Bus Size Register (read only) */
#define	LAAR		0x05		/* LA Address Register (write only */

/**** BSR Definitions ****/
#define	BUS16BIT	0x01		/* Bit 0 tells if the bus is 16 bit */

/**** LAAR Definitions ****/
#define	MEM16ENB	0x80		/* Enables 16bit shrd RAM for host */
#define	LAN16ENB	0x40		/* Enables 16bit shrd RAM for LAN */
#define	LA23		0x10		/* Address lines for enabling */
#define	LA22		0x08		/*    shared RAM above 1Mbyte */
#define	LA21		0x04		/*    in host memory */
#define	LA20		0x02
#define	LA19		0x01

/**** General Definitions ****/
#define	LAAR_MASK	0x21
#define	INIT_LAAR_VALUE	0x01

