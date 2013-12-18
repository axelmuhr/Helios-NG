
/* sys/uio.h :  UNIX multi-buffer i/o emulation structures			*/

#ifndef _uio_h
#define _uio_h

#if !defined _types_h
typedef struct iovec
{
	unsigned char *	iov_base;	/* start of buffer */
	signed int	iov_len;	/* length of buffer in bytes */
	
} _iovec;
#endif

#define MAXIOV	16	/* maximum number of _iovec structures in an array passed to readv()/write() */


extern int	readv(  int file_descriptor, struct iovec * piovectors, int num_iovectors );
extern int	writev( int file_descriptor, struct iovec * piovectors, int num_iovectors );

#endif

/* end of sys/uio.h */

