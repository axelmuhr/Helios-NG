/******************************************************************************
*******************************************************************************
	A George Kalwitz Production, 1990
*******************************************************************************
******************************************************************************/

/*--- 8390 Registers ---*/
#define OFF_8390	0x10	/* offset of the 8390 chip */

/*--  page 0, rd --*/
#define CR	OFF_8390+0x00		/* Command Register	*/
#define CLDA0	OFF_8390+0x01		/* Current Local DMA Address 0 */
#define CLDA1	OFF_8390+0x02		/* Current Local DMA Address 1 */
#define BNRY	OFF_8390+0x03		/* Boundary Pointer */
#define TSR	OFF_8390+0x04		/* Transmit Status Register */
#define NCR	OFF_8390+0x05		/* Number of Collisions Register */
#define FIFO	OFF_8390+0x06		/* FIFO */
#define ISR	OFF_8390+0x07		/* Interrupt Status Register */
#define CRDA0	OFF_8390+0x08		/* Current Remote DMA Address 0 */
#define CRDA1	OFF_8390+0x09		/* Current Remote DMA Address 1 */
/* OFF_8390+0x0A is reserved */
/* OFF_8390+0x0B is reserved */
#define RSR	OFF_8390+0x0C		/* Receive Status Register */
#define CNTR0	OFF_8390+0x0D		/* Frame Alignment Errors */
#define CNTR1	OFF_8390+0x0E		/* CRC Errors */
#define CNTR2	OFF_8390+0x0F		/* Missed Packet Errors */

/*-- page 0, wr --*/
/*	CR	OFF_8390+0x00		   Command Register	*/
#define PSTART	OFF_8390+0x01		/* Page Start Register */
#define PSTOP	OFF_8390+0x02		/* Page Stop Register */
/*	BNDY	OFF_8390+0x03		   Boundary Pointer	*/
#define TPSR	OFF_8390+0x04		/* Transmit Page Start Register */
#define TBCR0	OFF_8390+0x05		/* Transmit Byte Count Register 0*/
#define TBCR1	OFF_8390+0x06		/* Transmit Byte Count Register 1*/
/*	ISR	OFF_8390+0x07		   Interrupt Status Register	*/
#define RSAR0	OFF_8390+0x08		/* Remote Start Address Register 0 */
#define RSAR1	OFF_8390+0x09		/* Remote Start Address Register 1 */
#define RBCR0	OFF_8390+0x0A		/* Remote Byte Count Register 0 */
#define RBCR1	OFF_8390+0x0B		/* Remote Byte Count Register 1 */
#define RCR	OFF_8390+0x0C		/* Receive Configuration Register */
#define TCR	OFF_8390+0x0D		/* Transmit Configuration Register */
#define DCR	OFF_8390+0x0E		/* Data Configuration Register */
#define IMR	OFF_8390+0x0F		/* Interrupt Mask Register */

/*-- page 1, rd and wr */
/*	CR	OFF_8390+0x00		   Control Register	*/
#define PAR0	OFF_8390+0x01		/* Physical Address Register 0 */
#define PAR1	OFF_8390+0x02		/*			     1 */
#define PAR2	OFF_8390+0x03		/*			     2 */
#define PAR3	OFF_8390+0x04		/*			     3 */
#define PAR4	OFF_8390+0x05		/*			     4 */
#define PAR5	OFF_8390+0x06		/*			     5 */
#define CURR	OFF_8390+0x07		/* Current Page Register */
#define	MAR0	OFF_8390+0x08		/* Multicast Address Register 0	*/
#define MAR1	OFF_8390+0x09		/* 			      1	*/
#define MAR2	OFF_8390+0x0A		/*			      2 */
#define	MAR3	OFF_8390+0x0B		/*			      3 */
#define	MAR4	OFF_8390+0x0C		/*			      4 */
#define MAR5	OFF_8390+0x0D		/*			      5 */
#define MAR6	OFF_8390+0x0E		/*			      6 */
#define MAR7	OFF_8390+0x0F		/*			      7 */

/*-- page 2, rd --*/
#define	BLOCK_ADDR	OFF_8390+0x06
#define	ENHANCEMENT	OFF_8390+0x07

/*-- page 2, wr --*/
/*
#define	BLOCK_ADDR	OFF_8390+0x06
#define	ENHANCEMENT	OFF_8390+0x07
*/

/*-- Command Register CR description */
#define STP		0x01	/* stop; software reset */
#define STA		0x02	/* start */
#define TXP		0x04	/* transmit packet */
#define	RD0		0x08
#define	RD1		0x10
#define	RD2		0x20
#define RRD		0x08	/* remote DMA command - remote read */

#define RWR		0x10	/* remote DMA command - remote write */
#define SPK		0x18	/* remote DMA command - send packet */
#define ABR		0x20	/* remote DMA command - abrt/cmplt remote DMA */

#define PS0		0x00	/* register page select - 0 */
#define PS1		0x40	/* register page select - 1 */
#define PS2		0x80	/* register page select - 2 */

#define	PS0_STA		0x22	/* page select 0 with start bit maintained */
#define	PS1_STA		0x62	/* page select 1 with start bit maintained */
#define	PS2_STA		0x0A2	/* page select 2 with start bit maintained */

/*-- Interrupt Status Register ISR description */
#define PRX		0x01	/* packet received no error */
#define PTX		0x02	/* packet transmitted no error */
#define RXE		0x04	/* receive error */
#define TXE		0x08	/* transmit error */
#define OVW		0x10	/* overwrite warning */
#define CNT		0x20	/* counter overflow */
#define RDC		0x40	/* remote DMA complete */
#define RST		0x80	/* reset status */

/*-- Interrupt Mask Register IMR description */
#define PRXE		0x01	/* packet received interrupt enable */
#define PTXE		0x02	/* packet transmitted interrupt enable */
#define RXEE		0x04	/* receive error interrupt enable */
#define TXEE		0x08	/* transmit error interrupt enable */
#define OVWE		0x10	/* overwrite warning interrupt enable */
#define CNTE		0x20	/* counter overflow interrupt enable */
#define RDCE		0x40	/* DMA complete interrupt enable */

/*-- Data Configuration Register DCR description */
#define WTS		0x01	/* word transfer select */
#define BOS		0x02	/* byte order select */
#define LAS		0x04	/* long address select */
#define BMS		0x08	/* burst DMA select */
#define AINIT		0x10	/* autoinitialize remote */

#define FTB2		0x00	/* receive FIFO threshold select - 2 bytes */
#define FTB4		0x20	/* receive FIFO threshold select - 4 bytes */
#define FTB8		0x40	/* receive FIFO threshold select - 8 bytes */
#define FTB12		0x60	/* receive FIFO threshold select - 12 bytes */

/*-- Transmit Configuration Register TCR description */
#define MCRC		0x01	/* manual crc generation */
#define LB1		0x02	/* mode 1; internal loopback LPBK=0 */
#define LB2		0x04	/* mode 2; internal loopback LPBK=1 */
#define LB3		0x06	/* mode 3; internal loopback LPBK=0 */

#define ATD		0x08	/* auto transmit disable */
#define OFST		0x10	/* collision offset enable */

/*-- Transmit Status Register TSR description --*/
#define XMT		0x01	/* packet transmitted without error */
#define COL		0x04	/* transmit collided */
#define ABT		0x08	/* transmit aborted */
#define CRS		0x10	/* carrier sense lost - xmit not aborted */
#define FU		0x20	/* FIFO underrun */
#define CDH		0x40	/* CD heartbeat */
#define OWC		0x80	/* out of window collision - xmit not aborted */

/*-- Receive Configuration Register RCR description --*/
#define SEP		0x01	/* save error packets */
#define AR		0x02	/* accept runt packet */
#define AB		0x04	/* accept broadcast */
#define AM		0x08	/* accept multicast */
#define PRO		0x10	/* promiscuous physical */
#define MON		0x20	/* monitor mode */

/*--Receive Status Register RSR description --*/
#define RCV		0x01	/* packet received intact */
#define CRC		0x02	/* CRC error */
#define FAE		0x04	/* frame alignment error */
#define FO		0x08	/* FIFO overrun */
#define MPA		0x10	/* missed packet */
#define PHY		0x20	/* physical/multicast address */
#define DIS		0x40	/* receiver disable */
#define DFR		0x80	/* deferring */

