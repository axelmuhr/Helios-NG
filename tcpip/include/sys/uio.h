
/* for source compatability */

#ifndef __uio_h
#define __uio_h

struct uio
{
	struct iovec	*uio_iov;
	int		uio_iovcnt;
	int		uio_offset;
	int		uio_segflg;
	int		uio_resid;
};

#define	UIO_READ	1
#define UIO_WRITE	2

#define	UIO_USERSPACE	3

#define B_WRITE		4
#define B_READ		5

#endif
