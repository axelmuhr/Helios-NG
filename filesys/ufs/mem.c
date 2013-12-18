/* (C)1992 Perihelion Software Limited                                */
/* Author: Alex Schuilenburg                                          */
/* Date: 29 July 1992                                                 */
/* File: mem.c                                                        */
/*                                                                    */
/* This file contains the memory device for access though the file    */
/* system to the transputer memory.                                   */
/*                                                                    */
/* $Id: mem.c,v 1.1 1992/09/16 09:29:06 al Exp $ */
/* $Log: mem.c,v $
 * Revision 1.1  1992/09/16  09:29:06  al
 * Initial revision
 * */

#include "param.h"
#include "conf.h"
#include "buf.h"
#include "systm.h"
#include "uio.h"
#include "malloc.h"

mm_rw(dev, uio, flags)
	dev_t dev;
	struct uio *uio;
	int flags;
{
	register int o;
	register u_int c, v;
	register struct iovec *iov;
	int error = 0;
	caddr_t zbuf = NULL;

	while (uio->uio_resid > 0 && error == 0) {
		iov = uio->uio_iov;
		if (iov->iov_len == 0) {
			uio->uio_iov++;
			uio->uio_iovcnt--;
			if (uio->uio_iovcnt < 0)
				panic("mmrw");
			continue;
		}
		switch (minor(dev)) {

/* minor device 0 is physical memory */
		case 0:
			/* Helios memory is read only */
			if (uio->uio_rw == UIO_WRITE) {
				error = EACCES;
				break;
			}
			c = iov->iov_len;
			error = uiomove((caddr_t)uio->uio_offset, (int)c, uio);
			continue;

/* minor device 1 is kernel memory (in helios the same as vm) */
		case 1:
			/* Helios memory is read only */
			if (uio->uio_rw == UIO_WRITE) {
				error = EACCES;
				break;
			}
			c = iov->iov_len;
			error = uiomove((caddr_t)uio->uio_offset + 0x80000000,
				 (int)c, uio);
			continue;

/* minor device 2 is EOF/RATHOLE */
		case 2:
			if (uio->uio_rw == UIO_READ)
				return (0);
			c = iov->iov_len;
			break;

/* minor device 12 (/dev/zero) is source of nulls on read, rathole on write */
		case 12:
			if (uio->uio_rw == UIO_WRITE) {
				c = iov->iov_len;
				break;
			}
			if (zbuf == NULL) {
				zbuf = (caddr_t)
				    malloc(CLBYTES, M_TEMP, M_WAITOK);
				bzero(zbuf, CLBYTES);
			}
			c = MIN(iov->iov_len, CLBYTES);
			error = uiomove(zbuf, (int)c, uio);
			continue;

		default:
			return (ENXIO);
		}
		if (error)
			break;
		iov->iov_base += c;
		iov->iov_len -= c;
		uio->uio_offset += c;
		uio->uio_resid -= c;
	}
	if (zbuf)
		free(zbuf, M_TEMP);
	return (error);
}

