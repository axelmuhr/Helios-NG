/*
 * File:	hw_glap.h
 * Subsystem:	Helios-ARM Executive
 * Author:	P.A.Beskeen
 * Date:	Nov '92
 *
 * Description: Gnome Link Adapter Podule (GLAP) manifests.
 *
 *
 * RcsId: $Id$
 *
 * (C) Copyright 1992 Perihelion Software Ltd.
 *     All Rights Reserved.
 * 
 */

#ifndef	__hw_glap_h
#define	__hw_glap_h


/* Gnome transputer link adapter is an Acorn expansion card. */

#include <ARM/xcb.h>


/* Base of link adapter registers in expansion card space. */

#define GLAP_base		0x2000


/* Gnome Link Adapter Podule register set. */

typedef struct glap_regs {
    volatile unsigned char
 	read_data,	_pad0[3],
	write_data,	_pad1[3],
	input_status,	_pad2[3],
	output_status,	_pad3[3],
	control_status,	_pad4[3];
} glap_regs;


/* Return address of link adapter for given card slot. */

#define GLAP_LinkAdapter(l) \
		((glap_regs *)(XCB_ADDRESS(FAST, (l)) + GLAP_base))


/* Control register bits. */

#define GLAP_ResetOut		0x01
#define GLAP_AnalyseOut		0x02
#define GLAP_LinkSpeed20Mhz	0x04	/* 0 = 10Mhz */
#define GLAP_ChipReset		0x08	/* Set link adapter rst (min. 1.6uS) */
#define GLAP_ErrorIn		0x16	/* Read error pin value */


/* Input status register bits. */

#define GLAP_InputReady		0x1
#define GLAP_ReadIntrEnable	0x2


/* Output status register bits. */

#define GLAP_OutputReady	0x1
#define GLAP_WriteIntrEnable	0x2


#endif /*__hw_glap_h*/



/* End of hw_glap.h */
