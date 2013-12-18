
/*
 * 	@(#)smioctl.h	1.2	(ULTRIX)	1/30/89
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
 * 14-Jul-88  -- vasudev (Vasudev K. Bhandarkar)
 *       
 *      Removed sched_flags from the SM_Info data structure and
 *      deleted spurious irrelevant comments.
 *
 * 14-May-87  -- fred (Fred Canter)
 *	Added QD_KERN_UNLOOP ioctl definition. This is a duplicate of
 *	QIOKERNUNLOOP, but is needed!
 *
 * 19-Mar-87  -- fred (Fred Canter)
 *	Added screen saver ioctls.
 *
 * 30-Aug-86  -- rafiey (Ali Rafieymehr)
 *	Changes for smscreen (console message window) support.
 *
 *  5-Aug-86  -- rafiey (Ali Rafieymehr)
 *	Minor change for real VAXstar bitmap graphics driver.
 *
 * 18-Jun-86  -- rafiey (Ali Rafieymehr)
 *	Created this header file for the VAXstar monochrome display driver.
 *	Derived from qvioctl.h.
 *
 **********************************************************************/

/*
 *
 * Ioctl definitions for the VAXstar Monochrome
 *
 */
#ifdef KERNEL
#include "../io/uba/smevent.h"
#include "../h/ioctl.h"
#include "../io/uba/smreg.h"
#else
#include "smevent.h"
#include <sys/ioctl.h>
#include "smreg.h"
#endif

struct sm_kpcmd {
	char nbytes;		/* number of bytes in parameter */
	unsigned char cmd;	/* command to be sent, peripheral bit will */
				/* be forced by driver */
	unsigned char par[2];	/* bytes of parameters to be sent */
};
/*
 * VAXstar Monochrome information block
 */

	/* wc added typedef here - sep 10 - dlh */
typedef struct sm_info {
	smEventQueue qe;		/* event & motion queues	*/
	short	mswitches;		/* current value of mouse buttons */
	smCursor tablet;		/* current tablet position	*/
	short	tswitches;		/* current tablet buttons NI!	*/
	smCursor cursor;		/* current cursor position	*/
	short	row;			/* screen row			*/
	short	col;			/* screen col			*/
	short	max_row;		/* max character row		*/
	short	max_col;		/* max character col		*/
	short	max_x;			/* max x position		*/
	short	max_y;			/* max y position		*/
	short	max_cur_x;		/* max cursor x position 	*/
	short	max_cur_y;		/* max cursor y position	*/
	int	version;		/* version of driver		*/
/*
 * This location is critical (for version) since old versions of driver had
 * the pointer to the bitmap here and the server needs to be able to detect
 * an old driver, since it will have to do different work then.
 */
	char	*bitmap;		/* bit map position		*/
        short   *scanmap;               /* scanline map position        */
	short	*cursorbits;		/* cursor bit position		*/
	struct	nb_regs	*smaddr;	/* virtual address           	*/
	smCursor mouse;			/* atomic read/write		*/
	smBox	mbox;			/* atomic read/write		*/
	short	mthreshold;		/* mouse motion parameter	*/
	short	mscale;			/* mouse scale factor (if 
					   negative, then do square).	*/
	short	min_cur_x;		/* min cursor x position	*/
	short	min_cur_y;		/* min cursor y position	*/
} SM_Info;

/*
 * CAUTION:
 *	The numbers of these ioctls must match
 *	the ioctls in qvioctl.h
 */
#define QIOCGINFO 	_IOR('q', 1, struct sm_info)	/* get the info	 */
#define QIOCSMSTATE	_IOW('q', 2, smCursor)		/* set mouse pos */
#define QIOCINIT	_IO('q', 4)			/* init screen   */
#define QIOCKPCMD	_IOW('q', 5, struct sm_kpcmd)	/* keybd. per. cmd */
#define QIOCADDR	_IOR('q', 6, struct sm_info *)	/* get address */
#define	QIOWCURSOR	_IOW('q', 7, short[32])	/* write cursor bit map */
#define QIOKERNLOOP	_IO('q', 8)   /*re-route kernel console output */
#define QIOKERNUNLOOP	_IO('q', 9)   /*don't re-route kernel console output */
#define QIODISPON	_IO('q', 10)  /*turn the display on */
#define QIOVIDEOON	_IO('q', 10)			/* turn on the video */
#define QIODISPOFF	_IO('q', 11)  /*turn the display off */
#define	QIOVIDEOOFF	_IO('q', 11)			/* turn off the video */

#define	QD_KERN_UNLOOP	_IO('g', 21)	/* RESERVED for DIGITAL, DON'T CHANGE */
