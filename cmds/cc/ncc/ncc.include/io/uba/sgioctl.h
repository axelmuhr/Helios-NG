
/*
 * 	@(#)sgioctl.h	1.2	(ULTRIX)	1/30/89
 */

/************************************************************************
 *									*
 *			Copyright (c) 1986, 1987 by			*
 *		Digital Equipment Corporation, Maynard, MA		*
 *			All rights reserved.				*
 *									*
 *   This software is furnished under a license and may be used and	*
 *   copied  only  in accordance with the terms of such license and	*
 *   with the  inclusion  of  the  above  copyright  notice.   This	*
 *   software  or  any  other copies thereof may not be provided or	*
 *   otherwise made available to any other person.  No title to and	*
 *   ownership of the software is hereby transferred.			*
 *									*
 *   This software is  derived  from  software  received  from  the	*
 *   University    of   California,   Berkeley,   and   from   Bell	*
 *   Laboratories.  Use, duplication, or disclosure is  subject  to	*
 *   restrictions  under  license  agreements  with  University  of	*
 *   California and with AT&T.						*
 *									*
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or  reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/

/***********************************************************************
 *
 * Modification History:
 *
 * 19-Mar-87  -- fred (Fred Canter for Ali Rafieymehr)
 *	Added video & cursor on/off ioctls.
 *
 * 13-Dec-86  -- rafiey (Ali Rafieymehr)
 *	Extensive changes to go along with the semi-working
 *	VAXstar color driver (runs on early proto board).
 *
 * 18-Jun-86  -- rafiey (Ali Rafieymehr)
 *	Created this header file for the VAXstar color driver.
 *	Derived from qdioctl.h.
 *
 **********************************************************************/

#ifdef KERNEL
#include "../h/ioctl.h"
#else
#include <sys/ioctl.h>
#endif


#define QD_GETEVENT	_IOR('g', 1, struct _vs_event) /* get oldest event */
#define QD_WTCURSOR	_IOW('g', 2, short[32])       /* write cursor bitmap */
#define QD_RDCURSOR	_IOR('g', 3, 64)           /* read cursor bitmap */
#define QD_CLRSCRN	_IO('g', 4) 		/* clear the screen */
#define QD_RDCONFIG	_IOR('g', 5, short) /* read VAXstar(color) configuration */
#define QD_PRGMOUSE	_IOW('g', 6, char)	/* program mouse */
#define QD_PRGTABLET	_IOW('g', 7, char) 	/* program tablet */
#define QD_PRGKBD	_IOW('g', 8, struct prgkbd) /* program LK201 kbd */
#define QD_MAPDEVICE	_IOR('g', 9, struct sgmap) /* map device to user */
#define QD_MAPIOBUF 	_IOWR('g', 10, caddr_t)     /* map DMA iobuf to user */
#define QD_MAPEVENT	_IOR('g', 11, caddr_t)     /* map event queue to user */
#define QD_PRGCURSOR	_IOW('g', 12, struct prg_cursor) /* program cursor */
#define QD_RESET	_IO('g', 13)  	    /* set device & driver defaults */
#define QD_POSCURSOR	_IOW('g', 14, struct _vs_cursor) /* position cursor */
#define QD_SET		_IO('g', 15)  	    /* set DUART & driver defaults */
#define QD_MAPSCROLL    _IOR('g', 16, caddr_t)  /* map scroll param area */
#define QD_UNMAPSCROLL  _IO('g', 17)            /* unmap scroll param area */
#define QD_MAPCOLOR     _IOR('g', 18, caddr_t)  /* map color map write buf */
#define QD_UNMAPCOLOR   _IO('g', 19)            /* unmap color map write buf */
#define QD_KERN_LOOP    _IO('g', 20)       /* detour kernel console output */
#define QD_KERN_UNLOOP  _IO('g', 21)       /* un-detour kernel console output */
#define QD_PRGTABRES	_IOW('g', 22, short) /* program tablet resolution */
#define QD_VIDEOON	_IO('g',23)	      /* turn on the video */
#define QD_VIDEOOFF	_IO('g',24)	      /* turn off the video */
#define QD_CURSORON	_IO('g',25)	      /* turn on the cursor */
#define QD_CURSOROFF	_IO('g',26)	      /* turn off the cursor */

