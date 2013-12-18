/*
 * 	@(#)qvioctl.h	1.2	(ULTRIX)	1/30/89
 */

/************************************************************************
 *									*
 *			Copyright (c) 1984, 1987 by			*
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

/************************************************************************
 *
 * Modification History:
 *
 * 18-Jun-87  rafiey (Ali Rafieymehr)
 *	Added QIOWCURSOR ioctl definition. This ioctl is used only by
 *	VAXstar monochrome.
 *
 * 14-May-87  -- fred (Fred Canter)
 *	Added QD_KERN_UNLOOP ioctl definition. This is a duplicate of
 *	QIOKERNUNLOOP, but is needed!
 *
 * 19-Mar-87  fred (Fred Canter)
 *	Changes some ioctl numbers to be consistent with smioctl.h.
 *
 * 08-Jan-87  rafiey (Ali Rafieymehr)
 *	Added two new ioctls to turn the video on and off. These two
 *	are to be used for save screen.
 *
 ************************************************************************/

/*
 * Ioctl definitions for the qvss.
 *
 */
#ifdef KERNEL
#include "qvevent.h"
#include "../h/ioctl.h"
#include "../io/uba/qvreg.h"
#else
#include "qvevent.h"
#include <sys/ioctl.h>
#include "qvreg.h"
#endif

struct qv_kpcmd {
	char nbytes;		/* number of bytes in parameter */
	unsigned char cmd;	/* command to be sent, peripheral bit will */
				/* be forced by driver */
	unsigned char par[2];	/* bytes of parameters to be sent */
};
/*
 * qvss information block
 */

qvCursor t;

typedef struct qv_info {
	qvEventQueue qe;                /* event & motion queues        */
	short	mswitches;		/* current value of mouse buttons */
	qvCursor tablet;		/* current tablet position	*/
	short	tswitches;		/* current tablet buttons NI!	*/
	qvCursor cursor;		/* current cursor position	*/
	short	row;			/* screen row			*/
	short	col;			/* screen col			*/
	short	max_row;		/* max character row		*/
	short	max_col;		/* max character col		*/
	short	max_x;			/* max x position		*/
	short	max_y;			/* max y position		*/
	short	max_cur_x;		/* max cursor y position 	*/
	short	max_cur_y;		/* max cursor y position	*/
	char	*bitmap;		/* bit map position		*/
	short	*scanmap;		/* scanline map position	*/
	short	*cursorbits;		/* cursor bit position		*/
	struct	qvdevice *qvaddr;	/* virtual address of the csr	*/
	qvCursor mouse;			/* atomic read/write		*/
	qvBox	mbox;			/* atomic read/write		*/
	short	mthreshold;		/* mouse motion parameter	*/
	short	mscale;			/* mouse scale factor (if 
					   negative, then do square).	*/
	short   min_cur_x;              /* min cursor x position        */
	short   min_cur_y;              /* min cursor y position	*/

} QVSS_Info;
typedef struct qv_info vsIoAddr;

#define MOTION_BUFFER_SIZE 100


/*
 * CAUTION:
 *	The numbers of these ioctls must match
 *	the ioctls in smioctl.h.
 */
#define QIOCGINFO 	_IOR('q', 1, struct qv_info)	/* get the info	 */
#define QIOCSMSTATE	_IOW('q', 2, qvCursor)		/* set mouse pos */
#define QIOCINIT	_IO('q', 4)			/* init screen   */
#define QIOCKPCMD	_IOW('q', 5, struct qv_kpcmd)	/* keybd. per. cmd */
#define QIOCADDR	_IOR('q', 6, struct qv_info *)	/* get address */
#define	QIOWCURSOR	_IOW('q', 7, short[32])	/* write cursor bit map */
#define QIOKERNLOOP	_IO('q', 8)   /*re-route kernel console output */
#define QIOKERNUNLOOP	_IO('q', 9)   /*don't re-route kernel console output */
#define QIOVIDEOON	_IO('q', 10)			/* turn on the video */
#define	QIOVIDEOOFF	_IO('q', 11)			/* turn off the video */

#define	QD_KERN_UNLOOP	_IO('g', 21)	/* RESERVED for DIGITAL, DON'T CHANGE */
