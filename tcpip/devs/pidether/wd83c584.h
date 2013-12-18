/*
 * File:	wd83c584.h
 * Subsystem:	VLSI PID Western Digital ethernet device driver.
 * Author:	J.Smith, updates P.A.Beskeen.
 * Date:	April '93
 *
 * Description: Hardware manifests for the Westerd Digital WD83C584
 *		Bus Interface device.
 *
 *		The WD83C584 is a single device bus interface controller
 *		enabling a LAN controller to function with the standard
 *		PC ISA bus. It provides interfaces onto the PC bus,
 *		the LAN controller, ROM, EEPROM and shared memory.
 *
 *		This chip is used by SMC 8/16 bit ethernet cards plugged into
 *		the VLSI PID's option bus (PC ISA compatible bus).
 *
 * RcsId: $Id: wd83c584.h,v 1.2 1994/03/17 12:49:35 paul Exp $
 *
 * Copyright (C) 1993 Perihelion Software Ltd.
 * Copyright (C) 1993, VLSI Technology Inc. All Rights Reserved.
 * 
 * RcsLog: $Log: wd83c584.h,v $
 * Revision 1.2  1994/03/17  12:49:35  paul
 * unknown changes (checkin forced by NC)
 *
 * Revision 1.1  1993/04/27  16:25:22  paul
 * Initial revision
 *
 */

/*---------------------------------------------------------------------------*/

#ifndef __wd83c584_h
#define __wd83c584_h

/*---------------------------------------------------------------------------*/
/*
 * After "power-up" the 83C584 accesses the external EEPROM for its initial
 * configuration. About 2milli-seconds after this point the ROM and I/O ports
 * are enabled. The shared memory (RAM) must be enabled explicitly by
 * software. INIT0-1 is mapped to the three jumper position on SMC cards. These
 * are by the user to select different address configurations.
 *
 * EEPROM register banks:
 *	0x00 -> 0x07	Initial (recall) I/O state (INIT0-2 = 0x7)
 *	0x08 -> 0x0F	Initial (recall) I/O state (INIT0-2 = 0x6)
 *	0x10 -> 0x17	Initial (recall) I/O state (INIT0-2 = 0x5)
 *	0x18 -> 0x1F	Initial (recall) I/O state (INIT0-2 = 0x4)
 *	0x20 -> 0x27	Initial (recall) I/O state (INIT0-2 = 0x3)
 *	0x28 -> 0x2F	Initial (recall) I/O state (INIT0-2 = 0x2)
 *	0x30 -> 0x37	Initial (recall) I/O state (INIT0-2 = 0x1)
 *	0x38 -> 0x3F	Initial (recall) I/O state (INIT0-2 = 0x0)
 * 	0x40 -> 0x47	LAN address, board ID and checksum
 *	0x48 -> 0x4F	*unused*
 * 	0x50 -> 0x57	engineering bytes (initialised board information)
 *			50    : b0-2 = media type; 0=starlan; 1=ethernet; 2=twisted pair; 3=ew
 *			        b3-4 = irq src; 0=primary; 1=alt1; 2=alt2; 3=alt3
 *			        b5-7 = ram size; 0=res1; 1=res2; 2=8K; 3=16K; 4=32K; 5=64K; 6=res3; 7=res4
 *			51    : b0-2 = bus type; 0=AT; 1=MCA; 2=EISA
 *			        b3-4 = bus size; 0=8bit; 1=16bit; 2=32bit; 3=64bit
 *			52-57 : undefined
 *	0x58 -> 0x5F	*unused*
 * 	0x60 -> 0x67	*unused*
 *	0x68 -> 0x6F	*unused*
 * 	0x70 -> 0x77	*unused*
 *	0x78 -> 0x7F	*unused*
 */
/*---------------------------------------------------------------------------*/

typedef struct {
	vubyte MSR;  const vubyte _pad0[3]; /* Memory Select Reg */
	vubyte ICR;  const vubyte _pad1[3]; /* Interface Configuration Reg */
	vubyte IAR;  const vubyte _pad2[3]; /* I/O Address Reg */
	/* Which register is mapped is controlled with the ICR Reg */
	vubyte BIO;  const vubyte _pad3[3]; /* BIOS ROM addr Reg */
# define       EAR	BIO	            /* EEPROM Address Reg */
	vubyte IRR;  const vubyte _pad4[3]; /* Interrupt Request Reg */
	vubyte LAAR; const vubyte _pad5[3]; /* LA Address Reg */
	vubyte IJ;   const vubyte _pad6[3]; /* Initialisation Jumpers */
	vubyte GP2;  const vubyte _pad7[3]; /* General Purpose data Reg */

	/* LAN Address Registers */
	struct {
		ubyte valid;
		     const vubyte _pad8[3];
	} LAR[8];
	/* The LAN Controller registers appear immediately after this struct */
} wd83c584;

/*---------------------------------------------------------------------------*/
/* MSR : Memory Select Register */

/* Writing a 1, and then a 0 to this bit resets the LAN controller HW */
#define MSR_RST		(1 << 7) /* LAN controller reset bit */

/* Writing a 1 to this bit enables the shared memory */
#define MSR_MENB	(1 << 6) /* shared memory enable bit */

/* In 16-bit mode, A19 is set by the LAAR register */
#define MSR_RA_mask	(0x3F)	 /* lo-6bits are bits A13-A18 of the memory */

/*---------------------------------------------------------------------------*/
/* ICR : Interface Configuration Register */

/* Set to 1 to store currently mapped registers into the EEPROM. This will
 * take around 200ms, with the bit being reset when the store has completed.
 */
#define ICR_STO		(1 << 7) /* EEPROM store */

/* Set to 1 to recall the I/O address information from the EEPROM. The bit
 * is reset when the recall has been completed.
 */
#define ICR_RIO		(1 << 6) /* Recall I/O address from EEPROM */

/* Set to 1 to recall registers 0x00,0x02->0x07 */
#define ICR_RX7		(1 << 5) /* Recall all but I/O and LAN address */

/* Set to 1 to recall registers 0x08->0x0F */
#define ICR_RLA		(1 << 4) /* Recall the LAN address */

/* This bit contains the physical size of the SRAM chips; 0=8K; 1=32K.
 * When used in conjunction with the LAAR_LE16EN bit it gives the size
 * of the RAM window in host memory.
 *	ICR_MSZ	LAAR_LE16EN
 *	--------------------------------------------------------------
 *	0	0		8K
 *	0	1		16K
 *	1	0		32K
 *	1	1		64K
 */
#define ICR_MSZ		(1 << 3) /* SRAM size */

/* This bit is used in conjunction with the IRR register */
#define ICR_IR2		(1 << 2) /* IRQ lines selection */

/* 0 = BIO; 1 = EAR */
#define ICR_OTHER	(1 << 1) /* BIO/EAR register select */

/* When the board is on an 8-bit bus, this bit can be changed. If on a
 * 16-bit bus it will always read 1.
 */
#define ICR_BIT16	(1 << 0) /* 16-bit bus detect */

/*---------------------------------------------------------------------------*/
/* IAR : I/O Address Register */

#define IAR_b15b13_mask	(0xE0) /* bits 15-13 of I/O address */
#define IAR_b9b5_mask	(0x1F) /* bits 9-5 of I/O address */

/*---------------------------------------------------------------------------*/
/* BIO : BIOS ROM address register */

/* 0x00 = noROM; 0x40 = 16K; 0x80 = 32K; 0xC0 = 64K */
#define BIO_RS_mask	  (0xC0) /* BIOS ROM size mask */

/* These bits correspond to system memory address bits A14-A18 */
#define BIO_BA18BA14_mask (0x3E) /* BIOS ROM memory address bits */

/* This bit can be set to 1 to enable the selected IRQ line to be raised. The
 * bit must be cleared back to 0 to remove the IRQ.
 */
#define BIO_INT (1 << 0) /* Software interrupt */

/*---------------------------------------------------------------------------*/
/* EAR : EEPROM Address Register
 * This register allows the EEPROM to be accessed. This register can only be
 * accessed if ICR_OTHER is 1.
 */

/* These bits are used to select the bank of 8 EEPROM registers */
#define EAR_A6A3_mask	(0xF0)	/* EEPROM address A3-A6 bits */

#define EAR_RAM		(1 << 3) /* 32K RAM device in ROM socket */

#define EAR_RPE		(1 << 2) /* ROM paging enable */
#define EAR_RP_mask	(0x03)	 /* ROM page address - 16K chunks */

/*---------------------------------------------------------------------------*/
/* IRR : Interrupt Request Register */

/* Enables the selected IRQ line */
#define IRR_IEN (1 << 7) /* Interrupt enable */

/* When used in conjunction with the ICR_IR2 bit:
 *	ICR_IR2	IRR_IR1	IRR_IR0
 *	-----------------------
 *	0	0	0	IRQ2/9
 *	0	0	1	IRQ3
 *	0	1	0	IRQ5
 * 	0	1	1	IRQ7
 *	1	0	0	IRQ10
 * 	1	0	1	IRQ11
 * 	1	1	0	IRQ15
 * 	1	1	1	IRQ4
 */
#define IRR_IR1 (1 << 6) /* Interrupt Request line select */
#define IRR_IR0 (1 << 5) /* Interrupt Request line select */

/* This bit is set to say that 32K of Flash is available in the ROM socket */
#define IRR_FLSH (1 << 4) /* Flash memory */

/* The output pins reflect the state of these three data bits */
#define IRR_OUT3 (1 << 3) /* output pin 3 */
#define IRR_OUT2 (1 << 2) /* output pin 2 */
#define IRR_OUT1 (1 << 1) /* output pin 1 */

#define IRR_0WS8 (1 << 0) /* 8bit memory zero-wait-state enable */

/*---------------------------------------------------------------------------*/
/* LAAR : LA Address Register */

#define LAAR_M16EN (1 << 7) /* Enable 16bit memory accesses */

#define LAAR_L16EN (1 << 6) /* Enable 16bit LAN operation */

#define LAAR_0WS16 (1 << 5) /* Enable 0wait-state operation in 16bit mode */

/* These bits provide A23-A19 of the system address */
#define LAAR_LA23LA19_mask (0x1F) /* LA Memory decode bits */

/* Set address line LA19 */
#define LAAR_LA19   1

/*---------------------------------------------------------------------------*/
/* IJ : Initialisation Jumpers */

#define IJ_IN_mask   (0x60) /* state of the 2 input pins */

#define IJ_INIT_mask (0x07) /* state of the 3 initialise pins */

/*---------------------------------------------------------------------------*/
/* GP2 : General Purpose data registerd
 * This is a generally available R/W register which may be used as required
 * by the host software.
 */

/*---------------------------------------------------------------------------*/
/* LAR0..7 : LAN address registers
 *
 * LAR0..2 = Globally assigned address block
 * LAR3..5 = Unique board address
 * LAR6    = Board ID byte
 * LAR7    = Checksum (LAR0..7 sum should be 0xFF)
 */

#define LAR6_BUSTYPE   (1 << 7) /* 1 if MCA bus device */
#define LAR6_RAMSIZE   (1 << 6) /* depends on board type */
#define LAR6_SOFT      (1 << 5) /* soft configuration possible */
#define LAR6_REV_mask  (0x1E)   /* board revision bits */
#define LAR6_REV_shift (1)      /* shift to reach the board revision bits */
#define LAR6_MEDIA     (1 << 0) /* media type bit : 0 = <see below>; 1 = Ethernet */

/* If LAR6_MEDIA is clear, and the board REV_ID is 1, then the media
 * is StarLAN, otherwise it is "twisted pair".
 */

/*---------------------------------------------------------------------------*/

#endif /* __wd83c584_h */

/*---------------------------------------------------------------------------*/
/*> EOF wd83c584.h <*/
