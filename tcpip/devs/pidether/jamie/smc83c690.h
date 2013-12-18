/*> smc83c690.h <*/
/* Hardware description of the SMC 83C690 Ethernet LAN Controller.
 *
 * $Revision: 1.2 $
 *   $Author: jsmith $
 *     $Date: 93/04/02 17:31:34 $
 *
 * Copyright (c) 1993, VLSI Technology Inc. All Rights Reserved.
 */
/*---------------------------------------------------------------------------*/

#ifndef __smc83c690_h
#define __smc83c690_h

/*---------------------------------------------------------------------------*/
/* The 83C690 Ethernet LAN Controller is designed to interface with networks
 * such as Ethernet, Cheapernet and StarLAN. It is functionally similar to
 * the National DP8390 device, but provides several new registers.
 *
 * Features:
 *	IEEE 802.3 protocol for networks (Ethernet, Cheapernet, StarLAN).
 *	DMA between memory and host.
 *	Programmable wait-states and slot times.
 *	Full duplex loopback capability.
 *	Supports physical, promiscuous and broadcast address filtering.
 */
/*---------------------------------------------------------------------------*/
/* The LAN registers are mapped into the register space in four (4) banks.
 * Unfortunately, a register may be mapped into a different bank depending
 * on wether it is being read or written.
 */

typedef struct
{
 vubyte COMMAND ; const vubyte _pad0[3] ;
 vubyte TRINCRL ; const vubyte _pad1[3] ;
 vubyte TRINCRH ; const vubyte _pad2[3] ;
 vubyte BOUND ;   const vubyte _pad3[3] ;
 vubyte TSTAT ;   const vubyte _pad4[3] ;
 vubyte COLCNT ;  const vubyte _pad5[3] ;
                  const vubyte _pad6[4] ;
 vubyte INTSTAT ; const vubyte _pad7[3] ;
                  const vubyte _pad8[4] ;
                  const vubyte _pad9[4] ;
                  const vubyte _padA[4] ;
                  const vubyte _padB[4] ;
 vubyte RSTAT ;   const vubyte _padC[3] ;
 vubyte ALICNT ;  const vubyte _padD[3] ;
 vubyte CRCCNT ;  const vubyte _padE[3] ;
 vubyte MPCNT ;   const vubyte _padF[3] ;
} smc83c690_page0_read ;

typedef struct
{
 vubyte COMMAND ; const vubyte _pad0[3] ;
 vubyte RSTART ;  const vubyte _pad1[3] ;
 vubyte RSTOP ;   const vubyte _pad2[3] ;
 vubyte BOUND ;   const vubyte _pad3[3] ;
 vubyte TSTART ;  const vubyte _pad4[3] ;
 vubyte TCNTL ;   const vubyte _pad5[3] ;
 vubyte TCNTH ;   const vubyte _pad6[3] ;
 vubyte INTSTAT ; const vubyte _pad7[3] ;
                  const vubyte _pad8[4] ;
                  const vubyte _pad9[4] ;
                  const vubyte _padA[4] ;
                  const vubyte _padB[4] ;
 vubyte RCON ;    const vubyte _padC[3] ;
 vubyte TCON ;    const vubyte _padD[3] ;
 vubyte DCON ;    const vubyte _padE[3] ;
 vubyte INTMASK ; const vubyte _padF[3] ;
} smc83c690_page0_write ;

typedef struct
{
 vubyte COMMAND ; const vubyte _pad0[3] ;
 vubyte STA0 ;    const vubyte _pad1[3] ;
 vubyte STA1 ;    const vubyte _pad2[3] ;
 vubyte STA2 ;    const vubyte _pad3[3] ;
 vubyte STA3 ;    const vubyte _pad4[3] ;
 vubyte STA4 ;    const vubyte _pad5[3] ;
 vubyte STA5 ;    const vubyte _pad6[3] ;
 vubyte CURR ;    const vubyte _pad7[3] ;
                  const vubyte _pad8[4] ;
                  const vubyte _pad9[4] ;
                  const vubyte _padA[4] ;
                  const vubyte _padB[4] ;
                  const vubyte _padC[4] ;
                  const vubyte _padD[4] ;
                  const vubyte _padE[4] ;
                  const vubyte _padF[4] ;
} smc83c690_page1 ;

typedef struct
{
 vubyte COMMAND ; const vubyte _pad0[3] ;
 vubyte RSTART ;  const vubyte _pad1[3] ;
 vubyte RSTOP ;   const vubyte _pad2[3] ;
                  const vubyte _pad3[4] ;
 vubyte TSTART ;  const vubyte _pad4[3] ;
 vubyte NEXT ;    const vubyte _pad5[3] ;
 vubyte BLOCK ;   const vubyte _pad6[3] ;
 vubyte ENH ;     const vubyte _pad7[3] ;
                  const vubyte _pad8[4] ;
                  const vubyte _pad9[4] ;
                  const vubyte _padA[4] ;
                  const vubyte _padB[4] ;
 vubyte RCON ;    const vubyte _padC[3] ;
 vubyte TCON ;    const vubyte _padD[3] ;
 vubyte DCON ;    const vubyte _padE[3] ;
 vubyte INTMASK ; const vubyte _padF[3] ;
} smc83c690_page2_read ;

typedef struct
{
 vubyte COMMAND ; const vubyte _pad0[3] ;
 vubyte TRINCRL ; const vubyte _pad1[3] ;
 vubyte TRINCRH ; const vubyte _pad2[3] ;
                  const vubyte _pad3[4] ;
                  const vubyte _pad4[4] ;
 vubyte NEXT ;    const vubyte _pad5[3] ;
 vubyte BLOCK ;   const vubyte _pad6[3] ;
 vubyte ENH ;     const vubyte _pad7[3] ;
                  const vubyte _pad8[4] ;
                  const vubyte _pad9[4] ;
                  const vubyte _padA[4] ;
                  const vubyte _padB[4] ;
                  const vubyte _padC[4] ;
                  const vubyte _padD[4] ;
                  const vubyte _padE[4] ;
                  const vubyte _padF[4] ;
} smc83c690_page2_write ;

typedef struct
{
 vubyte COMMAND ; const vubyte _pad0[3] ;
 vubyte TEST ;    const vubyte _pad1[3] ;
                  const vubyte _pad2[4] ;
                  const vubyte _pad3[4] ;
                  const vubyte _pad4[4] ;
                  const vubyte _pad5[4] ;
                  const vubyte _pad6[4] ;
                  const vubyte _pad7[4] ;
                  const vubyte _pad8[4] ;
                  const vubyte _pad9[4] ;
                  const vubyte _padA[4] ;
                  const vubyte _padB[4] ;
                  const vubyte _padC[4] ;
                  const vubyte _padD[4] ;
                  const vubyte _padE[4] ;
                  const vubyte _padF[4] ;
} smc83c690_page3 ;

typedef struct
{
 union
 {
  smc83c690_page0_read  p0r ;
  smc83c690_page0_write p0w ;
  smc83c690_page1       p1r ;
  smc83c690_page1       p1w ;
  smc83c690_page2_read  p2r ;
  smc83c690_page2_write p2w ;
  smc83c690_page3       p3r ;
  smc83c690_page3       p3w ;
 } u ;
} smc83c690 ;

/*---------------------------------------------------------------------------*/
/*-- alphabetical register listing ------------------------------------------*/
/*---------------------------------------------------------------------------*/
/* ALICNT : Alignment error counter
 * It is incremented by the receive unit when a packet is received with a
 * frame alignment error. The counter increments to 0xFF, and then stops.
 * It is cleared back to 0x00 when it is read.
 */
/*---------------------------------------------------------------------------*/
/* BLOCK : Page memory address register
 * The contents of this register are used as A16-23 during each memory
 * transfer cycle.
 */
/*---------------------------------------------------------------------------*/
/* BOUND : Buffer ring boundary
 * To prevent overflow in the buffer ring, the receive boundary page register
 * points to the oldest receive buffer. The DMA engine compares the contents
 * of this register against the contents of the next buffer address. If the
 * contents match then the DMA operation is aborted. Because all buffers are
 * aligned on 256byte boundaries this register only contains bits for A8-15.
 */
/*---------------------------------------------------------------------------*/
/* COLCNT : Collision counter
 * This register contains the number of collisions detected during the
 * transmission of the current (or most recent) packet. It is cleared at
 * the start of a transmission. If more than 15 collisions are detected
 * then the TSTAT_ABORT bit is set, and the count reset.
 */
/*---------------------------------------------------------------------------*/
/* COMMAND : device control register
 * This register is used to initialise the device and control transmissions.
 */

#define COMMAND_PS_mask	(0xC0)	/* register page select */
/* These 2bits control which register bank is currently visible */

#define COMMAND_TXP (1 << 2) /* Transmit packet */
/* This bit should be set after the transmit buffer and control
 * registers have been initialised. The bit is cleared upon completion
 * or abortion of the transmission. The host can clear the bit by
 * entering test-mode and explicitly writing this bit.
 */

#define COMMAND_STA (1 << 1) /* Start */
/* This bit is set to start normal Rx/Tx operation. Once set the
 * device will remain active until a reset or Stop event occurs.
 */

#define COMMAND_STP (1 << 0) /* Stop */
/* This bit when set will take the device offline. It will then finish
 * partially completed Rx/Tx operations, before resetting the hardware.
 */

/*---------------------------------------------------------------------------*/
/* CRCCNT : CRC error counter
 * This register is incremented by the receive unit when a packet with a
 * CRC error is received. The counter increments until 0xFF and then stops.
 * The value is reset to 0x00 when the register is read.
 */
/*---------------------------------------------------------------------------*/
/* CURR : Current frame buffer address
 * This register is used to reference the first buffer used to store the
 * current frame. The register should be initialised after the device has
 * been reset and then not touched by the host software, unless the ring
 * buffer overflows. Note: because all buffers are aligned on 256byte
 * boundaries this register only specifies A8-15.
 */
/*---------------------------------------------------------------------------*/
/* DCON : Data configuration register
 * This defines the characteristics of the memory interface.
 */

#define DCON_BSIZE_mask (0x60) /* Burst size mask */
/* These 2bits define the length of DMA bursts and the FIFO threshold at
 * which they are triggered.
 *	BSIZE	BURST	  RxTRIGGER   TxTRIGGER
 *	-----	-----     ---------   ---------
 *	0x0	2bytes    >=2	      <=4
 *	0x1	4bytes    >=4	      <=12
 *	0x2	8bytes    >=8	      <=8
 *	0x3	12bytes   >=12	      <=4
 */

#define DCON_BUS16 (1 << 0) /* 0=8bit; 1=16bit; DMA data transfers */

/*---------------------------------------------------------------------------*/
/* ENH : Enable hardware features
 * This register is used to enable features that are unique to the 83C690
 */

#define ENH_WAIT_mask (0xC0) /* wait-states */
/* This 2bit field defines the default number of wait-states that are
 * inserted into every DMA cycle.
 */
#define ENH_WAIT_0 (0x00) /* 0 wait-states */
#define ENH_WAIT_1 (0x40) /* 1 wait-state */
#define ENH_WAIT_2 (0x80) /* 2 wait-states */
#define ENH_WAIT_3 (0xC0) /* 3 wait-states */

#define ENH_SLOT_mask (0x18) /* slot-time */
/* This 2bit field selects the slot time used by the device */
#define ENH_SLOT_512	 (0x00) /* 512 bit-times : Ethernet, StarLAN */
#define ENH_SLOT_512_ALT (0x08) /* ditto */
#define ENH_SLOT_256	 (0x10) /* 256 bit-times */
#define ENH_SLOT_1024	 (0x18) /* 1024 bit-times */

/*---------------------------------------------------------------------------*/
/* INTMASK : Interrupt source mask
 * This register is used to mask interrupt sources. A mask bit of 1 allows
 * the interrupt, whilst a mask bit of 0 blocks that interrupt.
 */

#define INTMASK_XDCE (1 << 6) /* ??? */
#define INTMASK_CNTE (1 << 5) /* network error counter MSB set */
#define INTMASK_OVWE (1 << 4) /* buffer ring overwrite */
#define INTMASK_TXEE (1 << 3) /* Transmit error */
#define INTMASK_RXEE (1 << 2) /* Receive error */
#define INTMASK_PTXE (1 << 1) /* Packet Transmit */
#define INTMASK_PRXE (1 << 0) /* Packet Received */

/*---------------------------------------------------------------------------*/
/* INTSTAT : Interrupt status register
 * This register allows pending interrupts to be viewed. Interrupts can be
 * cleared by writing a 1 into the corresponding bit in this register. The
 * IRQ signal is active as long as any un-masked interrupt bit remains set.
 */

#define INTSTAT_RST (1 << 7) /* reset status */
#define INTSTAT_CNT (1 << 5) /* error counter overflow */
#define INTSTAT_OVW (1 << 4) /* overwrite warning */
#define INTSTAT_TXE (1 << 3) /* transmit error */
#define INTSTAT_RXE (1 << 2) /* receive error */
#define INTSTAT_PTX (1 << 1) /* packet transmitted */
#define INTSTAT_PRX (1 << 0) /* packet received */

/*---------------------------------------------------------------------------*/
/* MPCNT : Missed Packet error counter
 * This register is incremented when a packet cannot be received due to a
 * lack of receive buffers, FIFO overflow or because the received is in
 * monitor mode. The counter increments until 0xFF and then stops. It is
 * cleared back to 0x00 when read.
 */
/*---------------------------------------------------------------------------*/
/* NEXT : This is a working register of the DMA engine. It holds a pointer
 * to the next 256byte aligned buffer to be opened (hence A8-15 are held
 * in the register).
 */
/*---------------------------------------------------------------------------*/
/* RCON : Receive configuration register
 * This register controls the mode of the receiver.
 */

#define RCON_MON   (1 << 5) /* monitor mode */
#define RCON_PROM  (1 << 4) /* promiscuous reception mode */
#define RCON_GROUP (1 << 3) /* multicast (group) reception mode */
#define RCON_BROAD (1 << 2) /* broadcast reception mode */
#define RCON_RUNTS (1 << 1) /* runt packet (<64bytes) reception mode */
#define RCON_SEP   (1 << 0) /* save error packets reception mode */

/*---------------------------------------------------------------------------*/
/* RSTART : Receive start page register
 * Points to the start of the receive buffer ring. Because all buffers are
 * aligned on 256byte boundaries, only A8-15 are held in this register.
 */
/*---------------------------------------------------------------------------*/
/* RSTAT : Receive Status register
 * This reports the status of the most recently received packet. At the
 * start of a reception all of the bits, apart from RSTAT_DIS, are cleared.
 */

#define RSTAT_DFR   (1 << 7) /* Deferring (jabber condition) */
#define RSTAT_DIS   (1 << 6) /* received disabled (monitor mode) */
#define RSTAT_GROUP (1 << 5) /* group address (multicast or broadcast) */
#define RSTAT_MPA   (1 << 4) /* missed packet address */
#define RSTAT_OVER  (1 << 3) /* FIFO overrun */
#define RSTAT_FAE   (1 << 2) /* Frame Alignment Error */
#define RSTAT_CRC   (1 << 1) /* CRC error */
#define RSTAT_PRX   (1 << 0) /* Packet received OK */

/*---------------------------------------------------------------------------*/
/* RSTOP : last receive buffer in the ring
 * Prior to wrapping around to the RSTART buffer, the RSTOP register points
 * at the last receive buffer in the ring. Since all buffers are aligned
 * on 256byte boundaries, only A8-15 are required by this register.
 */
/*---------------------------------------------------------------------------*/
/* STA0..5 : station address
 * These registers hold the nodes individual station address.
 */
/*---------------------------------------------------------------------------*/
/* TCNTH : transmit count high
 * TCNTL : transmit count low
 * These two registers contain the transmit frame length byte count. The count
 * includes DA, SA and data fields. If CRC generation is inhibited then the
 * count should also include the CRC field.
 */
/*---------------------------------------------------------------------------*/
/* TCON : Transmit configuration register
 * 

#define TCON_LB_mask (0x06) /* loopback mask */
/* This 2bit field defines the loopback options */
#define TCON_LB_none        (0x0) /* normal operation */
#define TCON_LB_internal    (0x1) /* internal 83C690 loopback */
#define TCON_LB_external_hi (0x2) /* external loopback when LOOP pin high */
#define TCON_LB_external_lo (0x3) /* external loopback when LOOP pin low */

#define TCON_CRCN (1 << 0) /* Inhibit generation of CRCs */

/*---------------------------------------------------------------------------*/
/* TEST : 83C690 test control
 * This register should not be written to under normal circumstances.
 */
/*---------------------------------------------------------------------------*/
/* TRINCRH : address incrementer high
 * TRINCRL : address incrementer low
 * These two registers hold the approximate address for the next DMA
 * operation.
 */
/*---------------------------------------------------------------------------*/
/* TSTART : Transmit start page register
 * This points at an assembled packet that is ready to be transmitted. Since
 * all packets are aligned on 256byte boundaries, only A8-15 are required by
 * this register.
 */
/*---------------------------------------------------------------------------*/
/* TSTAT : Transmit status
 * This register reports events that occur whilst a packet is transmitted. All
 * bits are cleared prior to the transmission of a packet.
 */

#define TSTAT_OWC   (1 << 7) /* out-of-window */
#define TSTAT_CDH   (1 << 6) /* collision detect heart-beat */
#define TSTAT_UNDER (1 << 5) /* FIFO underrun */
#define TSTAT_CRL   (1 << 4) /* Carrier sense lost */
#define TSTAT_ABORT (1 << 3) /* aborted due to excessive collisions */
#define TSTAT_TWC   (1 << 2) /* transmitted frame collided at least once */
#define TSTAT_NDT   (1 << 1) /* non-deferred transmission */
#define TSTAT_PTX   (1 << 0) /* packet transmitted or FIFO underrun */

/*---------------------------------------------------------------------------*/

#endif /* __smc83c690_h */

/*---------------------------------------------------------------------------*/
/*> EOF smc83c690.h <*/
