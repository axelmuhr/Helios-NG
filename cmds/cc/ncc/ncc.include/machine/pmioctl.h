
/*
 *	@(#)pmioctl.h	1.2.1.1	(ULTRIX)	11/9/89
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

#ifdef KERNEL
#include "../machine/pmevent.h"
#include "../h/ioctl.h"
#include "../machine/pmreg.h"
#include "../machine/cfbreg.h"
#else
#include "pmevent.h"
#include <sys/ioctl.h>
#include "pmreg.h"
#include "cfbreg.h"
#endif

struct pm_kpcmd {
	char nbytes;		/* number of bytes in parameter */
	unsigned char cmd;	/* command to be sent, peripheral bit will */
				/* be forced by driver */
	unsigned char par[2];	/* bytes of parameters to be sent */
};

#define PM_TYPE 1
#define CFB_TYPE 2

struct cfbColorMap {
	unsigned char red;
	unsigned char green;
	unsigned char blue;
};

typedef struct pm_info {
	pmEventQueue qe;		/* event & motion queues	*/
	short	mswitches;		/* current value of mouse buttons */
	pmCursor tablet;		/* current tablet position	*/
	short	tswitches;		/* current tablet buttons NI!	*/
	pmCursor cursor;		/* current cursor position	*/
	short	row;			/* screen row			*/
	short	col;			/* screen col			*/
	short	max_row;		/* max character row		*/
	short	max_col;		/* max character col		*/
	short	max_x;			/* max x position		*/
	short	max_y;			/* max y position		*/
	short	max_cur_x;		/* max cursor x position 	*/
	short	max_cur_y;		/* max cursor y position	*/
	int	version;		/* version of driver		*/
	char	*bitmap;		/* bit map position		*/
        short   *scanmap;               /* scanline map position        */
	short	*cursorbits;		/* cursor bit position		*/
	short	*pmaddr;		/* virtual address           	*/
	char    *planemask;		/* plane mask virtual location  */
	pmCursor mouse;			/* atomic read/write		*/
	pmBox	mbox;			/* atomic read/write		*/
	short	mthreshold;		/* mouse motion parameter	*/
	short	mscale;			/* mouse scale factor (if 
					   negative, then do square).	*/
	short	min_cur_x;		/* min cursor x position	*/
	short	min_cur_y;		/* min cursor y position	*/

/*  Extra structs for 3max cfb */

	int	dev_type;		/* device type id		*/
	char	*framebuffer;		/* base pointer of framebuffer	*/
	volatile struct bt459 *bt459;	/* base pointer of bt459 regs	*/
	int	slot;			/* slot in maxbus		*/
	char	cursor_map[1024];	/* cursor map for server	*/
	u_char	bgRgb[3];		/* background cursor color	*/
	u_char	fgRgb[3];		/* foreground cursor color	*/
	
} PM_Info;

typedef struct _ColorMap {
	short  Map;
	unsigned short index;
	struct {
		unsigned short red;
		unsigned short green;
		unsigned short blue;
	} Entry;
} ColorMap;

/*
 * CAUTION:
 *	The numbers of these ioctls must match
 *	the ioctls in qvioctl.h
 */
#define QIOCGINFO 	_IOR('q', 1, struct pm_info *)	/* get the info	 */
#define QIOCPMSTATE	_IOW('q', 2, pmCursor)		/* set mouse pos */
#define	QIOWCURSORCOLOR	_IOW('q', 3, unsigned int [6])	/* bg/fg r/g/b */
#define QIOCINIT	_IO('q', 4)			/* init screen   */
#define QIOCKPCMD	_IOW('q', 5, struct pm_kpcmd)	/* keybd. per. cmd */
#define QIOCADDR	_IOR('q', 6, struct pm_info *)	/* get address */
#define	QIOWCURSOR	_IOW('q', 7, short[32])	/* write cursor bit map */
#define QIOKERNLOOP	_IO('q', 8)   /*re-route kernel console output */
#define QIOKERNUNLOOP	_IO('q', 9)   /*don't re-route kernel console output */
#define QIOVIDEOON	_IO('q', 10)			/* turn on the video */
#define	QIOVIDEOOFF	_IO('q', 11)			/* turn off the video */
#define QIOSETCMAP      _IOW('q', 12, ColorMap)
#define	QD_KERN_UNLOOP	_IO('g', 21)	/* RESERVED for DIGITAL, DON'T CHANGE */
#define QIOWLCURSOR	_IO('q', 13)	/* write 64 x 64 cursor */
