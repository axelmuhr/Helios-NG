/* $Header: /giga/HeliosRoot/Helios/servers/serial/RCS/serialdev.c,v 1.5 1991/10/09 11:23:36 paul Exp $ */
/* $Source: /giga/HeliosRoot/Helios/servers/serial/RCS/serialdev.c,v $ */
/************************************************************************/ 
/* serialdev.c - Helios/ARM device driver for serial line on AB		*/
/*		 Functional Prototype, Heval board, & The Book itself	*/
/*									*/
/* Copyright 1990, 1991 Active Book Company Ltd., Cambridge, England	*/
/*									*/
/* Author: Brian Knight, 14th February 1990				*/
/************************************************************************/


/************************************************************************/
/* This driver is for NEC 72001 serial chip. It supports two subdevices	*/
/* corresponding to the two serial channels of the chip.		*/
/* 									*/
/* Only asynchronous mode is supported at present.			*/
/************************************************************************/

/*
 * $Log: serialdev.c,v $
 * Revision 1.5  1991/10/09  11:23:36  paul
 * changed include path for fproto.h
 *
 * Revision 1.4  1991/05/31  15:04:55  paul
 * brians changes
 *
 * Revision 1.4  91/02/20  13:20:07  brian
 * Ported to Heval board.
 * 
 * Revision 1.3  90/10/07  09:40:59  brian
 * Checkpoint before moving to SMT
 * 
 * Revision 1.2  90/07/05  16:17:52  brian
 * Checkpoint before fiddling with semaphore handling
 * 
 * Revision 1.1  90/06/12  10:01:08  brian
 * Initial revision
 * 
 * Revision 1.3  90/05/01  09:26:02  brian
 * Checkpoint before moving to functional prototype board.
 * 
 * Revision 1.2  90/03/14  12:34:23  brian
 * Checkpoint before revising various internal interfaces
 * 
 * Revision 1.1  90/03/08  12:22:55  brian
 * Initial revision
 * 
 */

/* #define TRACE */
/* #define DEBUG */
#define RTSFUDGE	/* RTS control tweaking for auto-enable mode */
#define RTSFUDGE2 	/* Another variant */

/* Fool helios.h into defining Code */
#define in_kernel
#include <helios.h>
#undef in_kernel
#include <syslib.h>
#include <device.h>
#include <codes.h>
#include <root.h>
#include <event.h>
#include <stdio.h>
#include <attrib.h>
#include <abcARM/fproto.h>
#include <abcARM/heval.h>
#include <abcARM/ABClib.h>
#include <dev/serialdev.h>
#include <mcdep.h>

/*----------------------------------------------------------------------*/

/* Driver configuration */

#undef  SERIALDEBUG            /* Turn off debugging messages        */
#define CHANNELA           0   /* Index of Channel A regs in arrays  */
#define CHANNELB           1   /* Index of Channel B regs in arrays  */
#define NCHANNELS          2   /* Number of serial channels	     */
#define NCONTROLREGS      16   /* Number of control/status regs per channel */
#define FP_XTALFREQ  7372800   /* Frequency of crystal used for BRGs on FP */
#define RTSTHRESHOLD	 128   /* Buffer slots left when RTS turned off */
#define XOFFTHRESHOLD	 128   /* Buffer slots left when XOFF issued */
#define XONTHRESHOLD     (XOFFTHRESHOLD + 16) /* Slots left when XON issued */
#define RXBUFSIZE	 (XOFFTHRESHOLD + 32) /* Size of internal rx buffer */

/* Flow control characters */

#define XON		0x11	/* Sent to allow other end to transmit	*/
#define XOFF		0x13	/* Sent to stop other end transmitting	*/

/* Offsets of device registers from base */

#define CHANNELADATA    0x00
#define CHANNELACONTROL 0x08
#define CHANNELBDATA    0x10
#define CHANNELBCONTROL 0x18

/* Safe update of flags in foreground thread of driver */

#define SAFESETFLAGS(dcb, flags, bits) \
          DisableIRQ((dcb)->intSourceBit); \
  	  (flags) |= (bits); \
          EnableIRQ((dcb)->intSourceBit)

#define SAFECLEARFLAGS(dcb, flags, bits) \
          DisableIRQ((dcb)->intSourceBit); \
  	  (flags) &= ~(bits); \
          EnableIRQ((dcb)->intSourceBit)

/* Types of hardware supported.						*/
/* These are the codes returned by the exec_Version SWI			*/

#define SYSTEMTYPE_FPROTO	0xFF	/* Functional prototype		*/
#define SYSTEMTYPE_HEVAL	0xFE	/* Hercules evaluation board	*/
#define SYSTEMTYPE_AB1		0xAB	/* Active Book 1		*/


/*----------------------------------------------------------------------*/

/*----------------------------------------------------------------------*/
/* Bit positions and field values in the control registers.		*/
/* (Not all of those applicable to COP and BOP modes are included.)	*/
/*----------------------------------------------------------------------*/

/* Control Register 0 commands */
#define CR0_NOOP	 0x00	/* No-op (used to set register pointer) */
#define CR0_HIGHPTR	 0x08	/* Set register pointer above 7		*/
#define CR0_RESETESLATCH 0x10	/* Re-enable External/Status bits latch */ 
#define CR0_CHANNELRESET 0x18	/* Reset this channel			*/
#define CR0_ENABLENEXTRI 0x20 	/* Enable next receive char interrupt	*/
#define CR0_RESETTXINT	 0x28	/* Reset tx interrupt/DMA pending	*/
#define CR0_ERRORRESET	 0x30	/* Error reset				*/
#define CR0_ENDOFINT	 0x38	/* (Channel A only) Notify end of interrupt */

/* Control register 1 */
#define CR1_ESINTENABLE	 0x01	/* Enable int on External/Status change	*/
#define CR1_TXINTENABLE	 0x02	/* Enable transmit interrupts		*/
#define CR1_1STTXIENABLE 0x04	/* Enable First Transmit interrupt	*/

#define CR1_RXINTMODE	 0x18	/* Receive interrupt mode field		*/
#define CR1_RXIDISABLE	 0x00	/* Disable receive interrupts		*/
#define CR1_1STRXINT	 0x08	/* Interrupt on first received char	*/
#define CR1_ALLRXINT1	 0x10	/* Interrupt on all rx chars & parity error */
#define CR1_ALLRXINT2	 0x18	/* Int on all rx chars but not parity error */

#define CR1_1STRXIMASK	 0x20	/* Mask First Receive interrupt		*/
#define CR1_OVERRUNINT	 0x40	/* Set Special Mode for overrun error	*/
#define CR1_SHORTFRAME	 0x80	/* Enable short frame detection (BOP only) */

/* Control register 2A - System interface mode */
#define CR2A_INTDMAMODE	 0x03	/* DMA/interrupt mode field		*/
#define CR2A_BOTHINT	 0x00	/* Both A & B interrupt driven		*/
#define CR2A_ADMABINT	 0x01	/* Channel A uses DMA, B uses ints	*/
#define CR2A_BOTHDMA	 0x02	/* Both A & B use DMA			*/
/*			 0x03      is illegal				*/

#define CR2A_RXBGTTXA	 0x04	/* 0 for TxA > RxB, 1 for RxB > TxA	*/

#define CR2A_OUTVECTTYPE 0x38	/* Output vector type field		*/
#define CR2A_TYPEA1	 0x00   /* See the manual!			*/
#define CR2A_TYPEA2	 0x08
#define CR2A_TYPEA3	 0x10
#define CR2A_TYPEB1	 0x18
#define CR2A_TYPEB2	 0x20

#define CR2A_AFFECTSVEC	 0x40	/* Set to make status affect int vector */
#define CR2A_VECTORED	 0x80	/* Set for vectored interrupts		*/

/* Control register 2B - Interrupt vector initial value: no subfields */

/* Control register 3 - Reception control */
#define CR3_RXENABLE	 0x01	/* Enable reception			*/
#define CR3_MULTICAST	 0x02	/* (COP and BOP modes only)		*/
#define CR3_ADDRSEARCH	 0x04	/* (BOP mode only)			*/
#define CR3_RXCRCENABLE	 0x08	/* (COP and BOP modes only)		*/
#define CR3_HUNTPHASE	 0x10	/* (COP and BOP modes only)		*/
#define CR3_AUTOENABLE	 0x20	/* Allow CTS/DCD pin to control rx	*/

#define CR3_RXCHARLEN	 0xC0	/* Rx character length field		*/
#define CR3_5BITS	 0x00
#define CR3_7BITS	 0x40
#define CR3_6BITS	 0x80
#define CR3_8BITS	 0xC0

/* Control register 4 - Protocol selection */
#define CR4_PARITYENABLE 0x01	/* Enable parity generation/checking	*/
#define CR4_EVENPARITY   0x02	/* 0 for odd parity			*/

#define CR4_TXSTOPBITS	 0x0C	/* Field for number of tx stop bits	*/
/*		         0x00      is illegal				*/
#define CR4_1STOP	 0x04	/* 1 stop bit				*/
#define CR4_15STOP	 0x08	/* 1.5 stop bits			*/
#define CR4_2STOP	 0x0C	/* 2 stop bits				*/

#define CR4_PROTOCOLMODE 0x30	/* Protocol mode (COP & BOP only)	*/

#define CR4_CLOCKRATE	 0xC0	/* Clock rate field. This gives the	*/
#define CR4_TIMES1	 0x00	/* number of clock ticks per bit.	*/
#define CR4_TIMES16	 0x40	/* *16 is the usual value in 		*/
#define CR4_TIMES32	 0x80	/* asynchronous mode.			*/
#define CR4_TIMES64	 0xC0

/* Control register 5 - Transmission control */
#define CR5_TXCRCENABLE	 0x01	/* (COP and BOP only)			*/
#define CR5_RTS		 0x02	/* 1 to assert RTS o/p (non-auto mode)  */
#define CR5_CRCPOLY	 0x04	/* (COP and BOP only)			*/
#define CR5_TXENABLE	 0x08	/* Enable transmission			*/
#define CR5_SENDBREAK	 0x10	/* Send break signal			*/

#define CR5_TXCHARLEN	 0x60	/* Tx character length field		*/
#define CR5_5BITS	 0x00
#define CR5_7BITS	 0x20
#define CR5_6BITS	 0x40
#define CR5_8BITS	 0x60

#define CR5_DTR		 0x80	/* Sets logical value of DTR output	*/

/* Control registers 6-9 not used in asynchronous mode. */

/* Control register 10 - Data format */
#define CR10_DATAFORMAT	 0x60	/* Serial data format			*/
#define CR10_NRZ	 0x00	/* Normal setting for asynchronous mode	*/
#define CR10_NRZI	 0x20
#define CR10_FM1	 0x40
#define CR10_FM0	 0x60

/* Control register 11 - External/Status interrupt enable bits 		*/
#define CR11_BRGIE	 0x01	/* Baud Rate Generator 			*/
#define CR11_IDLEDETIE	 0x02	/* Idle Detect (COP & BOP only)		*/
#define CR11_ALLSENTIE	 0x04	/* All Sent				*/
#define CR11_DCDIE	 0x08	/* Change in DCD input			*/
#define CR11_SYNCHUNTIE	 0x10	/* Change in SYNC/Hunt			*/
#define CR11_CTSIE	 0x20	/* Change in CTS input			*/
#define CR11_URUNEOMIE	 0x40	/* Tx Underrun/End Of Message (COP, BOP)*/
#define CR11_BREAKIE	 0x80	/* Break/Abort 				*/

/* Control register 12 - Baud Rate Generator Control */
#define CR12_RXBRGSET	 0x01	/* Set rx BRG reg (in next 2 writes)	*/
#define CR12_TXBRGSET	 0x02	/* Set tx BRG reg (in next 2 writes)	*/
#define CR12_RXBRGIE	 0x04	/* Enable rx BRG interrupt		*/
#define CR12_TXBRGIE	 0x08	/* Enable tx BRG interrupt		*/
/*                       0x30	   Unused bits				*/
#define CR12_BRGFORDPLL	 0x40	/* Which BRG is DPLL clock: 0->rx, 1->tx*/
#define CR12_BRGFORTRXC	 0x80	/* Which BRG is TXRC o/p: 0->rx, 1->tx	*/

/* Control register 13 - Transmit Data Length Counter control + Standby	*/
#define CR13_STANDBYMODE 0x01	/* 0: no operation, 1: set standby mode	*/

/* Control register 14 - DPLL, BRG and Test Mode */
#define CR14_TXBRGENABLE 0x01	/* Enable tx Baud Rate Generator	*/
#define CR14_RXBRGENABLE 0x02	/* Enable rx Baud Rate Generator	*/
#define CR14_BRGSYSCLK	 0x04	/* BRG source: 0: xtal/STRxC, 1: sys clk*/
#define CR14_LOOPTEST	 0x08	/* Enable echo loop test		*/
#define CR14_SELFTEST	 0x10	/* Enable local self test		*/

#define CR14_DPLLCOMMAND 0xE0	/* Commands to control DPLL circuit	*/
#define CR14_DPLLNOOP	 0x00	/* No operation				*/
#define CR14_DPLLSEARCH	 0x20	/* Enter search				*/
#define CR14_DPLLRMC	 0x40	/* Reset missing clock (FM mode only)	*/
#define CR14_DPLLDISABLE 0x60	/* Disable DPLL				*/
#define CR14_DPLLBRG	 0x80	/* Select BRG as source clock for DPLL	*/
#define CR14_DPLLXTAL	 0xA0	/* Select Xtal/STRxC as DPLL clock	*/
#define CR14_DPLLRXFM	 0xC0	/* Receive FM format data		*/
#define CR14_DPLLRXNRZI	 0xE0	/* Receive NRZI format data		*/

/* Control register 15 - transmit/receive clock selection */
#define CR15_TRXCSOURCE	 0x03	/* Select source for TRxC output	*/
#define CR15_TRXCXTAL	 0x00	/* Internal crystal oscillator		*/
#define CR15_TRXCTXCLK	 0x01	/* Transmit clock			*/
#define CR15_TRXCBRG	 0x02	/* Tx or rx BRG (selected in CR12)	*/
#define CR15_TRXCDPLL	 0x03	/* DPLL clock				*/

#define CR15_TRXCOUTPUT	 0x04	/* 0: TXrC is input; 1: TXrC is output	*/

#define CR15_TXCLKSELECT 0x18	/* Select clock to be used as tx clock	*/
#define CR15_TXCLKSTRXC	 0x00	/* STRxC input				*/
#define CR15_TXCLKTRXC	 0x08	/* TRxC input				*/
#define CR15_TXCLKTXBRG	 0x10	/* Transmit BRG				*/
#define CR15_TXCLKDPLL	 0x18	/* DPLL clock				*/

#define CR15_RXCLKSELECT 0x60	/* Select clock to be used as rx clock	*/
#define CR15_RXCLKSTRXC	 0x00	/* STRxC input				*/
#define CR15_RXCLKTRXC	 0x20	/* TRxC input				*/
#define CR15_RXCLKRXBRG	 0x40	/* Receive BRG				*/
#define CR15_RXCLKDPLL	 0x60	/* DPLL clock				*/

#define CR15_XTALSELECT	 0x80	/* 1 to enable internal xtal oscillator	*/

/*----------------------------------------------------------------------*/
/* Bit positions and field values in the status registers.		*/
/* (Not all of those applicable to COP and BOP modes are included.)	*/
/*----------------------------------------------------------------------*/

/* Status Register 0 - transmit/receive operation status */
#define SR0_RXDATA	 0x01	/* Data in last stage of rx buffer	*/
#define SR0_SENDINGABORT 0x02	/* (COP mode only)			*/
#define SR0_TXBUFEMPTY	 0x04	/* Transmit buffer empty		*/
#define SR0_SHORTFRAME	 0x08	/* Short frame detect (BOP only)	*/
#define SR0_PARITYERROR	 0x10	/* Parity error detected		*/
#define SR0_RXOVERRUN	 0x20	/* Receive overrun error		*/
#define SR0_FRAMINGERROR 0x40	/* Framing error detected		*/
#define SR0_ENDOFFRAME	 0x80	/* BOP mode only			*/

/* Status Register 1 - External/Status bits */
#define SR1_BRGZEROCOUNT 0x01	/* BRG count has reached zero		*/
#define SR1_IDLEDETECT	 0x02	/* Idle detect (BOP only)		*/
#define SR1_ALLSENT	 0x04	/* Transmit data all sent out		*/
#define SR1_DCD		 0x08	/* Reflects logical state of DCD input	*/
#define SR1_SYNCHUNT	 0x10	/* Reflects SYNC input			*/
#define SR1_CTS		 0x20	/* Reflects CTS input			*/
#define SR1_TXUNDERRUN	 0x40	/* Tx underrun (COP & BOP only)		*/
#define SR1_BREAKDETECT	 0x80	/* Break has been detected		*/

/* Status register 2B - Interrupt source 				*/
/* The layout of this register depends on the vector type set in CR2A.	*/
/* Vector type A */
#define SR2B_ASOURCE	 0x0C	/* Int source field for vector type A	*/
#define SR2B_ATXBUFEMPTY 0x00	/* Transmit buffer empty		*/
#define SR2B_AES	 0x04	/* External/status interrupt		*/
#define SR2B_ARXDATA	 0x08	/* Rx data available			*/
#define SR2B_ASPECRXCOND 0x0C	/* Special rx condition			*/

#define SR2B_ACHANNELA	 0x10	/* 0: channel B, 1: channel A		*/

/* Vector type B */
#define SR2B_BSOURCE	 0x03	/* Int source field for vector type B	*/
#define SR2B_BTXBUFEMPTY 0x00	/* Transmit buffer empty		*/
#define SR2B_BES	 0x01	/* External/status interrupt		*/
#define SR2B_BRXDATA	 0x02	/* Rx data available			*/
#define SR2B_BSPECRXCOND 0x03	/* Special rx condition			*/

#define SR2B_BCHANNELA	 0x04	/* 0: channel B, 1: channel A		*/

/* Status register 3 - BRG zero count and number of fraction bits */
#define SR3_RXBRGZERO	 0x08	/* Rx BRG count has reached zero	*/
#define SR3_TXBRGZERO	 0x10	/* Tx BRG count has reached zero	*/

/* Status register 4A - Interrupt pending status */
#define SR4A_BESINT	 0x01	/* Channel B External/status interrrupt	*/
#define SR4A_BTXINT	 0x02	/* Channel B tx interrupt		*/
#define SR4A_BRXINT	 0x04	/* Channel B rx interrupt		*/
#define SR4A_AESINT	 0x08	/* Channel A External/status interrrupt	*/
#define SR4A_ATXINT	 0x10	/* Channel A tx interrupt		*/
#define SR4A_ARXINT	 0x20	/* Channel A rx interrupt		*/
#define SR4A_BSPECRX	 0x40	/* Channel B special rx condition	*/
#define SR4A_ASPECRX	 0x80	/* Channel A special rx condition	*/

/* Status registers 5, 6 and 7 do not exist.				*/
/* Status registers 8, 9 and 10 are irrelevant in asynchronous mode.	*/
/* Status register 11 - E/S interrupt mask (reflects contents of CR11)	*/
/* Status register 12 - Low byte of Rx BRG counter constant		*/
/* Status register 13 - High byte of Rx BRG counter constant		*/
/* Status register 14 - Low byte of Tx BRG counter constant		*/
/* Status register 15 - High byte of Tx BRG counter constant		*/

/*----------------------------------------------------------------------*/

typedef enum TxOrRx {Tx, Rx} TxOrRx;
typedef unsigned char u_char;
typedef volatile unsigned char vu_char;

/* Structure to hold information about one channel of the serial device	*/
/* Many fields are volatile as they are either memory-mapped device	*/
/* registers or things which may be accessed in interrupt routines.	*/

typedef struct SerialChannel
{
  vu_char       *dataReg;     /* Memory-mapped data register        	*/
  vu_char       *conStatReg;  /* Memory-mapped control/status reg   	*/
  u_char        *readPtr;     /* where to put next input byte       	*/
  u_char	*readEnd;     /* byte after end of input buffer     	*/
  SaveState	readSaveState;/* foreground read process waits here	*/
  SerialReq     *readReq;     /* request for current read op        	*/
  u_char	*writePtr;    /* where to get next output byte      	*/
  u_char	*writeEnd;    /* byte after end of output buffer    	*/
  SaveState	writeSaveState;/* foreground write process waits here	*/
  SerialReq     *writeReq;    /* request for current write op       	*/
  Attributes	attributes;   /* set of attributes			*/
  unsigned int	flags;        /* Various flags			    	*/
  vu_char       controlCopy[NCONTROLREGS]; /* Soft copies of control regs */
  u_char	rxBuf[RXBUFSIZE]; /* Circular buffer for reception  	*/
  u_char	*rxBufEnd;    /* Byte after end of buffer		*/
  u_char	*nextData;    /* Next data byte in buffer	    	*/
  u_char	*nextFree;    /* Next free slot 			*/
  /* Buffer is empty if nextData == nextFree, full if nextFree points 	*/
  /* to slot before nextData. To prevent races, nextData is incremented */
  /* only by the foreground thread of the driver, and nextFree is	*/
  /* incremented only in interrupt routines. 				*/
  struct SerialDCB *dcb;      /* pointer back to parent DCB		*/
#ifdef TRACE
  unsigned int	traceCount;   /* used for tracing events		*/
#endif
  unsigned int  signalCount;  /* for debugging */
  unsigned int  waitCount;    /* for debugging */
} SerialChannel;

/* Bits in flags field (most applicable to one channel direction only) */

#define SCF_OPEN	0x0001	/* Channel is open			    */
#define SCF_SWOVERRUN   0x0002	/* Read too slow, buffer has overflowed     */
#define SCF_HWOVERRUN	0x0004	/* Hardware overrun (reported by 72001)	    */
#define SCF_OVERRUN	(SCF_SWOVERRUN | SCF_HWOVERRUN) /* Either overrun   */
#define SCF_RTSON	0x0008	/* Set when RTS output asserted		    */
#define SCF_XOFFREC	0x0010	/* XOFF received			    */
#define SCF_SENDXON	0x0020	/* Send XON character			    */
#define SCF_SENDXOFF	0x0040	/* Send XOFF character			    */
#define SCF_XOFFSENT	0x0080	/* XOFF char sent (cleared when XON sent)   */
#define SCF_FIRSTREAD   0x0100   /* For ignoring overrun before first read  */
#define SCF_READWAIT	0x0200	/* Foreground waiting for read int routine  */
				/* (hence readSaveState valid)		    */
#define SCF_WRITEWAIT	0x0400	/* Foreground waiting for write int routine */
				/* (hence writeSaveState valid)		    */

/* Device Control Block	*/

typedef struct SerialDCB
{
  DCB           dcb;	     	    	/* Standard DCB                      */
  SerialChannel channel[NCHANNELS];	/* Info on each serial channel       */
  Semaphore     deviceLock;             /* For serialising access to device  */
  Event         intHandler;             /* Event structure for int handler   */
  int	        logDev;                 /* Logical device number             */
  int		systemType;		/* System driver is running on	     */
  int		intSourceBit;		/* Source/mask bit for serial int    */
  int		clockFreq;		/* Frequency of clock used for BRGs  */
} SerialDCB, *RefSerialDCB;

/*----------------------------------------------------------------------*/

/* Forward references */

int  SerialInt(SerialDCB *dcb);
void DevOperate(SerialDCB *dcb, SerialReq *req);
word DevClose(SerialDCB *dcb);
static void SetControlReg(SerialChannel *channel, int reg, u_char value,
			  int foreground);
static void SetControlBits(SerialChannel *channel, int reg, u_char bits,
			   int foreground);
static void ClearControlBits(SerialChannel *channel, int reg, u_char bits,
			     int foreground);
static void InitControlRegs(SerialChannel *channel);
static void ResetChannel(SerialChannel *channel);
static void SetBaudRate(SerialChannel *channel, TxOrRx direction, 
			int baudRate);
static u_char ReadStatusReg(SerialChannel *channel, int reg);
static int SerialRead(SerialDCB *dcb, SerialReq *req);
static int SerialWrite(SerialDCB *dcb, SerialReq *req);
static int AbortSerialRead(SerialDCB *dcb, SerialReq *req);
static int AbortSerialWrite(SerialDCB *dcb, SerialReq *req);
#ifdef events
static int SerialGetEvent(SerialDCB *dcb, SerialReq *req);
#endif
static void RxInterrupt(SerialDCB *dcb, int chanNum);
static void TxInterrupt(SerialDCB *dcb, int chanNum);
static void ESInterrupt(SerialDCB *dcb, int chanNum);
static void SpecRxInterrupt(SerialDCB *dcb, int chanNum);
static void SerialGetInfo(SerialChannel *channel, SerialReq *req);
static void SerialSetInfo(SerialChannel *channel, SerialReq *req);
static void SetChannelAttributes(SerialChannel *channel, Attributes *attr);
static int  SpeedAttributeToBaudRate(int speedAttr);
static void SetDefaultAttributes(SerialChannel *channel);
static int  FindSystemType(void);

/*----------------------------------------------------------------------*/

#ifdef TRACE
/* Trace events on screen */
static void
TraceEvent(SerialChannel *channel, int event, int v1, int v2, int v3)
{
  int *poke = (int *)(0x00740000 + (channel->traceCount % 400)*256);

  poke[4] = channel->traceCount;
  poke[3] = event;
  poke[2] = v1;
  poke[1] = v2;
  poke[0] = v3;

  ++channel->traceCount;
}
#endif

/*----------------------------------------------------------------------*/

RefSerialDCB
DevOpen(Device *dev, SerialDevInfo *info)
{
  SerialDCB  *dcb;
  vu_char    *regs;
  int	     i;
  int	     systemType = FindSystemType();
  int	     intSourceBit, clockFreq;

  /* The `info' argument tells us which logical serial device to open.  */
  /* There is only one serial device on the functional prototype board	*/
  /* (and this driver does not support podules).			*/

  if (info->logDevNum != 0) return NULL; /* No such serial device */

  /* Determine the hardware address of the serial chip */
  switch (systemType)
  {
#ifdef __AB1_H_
  case SYSTEMTYPE_AB1:  
    regs         = (vu_char *)AB1_SERIAL_BASE;  
    intSourceBit = AB1_INT_SERIAL;
    clockFreq 	 = AB1_SERIAL_CLOCK_FREQ; IOdebug("Don't know serial clock frequency for AB1");
    break;
#endif /* __AB1_H_ */

#ifdef __HEVAL_H_

#define FAX_IRH		   0x00008	/* IRH    [7:0]   write only 	*/
#define FAX_IRL		   0x0000C	/* IRL    [7:0]   write only 	*/
#define FAX_FLG		   0x1002C	/* FLG    [7:1]   read only  	*/

  case SYSTEMTYPE_HEVAL:  
    regs         = (vu_char *)HEVAL_SERIAL_BASE;
    intSourceBit = HEVAL_INT_SERIAL;
    clockFreq	 = HEVAL_SERIAL_CLOCK_FREQ;

    /* Heval bodge: the fax chip shares an external request line with	*/
    /* the serial chip, so it is essential to put the fax chip into	*/
    /* parallel mode to stop it waggling this line.			*/
    /* This will eventually be done in the executive start-up.		*/

#define FAXBODGELIMIT 10000 /* Limit on polling loops */
    {
      volatile ubyte *faxRegs = (ubyte *)HEVAL_FAX_BASE;
      volatile int   i = 0;

      IOdebug("Heval fax bodge");

      for (i = 0; i < FAXBODGELIMIT; ++i)
      {
	if ((faxRegs[FAX_FLG] & 0x02) != 0) break; /* Wait for IRE */
      }

      if (i >= FAXBODGELIMIT)
	IOdebug("Fax bodge timed out (no fax chip?)");
      else
      {
	faxRegs[FAX_IRH] = 0x08; /* Set Parallel Enable bit in PARM reg	*/
	faxRegs[FAX_IRL] = 0x05;

	for (i = 0; i < FAXBODGELIMIT; ++i)
	{
	  if ((faxRegs[FAX_FLG] & 0x02) != 0) break; /* Wait for IRE */
	}

	faxRegs[FAX_IRH] = 0x7F;  /* STP cmd to make PARM change take effect */
	faxRegs[FAX_IRL] = 0xFF;
      }
    }

    break;
#endif /* __HEVAL_H_ */

#ifdef  _FPROTO_H_
  case SYSTEMTYPE_FPROTO: 
    regs         = (vu_char *)SERIAL_BASE;       
    intSourceBit = INT_SERIAL;
    clockFreq	 = FP_XTALFREQ; /* Serial chip has its own crystal on FP */
    break;
#endif /* _FPROTO_H_ */

  default: return NULL; /* Unsupported system type */
  }

  dcb = Malloc(sizeof(SerialDCB));
  if (dcb == NULL) return NULL;
  
  dcb->dcb.Device   = dev;
  dcb->dcb.Operate  = DevOperate;
  dcb->dcb.Close    = DevClose;
  InitSemaphore(&dcb->deviceLock, 1);
  dcb->logDev       = info->logDevNum;
  dcb->systemType   = systemType;
  dcb->intSourceBit = intSourceBit;
  dcb->clockFreq    = clockFreq;
  
  /* Initialise the channel structures and registers */
  dcb->channel[CHANNELA].dataReg    = regs + CHANNELADATA;
  dcb->channel[CHANNELA].conStatReg = regs + CHANNELACONTROL;
  dcb->channel[CHANNELB].dataReg    = regs + CHANNELBDATA;
  dcb->channel[CHANNELB].conStatReg = regs + CHANNELBCONTROL;

  for (i = 0; i < NCHANNELS; ++i)
  {
    SerialChannel *channel = &dcb->channel[i];

    channel->dcb = dcb;
    ResetChannel(channel);
  }

  /* Set up the control registers which apply to both channels 		*/
  /* CR2A: both channels interrupt driven, RxB > TxA, vectored,     	*/
  /*       output vector type A3 (so PRI input line is ignored).	*/
  /* CR2B: interrupt vector unused, but set to known value to be tidy 	*/
  SetControlReg(&dcb->channel[CHANNELA], 2, 
		CR2A_BOTHINT | CR2A_RXBGTTXA | CR2A_TYPEA3 | CR2A_VECTORED, 1);
/*		CR2A_BOTHINT | CR2A_RXBGTTXA | CR2A_TYPEA1, 1); */
  SetControlReg(&dcb->channel[CHANNELB], 2, 0, 1);

  /* Register 15 (tx/rx clock source selection) needs to be set		*/
  /* differently for each channel on the functional prototype. The 	*/
  /* crystal is	connected to channel B. Channel A is uncommitted, but   */
  /* all its signals are connected to a header, so it is probably best	*/
  /* to configure TRxCA as an input.					*/

  /* Channel A: TRxC is input, no xtal */
  SetControlReg(&dcb->channel[CHANNELA], 15, 
		CR15_TXCLKTXBRG | CR15_RXCLKRXBRG, 1);

  /* Channel B setting depends on the hardware				*/

  switch (dcb->systemType)
  {
  case SYSTEMTYPE_FPROTO:
    /* Channel B: TRxC is output (unused); BRG input is xtal */
    SetControlReg(&dcb->channel[CHANNELB], 15, 
		  CR15_TRXCXTAL   | CR15_TRXCOUTPUT |
		  CR15_TXCLKTXBRG | CR15_RXCLKRXBRG | CR15_XTALSELECT, 1);
    break;

  case SYSTEMTYPE_HEVAL:
  case SYSTEMTYPE_AB1:
    /* Channel B: TRxC is input (unused); no xtal */
    SetControlReg(&dcb->channel[CHANNELB], 15, 
		  CR15_TXCLKTXBRG | CR15_RXCLKRXBRG, 1);
    break;

  default:
    IOdebug("unknown system type %d", dcb->systemType);
    break;
  }


  /* Set up the interrupt routine */
 
  dcb->intHandler.Pri  = dcb->intSourceBit; /* Int source for this handler */
  dcb->intHandler.Code = (WordFnPtr)SerialInt;
  dcb->intHandler.Data = dcb;    /* Pass DCB address to int routine */
  SetEvent(&dcb->intHandler); 

  EnableIRQ(dcb->intSourceBit); /* Allow serial device to generate IRQs */

  return dcb;
}

/************************************************************************/
/* Reset one serial channel's structure and registers.			*/
/************************************************************************/
static void
ResetChannel(SerialChannel *channel)
{
  InitControlRegs(channel); /* Reset channel and set default state */
  /* No interrupts will happen now */
  SetDefaultAttributes(channel); /* This will alter some control regs */
  /* Could rationalise InitControlRegs and SetDefaultAttributes so that	*/
  /* things do not get set twice.					*/
  channel->flags = SCF_FIRSTREAD;
  channel->rxBufEnd = &channel->rxBuf[0] + RXBUFSIZE;
  channel->nextData = &channel->rxBuf[0];
  channel->nextFree = channel->nextData;
  channel->readReq  = 0;
  channel->writeReq = 0;
#ifdef TRACE
  channel->traceCount = 0;
#endif
  channel->signalCount = 0;
  channel->waitCount   = 0;
}

/************************************************************************/
/* Initialise the control registers for one serial channel.		*/
/************************************************************************/
static void
InitControlRegs(SerialChannel *channel)
{
  int delay;

#define SCR(r, v) SetControlReg(channel, (r), (v), 1 /* foreground */)

  SCR(0, CR0_CHANNELRESET); /* Reset channel first */

  /* Wait at least 3 system clocks (3/8 us) after reset before writing	*/
  /* another register.							*/
  for (delay = 0; delay < 100; ++delay); /* Need a proper way to wait... */

  SCR(1, 0); /* All interrupts disabled */

  /* CR2A and CR2B have global effects and are set elsewhere */

  /* 8 bit rx chars, DCD/CTS disabled, reception disabled */
  SCR(3, CR3_8BITS);

  /* No parity, 1 stop bit, *16 clock rate */
  SCR(4, CR4_1STOP | CR4_TIMES16);

  /* RTS off, tx disabled, no break, 8 bit tx chars, DTR off */
  SCR(5, CR5_8BITS);

  SCR( 6, 0); /* Irrelevant in asynchronous mode */
  SCR( 7, 0); /* Irrelevant in asynchronous mode */
  SCR( 8, 0); /* Irrelevant in asynchronous mode */
  SCR( 9, 0); /* Irrelevant in asynchronous mode */

  /* Set NRZ format for asynchronous mode */
  SCR(10, CR10_NRZ);

  SCR(11, 0); /* All E/S interrupts disabled */

  /* Disable BRG interrupts; set baud rates (done via CR12) */
  SCR(12, 0);
  SetBaudRate(channel, Tx, DEFAULTTXBAUDRATE);
  SetBaudRate(channel, Rx, DEFAULTRXBAUDRATE);

  /* Disable Tx Data Length Counter, disable standby mode */
  SCR(13, 0);

  switch (channel->dcb->systemType)
  {
  case SYSTEMTYPE_FPROTO:
    /* Enable both BRGs, set xtal as BRG src clk, disable test modes & DPLL */
    SCR(14, CR14_TXBRGENABLE | CR14_RXBRGENABLE);
    break;

  case SYSTEMTYPE_HEVAL:
  case SYSTEMTYPE_AB1:
    /* Enable both BRGs, set system clock as BRG source clock,	 	*/
    /* disable test modes & DPLL 					*/
    SCR(14, CR14_BRGSYSCLK | CR14_TXBRGENABLE | CR14_RXBRGENABLE);
    break;

  default:
    IOdebug("unknown system type %d", channel->dcb->systemType);
    break;
  }
#undef SCR
}

/*----------------------------------------------------------------------*/

word DevClose(SerialDCB *dcb)
{
  int i;

  Wait(&dcb->deviceLock);

#ifdef SERIALDEBUG
  /* IOdebug("serial close: log %d", dcb->logDev); */
#endif /* SERIALDEBUG */

  DisableIRQ(dcb->intSourceBit); /* Disable interrupts from serial device */

  /* Reset both channels and their control registers */
  for (i = 0; i < NCHANNELS; ++i)
  {
    SerialChannel *channel = &dcb->channel[i];

    InitControlRegs(channel); /* Reset channel and set default state */
    channel->flags = SCF_FIRSTREAD;
  }

  RemEvent(&dcb->intHandler);

  Free(dcb);  
  /* IOdebug("returning from DevClose"); */
  return Err_Null;
}

/*----------------------------------------------------------------------*/

void DevOperate(SerialDCB *dcb, SerialReq *req)
{
  word error    = 0;
  word function = req->DevReq.Request;
  int  chanNum  = (int)req->DevReq.SubDevice;

  /* IOdebug("DevOperate: request %x", req->DevReq.Request); */

  /* Make sure that the channel number is in range */
  if (chanNum < 0 || chanNum >= NCHANNELS)
    error = SERIAL_SERRORBADSUBDEVICE;
  else
  { 
    SerialChannel *channel = &dcb->channel[chanNum];

    if ((channel->flags & SCF_OPEN == 0) && (function != FS_OpenChannel))
      error = SERIAL_SERRORNOTOPEN; /* Insist channel is opened before use */
    else
    {
      switch (function)
      {
        case FG_Read:	
          Wait(&dcb->deviceLock);
	  error = SerialRead(dcb, req);
	  Signal(&dcb->deviceLock);
	  break;

	case FG_Write:
	  Wait(&dcb->deviceLock);
	  error = SerialWrite(dcb, req);
	  Signal(&dcb->deviceLock);
	  break;
      
        case FS_AbortRead:	
          Wait(&dcb->deviceLock);
	  error = AbortSerialRead(dcb, req);
	  Signal(&dcb->deviceLock);
	  break;

	case FS_AbortWrite:
	  Wait(&dcb->deviceLock);
	  error = AbortSerialWrite(dcb, req);
	  Signal(&dcb->deviceLock);
	  break;
      
	case FG_GetInfo:	
	  Wait(&dcb->deviceLock);
	  SerialGetInfo(channel, req);
	  Signal(&dcb->deviceLock);
	  break;
      
	case FG_SetInfo:	
	  Wait(&dcb->deviceLock);
	  SerialSetInfo(channel, req);
	  Signal(&dcb->deviceLock);
	  break;

#ifdef events      
	case FG_GetEvent:	
	  Wait(&dcb->deviceLock);
	  error = SerialGetEvent(dcb, req);
	  Signal(&dcb->deviceLock);
	  break;
#endif /* events */
      
	case FS_OpenChannel:
	  /* Start to use one channel of the serial chip */
	  Wait(&dcb->deviceLock);
	  if (channel->flags & SCF_OPEN)
	    error = SERIAL_SERRORINUSE;
	  else
	  {
	    ResetChannel(channel);
	    SAFESETFLAGS(dcb, channel->flags, SCF_OPEN | SCF_FIRSTREAD);
            SetControlBits(channel, 1, CR1_ALLRXINT1, 1); /* Enable rx ints */
            SetControlBits(channel, 3, CR3_RXENABLE, 1); /* Enable reception */
#ifdef RTSFUDGE2
	    SetControlBits(channel, 5, CR5_DTR, 1);
#else
	    /* Turn on DTR and RTS */
	    SetControlBits(channel, 5, CR5_DTR | CR5_RTS, 1);
#endif
	    SAFESETFLAGS(dcb, channel->flags, SCF_RTSON);
#if 1
	    /* Fudge for auto enable mode: set auto enable mode while	*/
	    /* the RTS control bit is 1, then set the RTS control bit	*/
	    /* to zero. According to table 5-5 in the 72001 data sheet,	*/
	    /* this should activate automatic control of the RTS output.*/
	    /* However, it doesn't seem to work....			*/

	    IOdebug("auto enable RTS fudge");
	    SetControlBits(channel, 3, CR3_AUTOENABLE, 1);
	    ClearControlBits(channel, 5, CR5_RTS, 1);
#endif
	  }
	  Signal(&dcb->deviceLock);
	  break;
      
	case FS_CloseChannel:
	  Wait(&dcb->deviceLock);
	  if (channel->flags & SCF_OPEN)
	    ResetChannel(channel); /* This will turn off DTR and RTS */
	  else
	    error = SERIAL_SERRORNOTOPEN;
	  Signal(&dcb->deviceLock);
	  break;
        
	case FS_GetNumLines: /* Return no. of channels this device supports */
	  Wait(&dcb->deviceLock);
	  req->Actual = NCHANNELS;
	  Signal(&dcb->deviceLock);
	  break;
      
	case FS_SetReg:	/* Set a control register (for debugging only) */
	  {
	    int    regNum = ((int)req->Size & 0x0F);
	    u_char value  = (int)req->Buf & 0xFF;

	    Wait(&dcb->deviceLock);
	    SetControlReg(channel, regNum, value, 1);
	    Signal(&dcb->deviceLock);
	    break;
	  }

	case FS_GetStatusReg: /* Read a status register (for debugging only) */
	  {
	    int    regNum = ((int)req->Size & 0x0F);

	    Wait(&dcb->deviceLock);
	    req->Actual = ReadStatusReg(channel, regNum);
	    Signal(&dcb->deviceLock);
	    break;
	  }

	case FS_GetControlReg: /* Read the soft copy of a control register */
	  {
	    int    regNum = ((int)req->Size & 0x0F);

	    Wait(&dcb->deviceLock);
	    req->Actual = dcb->channel[chanNum].controlCopy[regNum];
	    Signal(&dcb->deviceLock);
	    break;
	  }

	default:
	  error = EG_FnCode;
	  break;
      }
    }
  }
  
  req->DevReq.Result = error;      /* Set error/result */
  (*req->DevReq.Action)(dcb, req); /* Call back client's Action() routine */
}


/*----------------------------------------------------------------------*/

/************************************************************************/
/* Read routine								*/
/************************************************************************/

static int
SerialRead(SerialDCB *dcb, SerialReq *req)
{
  int           error      = 0;
  int           chanNum	   = (int)req->DevReq.SubDevice;
  SerialChannel *channel   = &dcb->channel[chanNum];
  u_char        *bufPtr    = req->Buf;
  u_char        *readPtr   = bufPtr;
  u_char        *readEnd   = readPtr + req->Size;
  u_char	*nextData;
  u_char	*nextFree;
  u_char	*rxBufEnd;
  int		freeBytes;
  word		timeout    = req->DevReq.Timeout;
  int		intsOn; /* PART OF FUDGE BELOW */
  
  /* Check no other read in progress */
  
  if (channel->readReq) 
  {
    error = SERIAL_SERRORINUSE;  /* Read already in progress */
    IOdebug("read already in progress");
    goto ReadExit;
  }

  /* First collect any bytes which have already been received into the	*/
  /* driver's private buffer. Disable receiver interrupts while doing	*/
  /* this so we can be sure that we have got everything from this 	*/
  /* buffer before waiting for the interrupt routine to get the rest.	*/
  /* There could be an interrupt latency problem here if the private	*/
  /* buffer is so large that the receiver's fifo can fill up while it 	*/
  /* is being copied. This could be solved by copying most of the data	*/
  /* before disabling interrupts, then entering the code below.		*/
  /* Interrupts must remain disabled up to the point where 		*/
  /* SchedulerDispatch() is called, so the interrupt routine cannot	*/
  /* attempt to resume this process before it has waited.		*/

  IntsOff(); /* FUDGE TO PREVENT THIS CODE FROM BEING DESCHEDULED */
  intsOn = 0;

  ClearControlBits(channel, 1, CR1_ALLRXINT1, intsOn); /* Disable rx ints */
  nextData = channel->nextData;
  nextFree = channel->nextFree;
  rxBufEnd = channel->rxBufEnd;

  while ((readPtr < readEnd) && (nextData != nextFree))
  {
    *readPtr++ = *nextData++;
    if (nextData >= rxBufEnd) nextData = channel->rxBuf; /* Circular buffer */
  }

  /* Update buffer pointer */
  channel->nextData = nextData; /* Foreground thread may update nextData */
  
  /* If overrun has occurred, return just the data we've already got	*/
  /* (without re-enabling receiver interrupts).				*/

  if (channel->flags & SCF_OVERRUN) goto ReadExit;

  /* Calculate number of free bytes in driver's private buffer */

  freeBytes = nextData - nextFree - 1;
  if (freeBytes < 0) freeBytes += RXBUFSIZE;

  /* If the RTS output was disabled, and the driver's private buffer is	*/
  /* no longer too full, it can be turned on again.			*/

  if ((channel->flags & SCF_RTSON) == 0 && (freeBytes > RTSTHRESHOLD))
  {
    SetControlBits(channel, 5, CR5_RTS, intsOn);
    channel->flags |= SCF_RTSON;
  }

  /* If we have issued an XOFF and the buffer is no longer too full,	*/
  /* we can issue the corresponding XON.				*/
  if (((channel->flags & SCF_XOFFSENT) != 0) && (freeBytes > XONTHRESHOLD))
  {
    channel->flags = (channel->flags | SCF_SENDXON) & ~SCF_XOFFSENT;

    /* Make sure the tx side is active if we need a character sent 	*/
    /* Disable transmission so it can be enabled after int is enabled! 	*/
    ClearControlBits(channel, 5, CR5_TXENABLE, intsOn);
    /* Enable tx interrupts and first tx interrupts */
    SetControlBits(channel, 1, CR1_TXINTENABLE | CR1_1STTXIENABLE, intsOn);
    /* Enable transmission AFTER enabling first tx interrupt */
    SetControlBits(channel, 5, CR5_TXENABLE, intsOn);
  }

  /* If the buffer is not yet full and the timeout is non-zero, wait	*/
  /* for the interrupt routine to receive the remaining bytes.		*/  

  if ((readPtr < readEnd) && (timeout > 0))
  {
    /* Receiver interrupts must be off here while we set things up for	*/
    /* the interrupt routine.						*/
    channel->readReq = req;     /* Store request (and lock out other reads) */
    channel->readPtr = readPtr; /* Write back the pointer 		  */
    channel->readEnd = readEnd;
    channel->flags |= SCF_READWAIT; /* Safe as ints still disabled */
    /* Implementation of non-zero timeouts TBD */
    SetControlBits(channel, 1, CR1_ALLRXINT1, intsOn); /* Enable rx ints */

    Signal(&dcb->deviceLock); /* Allow other processes in while we sleep    */
    			      /* Must be done after channel->readReq is set */

#ifdef TRACE
    TraceEvent(channel, 2, (int)channel->readLock.Count,
	       (int)channel->readLock.Head, (int)channel->readLock.Tail);
#endif

    /* Wait for int routine to fill buffer (or read to be aborted).	*/
    /* We must enter the scheduler with interrupts still disabled, so	*/
    /* the interrupt routine cannot run before this process sleeps.	*/

    SchedulerDispatch(&channel->readSaveState); 
    IntsOn();	/* Re-enable immediately process is resumed */

#ifdef TRACE
    TraceEvent(channel, 0xE, (int)channel->readLock.Count,
	       (int)channel->readLock.Head, (int)channel->readLock.Tail);
#endif

    Wait(&dcb->deviceLock);   /* Reclaim the lock on the physical device */
    readPtr = channel->readPtr; /* Get updated pointer */
  }
  else
    SetControlBits(channel, 1, CR1_ALLRXINT1, intsOn); /* Reenable rx ints */

  /* Must be holding dcb->deviceLock on arrival here */
ReadExit:
  IntsOn(); /* END OF FUDGE */
  if (channel->flags & SCF_OVERRUN) error = SERIAL_SERROROVERRUN;

#if 1
  if (channel->flags & SCF_HWOVERRUN) IOdebug("h/w overrun");
  if (channel->flags & SCF_SWOVERRUN) IOdebug("s/w overrun");
#endif

  channel->readReq = 0; /* Allow other reads */
  req->Actual = readPtr - bufPtr;
  return error;
}


/************************************************************************/
/* Write routine							*/
/************************************************************************/

static int
SerialWrite(SerialDCB *dcb, SerialReq *req)
{
  int           error      = 0;
  int           chanNum	   = (int)req->DevReq.SubDevice;
  SerialChannel *channel   = &dcb->channel[chanNum];
  u_char        *bufPtr    = req->Buf;
  u_char        *writePtr  = bufPtr;
  u_char        *writeEnd  = writePtr + req->Size;
  /* word		timeout    = req->DevReq.Timeout; */
  /* Write timeout TBD */
  
  /* Check no other write in progress */
  
  if (channel->writeReq) 
  {
    error = SERIAL_SERRORINUSE;  /* Write already in progress */
    goto WriteExit;
  }

  channel->writeReq = req;	/* Store request (and lock out other writes) */

  channel->writePtr = writePtr; /* Write back the pointer */

  /* If there is anything in the buffer, wait for the interrupt routine	*/
  /* to transfer it.							*/
  
  if (writePtr < writeEnd)
  {
    channel->writeEnd = writeEnd;
    /* Disable transmission so it can be enabled after int is enabled! */
    ClearControlBits(channel, 5, CR5_TXENABLE, 1);

    /* Interrupts must be disabled during the period between enabling	*/
    /* the device interrupt and entering the scheduler, so that the	*/
    /* interrupt routine cannot run until this process has slept 	*/
    /* (otherwise it could try to resume it too early).			*/

    IntsOff();

    /* Enable tx interrupts and first tx interrupts */
    SetControlBits(channel, 1, CR1_TXINTENABLE | CR1_1STTXIENABLE, 0);
    /* Enable transmission AFTER enabling first tx interrupt */

#ifdef RTSFUDGE2
    SetControlBits(channel, 5, CR5_TXENABLE | CR5_RTS, 0);
#else
    SetControlBits(channel, 5, CR5_TXENABLE, 0);
#endif
    Signal(&dcb->deviceLock); /* Allow other processes in while we sleep */

    /* Wait for int routine to empty buffer (or write to be aborted).	*/
    /* We must enter the scheduler with interrupts still disabled, so	*/
    /* the interrupt routine cannot run before this process sleeps.	*/

    channel->flags |= SCF_WRITEWAIT; /* Record that writeSaveState is valid */
    SchedulerDispatch(&channel->writeSaveState); 
    IntsOn();	/* Re-enable immediately process is resumed */

    /* Interrupt routine has now disabled transmission and tx interrupts */
    Wait(&dcb->deviceLock);       /* Reclaim the lock on the physical device */
    writePtr = channel->writePtr; /* Get updated pointer */
  }

  /* Must be holding dcb->deviceLock on arrival here */
WriteExit:
  req->Actual = writePtr - bufPtr;
  channel->writeReq = 0; /* Allow other writes */
  return error;
}

/************************************************************************/
/* GetInfo routine							*/
/************************************************************************/

static void
SerialGetInfo(SerialChannel *channel, SerialReq *req)
{
  Attributes *attr = (Attributes *)req->Buf;

  *attr = channel->attributes; /* Copy attributes structure into buffer */
}


/************************************************************************/
/* SetInfo routine							*/
/************************************************************************/

static void
SerialSetInfo(SerialChannel *channel, SerialReq *req)
{
  Attributes *attr = (Attributes *)req->Buf;

  /* IOdebug("SerialSetInfo"); */
  SetChannelAttributes(channel, attr);
}


/*----------------------------------------------------------------------*/

/************************************************************************/
/* Routines to abort read and write operations.				*/
/* These are provided primarily for use when a serial line is used as a	*/
/* Helios link.								*/
/* If a transfer was in progress it is halted and a pointer to the	*/
/* SaveState of the waiting process is returned in req->Actual; the	*/
/* process is not Resumed.						*/
/* If no transfer was in progress, req->Actual is set to zero.		*/
/************************************************************************/

static int
AbortSerialRead(SerialDCB *dcb, SerialReq *req)
{
  int           chanNum	   = (int)req->DevReq.SubDevice;
  SerialChannel *channel   = &dcb->channel[chanNum];

  /* Disable ints to make next part atomic. Disabling just the serial	*/
  /* device ints is no good, as this process may be descheduled, 	*/
  /* leaving them disabled for too long.				*/

  IntsOff();

  if (channel->flags & SCF_READWAIT)
  {
    /* Clear the SCF_READWAIT flag to prevent the rx interrupt routine	*/
    /* doing anything further with this request. (Any received bytes	*/
    /* will just go into the internal circular buffer.)			*/
    /* Clearing this flag also prevents this SaveState being returned	*/
    /* again.								*/

    channel->flags &= ~SCF_READWAIT;

    /* Return SaveState of waiting process */
    req->Actual = (word)&channel->readSaveState; 
  }
  else
    req->Actual = 0;	/* Nothing aborted so no SaveState to return */

  IntsOn();

  return 0; /* Always succeeds */
}


static int
AbortSerialWrite(SerialDCB *dcb, SerialReq *req)
{
  int           chanNum	   = (int)req->DevReq.SubDevice;
  SerialChannel *channel   = &dcb->channel[chanNum];

  /* Disable ints to make next part atomic. Disabling just the serial	*/
  /* device ints is no good, as this process may be descheduled, 	*/
  /* leaving them disabled for too long.				*/

  IntsOff();

  if (channel->flags & SCF_WRITEWAIT)
  {
    /* Clear the SCF_WRITEWAIT flag to prevent this SaveState being	*/
    /* returned again.							*/

    channel->flags &= ~SCF_WRITEWAIT;

    /* Disable transmitter interrupts to abort the transmission.	*/
    /* (Do not disable transmission in the chip as this can clip the	*/
    /* last character.)							*/

    ClearControlBits(channel, 1, CR1_TXINTENABLE | CR1_1STTXIENABLE, 0);

#ifdef RTSFUDGE
    ClearControlBits(channel, 5, CR5_RTS, 0); /* No longer want to transmit */
#endif

    /* Return SaveState of waiting process */
    req->Actual = (word)&channel->writeSaveState;
  }
  else
    req->Actual = 0;	/* Nothing aborted so no SaveState to return */

  IntsOn();

  return 0; /* Always succeeds */
}

/*----------------------------------------------------------------------*/

/************************************************************************/
/*			Attribute Handling				*/
/*									*/
/* There is a separate procedure for setting or clearing each family of	*/
/* attributes, normally invoked from SetChannelAttributes via		*/
/* UpdateAttribute below.						*/
/************************************************************************/

static void HandleIgnParAttr(SerialChannel *channel, Attribute attr, int set)
{
  channel = channel; attr = attr; set = set;
}

static void HandleParMrkAttr(SerialChannel *channel, Attribute attr, int set)
{
  channel = channel; attr = attr; set = set;
}

static void HandleInPckAttr(SerialChannel *channel, Attribute attr, int set)
{
  channel = channel; attr = attr; set = set;
}

static void HandleIXONAttr(SerialChannel *channel, Attribute attr, int set)
{
  channel = channel; attr = attr; set = set;
}

static void HandleIXOFFAttr(SerialChannel *channel, Attribute attr, int set)
{
  channel = channel; attr = attr; set = set;
}
      
static void HandleIstripAttr(SerialChannel *channel, Attribute attr, int set)
{
  channel = channel; attr = attr; set = set;
}

static void HandleIgnoreBreakAttr(SerialChannel *channel, Attribute attr,
				  int set)
{
  channel = channel; attr = attr; set = set;
}

static void HandleBreakIntAttr(SerialChannel *channel, Attribute attr, int set)
{
  channel = channel; attr = attr; set = set;
}

static void HandleCstopbAttr(SerialChannel *channel, Attribute attr, int set)
{
  channel = channel; attr = attr; set = set;
}

static void HandleCreadAttr(SerialChannel *channel, Attribute attr, int set)
{
  channel = channel; attr = attr; set = set;
}

static void HandleParEnbAttr(SerialChannel *channel, Attribute attr, int set)
{
  channel = channel; attr = attr; set = set;
}

static void HandleParOddAttr(SerialChannel *channel, Attribute attr, int set)
{
  channel = channel; attr = attr; set = set;
}

static void HandleHupClAttr(SerialChannel *channel, Attribute attr, int set)
{
  channel = channel; attr = attr; set = set;
}

static void HandleCLocalAttr(SerialChannel *channel, Attribute attr, int set)
{
  channel = channel; attr = attr; set = set;
}

static void HandleCsizeAttr(SerialChannel *channel, Attribute attr, int set)
{
  if (set)
  {
    int rxSize, txSize;

    ClearControlBits(channel, 3, CR3_RXCHARLEN, 1); /* Clear size field */
    ClearControlBits(channel, 5, CR5_TXCHARLEN, 1);

    switch (attr)
    {
      case RS232_Csize_5: rxSize = CR3_5BITS; txSize = CR5_5BITS; break;
      case RS232_Csize_6: rxSize = CR3_6BITS; txSize = CR5_6BITS; break;
      case RS232_Csize_7: rxSize = CR3_7BITS; txSize = CR5_7BITS; break;
      case RS232_Csize_8: rxSize = CR3_8BITS; txSize = CR5_8BITS; break;
    }

    SetControlBits(channel, 3, rxSize, 1);
    SetControlBits(channel, 5, txSize, 1);
  }
}

static void UpdateInputSpeed(SerialChannel *channel, Attributes *old)
{
  int oldSpeed = (int)GetInputSpeed(old);
  int newSpeed = (int)GetInputSpeed(&channel->attributes);

  if (oldSpeed != newSpeed)
  {
    int newBaudRate = SpeedAttributeToBaudRate(newSpeed);

    /* Leave the old speed set if the new one is invalid or zero */
    if (newBaudRate <= 0)
      SetInputSpeed(&channel->attributes, oldSpeed);
    else
      SetBaudRate(channel, Rx, newBaudRate);
  }
}

static void UpdateOutputSpeed(SerialChannel *channel, Attributes *old)
{
  int oldSpeed = (int)GetOutputSpeed(old);
  int newSpeed = (int)GetOutputSpeed(&channel->attributes);

  if (oldSpeed != newSpeed)
  {
    int newBaudRate = SpeedAttributeToBaudRate(newSpeed);

    /* Leave the old speed set if the new one is invalid or zero */
    if (newBaudRate <= 0)
      SetOutputSpeed(&channel->attributes, oldSpeed);
    else
      SetBaudRate(channel, Tx, newBaudRate);
  }
}


/************************************************************************/
/* Test whether a particular attribute value has changed, and if so	*/
/* call the supplied procedure to make the change. On entry, the new	*/
/* state of the attribute has already been set in the channel structure	*/
/* and the old set of attributes is provided as a parameter.		*/
/************************************************************************/

static void
UpdateAttribute(SerialChannel *channel, Attributes *oldAttrs,
		Attribute attr,
		void attrHandler(SerialChannel *, Attribute, 
				 int /* TRUE for set, FALSE for unset */))
{
  int setNow = (int)IsAnAttribute(&channel->attributes, attr);
  if (IsAnAttribute(oldAttrs, attr) != setNow)
    attrHandler(channel, attr, setNow); /* This attribute has changed */
}

/************************************************************************/
/* Apply the given set of attributes to a channel.			*/
/************************************************************************/
static void
SetChannelAttributes(SerialChannel *channel, Attributes *attr)
{
  Attributes old = channel->attributes;
  
  channel->attributes = *attr; /* Record new attribute set in the channel */
  
  /* Check each attribute in turn, and act on any which have changed. */

  UpdateAttribute(channel, &old, RS232_IgnPar,         HandleIgnParAttr);
  UpdateAttribute(channel, &old, RS232_ParMrk,         HandleParMrkAttr);
  UpdateAttribute(channel, &old, RS232_InPck,          HandleInPckAttr);
  UpdateAttribute(channel, &old, RS232_IXON,	       HandleIXONAttr);
  UpdateAttribute(channel, &old, RS232_IXOFF,	       HandleIXOFFAttr);      
  UpdateAttribute(channel, &old, RS232_Istrip,	       HandleIstripAttr);
  UpdateAttribute(channel, &old, RS232_IgnoreBreak,    HandleIgnoreBreakAttr);
  UpdateAttribute(channel, &old, RS232_BreakInterrupt, HandleBreakIntAttr);

  UpdateAttribute(channel, &old, RS232_Cstopb,	       HandleCstopbAttr);
  UpdateAttribute(channel, &old, RS232_Cread,	       HandleCreadAttr);
  UpdateAttribute(channel, &old, RS232_ParEnb,	       HandleParEnbAttr);
  UpdateAttribute(channel, &old, RS232_ParOdd,	       HandleParOddAttr);
  UpdateAttribute(channel, &old, RS232_HupCl,	       HandleHupClAttr);
  UpdateAttribute(channel, &old, RS232_CLocal,	       HandleCLocalAttr);

  UpdateAttribute(channel, &old, RS232_Csize_5,	       HandleCsizeAttr);
  UpdateAttribute(channel, &old, RS232_Csize_6,	       HandleCsizeAttr);
  UpdateAttribute(channel, &old, RS232_Csize_7,	       HandleCsizeAttr);
  UpdateAttribute(channel, &old, RS232_Csize_8,	       HandleCsizeAttr);

  UpdateInputSpeed(channel, &old);
  UpdateOutputSpeed(channel, &old);
}

/************************************************************************/
/* Convert a speed attribute value into a baud rate.			*/
/************************************************************************/
static int
SpeedAttributeToBaudRate(int speedAttr)
{
  switch(speedAttr)
  {	
    case RS232_B0:    	return     0; /* Also implies break should be sent */
    case RS232_B50:   	return	  50;
    case RS232_B75:   	return	  75;
    case RS232_B110:  	return	 110;
    case RS232_B134:  	return	 134;
    case RS232_B150:  	return	 150;
    case RS232_B200:  	return	 200;
    case RS232_B300:  	return	 300;
    case RS232_B600:  	return	 600;
    case RS232_B1200: 	return	1200;
    case RS232_B1800: 	return	1800;
    case RS232_B2400: 	return	2400;
    case RS232_B4800: 	return	4800;
    case RS232_B9600: 	return	9600;
    case RS232_B19200:	return 19200;
    case RS232_B38400:	return 38400;
  }

  return -1; /* invalid value */
}

/************************************************************************/
/* Set up the default attributes for a channel.				*/
/************************************************************************/
static void
SetDefaultAttributes(SerialChannel *channel)
{
  Attributes attrs;
  
  attrs.Input   = 0; /* Static initialisation doesn't work with -Zr ncc opt */
  attrs.Output  = 0;
  attrs.Control = 0;
  attrs.Local   = 0;
  attrs.Min     = 0;
  attrs.Time    = 0;

  channel->attributes = attrs; /* Start with null set of attributes */
  
  SetInputSpeed(&attrs, RS232_B9600);
  SetOutputSpeed(&attrs, RS232_B9600);
  AddAttribute(&attrs, RS232_Csize_8);
  AddAttribute(&attrs, RS232_IgnPar);
  AddAttribute(&attrs, RS232_IXON);
  AddAttribute(&attrs, RS232_IXOFF);
  AddAttribute(&attrs, RS232_CLocal);
  AddAttribute(&attrs, RS232_BreakInterrupt);

  SetChannelAttributes(channel, &attrs);
}

/************************************************************************/
/* Find the type of system on which this driver is running.		*/
/************************************************************************/
static int
FindSystemType(void)
{
  word outRegs[11];

#define EXEC_VERSION_SWI 0x0030001F	/* Should not be defined here! */

  IOdebug("Bodge: assuming exec_Version is SWI &%x", EXEC_VERSION_SWI);

  if (execSWI(EXEC_VERSION_SWI, outRegs, outRegs) == 0)
  {
#if 0
    IOdebug("version %x, name '%s', timestamp %x", 
	    outRegs[0], outRegs[1], outRegs[2]);
#endif
    return (int)((outRegs[0] >> 16) & 0xFF); /* Return h/w identifier field */
  }
  else
  {
    IOdebug("serialdev: exec_Version SWI failed - assuming Heval board");
    return SYSTEMTYPE_HEVAL;
  }
}

/*----------------------------------------------------------------------*/

/************************************************************************/
/*			Interrupt routines				*/
/************************************************************************/

/************************************************************************/
/* Low-level device interrupt routine.  It is called from the executive */
/* and passed the DCB address.						*/
/************************************************************************/

int SerialInt(SerialDCB *dcb)
{
  /* Interrupts pending register */
  u_char sr4a = ReadStatusReg(&dcb->channel[CHANNELA], 4);
  
  /* Determine which interrupts are outstanding from flags in SR4A */
  
  if (sr4a & SR4A_ARXINT)  RxInterrupt(dcb, CHANNELA);
  if (sr4a & SR4A_BRXINT)  RxInterrupt(dcb, CHANNELB);
  if (sr4a & SR4A_ATXINT)  TxInterrupt(dcb, CHANNELA);
  if (sr4a & SR4A_BTXINT)  TxInterrupt(dcb, CHANNELB);
  if (sr4a & SR4A_AESINT)  ESInterrupt(dcb, CHANNELA);
  if (sr4a & SR4A_BESINT)  ESInterrupt(dcb, CHANNELB);
  if (sr4a & SR4A_ASPECRX) SpecRxInterrupt(dcb, CHANNELA);
  if (sr4a & SR4A_BSPECRX) SpecRxInterrupt(dcb, CHANNELB);

  /* Notify controller that interrupt servicing is complete via CR0A */
  if (sr4a) SetControlReg(&dcb->channel[CHANNELA], 0, CR0_ENDOFINT, 0);
  return sr4a; /* Return 0 if not my interrupt */
}

/************************************************************************/
/* Routine called from RxInterrupt to handle a received XON or XOFF	*/
/* character.						      		*/
/************************************************************************/
static void
XonOrXoffReceived(SerialChannel *channel, u_char ch)
{
  if (ch == XON)
  {
    int wasStopped = channel->flags & SCF_XOFFREC;
    channel->flags &= ~SCF_XOFFREC; /* Allow tx side to start again */
    
    if (wasStopped)
    {
      /* Set tx side going again (and expect it to deal correctly	*/
      /* with an unwanted interrupt).				*/
      /* Disable tx so it can be enabled after int is enabled! */
      ClearControlBits(channel, 5, CR5_TXENABLE, 0);
      /* Enable tx interrupts and first tx interrupts */
      SetControlBits(channel, 1, CR1_TXINTENABLE | CR1_1STTXIENABLE, 0);
      /* Enable transmission AFTER enabling first tx interrupt */
      SetControlBits(channel, 5, CR5_TXENABLE, 0);
    }
  }
  else if (ch == XOFF)
  {
    channel->flags |= SCF_XOFFREC; /* Ask tx side to stop sending */
  }
}

#ifdef DEBUG
/************************************************************************/
/* Check the validity of a semaphore.					*/
/************************************************************************/

static void
CheckSemaphore(Semaphore *sema)
{
  word count = sema->Count;
  word *head = (word *)sema->Head;
  word *tail = (word *)sema->Tail;
}
#endif

/************************************************************************/
/* Rx interrupt routine							*/
/*									*/
/* This is called when there is at least one character in the rx buffer.*/
/* If there is a request outstanding, then its buffer is used first. If */
/* there is no request (or its buffer is full), characters are put in	*/
/* the driver's private circular buffer.				*/
/* Overrun flags are set if either the hardware fifo or the circular	*/
/* buffer overflows.							*/
/************************************************************************/

static void 
RxInterrupt(SerialDCB *dcb, int chanNum)
{
  SerialChannel *channel   = &dcb->channel[chanNum];
  vu_char       *statusReg = channel->conStatReg;
  vu_char       *dataReg   = channel->dataReg;
  int		reqWaiting = (channel->readReq != 0);
  int 		mayBeMore  = 1; /* True if may be more in fifo */
  int		freeBytes;
  word		outputFC   = IsAnAttribute(&channel->attributes, RS232_IXON);
  word		inputFC	   = IsAnAttribute(&channel->attributes, RS232_IXOFF);
  int		charsGot   = 0; /* Chars read on this interrupt (debugging) */

  /* Flag overrun if reception FIFO has filled up */

  if (*statusReg & SR0_RXOVERRUN)
  { 
    channel->flags |= SCF_HWOVERRUN;
  }

  /* Fill the request buffer (if any) first. */
   
  if (reqWaiting && ((channel->flags & SCF_READWAIT) != 0))
  {
    /* The circular buffer should be empty at this point. (Could check!)*/
    /* Read data until the request buffer is full or the fifo is empty.	*/
    u_char *readPtr = channel->readPtr;
    u_char *readEnd = channel->readEnd;

    /* Look for XON and XOFF chars if output flow control is enabled */
    if (outputFC)
    {
      while ((*statusReg & SR0_RXDATA) && (readPtr < readEnd))
      {
	u_char ch = *dataReg;
	++charsGot;

	if ((ch == XON) || (ch == XOFF))
	  XonOrXoffReceived(channel, ch);
	else
	  *readPtr++ = ch;
      }
    }
    else /* Output flow control disabled */
    {
      while ((*statusReg & SR0_RXDATA) && (readPtr < readEnd))
	{ *readPtr++ = *dataReg; ++charsGot; }
    }

    channel->readPtr = readPtr; /* Write back buffer pointer */

    /* If the buffer is full or overrun has occurred, signal the	*/
    /* foreground thread of the driver.					*/

    if ((readPtr >= readEnd) || (channel->flags & SCF_OVERRUN))
    {
      channel->flags &= ~SCF_READWAIT; /* Ensure only one Resume per wait */
#ifdef DEBUG
      CheckSemaphore(&channel->readLock);
#endif
#ifdef TRACE
      TraceEvent(channel, 6, (int)channel->readLock.Count,
		 (int)channel->readLock.Head, (int)channel->readLock.Tail);
#endif
      channel->flags &= ~SCF_READWAIT; /* readSaveState is no longer valid */
      Resume(&channel->readSaveState); /* Wake up foreground driver thread */
    }
    else
      mayBeMore = 0; /* Request buffer was not filled, so no more */
  }
  
  /* Put any remaining characters in the private circular buffer.	*/
  /* `mayBeMore' is used to avoid this buffer catching a byte which	*/
  /* arrives just after the request buffer loop has terminated.		*/
  
  if (mayBeMore)
  {
    u_char *nextData = channel->nextData;
    u_char *nextFree = channel->nextFree;
    u_char *rxBufEnd = channel->rxBufEnd;

    while (*statusReg & SR0_RXDATA)
    {
      u_char ch = *dataReg; ++charsGot;

      /* Intercept flow control chars if required */
      if (outputFC && ((ch == XON) || (ch == XOFF)))
	XonOrXoffReceived(channel, ch);
      else
      {
	u_char *nextNextFree = nextFree + 1;
      
	if (nextNextFree >= rxBufEnd) nextNextFree = &channel->rxBuf[0];
	if (nextNextFree == nextData)
	{
	  channel->flags |= SCF_SWOVERRUN; /* Circular buff was already full */
	  break;
	}

	*nextFree = ch;
	nextFree  = nextNextFree;
      
	/* Determine how much space is left in the circular buffer	*/
	/* to see if any flow control is needed.			*/
	freeBytes = nextData - nextFree - 1;
	if (freeBytes < 0) freeBytes += RXBUFSIZE;
#ifdef DEBUG
        *(int *)0x758300 = 0xFFFFFFFF << (freeBytes/5);
#endif
	if ((freeBytes <= RTSTHRESHOLD) && (channel->flags & SCF_RTSON))
	{
	  ClearControlBits(channel, 5, CR5_RTS, 0);
	  channel->flags &= ~SCF_RTSON;
	}

	/* Handle Xon/Xoff flow control on input if enabled */
	if (inputFC)
	{
	  int charToSend = 0;

	  if (freeBytes <= XOFFTHRESHOLD)
	  {
	    /* Send XOFF for every excess character */
	    charToSend = 1;
	    channel->flags |= (SCF_SENDXOFF | SCF_XOFFSENT);
	  }

	  /* Make sure the tx side is active if we need a character sent */
	  if (charToSend)
	  {
	    /* Disable tx so it can be enabled after int is enabled! */
	    ClearControlBits(channel, 5, CR5_TXENABLE, 0);
	    /* Enable tx interrupts and first tx interrupts */
	    SetControlBits(channel, 1, CR1_TXINTENABLE | CR1_1STTXIENABLE, 0);
	    /* Enable transmission AFTER enabling first tx interrupt */
	    SetControlBits(channel, 5, CR5_TXENABLE, 0);
	  }
	}
      }
    }

    /* Write back the end-of-buffer pointer. Only the interrupt routine	*/
    /* updates this, to avoid races.					*/
    channel->nextFree = nextFree; 
  }

#ifdef DEBUG
  /* Display a simple bar showing number of chars got in this int */
  *(int *)0x758200 = (*(int *)0x758200 >> 1) | 
                     (0xFFFFFFFF >> (charsGot*8));
#endif

  /* Write to unusual location to trigger logic analyser on late interrupt */
  /* if (charsGot > 1) *(int *)0x74204C = 0xAAAAAAAA; */
  
  /* Disable reception, receiver interrupts and RTS output if		*/
  /* either form of overrun has occurred.				*/

  if (channel->flags & SCF_OVERRUN)
  {
    ClearControlBits(channel, 1, CR1_ALLRXINT1, 0);
    ClearControlBits(channel, 3, CR3_RXENABLE,  0);
    ClearControlBits(channel, 5, CR5_RTS,       0);
    channel->flags &= ~SCF_RTSON;
  }
}

/************************************************************************/
/* Transmission interrupt routine					*/
/*									*/
/* This is called when the tx buffer is not full and either there are	*/
/* client data bytes to be sent, or an Xon or Xoff flow-control		*/
/* character is needed.			 				*/
/************************************************************************/

static void 
TxInterrupt(SerialDCB *dcb, int chanNum)
{
  SerialChannel *channel   = &dcb->channel[chanNum];
  u_char        *writePtr  = channel->writePtr;
  u_char        *writeEnd  = channel->writeEnd;
  vu_char       *statusReg = channel->conStatReg;
  vu_char       *dataReg   = channel->dataReg;
  int		dataCharSent = 0; /* Genuine data char transmitted */
  int		moreData     = 0; /* Some data still to be sent    */

  /* First see if we need to issue a flow control character.		*/
  /* If both Xoff & Xon are wanted, send Xon only and clear both flags.	*/
  /* There must be one slot free in the tx buffer after an interrupt.	*/

  if (channel->flags & (SCF_SENDXON | SCF_SENDXOFF))
  {
    *dataReg = ((channel->flags & SCF_SENDXON) ? XON : XOFF); /* Send 1 char */
    channel->flags &= ~(SCF_SENDXON | SCF_SENDXOFF); /* Clear both flags */
  }

  if (writePtr >= writeEnd) goto WriteIntExit; /* No data to send */

#ifdef RTSFUDGE2
  /* Set RTS control bit before writing tx data to allow auto enable	*/
  /* mode to work 							*/
  SetControlBits(channel, 5, CR5_RTS, 0);
#endif

  /* Write data until the buffer is empty or the fifo is full.    */

  while ((*statusReg & SR0_TXBUFEMPTY) && (writePtr < writeEnd))
    *dataReg = *writePtr++;

  channel->writePtr = writePtr; /* Write back buffer pointer    */
  dataCharSent      = 1;

#ifdef RTSFUDGE2
  /* Clear RTS control bit so chip can turn off RTS output when		*/
  /* `all sent' becomes true.						*/
  ClearControlBits(channel, 5, CR5_RTS, 0);
#endif

  /* If the transfer is not complete, return leaving the interrupt */
  /* enabled and without waking up the top half.		   */

  if (writePtr < writeEnd) moreData = 1;

  /* Common exit: `moreData' is set if the transfer is incomplete */

WriteIntExit:
  if (!moreData)
  {
    /* Disable transmitter interrupts but not transmission */
    ClearControlBits(channel, 1, CR1_TXINTENABLE | CR1_1STTXIENABLE, 0);
    /* ClearControlBits(channel, 5, CR5_TXENABLE, 0); zaps last char */

#if 0
    /* Fudge: poll waiting for `all sent' flag to come on */
    {
      int i = 0;
      while ((ReadStatusReg(channel, 1) & SR1_ALLSENT) == 0)
	*(int *)0x021E7100 = ++i;

#if 0
      /* Wait a similar time in case `all sent' comes on too early */
      for (i = 0; i < 512; ++i)
      {
	ReadStatusReg(channel, 1);
	*(int *)0x021E7104 = i;
      }
#endif
    }

    ClearControlBits(channel, 5, CR5_RTS, 0); /* No longer want to transmit */
#endif

    if (dataCharSent)
    {
      channel->flags &= ~SCF_WRITEWAIT; /* writeSaveState is no longer valid */
      Resume(&channel->writeSaveState); /* Wake up top half of driver */
    }
  }
}

/************************************************************************/
/* External/Status interrupt routine					*/
/*									*/
/* This is called when one of the E/S flags changes state.		*/
/************************************************************************/

static void 
ESInterrupt(SerialDCB *dcb, int chanNum)
{
  SerialChannel *channel   = &dcb->channel[chanNum];

  SetControlReg(channel, 0, CR0_RESETESLATCH, 0);
  /* ESInterrupt TBD */
}


/************************************************************************/
/* Special Rx Condition interrupt routine				*/
/*									*/
/* This is called when there is at least one character in the rx buffer.*/
/************************************************************************/

static void 
SpecRxInterrupt(SerialDCB *dcb, int chanNum)
{
  SerialChannel *channel   = &dcb->channel[chanNum];

  channel = channel; /* To suppress compiler warning */
  /* SpecRxInterrupt TBD */
}

/*----------------------------------------------------------------------*/

/************************************************************************/
/* Control Register routines						*/
/*									*/
/* Update a write-only control register and its soft copy.		*/
/* The caller should have claimed dcb->deviceLock beforehand to prevent */
/* clashes with other foreground processes.                             */
/* All the registers are mapped to the same byte. To access registers	*/
/* other than 0, the register number is first written to register 0 and	*/
/* then the second write will go to the specified register.		*/
/*									*/
/* THESE FUNCTIONS MUST NOT BE USED IN INTERRUPT ROUTINES AS THEY STAND.*/
/* Two problems:							*/
/*  i) Write only regs => 2 stage operation (soft copy, real reg)	*/
/*  ii) Accessing the registers is a 2-stage operation (write reg 	*/
/*      number, write register)						*/
/************************************************************************/

/*----------------------------------------------------------------------*/
/* Clear some bits and set others. This is useful for setting the value	*/
/* of a multi-bit field within a register.				*/
/* The `foreground' parameter should be set non-zero when this is	*/
/* called from a foreground thread to protect against simultaneous	*/
/* update in interrupt routines. It must be zero when this is called	*/
/* from an interrupt routine.						*/
/*----------------------------------------------------------------------*/
static void
ChangeControlBits(SerialChannel *channel, int reg, 
		  u_char bitsToClear, u_char bitsToSet, int foreground)
{
  SerialDCB	*dcb = channel->dcb;
  u_char        value;

  /* It would be better to disable only interrupts from this device 	*/
  /* within the critical region. However, this involves two writes to	*/
  /* control registers, so cannot be done atomically!			*/
  if (foreground) DisableIRQ(dcb->intSourceBit); /* Lock out int routines */
  value = channel->controlCopy[reg];
  value &= ~bitsToClear;
  value |= bitsToSet;
  channel->controlCopy[reg] = value; /* Set soft copy */

  if (reg) 
    *channel->conStatReg = reg; /* Set (non-0) reg number for next access */
  *channel->conStatReg = value; /* Put value in that register     	  */
#if 0
  if (foreground)
    IOdebug("set CR%d to %x", reg, value);
#endif
  if (foreground) EnableIRQ(dcb->intSourceBit);
}


/* Set the whole register */
static void 
SetControlReg(SerialChannel *channel, int reg, u_char value, int foreground)
{
  ChangeControlBits(channel, reg, 0xFF, value, foreground);
}

/* Set the specified bits */
static void
SetControlBits(SerialChannel *channel, int reg, u_char bits, int foreground)
{
  ChangeControlBits(channel, reg, 0, bits, foreground);
}

/* Clear the specified bits */
static void
ClearControlBits(SerialChannel *channel, int reg, u_char bits, int foreground)
{
  ChangeControlBits(channel, reg, bits, 0, foreground);
}

/*----------------------------------------------------------------------*/
/* Set the baud rate for one data direction of the given channel.	*/
/* Assumes that the clock rate is already set in CR4.			*/
/* Must not be called from interrupt routines.				*/
/*----------------------------------------------------------------------*/
static void
SetBaudRate(SerialChannel *channel, TxOrRx direction, int baudRate)
{
  int           timeConstant;	/* 16-bit value to go in BRG count register */
  int           clockRate;	/* Clock rate factor from CR4 */
  u_char        setBRGBit = (direction == Rx ? CR12_RXBRGSET : CR12_TXBRGSET);
  u_char        enableBit = (direction == Rx ? CR14_RXBRGENABLE 
			                     : CR14_TXBRGENABLE);
  vu_char       *conStatReg = channel->conStatReg;
  vu_char       dummy;
  SerialDCB	*dcb = channel->dcb;

  /* IOdebug("SetBaudRate %s %d", (direction == Tx ? "tx" : "rx"), baudRate);*/
  DisableIRQ(dcb->intSourceBit); /* Make update sequence atomic */
  switch (channel->controlCopy[4] & CR4_CLOCKRATE)
  {
    case CR4_TIMES1:	clockRate = 1;  break;
    case CR4_TIMES16:	clockRate = 16; break;
    case CR4_TIMES32:	clockRate = 32; break;
    case CR4_TIMES64:	clockRate = 64; break;
  }

  timeConstant = channel->dcb->clockFreq / (2 * baudRate * clockRate) - 2;
  /* IOdebug("time constant 0x%x", timeConstant); */
  ClearControlBits(channel, 14, enableBit, 0); /* Disable the BRG     */
  SetControlBits(channel, 12, setBRGBit, 0);   /* Command to set BRG  */
  *conStatReg = timeConstant & 0xFF;	       /* Send low byte first */
  *conStatReg = (timeConstant >> 8) & 0xFF;
  dummy       = *conStatReg; /* Dummy read to revert to CR0 (bug in chip) */
  ClearControlBits(channel, 12, setBRGBit, 0); /* Clear bit in soft copy! */
  SetControlBits(channel, 14, enableBit, 0);   /* Reenable the BRG    */
  EnableIRQ(dcb->intSourceBit);
}

/*----------------------------------------------------------------------*/

/************************************************************************/
/* Status Register routines						*/
/************************************************************************/

static u_char
ReadStatusReg(SerialChannel *channel, int reg)
{
  if (reg) 
    *channel->conStatReg = reg; /* Set (non-0) reg number for next access */
  return *channel->conStatReg;  /* Return status register contents	  */
}

/*----------------------------------------------------------------------*/

/* End of serialdev.c */
