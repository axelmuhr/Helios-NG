/*
 * $Header: ttydev.h,v 1.1 90/01/13 20:13:10 charles Locked $
 * $Source: /server/usr/users/charles/world/drawp/RCS/lochdr/sys/ttydev.h,v $
 *
 * Copyright (c) 1988 Acorn Computers Ltd., Cambridge, England
 *
 * $Desc$
 * $Log:	ttydev.h,v $
 * Revision 1.1  90/01/13  20:13:10  charles
 * Initial revision
 * 
 * Revision 1.8  89/12/15  16:34:40  pete
 * File unchanged
 * 
 * Revision 1.7  89/11/03  15:00:16  charles
 * File unchanged
 * 
 * Revision 1.6  89/08/15  13:43:10  charles
 * File unchanged
 * 
 * Revision 1.5  89/08/11  17:47:16  charles
 * File unchanged
 * 
 * Revision 1.4  89/08/10  19:53:28  charles
 * File unchanged
 * 
 * Revision 1.3  89/08/10  19:41:46  charles
 * File unchanged
 * 
 * Revision 1.2  89/08/10  19:00:59  charles
 * File unchanged
 * 
 * Revision 1.1  89/08/10  16:24:02  charles
 * Initial revision
 * 
 * Revision 1.2  89/08/10  15:46:49  charles
 * "just_to_release_lock"
 * 
 * Revision 1.1  89/07/10  15:32:10  charles
 * Initial revision
 * 
 * Revision 1.3  88/06/17  20:22:12  beta
 * Acorn Unix initial beta version
 * 
 */
/* @(#)ttydev.h	1.2 87/05/15 3.2/4.3NFSSRC */
/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)ttydev.h	7.1 (Berkeley) 6/4/86
 */

/*
 * Terminal definitions related to underlying hardware.
 */
#ifndef _TTYDEV_
#define	_TTYDEV_

/*
 * Speeds
 */
#define B0	0
#define B50	1
#define B75	2
#define B110	3
#define B134	4
#define B150	5
#define B200	6
#define B300	7
#define B600	8
#define B1200	9
#define	B1800	10
#define B2400	11
#define B4800	12
#define B9600	13
#define EXTA	14
#define EXTB	15

#ifdef KERNEL
/*
 * Hardware bits.
 * SHOULD NOT BE HERE.
 */
#define	DONE	0200
#define	IENABLE	0100

/*
 * Modem control commands.
 */
#define	DMSET		0
#define	DMBIS		1
#define	DMBIC		2
#define	DMGET		3
#endif
#endif
