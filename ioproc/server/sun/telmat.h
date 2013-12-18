#ifndef telmat_h
#define telmat.h

#define FALSE 0		/* general false flag */
#define TRUE  1		/* general true flag */

#ifndef _IOCTL_
#include <sys/ioctl.h>
#endif


#define VXV_RESET	_IOW(x,0,int)	    /* reset transputer card       */
#define LNRESET_ROOT	_IOW(x,1,int)
#define VXV_ANALYSE	_IOW(x,2,int)	    /* analyse transputer card     */
#define LNRESET_ANALYSE	_IOW(x,3,int)
#define VXV_STAT_READ 	_IOR(x,4,int)	    /* read transputer status      */
#define LNTESTREAD	_IOR(x,5,int)
#define VXV_STAT_WRITE	_IOR(x,6,int)  	    /* write transputer status     */
#define LNTESTWRITE  	_IOR(x,7,int)

#define VXV_STAT_ERR    _IOR(x,10,char)      /* error status RO            */
#define LNTESTERROR	_IOR(x,11,char)	 

#define VXV_R_TOR       _IOR(x,20,int)      /* timeout read R              */
#define LNGETTIMEOUT	_IOR(x,21,int)
#define VXV_W_TOR       _IOW(x,22,int)      /* timeout read W              */
#define LNSETTIMEOUT	_IOW(x,23,int)
#define VXV_R_TOW       _IOR(x,24,int)      /* timeout write R             */
#define VXV_W_TOW       _IOW(x,25,int)      /* timeout write W             */


#endif
