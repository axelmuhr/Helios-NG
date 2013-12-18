
/*
 *	@(#)itsioctl.h	1.2	(ULTRIX)	1/30/89
 */

/************************************************************************
 *									*
 *			Copyright (c) 1987 by				*
 *		Digital Equipment Corporation, Maynard, MA		*
 *			All rights reserved.				*
 *									*
 *   This software is furnished under a license and may be used and	*
 *   copied  only  in accordance with the terms of such license and	*
 *   with the  inclusion  of  the  above  copyright  notice.   This	*
 *   software  or  any	other copies thereof may not be provided or	*
 *   otherwise made available to any other person.  No title to and	*
 *   ownership of the software is hereby transferred.			*
 *									*
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or	reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/

/*
 * itsioctl.h
 *
 * Modification history
 *
 * ITEX/FG101 Series 100 frame buffer ioctl structures/definitions
 *
 *  2-Dec-86 - lp/rsp (Larry Palmer/Ricky Palmer)
 *
 *	Created prototype frame buffer driver.
 */

/*
 *	This  driver  provides user level programs with the ability to
 *	read  and  write  any  of  the	frame  buffer registers.  This
 *	capability  is	provided  via  the  ITSMAP  ioctl  request and
 *	subsequent  user  direct  access  to  the  registers and frame
 *	memory.   The  driver  initializes  the  scanner hardware in a
 *	default  state.   The same initialization is possible by using
 *	the  registers	directly.  Note that DMA is NOT possible since
 *	the board does not support local DMA transfers to another QBUS
 *	device.   The  documentation  provided	by  Imaging Technology
 *	should	be  consulted  for further details involving the frame
 *	buffer	hardware/software.   This  driver  is  provided  as  a
 *	service  to  Digital  Ultrix  customers  who  would  like  the
 *	capability to combine the frame buffer hardware with their GPX
 *	system	in  order  to  create  digital	images from a standard
 *	television  camera hookup.  The driver is provided "as-is" and
 *	is  NOT  supported  under any Digital agreements or contracts.
 *	Electronic   mail   concerning	the  driver  may  be  sent  to
 *	decvax!rsp  or	decvax!lp  but	there  is  no guarantee of any
 *	response or support.
 *
 *	This driver assumes the following configuration:
 *
 *		FG101 at 173000
 *		Memory Map @ 0x30200000
 *
 *	The availble ioctl requests for this driver are:
 *
 *	(1) ITSMAP - maps the frame buffer into user space.
 *	(2) ITSGRABIT - starts the frame buffer grabbing frames (30/sec).
 *	(3) ITSFREEZE - freezes a frame.
 *	(4) ITSZOOM - initiates a zoom;subsequent calls cycle through.
 *
 */

struct memory_map {
	char *buf;
	char *reg;
	int bytes;
};
struct	itsxypos {
	int	curx;
	int	cury;
};
#define ITSGRABIT	_IO('q', 7)			/* enable grabbing */
#define ITSFREEZE	_IO('q', 8)			/* freeze frame    */
#define ITSPS		_IOW('q', 9, struct itsxypos)	/* pan/scroll	   */
#define ITSMAP		_IOWR('q', 10, struct memory_map) /* map board	   */
#define ITSZOOM 	_IOW('q', 11, int *)		/* cyclic zoom	   */
