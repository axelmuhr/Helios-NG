/*------------------------------------------------------------------------
--                                                                      --
--          H E L I O S   I N P U T / O U T P U T   S E R V E R         --
--          ---------------------------------------------------         --
--                                                                      --
--               Copyright (C) 1990, Perihelion Software Ltd.,		--
--		  and Bleistein-Rohde Systemtechnik GmbH                --
--                            All Rights Reserved.                      --
--                                                                      --
--  unix386local.h                                                      --
--                                                                      --
--  Author: BLV				                                --
--                                                                      --
------------------------------------------------------------------------*/
/* SccsId: 3.8 28/3/90\ Copyright (C) 1990, Perihelion Software Ltd.        */

#ifdef Local_Module

#ident	"@(#)sgtty.h	2.3 - 88/05/27"
/*	3.0 SID #	1.1	*/
/*
 * Structure for stty and gtty system calls.
 */

struct sgttyb {
	char	sg_ispeed;		/* input speed */
	char	sg_ospeed;		/* output speed */
	char	sg_erase;		/* erase character */
	char	sg_kill;		/* kill character */
	int	sg_flags;		/* mode flags */
};

/*
 * Modes
 */
#define	XTABS	02
#define	LCASE	04
#define	CRMOD	020
#define	RAW	040
#define	ODDP	0100
#define	EVENP	0200
#define ANYP	0300
#define	NLDELAY	001400
#define	TBDELAY	002000
#define	CRDELAY	030000
#define	VTDELAY	040000
#define BSDELAY 0100000
#define ALLDELAY 0177400


#ident	"@(#)ttold.h	2.3 - 88/05/27"
/*
 * List of special characters
 */
struct tc	{
	unsigned char	t_intrc;
	unsigned char	t_quitc;
	unsigned char	t_startc;
	unsigned char	t_stopc;
	unsigned char	t_eofc;
	unsigned char	t_brkc;
};

#define tchars tc

#define	tIOC	('t'<<8)
#define	TIOCGETP	(tIOC|8)
#define	TIOCSETP	(tIOC|9)

#endif  /* Local_Module */

/**
*** This should be in /usr/include/sys/un.h, but is missing.
**/
struct	sockaddr_un {
	short	sun_family;		/* AF_UNIX */
	char	sun_path[108];		/* path name (gag) */
};


