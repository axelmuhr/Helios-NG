/*************************************************/
/*                                               */ 
/*   ARCHIPEL SA :   transputer.h                */ 
/*                                               */ 
/* Device driver for Volvox board :              */ 
/*                                               */
/*              VOLVOX-1/V                       */
/*                                               */
/* for SunOs v4.0.3                              */
/*                                               */
/* Release number :  version 1.0                 */ 
/*                                               */ 
/*            Copyright (c) 1989 ARCHIPEL SA     */
/*              All Rights Reserved              */
/*                                               */
/*************************************************/

/*
 * General-purpose symbols
 */
/*#define FALSE 0	*/	/* general false flag */
/*#define TRUE  1	*/	/* general true flag */

/*#ifndef _IOCTL_
#include <sys/ioctl.h>
#endif*/

#include <sys/ioccom.h>

/*
 * Device status flags
 */
#define ISR_DP	1	/* data present               */
#define ISR_IE  2	/* interrupt enable: intputit */
#define OSR_OR  1	/* output ready               */
#define OSR_IE  2	/* interrupt enable: outputit */
#define RZ_SP	16	/* reset principal system     */
#define ANA_SP  16	/* analyse principal system   */

#define MSK_ILO 8	/* IT link output mask        */
#define MSK_ILI 4	/* IT link input mask         */
#define MSK_IER 2       /* IT error                   */          
#define MSK_IST 1       /* IT soft                    */    


/*
 * Ioctl's have the command encoded in the lower word,
 * and the size of any in or out parameters in the upper
 * word.  The high 2 bits of the upper word are used
 * to encode the in/out status of the parameter; for now
 * we restrict parameters to at most 255 bytes.
 */


/*
 * Ioctl command codes
 */

#if (SOLARIS || __GNUC__)
#define VXV_RESET	_IOW('x',0,int)	    /* reset transputer card       */
#define VXV_ANALYSE	_IOW('x',1,int)	    /* analyse transputer card     */
#define VXV_STAT_READ 	_IOR('x',2,int)	    /* read transputer status      */
#define VXV_STAT_WRITE	_IOR('x',3,int)  	    /* write transputer status     */
#define VXV_TEST        _IO('x',4) 	    /* test transputer card        */

#define VXV_STAT_ERR    _IOR('x',10,char)      /* error status RO            */
#define VXV_STAT_IT     _IOR('x',11,char)      /* interupt status RO         */
#define VXV_STAT_MSKIT  _IOW('x',12,int)       /* interrupt mask WO          */

#define VXV_R_RBS       _IOR('x',20,int)      /* read buffer size R          */
#define VXV_W_RBS       _IOW('x',21,int)      /* read buffer size W          */
#define VXV_R_WBS       _IOR('x',22,int)      /* write buffer size R         */
#define VXV_W_WBS       _IOW('x',23,int)      /* write buffer size W         */

#define VXV_NDATR       _IOR('x',30,int)      /* number of read data RO      */
#define VXV_NDATW       _IOR('x',31,int)      /* number of write data RO     */

#define VXV_R_TOR       _IOR('x',40,int)      /* timeout read R              */
#define VXV_W_TOR       _IOW('x',41,int)      /* timeout read W              */
#define VXV_R_TOW       _IOR('x',42,int)      /* timeout write R             */
#define VXV_W_TOW       _IOW('x',43,int)      /* timeout write W             */
#define VXV_TRON        _IO('x',100)          /* trace on                    */
#define VXV_TROFF       _IO('x',101)          /* trace off                   */

/*    Handling of interrupt level : synchronous or asynchronous mode       */

#define VXV_FIONREAD    _IOR('x',127,int)   /* get # bytes to read           */
/* on output ready of link adaptor and async mode : a SIGIO is sent        */
#define VXV_WRITESYNC   _IO('x', 126)       /* set write sync mode           */
#define VXV_WRITEASYNC  _IOWR('x', 125,int)       /* set write async mode          */

/* on data present of link adaptor and async mode : a SIGIO is sent        */
#define VXV_READSYNC    _IO('x', 124)       /* set read sync mode            */
#define VXV_READASYNC   _IOWR('x', 123,int)       /* set read async mode           */

/* on error and async mode : a SIGURG is sent                              */
#define VXV_ERRORSYNC   _IOR('x', 122,int)  /* set error sync mode           */
#define VXV_ERRORASYNC  _IOWR('x',121,int) /* set error async mode          */

/* on async mode and software interrupt : a signal is sent */
#define VXV_SOFTITSYNC  _IOR('x', 120,int)  /* set soft it sync mode,        */
                                          /* return sig number             */
#define VXV_SOFTITASYNC _IOWR('x', 119,int)  /* set soft it async mode,       */
                                          /* and the signal number to send */
                                          /* default is SIGUSR1            */
#define VXV_SETMODE     _IOWR('x',118,int)   /* set mode for all              */
#define VXV_GETMODE     _IOR('x',117,int)   /* get mode for all              */

#else /* SOLARIS || __GNUC__ */
#define VXV_RESET	_IOW(x,0,int)	    /* reset transputer card       */
#define VXV_ANALYSE	_IOW(x,1,int)	    /* analyse transputer card     */
#define VXV_STAT_READ 	_IOR(x,2,int)	    /* read transputer status      */
#define VXV_STAT_WRITE	_IOR(x,3,int)  	    /* write transputer status     */
#define VXV_TEST        _IO(x,4) 	    /* test transputer card        */

#define VXV_STAT_ERR    _IOR(x,10,char)      /* error status RO            */
#define VXV_STAT_IT     _IOR(x,11,char)      /* interupt status RO         */
#define VXV_STAT_MSKIT  _IOW(x,12,int)       /* interrupt mask WO          */

#define VXV_R_RBS       _IOR(x,20,int)      /* read buffer size R          */
#define VXV_W_RBS       _IOW(x,21,int)      /* read buffer size W          */
#define VXV_R_WBS       _IOR(x,22,int)      /* write buffer size R         */
#define VXV_W_WBS       _IOW(x,23,int)      /* write buffer size W         */

#define VXV_NDATR       _IOR(x,30,int)      /* number of read data RO      */
#define VXV_NDATW       _IOR(x,31,int)      /* number of write data RO     */

#define VXV_R_TOR       _IOR(x,40,int)      /* timeout read R              */
#define VXV_W_TOR       _IOW(x,41,int)      /* timeout read W              */
#define VXV_R_TOW       _IOR(x,42,int)      /* timeout write R             */
#define VXV_W_TOW       _IOW(x,43,int)      /* timeout write W             */
#define VXV_TRON        _IO(x,100)          /* trace on                    */
#define VXV_TROFF       _IO(x,101)          /* trace off                   */

/*    Handling of interrupt level : synchronous or asynchronous mode       */

#define VXV_FIONREAD    _IOR(x,127,int)   /* get # bytes to read           */
/* on output ready of link adaptor and async mode : a SIGIO is sent        */
#define VXV_WRITESYNC   _IO(x, 126)       /* set write sync mode           */
#define VXV_WRITEASYNC  _IOWR(x, 125,int)       /* set write async mode          */

/* on data present of link adaptor and async mode : a SIGIO is sent        */
#define VXV_READSYNC    _IO(x, 124)       /* set read sync mode            */
#define VXV_READASYNC   _IOWR(x, 123,int)       /* set read async mode           */

/* on error and async mode : a SIGURG is sent                              */
#define VXV_ERRORSYNC   _IOR(x, 122,int)  /* set error sync mode           */
#define VXV_ERRORASYNC  _IOWR(x,121,int) /* set error async mode          */

/* on async mode and software interrupt : a signal is sent */
#define VXV_SOFTITSYNC  _IOR(x, 120,int)  /* set soft it sync mode,        */
                                          /* return sig number             */
#define VXV_SOFTITASYNC _IOWR(x, 119,int)  /* set soft it async mode,       */
                                          /* and the signal number to send */
                                          /* default is SIGUSR1            */
#define VXV_SETMODE     _IOWR(x,118,int)   /* set mode for all              */
#define VXV_GETMODE     _IOR(x,117,int)   /* get mode for all              */

#endif /* SOLARIS */

/*
 *
 *  Flag for ioctl set mode 
 *
 */
#define INPUTASYNC       0x00000001
#define INPUTNODELAY     0x00000002
#define INPUTBLOCKING    0x00000004
#define OUTPUTASYNC      0x00000010
#define OUTPUTNODELAY    0x00000020
#define OUTPUTBLOCKING   0x00000040
#define ERRORASYNC       0x00000100
#define SOFTITASYNC      0x00001000


/*
 *  default value for error and softit 
 */

#define SIG_DEFAULT_ON_ERROR    SIGURG
#define SIG_DEFAULT_ON_SOFTIT   SIGUSR1
#define SIG_DEFAULT_ON_READ     SIGIO
#define SIG_DEFAULT_ON_WRITE    SIGIO

/* 
 *   Device timeout.
 */

#define  VXV_TIME_NODELAY	 0
#define  VXV_TIME_BLOCKING      -1
