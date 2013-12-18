/*------------------------------------------------------------------------
--                                                                      --
--          H E L I O S   I N P U T / O U T P U T   S E R V E R         --
--          ---------------------------------------------------         --
--                                                                      --
--               Copyright (C) 1993, Perihelion Software Ltd.,		--
--		  and Bleistein-Rohde Systemtechnik GmbH                --
--                            All Rights Reserved.                      --
--                                                                      --
--  scounix.h                                                           --
--                                                                      --
--  Author: BLV				                                --
--                                                                      --
------------------------------------------------------------------------*/
/* SccsId: 3.8 28/3/90\ Copyright (C) 1990, Perihelion Software Ltd.        */


/*
** Localisation header file for SCO Unix
*/

#define SIGURG	SIGUSR2

	/* These ioctl calls have the Posix names */
#define TCGETS   TCGETA
#define TCSETS   TCSETA
#define TCSETSW  TCSETAW

#define getdtablesize() 60

#ifdef NEVER

	/* HP9000 Unix has a logname routine */
#define logname my_logname

	/* These ioctl calls have the Posix names */
#define TCGETS   TCGETATTR
#define TCSETS   TCSETATTR
#define TCSETSW  TCSETATTRD

#define statfs(a, b, c, d) statfs(a, b)

#define seteuid(a) setresuid(-1, a, -1)

#define getdtablesize() 60

#endif
