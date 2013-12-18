/* @(#)vcmd.h	1.1  (ULTRIX)        1/27/89     */

#ifndef _IOCTL_
#ifdef KERNEL
#include "../h/ioctl.h"
#else KERNEL
#include <sys/ioctl.h>
#endif KERNEL
#endif _IOCTL_

#define	VPRINT		0100
#define	VPLOT		0200
#define	VPRINTPLOT	0400

#define	VGETSTATE	_IOR('v', 0, int)
#define	VSETSTATE	_IOW('v', 1, int)
