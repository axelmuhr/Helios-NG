
/* errno.h : Posix Library error numbers   				*/

/* WARNING: this file is generated automatically from the master codes	*/
/* database (faults/master.fdb), any changes here will be overwritten.	*/


#ifndef __errno_h
#define __errno_h

#define	EZERO		    0		/* No error			*/
#define	E2BIG		    1		/* Arg list too long		*/
#define	EACCES		    2		/* Permission denied		*/
#define	EAGAIN		    3		/* Resource temporarily unavailable*/
#define	EBADF		    4		/* Bad file number		*/
#define	EBUSY		    5		/* Resource busy		*/
#define	ECHILD		    6		/* No child processes		*/
#define	EDEADLK		    7		/* Resource deadlock would occur*/
#define	EDOM		    8		/* Domain error			*/
#define	EEXIST		    9		/* Already exists		*/
#define	EFAULT		    10		/* Bad address			*/
#define	EFBIG		    11		/* File too large		*/
#define	EINTR		    12		/* Interrupted system call	*/
#define	EINVAL		    13		/* Invalid argument		*/
#define	EIO		    14		/* Input/output error		*/
#define	EISDIR		    15		/* Wrong operation applied to directory*/
#define	EMFILE		    16		/* Too many open files		*/
#define	EMLINK		    17		/* Too many links		*/
#define	ENAMETOOLONG	    18		/* Filename too long		*/
#define	ENFILE		    19		/* Too many open files in system*/
#define	ENODEV		    20		/* No such device		*/
#define	ENOENT		    21		/* No such file or directory	*/
#define	ENOEXEC		    22		/* Exec format error		*/
#define	ENOLCK		    23		/* No locks available		*/
#define	ENOMEM		    24		/* Not enough memory space	*/
#define	ENOSPC		    25		/* No space left on device	*/
#define	ENOTDIR		    26		/* Not a directory		*/
#define	ENOTEMPTY	    27		/* Directory not empty		*/
#define	ENOTTY		    28		/* Inappropriate I/O operation	*/
#define	ENXIO		    29		/* No such device or address	*/
#define	EPERM		    30		/* Operation not permitted	*/
#define	EPIPE		    31		/* Broken pipe			*/
#ifndef ERANGE
#define	ERANGE		    32		/* Result too large		*/
#endif
#define	EROFS		    33		/* Read only file system	*/
#define	ESPIPE		    34		/* Invalid seek			*/
#define	ESRCH		    35		/* No such process		*/
#define	EXDEV		    36		/* Improper link		*/
/* BSD Extensions (mostly for sockets)	   				*/
#define	EWOULDBLOCK	    37		/* Operation will block		*/
#define	EINPROGRESS	    38		/* Operation now in progress	*/
#define	EALREADY	    39		/* Operation already in progress*/
#define	ENOTSOCK	    40		/* Socket operation on non-socket*/
#define	EDESTADDRREQ	    41		/* Destination address required	*/
#define	EMSGSIZE	    42		/* Message too long		*/
#define	EPROTOTYPE	    43		/* Protocol wrong type for socket*/
#define	ENOPROTOOPT	    44		/* Bad protocol option		*/
#define	EPROTONOSUPPORT	    45		/* Protocol not supported	*/
#define	ESOCKTNOSUPPORT	    46		/* Socket type not supported	*/
#define	EOPNOTSUPP	    47		/* Operation not supported on socket*/
#define	EPFNOSUPPORT	    48		/* Protocol family not supported*/
#define	EAFNOSUPPORT	    49		/* Address family not supported	*/
#define	EADDRINUSE	    50		/* Address already in use	*/
#define	EADDRNOTAVAIL	    51		/* Cannot assign requested address*/
#define	ENETDOWN	    52		/* Network is down		*/
#define	ENETUNREACH	    53		/* Network is unreachable	*/
#define	ENETRESET	    54		/* Network dropped connection on reset*/
#define	ECONNABORTED	    55		/* Software caused connection abort*/
#define	ECONNRESET	    56		/* Connection reset by peer	*/
#define	ENOBUFS		    57		/* No buffer space available	*/
#define	EISCONN		    58		/* Socket is already connected	*/
#define	ENOTCONN	    59		/* Socket is not connected	*/
#define	ESHUTDOWN	    60		/* Cannot send after shutdown	*/
#define	ETIMEDOUT	    61		/* Connection timed out		*/
#define	ECONNREFUSED	    62		/* Connection refused		*/
#define	EHOSTDOWN	    63		/* Host is down			*/
#define	EHOSTUNREACH	    64		/* No route to host		*/
/* BSD 4.4 codes (for UFS)  		   				*/
#define	ENOTBLK		    65		/* Block devive required	*/
#define	EFTYPE		    66		/* Inappropriate file type or format*/
#define	ELOOP		    67		/* Too many levels of symbolic links*/
#define	EUSERS		    68		/* Too many users		*/
#define	EDQUOT		    69		/* Disc quota exceeded		*/
#define	ESTALE		    70		/* Stale NFS file handle	*/
#define	ETXTBSY		    71		/* Text file busy		*/
#define	EPROCLIM	    72		/* Process limited has been reached*/
/* psuedo-error returned inside UNIX kernel to modify return to process	*/
#define	ERESTART	    -1		/* Restart syscall		*/

#define MAX_PERROR 72          /* maximum posix error code */



extern int errno;

#ifndef _POSIX_SOURCE
#ifdef _BSD

extern char * sys_errlist[];    /* error message array          */
extern int    sys_nerr;         /* size of same                 */

#endif
#endif


#endif

/* end of errno.h	    		   				*/
