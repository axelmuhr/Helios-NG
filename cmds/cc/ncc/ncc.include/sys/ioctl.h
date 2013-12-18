
/*
 * 	@(#)ioctl.h	1.1.1.1	(ULTRIX)	11/9/89
 */

/************************************************************************
 *									*
 *			Copyright (c) 1985-1988 by			*
 *		Digital Equipment Corporation, Maynard, MA		*
 *			All rights reserved.				*
 *									*
 *   This software is furnished under a license and may be used and	*
 *   copied  only  in accordance with the terms of such license and	*
 *   with the  inclusion  of  the  above  copyright  notice.   This	*
 *   software  or  any	other copies thereof may not be provided or	*
 *   otherwise made available to any other person.  No title to and	*
 *   ownership of the software is hereby transferred.			*
 *									*
 *   This software is  derived	from  software	received  from	the	*
 *   University    of	California,   Berkeley,   and	from   Bell	*
 *   Laboratories.  Use, duplication, or disclosure is	subject  to	*
 *   restrictions  under  license  agreements  with  University  of	*
 *   California and with AT&T.						*
 *									*
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or	reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/

/*
 * ioctl.h
 *
 * Modification history
 *
 * Common ioctl definitions
 *
 * 25-Oct-89 - Janet L. Schank
 *
 *      Added a define for DEVGETGEOM
 *
 * 20-Sep-84 - Stephen Reilly
 *
 *	Add new ioctl functions for getting and setting disk
 *	parititioning.
 *
 *  6-Mar-85 - Larry Cohen
 *
 *	Add  ioctls  for  decnet: read physical addr, enable/clear
 *	loopback.
 *
 * 28-Mar-85 - David L Ballenger
 *
 *	Add  definitions  needed for System V emulation.  System V
 *	emulation is on when the symbol SYSTEM_FIVE is defined.
 *
 *  4-Apr-85 - Larry Cohen
 *
 *	Add ioctls to test and set/clear IINUSE in inode.  Supports
 *	open block if in use capability.
 *
 * 16-Sep-85 - Larry Cohen
 *
 *	Add new Berkeley ioctls to set/get window size.
 *	Rearrange ioctl definitions so they do not conflict with 43bsd.
 *	DECnet wakeup ioctl added in previous delta.
 *
 * 24-Sep-85 - Stephen Reilly
 *
 *	Added new request that will return the default partition table.
 *
 * 25-Sep-85 - Larry Cohen
 *
 *	Add window size structure (winsize).
 *
 * 20-Jan-86 - Barb Glover
 *
 *	Add ioctl calls for Error Logging.
 *
 *  4-Mar-86 - Ricky Palmer
 *
 *	Added tape ioctl definitions (moved from mtio.h). V2.0
 *	Added disk ioctl definitions (DKIOCDOP and DKIOCGET). V2.0
 *	Added generic device ioctl definition (DEVIOCGET). V2.0
 *	Moved tty structures to a new file (ttyio.h). V2.0
 *	Cleaned up file. V2.0
 *
 * 11-Mar-86 - Larry Palmer
 *
 *	Added new ioctls for n-bufferring.
 *
 * 12-Mar-86 - Larry Cohen
 *
 *	Add socket ioctl to keep track of device state. Requested
 *	by the DECnet folks.
 *
 *  1-May-86 - Ricky Palmer
 *
 *	Added DKIOCACC (disk access) ioctl request for bad block
 *	replacement effort. V2.0
 *
 * 03-Jun-86 - Barb Glover
 *	Added ELWARNON and ELWARNOFF to control console missed error
 *	printouts.
 *
 * 06-Jun-86 - Miriam Amos
 *
 *	Make additions and changes for svid termio.
 *
 * 09-jun-86 - Barb Glover
 *	Added ELGETTIME to save system startup time.
 *
 * 10-jun-86 - Tim Burke
 *	Added TERMIODISC which is used to specify termio line discipline. 
 *
 * 30-Jun-86 - Tim Burke
 *	Termio fix for 2.0F Baselevel.
 *
 * 30-Jun-86 - Peter Harbo
 *	Added LIOCINI to initialize lat tty's for host-initiated connects.
 *
 *  2-Jul-86 - David Metsky
 * 	Added casts to int to make lint stop complaining that there was
 * 	inconsistant use of the second argument in the ioctl call.
 *
 *  8-May-87 - Tim Burke
 *	Changed definition of L004000 to LPASS8.  This is being done to allow
 *	for 8-bit support with canonical processing.
 *
 * 23-Jul-87 - Vince Wallace
 *	Added definition of SIOCARPREQ.
 *
 * 27-Aug-87 - Tim Burke
 *	Changed definition of L001000 to LAUTOFLOW.  This allows the use of
 *	hardware managed start/stop ^S/^Q in the terminal drivers.
 *
 * 22-Oct-87 - Tim Burke
 *	Added ioctl definitions which are used for getting and setting the
 *	terminal attributes as specified by the POSIX termios data structure.
 *	Additions include: TCGETP, TCSANOW, TCSADRAIN, TCSADFLUSH.
 *	
 * 15-Jan-88	lp
 *	Merge of final 43BSD changes.
 *
 * 14-Mar-88 - Vince Wallace
 *	Added definition of LIOCTTYI.
 *
 * 20-Apr-88 - Tim Burke
 *	Moved kernel reference of <sgtty.h> to  "../h/sgtty.h".
 */

#ifndef _IOCTL_
#define _IOCTL_
#ifdef KERNEL
#include "../h/ttychars.h"
#include "../h/ttydev.h"
#include "../h/ttyio.h"
#ifndef _SGTTY_
#include "../h/sgtty.h"
#endif	_SGTTY_
#else  KERNEL
#ifndef SYSTEM_FIVE
#include <sys/ttychars.h>
#include <sys/ttydev.h>
#endif SYSTEM_FIVE
#include <sys/ttyio.h>
#ifndef _SGTTY_
#include <sgtty.h>
#endif	_SGTTY_
#endif KERNEL

#ifndef _IO
/*
 * Ioctl's have the command encoded in the lower word,
 * and the size of any in or out parameters in the upper
 * word.  The high 2 bits of the upper word are used
 * to encode the in/out status of the parameter; for now
 * we restrict parameters to at most 128 bytes.
 * The IOC_VOID field of 0x20000000 is defined so that new ioctls
 * can be distinguished from old ioctls.
 */
#define IOCPARM_MASK	0x7f		/* Parameters are < 128 bytes	*/
#define IOC_VOID	(int)0x20000000	/* No parameters		*/
#define IOC_OUT 	(int)0x40000000	/* Copy out parameters		*/
#define IOC_IN		(int)0x80000000	/* Copy in parameters		*/
#define IOC_INOUT	(int)(IOC_IN|IOC_OUT)
#define _IO(x,y)	(int)(IOC_VOID|(x<<8)|y)
#define _IOR(x,y,t)	(int)(IOC_OUT|((sizeof(t)&IOCPARM_MASK)<<16)|(x<<8)|y)
#define _IOW(x,y,t)	(int)(IOC_IN|((sizeof(t)&IOCPARM_MASK)<<16)|(x<<8)|y)
#define _IOWR(x,y,t)	(int)(IOC_INOUT|((sizeof(t)&IOCPARM_MASK)<<16)|(x<<8)|y)
#endif _IO

/* The tty i/o controls */
#define TIOCGETD	_IOR('t', 0, int) /* Get line discipline		*/
#define TIOCSETD	_IOW('t', 1, int) /* Set line discipline		*/
#define TIOCHPCL	_IO('t', 2)	/* Hangup on last close 	*/
#define TIOCMODG	_IOR('t', 3, int) /* Get modem control state	*/
#define TIOCMODS	_IOW('t', 4, int) /* Set modem control state	*/
#define TIOCM_LE	0x01		/* Line enable			*/
#define TIOCM_DTR	0x02		/* Data terminal ready		*/
#define TIOCM_RTS	0x04		/* Request to send		*/
#define TIOCM_ST	0x08		/* Secondary transmit		*/
#define TIOCM_SR	0x10		/* Secondary receive		*/
#define TIOCM_CTS	0x20		/* Clear to send		*/
#define TIOCM_CAR	0x40		/* Carrier detect		*/
#define TIOCM_CD	TIOCM_CAR
#define TIOCM_RNG	0x80		/* Ring 			*/
#define TIOCM_RI	TIOCM_RNG
#define TIOCM_DSR	0x100		/* Data set ready		*/
#define TIOCGETP	_IOR('t', 8,struct sgttyb_ULTRIX)/* Gtty params.	*/
#define TIOCSETP	_IOW('t', 9,struct sgttyb_ULTRIX)/* Stty params.	*/
#define TIOCSETN	_IOW('t',10,struct sgttyb_ULTRIX)/* No flushtty	*/
#define TIOCEXCL	_IO('t', 13)	/* Set exclusive use of tty	*/
#define TIOCNXCL	_IO('t', 14)	/* Reset exclusive use of tty	*/
#define TIOCFLUSH	_IOW('t', 16, int)/* Flush buffers		*/
#define TIOCSETC	_IOW('t',17,struct tchars)/* Set special chars.	*/
#define TIOCGETC	_IOR('t',18,struct tchars)/* get special chars.	*/

#ifndef SYSTEM_FIVE
/*
 * These are defined in <sys/termio.h> if we're doing ULTRIX System V
 * emulation.
 */
#define TANDEM		0x01		/* Send stopc on output q. full */
#define CBREAK		0x02		/* Half-cooked mode		*/
#define LCASE		0x04		/* Simulate lower case		*/
#define ECHO		0x08		/* Echo input			*/
#define CRMOD		0x10		/* Map \r to \r\n on output	*/
#define RAW		0x20		/* No i/o processing		*/
#define ODDP		0x40		/* Get/send odd parity		*/
#define EVENP		0x80		/* Get/send even parity 	*/
#define ANYP		0xc0		/* Get any parity/send none	*/
#define NLDELAY 	0x300		/* \n delay base		*/
#define NL0		0x000		/* No \n delay			*/
#define NL1		0x100		/* Tty 37 \n delay		*/
#define NL2		0x200		/* Vt05 \n delay		*/
#define NL3		0x300
#define TBDELAY 	0xc00		/* Horizontal tab delay base	*/
#define TAB0		0x000		/* No hor. tab delay		*/
#define TAB1		0x400		/* Tty 37 hor. tab delay	*/
#define TAB2		0x800
#define XTABS		0xc00		/* Expand tabs on output	*/
#define CRDELAY 	0x3000		/* \r delay base		*/
#define CR0		0x0000
#define CR1		0x1000		/* Tn 300 \r delay		*/
#define CR2		0x2000		/* Tty 37 \r delay		*/
#define CR3		0x3000		/* Concept 100 delay		*/
#define VTDELAY 	0x4000		/* Vertical tab delay base	*/
#define FF0		0x0000
#define FF1		0x4000		/* Tty 37 ver. tab delay	*/
#define BSDELAY 	0x8000		/* \b delay base		*/
#define BS0		0x0000
#define BS1		0x8000
#define ALLDELAY	(NLDELAY|TBDELAY|CRDELAY|VTDELAY|BSDELAY)
#define CRTBS		(int)0x10000 	/* Do backspacing for crt	*/
#define PRTERA		(int)0x20000 	/* \ ... / erase		*/
#define CRTERA		(int)0x40000 	/* " \b " to wipe out char	*/
#define TILDE		(int)0x80000 	/* Hazeltine tilde kludge	*/
#define MDMBUF		(int)0x100000	/* Start/stop output on c.intr. */
#define LITOUT		(int)0x200000	/* Literal output		*/
#define TOSTOP		(int)0x400000	/* SIGSTOP on background output */
#define FLUSHO		(int)0x800000	/* Flush output to terminal	*/
#define NOHANG		(int)0x1000000	/* No SIGHUP on carrier drop	*/
#define AUTOFLOW 	(int)0x2000000	/* IAUTO hardware start/stop	*/
#define CRTKIL		(int)0x4000000	/* Kill line with " \b "	*/
#define PASS8	 	(int)0x8000000	/* Allow 8-bit with canonical   */
#define CTLECH		(int)0x10000000	/* Echo control chars as ^X	*/
#define PENDIN		(int)0x20000000	/* tp->t_rawq needs reread	*/
#define DECCTQ		(int)0x40000000	/* Only ^Q starts after ^S	*/
#define BNOFLSH		(int)0x80000000	/* No output flush on signal	*/
#endif SYSTEM_FIVE

/* locals, from 127 down */
#define TIOCLBIS	_IOW('t', 127, int)	/* Bis local mode bits	*/
#define TIOCLBIC	_IOW('t', 126, int)	/* Bic local mode bits	*/
#define TIOCLSET	_IOW('t', 125, int)	/* Set entire l.m. word */
#define TIOCLGET	_IOR('t', 124, int)	/* Get local modes	*/
#define LCRTBS		(int)(CRTBS>>16)
#define LPRTERA 	(int)(PRTERA>>16)
#define LCRTERA 	(int)(CRTERA>>16)
#define LTILDE		(int)(TILDE>>16)
#define LMDMBUF 	(int)(MDMBUF>>16)
#define LLITOUT 	(int)(LITOUT>>16)
#define LTOSTOP 	(int)(TOSTOP>>16)
#define LFLUSHO 	(int)(FLUSHO>>16)
#define LNOHANG 	(int)(NOHANG>>16)
#define LAUTOFLOW 	(int)(AUTOFLOW>>16)
#define LCRTKIL 	(int)(CRTKIL>>16)
#define LPASS8	 	(int)(PASS8>>16)
#define LCTLECH 	(int)(CTLECH>>16)
#define LPENDIN 	(int)(PENDIN>>16)
#define LDECCTQ 	(int)(DECCTQ>>16)
#define LNOFLSH 	(int)(BNOFLSH>>16)
#define TIOCSBRK	_IO('t', 123)		/* Set break bit	*/
#define TIOCCBRK	_IO('t', 122)		/* Clear break bit	*/
#define TIOCSDTR	_IO('t', 121)		/* Set data term. ready */
#define TIOCCDTR	_IO('t', 120)		/* Clear data term. rdy.*/
#define TIOCGPGRP	_IOR('t', 119, int)	/* Get pgrp of tty	*/
#define TIOCSPGRP	_IOW('t', 118, int)	/* Set pgrp of tty	*/
#define TIOCSLTC	_IOW('t',117,struct ltchars)/* Set loc. sp. chars.*/
#define TIOCGLTC	_IOR('t',116,struct ltchars)/* Get loc. sp. chars.*/
#define TIOCOUTQ	_IOR('t', 115, int)	/* Output queue size	*/
#define TIOCSTI 	_IOW('t', 114, char)	/* Simulate term. input */
#define TIOCNOTTY	_IO('t', 113)		/* Void tty association */
#define TIOCPKT 	_IOW('t', 112, int)	/* Pty: set/clr. p. mode*/
#define TIOCPKT_DATA		0x00	/* Data packet			*/
#define TIOCPKT_FLUSHREAD	0x01	/* Flush packet 		*/
#define TIOCPKT_FLUSHWRITE	0x02	/* Flush packet 		*/
#define TIOCPKT_STOP		0x04	/* Stop output			*/
#define TIOCPKT_START		0x08	/* Start output 		*/
#define TIOCPKT_NOSTOP		0x10	/* No more ^S, ^Q		*/
#define TIOCPKT_DOSTOP		0x20	/* Now do ^S ^Q 		*/
#define TIOCPKT_IOCTL		0x40	/* Wake up if change term char. */
#define TIOCSTOP	_IO('t', 111)		/* Stop output, like ^S */
#define TIOCSTART	_IO('t', 110)		/* Start out., like ^Q	*/
#define TIOCMSET	_IOW('t', 109, int)	/* Set all modem bits	*/
#define TIOCMBIS	_IOW('t', 108, int)	/* Bis modem bits	*/
#define TIOCMBIC	_IOW('t', 107, int)	/* Bic modem bits	*/
#define TIOCMGET	_IOR('t', 106, int)	/* Get all modem bits	*/
#define TIOCREMOTE	_IOW('t', 105, int)	/* Remote input editing */
#define TIOCGWINSZ	_IOR('t', 104, struct winsize)	/* Get win. sz. */
#define TIOCSWINSZ	_IOW('t', 103, struct winsize)	/* Set win. sz. */
#define TIOCUCNTL	_IOW('t', 102, int)	/* Pty: set/clr u.c.mode*/
#define TIOCSMLB	_IO('t', 101)		/* Turn on loopback mode*/
#define TIOCCMLB	_IO('t', 100)		/* Turn off loop. mode	*/
#define TIOCNMODEM	_IOW('t', 99, int)	/* Ignore modem status	*/
#define TIOCMODEM	_IOW('t', 98, int)	/* Look at modem status */
#define TIOCWONLINE	_IO('t', 97)		/* Wait on online device*/
#define TIOCNCAR	_IO('t', 96)		/* Ignore soft carrier	*/
#define TIOCCAR 	_IO('t', 95)		/* Don't ignore s. car. */
#define	TCSBRK		_IO('t',94)		/* Flush q's w/ cnd. brk*/
#define	TCXONC		_IO('t',93)		/* Start/stop control	*/
#define	TCFLSH		_IO('t',92)		/* Cnd. q flushing	*/
#define	TCGETA		_IOR('t',91,struct termio)/* Get parameters	*/
#define	TCSETA		_IOW('t',90,struct termio)/* Set parameters	*/
#define	TCSETAW		_IOW('t',89,struct termio)/* Drain & set		*/
#define	TCSETAF		_IOW('t',88,struct termio)/* Drain, flush, & set	*/
#define TIOCMASTER	_IOW('t',87, int)		/* master ctrls flags   */
#define TIOAUTO		_IO('t', 86)		/* Autoflow status 	*/
#define TIOCSINUSE	FIOSINUSE		/* Test and set mutex	*/
#define TIOCCINUSE	FIOCINUSE		/* Clear mutex		*/
						/* POSIX get & sets	*/
#define	TCGETP		_IOR('t',85,struct termios)/* Get parameters	*/
#define	TCSANOW		_IOW('t',84,struct termios)/* Set parameters	*/
#define	TCSADRAIN	_IOW('t',83,struct termios)/* Drain & set		*/
#define	TCSADFLUSH	_IOW('t',82,struct termios)/* Drain, flush, & set	*/

#define OTTYDISC	0x00		/* Old, v7 std tty driver	*/
#define NETLDISC	0x01		/* Line discipline for berk net */
#define NTTYDISC	0x02		/* New tty discipline		*/
#define TABLDISC	0x03		/* Hitachi tablet discipline	*/
#define NTABLDISC	0x04		/* Gtco tablet discipline	*/
#define HCLDISC 	0x05		/* Half cooked discipline	*/
#define TERMIODISC	0x06		/* termio line discipline	*/
#define SLPDISC		0x07		/* BSD Serial Line IP		*/
					/* Line disc #'s 16-23 are 
					   reserved for local extension.*/

/* File i/o controls */
#define FIOCLEX 	_IO('f', 1)	/* Set exclusive use on fd	*/
#define FIONCLEX	_IO('f', 2)	/* Remove exclusive use 	*/
#define FIOSINUSE	_IO('f', 3)	/* Test & set IINUSE in inode	*/
#define FIOCINUSE	_IO('f', 4)	/* Clear mutex			*/
#define FIONREAD	_IOR('f', 127, int)	/* Get # bytes to read	*/
#define FIONBIO 	_IOW('f', 126, int)	/* Set/clear non-bl.i/o */
#define FIOASYNC	_IOW('f', 125, int)	/* Set/clear async i/o	*/
#define FIOSETOWN	_IOW('f', 124, int)	/* Set owner		*/
#define FIOGETOWN	_IOR('f', 123, int)	/* Get owner		*/
#define FIONBUF 	_IOW('f', 122, int)	/* N_buff i/o buf	*/
#define FIONONBUF	_IO('f', 121)		/* N_buff i/o on buf	*/
#define FIONBDONE	_IOW('f', 120, int)	/* N_buff i/o done buf	*/

/* Socket i/o controls */
#define SIOCSHIWAT	_IOW('s',  0, int)	/* Set high watermark	*/
#define SIOCGHIWAT	_IOR('s',  1, int)	/* Get high watermark	*/
#define SIOCSLOWAT	_IOW('s',  2, int)	/* Set low watermark	*/
#define SIOCGLOWAT	_IOR('s',  3, int)	/* Get low watermark	*/
#define SIOCATMARK	_IOR('s',  7, int)	/* At oob mark? 	*/
#define SIOCSPGRP	_IOW('s',  8, int)	/* Set process group	*/
#define SIOCGPGRP	_IOR('s',  9, int)	/* Get process group	*/

#define SIOCADDRT	_IOW('r', 10, struct rtentry)	/* Add route	*/
#define SIOCDELRT	_IOW('r', 11, struct rtentry)	/* Delete route */

#define SIOCSIFADDR	_IOW('i', 12, struct ifreq)	/* Set ifnet ad.*/
#define SIOCGIFADDR	_IOWR('i',13, struct ifreq)	/* Get ifnet ad.*/
#define SIOCSIFDSTADDR	_IOW('i', 14, struct ifreq)	/* Set p-p addr.*/
#define SIOCGIFDSTADDR	_IOWR('i',15, struct ifreq)	/* Get p-p addr.*/
#define SIOCSIFFLAGS	_IOW('i', 16, struct ifreq)	/* Set ifnet fl.*/
#define SIOCGIFFLAGS	_IOWR('i',17, struct ifreq)	/* Get ifnet fl.*/
#define SIOCGIFBRDADDR	_IOWR('i',18, struct ifreq)	/* Get broad.ad.*/
#define SIOCSIFBRDADDR	_IOW('i',19, struct ifreq)	/* Set broad.ad.*/
#define SIOCGIFCONF	_IOWR('i',20, struct ifconf)	/* Get ifnet ls.*/
#define SIOCGIFNETMASK	_IOWR('i',21, struct ifreq)	/* Get n.a.mask */
#define SIOCSIFNETMASK	_IOW('i',22, struct ifreq)	/* Set n.a.mask */
#define SIOCSPHYSADDR	_IOWR('i',23, struct ifreq)	/* Set phys. ad.*/
#define SIOCADDMULTI	_IOWR('i',24, struct ifreq)	/* Add m.c. ad. */
#define SIOCDELMULTI	_IOWR('i',25, struct ifreq)	/* Dele. m.c.ad.*/
#define SIOCRDCTRS	_IOWR('i',26, struct ctrreq)	/* Read if cntr.*/
#define SIOCRDZCTRS	_IOWR('i',27, struct ctrreq)	/* Read/0 if c. */
#define SIOCRPHYSADDR	_IOWR('i',28, struct ifdevea)	/* Read phy. ad.*/
#define SIOCSARP	_IOW('i', 30, struct arpreq)	/* Set arp entry*/
#define SIOCGARP	_IOWR('i', 31, struct arpreq)	/* Get arp entry*/
#define SIOCDARP	_IOW('i', 32, struct arpreq)	/* Del. arp ent.*/
#define SIOCENABLBACK	_IOW('i',33, struct ifreq)	/* Set in.ex.lb.*/
#define SIOCDISABLBACK	_IOW('i',34, struct ifreq)	/* Cl.in.ex.lpb.*/
#define SIOCSTATE	_IOWR('i',35, struct ifstate)	/* Device state */
#define LIOCSOL		_IOWR('i',36, struct solicit_1)	/* send solicit msg */
#define LIOCRES		_IOWR('i',37, struct response_1)	/* get response msg */
#define LIOCCMD		_IOWR('i',38, struct lat_ucom)	/* send command msg */
#define LIOCINI		_IOWR('i',39, struct lat_ini)	/* lat tty init */
#define SIOCARPREQ	_IOWR('i',40, struct ifreq)	/* arp request pkt */
#define SIOCGIFMETRIC _IOWR('i', 41, struct ifreq)      /* get IF metric */
#define SIOCSIFMETRIC _IOW('i', 42, struct ifreq)       /* set IF metric */
#define LIOCTTYI	_IOR('i', 43, struct ltattyi)	/* lat tty info */


/* Disk partition table i/o controls */
#define DIOCGETPT	_IOR('p', 1, struct pt)	/* Get disk paritition	*/
#define DIOCSETPT	_IOW('p', 2, struct pt)	/* Set disk paritition	*/
#define DIOCDGTPT	_IOR('p', 3, struct pt)	/* Get default disk par.*/

/* Error logging i/o controls */
#define ELSETPID	_IOWR('e', 0, struct elparam)	/* Set proc. id.*/
#define ELGETPID	_IOR('e', 1, int) 		/* Get proc. id.*/
#define ELMOVPTR	_IOW('e', 2, int) 		/* Update elpts.*/
#define ELREINIT	_IO('e', 3)			/* Reinit elbuf */
#define ELCLRPID	_IO('e', 4)			/* Clr. proc.id.*/
#define ELWARNOFF	_IO('e', 5)			/* disable warn.*/
#define ELWARNON	_IO('e', 6)			/* disable warn.*/
#define ELGETTIME	_IOR('e', 7, int)			/* Get strt time*/

/* Tape i/o controls */
#define MTIOCTOP	_IOW('m', 1, struct mtop) 	/* Do a tape op.*/
#define MTIOCGET	_IOR('m', 2, struct mtget)	/* Get status	*/

/* Disk i/o controls */
#define DKIOCHDR	_IO('d', 1)			/* Header r/w	*/
#define DKIOCDOP	_IOW('d', 2, struct dkop) 	/* Do a disk op.*/
#define DKIOCGET	_IOR('d', 3, struct dkget)	/* Get status	*/
#define DKIOCACC	_IOWR('d', 4, struct dkacc)	/* Disk access	*/

/* Generic device information i/o controls */
#define DEVIOCGET	_IOR('v', 1, struct devget)	/* Get dev.info.*/
#define DEVGETGEOM	_IOR('v', 2, DEVGEOMST )		/* Get geometry */

/*
 * The ioctl request code 'k' should be used for local language extension.
 * as an example, the Japanese Language Support Group uses the following:
 * #define TIOCKGET 	_IOR('k', 0, int)
 */

#endif _IOCTL_
