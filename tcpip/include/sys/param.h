
/* for source compatability */

#include <sys/types.h>
#include <errno.h>
#include <sys/time.h>

#define vax			/* YUK */
#define sun 0

#define BSD 43			/* pretend to be BSD 4.3 */

#define KERNEL

#define INET

#define __IN_SERVER__

#ifndef NULL
#define NULL 0
#endif

#define CLBYTES 1024
#define CLOFSET	(CLBYTES-1)
#define CLSHIFT 10

#define CLSIZE	CLBYTES
extern int mbmap;
#define	NBPG	1024
extern int proc;
#define CSYS	0

#define PZERO	0

extern int hz;

extern struct timeval SYSTIME;

#define time SYSTIME

#define LOG_ERR		3

#define SIGPIPE		10
#define SIGURG		16
#define	SIGIO		24

#define	SIRR		0

#define splx(x)		((x) == (x) ? 0 : 0)
#define splnet()	(0)
#define splimp()	(0)

#define suser()		(1)

#define mtpr(a,b)

#define gsignal(x,y)

#define bcopy(a,b,c)	memcpy(b,a,c)
#define	bzero(a,b)	memset(a,0,b)
#define bcmp(a,b,c)	memcmp(a,b,c)
#define copyout(a,b,c)	(bcopy(a,b,c),0)
#define copyin(a,b,c)	(bcopy(a,b,c),0)

#define MIN(a,b)	((a)<(b)?(a):(b))
#define imin(a,b)	MIN(a,b)
#define MAX(a,b)	((a)>(b)?(a):(b))

#define useracc(a,b,c)	(1)

#define MMalloc Malloc
#define FFree Free
