
/* used by ldker.c */
/*------------------------------------------------------------------------
--                                                                      --
--                     H E L I O S   N U C L E U S                      --
--                     ---------------------------                      --
--                                                                      --
--             Copyright (C) 1987, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- config.h								--
--                                                                      --
--	Configuration vector						--
--                                                                      --
--	Author:  NHG 7/9/87						--
--                                                                      --
------------------------------------------------------------------------*/
/* SccsId: %W% %G% Copyright (C) 1987, Perihelion Software Ltd.	*/
/* $Id: config.h,v 1.3 90/10/17 14:58:25 nick Exp $ */

#ifndef __config_h
#define __config_h

#ifndef __helios_h
#include <helios.h>
#endif

typedef struct LinkConf {
	byte	Flags;			/* initial flags		*/
	byte	Mode;			/* link mode			*/
	byte	State;			/* initial state		*/
	byte	Id;			/* link id			*/
} LinkConf;

typedef struct Config {
	word		PortTabSize;	/* # slots in port table	*/
	word		Incarnation;	/* what booter believes our incarnation is */
	word		*LoadBase;	/* address at which system was loaded */
	word		ImageSize;	/* size of system image		*/
	word		Date;		/* current system date		*/
	word		FirstProg;	/* offset of initial program	*/
	word		MemSize;	/* if > 0, size of RAM		*/
	word		Flags;		/* various flags, see root.h	*/
	word		Spare[1];	/* some spare slots		*/
	RPTR		MyName;		/* full path name		*/
	RPTR		ParentName;	/* ditto			*/
	word		NLinks;		/* number of links		*/
	word	LinkConf[4];	/* NLinks LinkConf structs 	*/
	/* name strings follow on from here */
} Config;

/* the LinkConf structures must be in the same order as the link	*/
/* channels in low memory.						*/


/* v1.2 permanently saves config after root */
/* #include <root.h> / * included at this point to reduce circular dependecies */
#define GetConfig() ((Config *)(GetRoot()->Configuration))


/* Image Vector */
/* resident libraries included in the system image *must* be in slot order */

#ifdef __HELIOSARM
/* Special in that it holds all libraries in its system image */
/* and doesn't have a bootstrap */
# define IVecISize	0
# define IVecKernel	1
# define IVecSysLib	2
# define IVecServLib	3
# define IVecUtil	4
# define IVecFPLib	5
# define IVecPosix	6
# define IVecCLib	7
# define IVecFault	8

/* add other libraries here */
# define IVecProcMan	9
# define IVecServers	10

#else

/* standard transputer nucleus contents: */
# define IVecISize	0
# define IVecKernel	1
# define IVecSysLib	2
# define IVecServLib	3
# define IVecUtil	4
# define IVecBootStrap	5
# define IVecProcMan	6
# define IVecServers	7
#endif

#endif

/* -- End of config.h */

