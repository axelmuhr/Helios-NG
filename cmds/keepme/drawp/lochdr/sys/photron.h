/*
 * $Header: photron.h,v 1.1 90/01/13 20:13:02 charles Locked $
 * $Source: /server/usr/users/charles/world/drawp/RCS/lochdr/sys/photron.h,v $
 */
 
/************************************************************************/
/* Very simple device driver to facilitate the re-programming of VIDC	*/
/*									*/
/* Copyright (c) 1989 Active Book Company Ltd., Cambridge, UK.		*/
/*									*/
/* Author: JGSmith, July 1989						*/
/************************************************************************/

#ifdef KERNEL
# include "ioctl.h"
#else
# include "ioctl.h"  /* Normally <sys/ioctl.h> */
#endif

/* "ioctls" provided by this driver */

#define PHOTRONVIDC		_IO('p',0)		  /* program VIDC */
#define PHOTRONVIDCWRITE	_IOW('p',1,unsigned int *)  /* write to VIDC */

/* EOF photron.h */
