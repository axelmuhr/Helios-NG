/*
 * $Header: ttychars.h,v 1.1 90/01/13 20:13:05 charles Locked $
 * $Source: /server/usr/users/charles/world/drawp/RCS/lochdr/sys/ttychars.h,v $
 *
 * Copyright (c) 1988 Acorn Computers Ltd., Cambridge, England
 *
 * $Desc$
 * $Log:	ttychars.h,v $
 * Revision 1.1  90/01/13  20:13:05  charles
 * Initial revision
 * 
 * Revision 1.8  89/12/15  16:34:35  pete
 * File unchanged
 * 
 * Revision 1.7  89/11/03  15:00:11  charles
 * File unchanged
 * 
 * Revision 1.6  89/08/15  13:43:05  charles
 * File unchanged
 * 
 * Revision 1.5  89/08/11  17:47:13  charles
 * File unchanged
 * 
 * Revision 1.4  89/08/10  19:53:25  charles
 * File unchanged
 * 
 * Revision 1.3  89/08/10  19:41:41  charles
 * File unchanged
 * 
 * Revision 1.2  89/08/10  19:00:55  charles
 * File unchanged
 * 
 * Revision 1.1  89/08/10  16:23:56  charles
 * Initial revision
 * 
 * Revision 1.2  89/08/10  15:46:45  charles
 * "just_to_release_lock"
 * 
 * Revision 1.1  89/07/10  15:31:14  charles
 * Initial revision
 * 
 * Revision 1.3  88/06/17  20:22:08  beta
 * Acorn Unix initial beta version
 * 
 */
/* @(#)ttychars.h	1.2 87/05/15 3.2/4.3NFSSRC */
/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)ttychars.h	7.1 (Berkeley) 6/4/86
 */

/*
 * User visible structures and constants
 * related to terminal handling.
 */
#ifndef _TTYCHARS_
#define	_TTYCHARS_
struct ttychars {
	char	tc_erase;	/* erase last character */
	char	tc_kill;	/* erase entire line */
	char	tc_intrc;	/* interrupt */
	char	tc_quitc;	/* quit */
	char	tc_startc;	/* start output */
	char	tc_stopc;	/* stop output */
	char	tc_eofc;	/* end-of-file */
	char	tc_brkc;	/* input delimiter (like nl) */
	char	tc_suspc;	/* stop process signal */
	char	tc_dsuspc;	/* delayed stop process signal */
	char	tc_rprntc;	/* reprint line */
	char	tc_flushc;	/* flush output (toggles) */
	char	tc_werasc;	/* word erase */
	char	tc_lnextc;	/* literal next character */
};

#define	CTRL(c)	('c'&037)

/* default special characters */
#define	CERASE	0177
#define	CKILL	CTRL(u)
#define	CINTR	CTRL(c)
#define	CQUIT	034		/* FS, ^\ */
#define	CSTART	CTRL(q)
#define	CSTOP	CTRL(s)
#define	CEOF	CTRL(d)
#define	CEOT	CEOF
#define	CBRK	0377
#define	CSUSP	CTRL(z)
#define	CDSUSP	CTRL(y)
#define	CRPRNT	CTRL(r)
#define	CFLUSH	CTRL(o)
#define	CWERASE	CTRL(w)
#define	CLNEXT	CTRL(v)
#endif
