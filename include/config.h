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
/* $Id: config.h,v 1.13 1993/07/27 13:59:18 paul Exp $ */

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

	MPtr		LoadBase;	/* address at which system was loaded */
	word		ImageSize;	/* size of system image		*/
					/* (overwritten by kernel) */

	word		Date;		/* current system date		*/
	word		FirstProg;	/* offset of initial program	*/
					/* 0 = default to procman	*/

	word		MemSize;	/* if > 0, size of RAM		*/
	word		Flags;		/* initial root struct flags 	*/

#ifdef __ARM
	word		Speed;		/* Link serial comms speed (baud) */
#else
	word		Spare[1];	/* some spare slots		*/
#endif
	RPTR		MyName;		/* full path name		*/
	RPTR		ParentName;	/* ditto			*/

	word		NLinks;		/* number of links		*/

#ifdef __C40
	LinkConf	LinkConf[6];	/* NLinks LinkConf structs 	*/
#else
	LinkConf	LinkConf[4];	/* NLinks LinkConf structs 	*/
#endif
	/* name strings follow on from here */
/* the LinkConf structures must be in the same order as the link	*/
/* channels in low memory.						*/
} Config;


/* v1.2 permanently saves config after root */
/* #include <root.h> / * included at this point to reduce circular dependecies */
#define GetConfig() ((Config *)(GetRoot()->Configuration))

/* define C40 specific Hardware Config flags in initial word sent to bootstrap */
#define	HW_NucleusLocalS0	0	/* load nuc. into local bus strobe 0 */
					/* above is default */
#define	HW_NucleusLocalS1	1	/* load nuc. into local bus strobe 1 */
#define	HW_NucleusGlobalS0	2	/* load nuc. into global bus strobe 0 */
#define	HW_NucleusGlobalS1	4	/* load nuc. into global bus strobe 1 */
#define	HW_PseudoIDROM		8	/* download and use pseudo IDROM */
#define	HW_ReplaceIDROM		16	/* download and replace existing IDROM */
#define	HW_CacheOff		32	/* dont enable cache */

#ifdef __C40
/* return hardware config word sent to our bootstrap */
word GetHWConfig(void);
#else
#define GetHWConfig() 0
#endif

/* Image Vector */
/* resident libraries included in the system image *must* be in slot order */

#ifdef __ABC
/* Special in that it holds all libraries in its system image */
/* and doesn't have a bootstrap */
# define IVecISize	0
# define IVecKernel	1
# define IVecSysLib	2
# define IVecServLib	3
# define IVecUtil	4
# define IVecABClib	5
# define IVecPosix	6
# define IVecCLib	7
# define IVecFault	8
# define IVecFPLib	9
# define IVecPatchLib	10

/* Add other libraries above here */
# define IVecProcMan	11
# define IVecServers	12

#else

/* Standard Transputer/C40/ARM nucleus contents: */
# define IVecISize	0
# define IVecKernel	1
# define IVecSysLib	2
# define IVecServLib	3
# define IVecUtil	4
#ifdef __ARM
# define IVecProcMan	5
# define IVecServers	6
#else
# define IVecBootStrap	5
# define IVecProcMan	6
# define IVecServers	7
#endif
#endif

#endif

/* -- End of config.h */

