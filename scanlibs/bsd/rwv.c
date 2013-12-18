/*
 * readv() and writev() emulation routines for Helios
 *
 * Originally written by Dr Nick Garnet.
 * 'Improved' (ahem) by Nick Clifton.
 *
 * Copyright (c) 1991-1993 Perihelion Software Ltd.
 * All Rights Reserved.
 */

/* $Id: rwv.c,v 1.4 1993/04/20 09:32:18 nickc Exp $ */

#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/uio.h>

extern void IOdebug( const char * message, ... );

/*
 * readv( fd, iov, num_iov ) - read data into iov structure
 *
 * read data from 'fd' into the buffers
 * pointed at by 'iov'.  At most 'num_iov'
 * will be filled, and each iovector will be completly
 * filled in turn, starting with the first.
 * Upon successful completion readv() returns the total
 * number of bytes read.  0 is returned upon end-of-file,
 * and -1 is returned upon error.  In this case errno is
 * set as for read() or possibly :-
 *
 * [EINVAL]	if 'num_iovectors' is less than or equal to 0 or
 * 		greater than MAXIOV .
 *
 * [EINVAL]	one of the iovectors had a negative length,
 *              or a NULL base address.
 *
 * [EINVAL]	the sum of the sizes of the iovector
 * 		lengths overflowed a 'unsigned long' integer.
 */

extern int
readv(
      int		fd,
      struct iovec *	iov,
      int 		num_iov )
{
  unsigned long		total  = 0;
  int			oflags = fcntl( fd, F_GETFL );
  int			nflags = oflags;
  
  
  if (oflags < 0)
    {
      /* errno set by the call to fcntl() */
      
      return -1;
    }
  
  if (num_iov <= 0 || num_iov > MAXIOV)
    {
      errno = EINVAL;
      
      return -1;
    }
  
  /* set the file into non-blocking mode so that we can pull 	*/
  /* out as much data as is available			   	*/
  
  nflags |= O_NONBLOCK;
  
  fcntl( fd, F_SETFL, nflags );
  
  while (num_iov--)
    {
      signed int 	size;
      signed int	len = iov->iov_len;
      
      
      if (iov->iov_base == NULL || len < 0)
	{
	  (void) fcntl( fd, F_SETFL, oflags );
	  
	  errno = EINVAL;
	  
	  return -1;
	}
      
      if (len == 0)
	{
	  /*
	   * do not bother reading zero bytes as this can
	   * cause read to return with EAGAIN, which means
	   * that readv will return -1 and so any data read
	   * into earlier vectors will be lost.  *sigh*
	   */
	  
	  iov++;
	  
	  continue;
	}
      
      size = read( fd, iov->iov_base, len );
      
      if (size < 0)
	{
	  int	save_err = errno;
	  
	  
	  /*
	   * There is a problem here - what if we
	   * are half way through filling the vectors
	   * when an error occurs.  Returning -1 (as we
	   * are supposed to do) will mean that the
	   * invoking software will loose all the
	   * data we have already read
	   */
	  
	  (void) fcntl( fd, F_SETFL, oflags );
	  
	  errno = save_err;
	  
	  if (errno == EAGAIN && total > 0)
	    {
	      /* EAGAIN is non-fatal */
	      
	      errno = 0;
	      
	      return (int)total;
	    }
	  
	  return -1;
	}
      
      if (total + size < total)
	{
	  /* overflow */
	  
	  (void) fcntl( fd, F_SETFL, oflags );
	  
	  errno = EINVAL;
	  
	  /* see comment with size < 0 */
	  
	  return -1;
	}
      
      total += size;
      
      if (size != len)
	{
	  break;
	}
      
      iov++;
    }
  
  (void) fcntl( fd, F_SETFL, oflags );
  
  return (int)total;
  
} /* readv */



/*
 * writev( fd, iov, num_iov ) - write data from iovec structure
 *
 * write data to 'fd' from the buffers
 * pointed at by 'iov'.  At most 'num_iov'
 * will be emptied, and each iovector will be completly
 * emptied in turn, starting with the first.
 * Upon successful completion writev() returns the total
 * number of bytes written.  -1 is returned upon error.
 * In this case errno is set as for write() or possibly :-
 *
 * [EINVAL]	if 'num_iov' is less than or equal to 0 or
 * 		greater than MAXIOV .
 *
 * [EINVAL]	one of the iovectors had a negative length
 *              or a NULL start address.
 *
 * [EINVAL]	the sum of the sizes of the iovector
 * 		lengths overflowed a 'unsigned long' integer.
 */

extern int
writev(
       int		fd,
       struct iovec *	iov,
       int 		num_iov )
{
  unsigned long		total = 0;
  

  if (num_iov <= 0 || num_iov > MAXIOV)
    {
      errno = EINVAL;
      
      return -1;
    }
  
  while (num_iov--)
    {
      signed int	amount = iov->iov_len;
      unsigned char *	start  = (unsigned char *)iov->iov_base;
      
      
      if (amount < 0 || start == NULL)
	{
	  errno = EINVAL;
	  
	  return -1;
	}
      
      while (amount > 0)
	{
	  signed int	size = write( fd, (char *)start, amount );

	  
	  /* check for an error result from the write */
	  
	  if (size < 0)
	    {
	      if (errno == EAGAIN && total > 0)
		{
		  /* EAGAIN is non-fatal */
		  
		  errno = 0;
		  
		  return (int)total;
		}
	      
	      return -1;
	    }

	  /* check to see if anything was written */
	  
	  if (size == 0)
	    {
	      return (int)total;
	    }

	  /* check for overflow in our counter */
	  
	  if (total + size < total)
	    {
	      /* overflow */
	      
	      errno = EINVAL;
	      
	      return -1;
	    }

	  /* increment counters and pointers and repeat loop */
	  
	  total  += size;
	  start  += size;
	  amount -= size;

	  if (amount > 0)
	    {
	      /*
	       * some bytes still left to write - we can either try again
	       * or return right now.  Currently (10/6/91) there is a bug
	       * in the POSIX write routine whereby two calls to write()
	       * in very quick succession, when the first call failed to
	       * write all the data specified, will cause the second write()
	       * to corrupt the data sent in the first write!  Hence we return
	       * here rather than carrying on the loop.
	       */
	      
	      return (int) total;
	    }	  
	}

      /* process next iovector */
      
      iov++;
    }

  /* finished - return number of bytes written */
  
  return (int)total;
  
} /* writev */
