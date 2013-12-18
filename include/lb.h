/* lb.h: Load Balancer Protocol Header				*/
/* %W% %G% (C) Copyright 1990, Perihelion Software Ltd.		*/
/* $Id: lb.h,v 1.2 1992/07/30 13:36:14 bart Exp $ */

#ifndef __helios_h
#include <helios.h>
#endif

#define 	AUXS	4	/* first auxiliary stream */


/* Header Protocol */
/* =============== */

/* 4 error bits */
#define 	LB_ERROR	0xF0000000L

/* Master Control Nibble */
/* all Control mssg's are qualified by the 16 bit Function LB_FN */
#define		LB_MASTER	0x0F000000L

/* Packet Mode e.g Broadcast/Sync etc. */
/* This field is present in every data packet and defines the 	 */
/* Mode for this packet.				         */


#define		LB_BROADCAST	0x00010000L
#define		LB_SYNC		0x00020000L

/* Function code (16 bits) */
#define		LB_FN		0x0000FFFFL

#define		Fn_Terminate	0x00000001L
#define		Fn_Abort	0x00000002L


/* Load Balancer Protocol Modes */
/* ============================ */
/* Defined by a Master Packet   */
/* with a bit in the MOde nibble*/

#define		LB_MODE		0x00F00000L

/* LineMode: The worker streams should be considered as simple  */
/*		line buffered streams. The packet data consists */
/*		of a single \n terminated line.		        */

#define		LINE_MODE	0x00100000L


/* ControlMode:	The worker streams are Control protocol streams */
/*		all packets consist of the standard header      */
/*		followed by packet data.			*/

#define		CONTROL_MODE	0x00200000L







/* Packet Header */

typedef struct LB_HEADER {
	
	word	size;
	word	control;
	
} LB_HEADER;


/* Packet */

typedef struct PACKET {
	
	LB_HEADER	header;
	byte		data[1];
	
} PACKET;


