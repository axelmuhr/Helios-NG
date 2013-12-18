/******************************************************************************
*******************************************************************************
	A George Kalwitz Production, 1990
*******************************************************************************
******************************************************************************/

#include	<stdio.h>
#include	<ctype.h>

#define FALSE 0
#define TRUE 1

#define	NUM_OF_BUFFS	4	/* number of buffers in rcv table */

/**** Western digital node bytes ****/
#define	WD_NODE_ADDR_0	0x00
#define	WD_NODE_ADDR_1	0x00
#define	WD_NODE_ADDR_2	0xC0

/**** BISTRO node bytes ****/
#define	BISTRO_NODE_ADDR_0	0x10
#define	BISTRO_NODE_ADDR_1	0x00
#define	BISTRO_NODE_ADDR_2	0x5A

#define	ALT_BISTRO_NODE_ADDR_0	0x08
#define	ALT_BISTRO_NODE_ADDR_1	0x00
#define	ALT_BISTRO_NODE_ADDR_2	0x5A

/**** NIC definitions ****/
#define	NIC_HEADER_SIZE	4		/* size of receive header */
#define	NIC_PAGE_SIZE	0x100		/* each page of rcv ring is 256 byte */
#define	XMT_ERR_MASK	0xD4
#define	RCV_ERR_MASK	0xDE

/**** Multicast definitions ****/
#define	MCA_REG	4			/* use forth mca reg for multicast */
#define	MCA_VAL	0x20			/* bit 5 of reg 4 allows multicast */
#define	POLYNOMIAL	0x04C11DB6	/* polynomial for calculating mca bit */

#define	ROM_SIG		0xAA55

/***************************************************************************

these are the memory map definitions for communicating with the WD8023E

****************************************************************************/
#define	TX_SPC			0x0000		/* offset of TX area */
#define	RX_SPC			0x0800		/* offset of RX area */
#define	GEN_SPC			0x4000		/* offset of work space */
#define	STAT_SPC		0x4010		/* offset of Stats area */
#define	TX_PEND_FLAG		0x4100		/* transmit pending flag */
#define	BAD_NXT_PAGE_FLAG	0x4102		/* bad next page ptr flag */
#define	MAIL_BOX		0x5000		/* offset of Mail Box area */

/*************************************************************************

these are just to give each register a unique value
	THEY ARE NOT THE OFFSETS INTO THE LAN BOARD OR THE 8390 CHIP

**************************************************************************/
#define	N_CR		0
#define	N_TCR		1
#define	N_RCR		2
#define	N_DCR		3
#define	N_ISR		4
#define	N_IMR		5
#define	N_PSTART	6
#define	N_PSTOP		7
#define	N_BNRY		8
#define	N_TPSR		9
#define	N_CURR		10
#define	N_TBCR0		11
#define	N_TBCR1		12
#define N_BLOCK_ADDR	13
#define N_ENHANCEMENT	14
#define	N_TSR		15
#define	N_NCR		16
#define	N_RSR		17
#define	N_CNTR0		18
#define	N_CNTR1		19
#define	N_CNTR2		20
#define	N_PAR0		21
#define N_PAR5		26

/* Batch line definitions */
#define B_NONE		0	/* only specifying params with batch line */
#define B_PARM		0x01	/* user put at least one param on batch line */
#define	B_SPEC		0x02	/* batch with specifying a adapter */
#define	B_SEND		0x04	/* batch send */
#define B_RESP		0x08	/* batch respond */
#define	B_TEST		0x10	/* batch test */
#define B_BIO		0x20	/* put baseio on batch line */

