/*
 * 	@(#)fgioctl.h	1.1.1.1	(ULTRIX)	5/31/89
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
 * 12-Jan-89 - jaw
 *	merge Xe changes for FFox driver.
 *
 * 11-Nov-88	darrell (for Allen Carito)
 *	Added macroes for SAVE/RESTORE
 *
 **********************************************************************/

#ifdef KERNEL
#include "../h/ioctl.h"
#else
#include <sys/ioctl.h>
#endif


#define FG_GETEVENT	_IOR('g', 1, struct _vs_event) /* get oldest event */
#define FG_WTCURSOR	_IOW('g', 2, u_long[64])       /* write cursor bitmap */
#define FG_CLRSCRN	_IO('g', 4) 		/* clear the screen */
#define FG_PRGKBD	_IOW('g', 8, struct prgkbd) /* program LK201 kbd */
#define FG_MAPDEVICE	_IOR('g', 9, struct fgmap) /* map device to user */
#define FG_MAPEVENT	_IOR('g', 11, caddr_t)     /* map event queue to user */
#define FG_PRGCURSOR	_IOW('g', 12, struct prg_cursor) /* program cursor */
#define FG_RESET	_IO('g', 13)  	    /* set device & driver defaults */
#define FG_POSCURSOR	_IOW('g', 14, struct _vs_cursor) /* position cursor */
#define FG_SET		_IO('g', 15)  	    /* set DUART & driver defaults */
#define FG_HALTGVAX     _IO('g', 16)	  /* halt the GVAX */
#define FG_UNHALTGVAX   _IO('g', 17)	  /* unhalt the GVAX */
#define FG_KERN_LOOP    _IO('g', 20)       /* detour kernel console output */
#define FG_KERN_UNLOOP  _IO('g', 21)       /* un-detour kernel console output */
#define QD_KERN_LOOP    _IO('g', 20)       /* detour kernel console output */
#define QD_KERN_UNLOOP  _IO('g', 21)       /* un-detour kernel console output */
#define FG_VIDEOON	_IO('g',23)	      /* turn on the video */
#define FG_VIDEOOFF	_IO('g',24)	      /* turn off the video */
#define FG_INITACHIP	_IO('g',27)	      /* init. achip */
#define FG_INITDCHIP	_IO('g',28)	      /* init. dchip */
#define FG_INITTCHIP	_IO('g',29)	      /* init. tchip */
#define FG_GETTIMING	_IOR('g',30, fg_timinginfo) /* get tchip values */
#define FG_SETTIMING	_IOW('g',31, fg_timinginfo) /* set tchip values */
#define	FG_GETCOMMAREA	_IOR('g', 32, fg_commarea_desc) /* get common area */
#define	FG_SETCURSTAB	_IOW('g', 33, struct fgcurstabupd) /* update cursor */
#define FG_SETFGPIXEL   _IOW('g', 34, u_long) /* set foreground pixel */
#define FG_SETBGPIXEL   _IOW('g', 35, u_long) /* set background pixel */

