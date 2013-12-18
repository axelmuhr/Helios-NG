/*------------------------------------------------------------------------
--                                                                      --
--  This header file is used to maintain version numbers.               --
--                                                                      --
------------------------------------------------------------------------*/

/* RcsId: $Id: sccs.h,v 1.48 1994/07/14 10:39:54 mgun Exp $ */

static char SccsId1[] =
" V3.115 14th July 1994";

#ifdef DEMONSTRATION
static char SccsId5 = "\n\rCopyright (C) Perihelion Distributed Software 1987-1994\r\n\
All rights reserved.\n\n\rDEMONSTRATION VERSION, READ ONLY. NOT FOR RESALE.\r\n\
This system will self-destruct in ten minutes.\n\n\r";
#else 
static char SccsId5[] =
"\n\rCopyright (C) Perihelion Distributed Software 1987-1994\r\n\
All rights reserved.\n\r";
#endif

#if ST
static char SccsId2[] = "ST";
#endif

#if PC

#ifdef SYNERGY
static char SccsId2[] = "PC (synergy)";
#else
#ifdef GEMINI
static char SccsId2[] = "TranStar Router";
#else
#if MSWINDOWS
static char SccsId2[] = "Windows";
#else
static char SccsId2[] = "PC";
#endif  /* MSWINDOWS */
#endif  /* GEMINI */
#endif  /* SYNERGY */

#endif

#if NECPC
static char SccsId2[] = "NECPC";
#endif

#if AMIGA
static char SccsId2[] = "Amiga";
#endif

#if TRIPOS
static char SccsId2[] = "Tripos";
#endif

#if VAX
static char SccsId2[] = "Vax/VMS";
#endif

#if OS2
static char SccsId2[] = "OS2";
#endif

#if OS9
static char SccsId2[] = "OS9";
#endif

#if XENIX
static char SccsId2[] = "Xenix";
#endif

#if APOLLO
static char SccsId2[] = "Apollo";
#endif

#if FLEXOS
static char SccsId2[] = "Flexos";
#endif

#if MAC
static char SccsId2[] = "Macintosh";
#endif

#if SUN
static char SccsId2[] = "Sun";
#endif

#if SM90
static char SccsId2[] = "SM90";
#endif

#if TR5
static char SccsId2[] = "TR5000";
#endif

#if i486V4
static char SccsId2[] = "i486 System V.4";
#endif

#if SCOUNIX
static char SccsId2[] = "SCO Unix";
#endif

#if ARMBSD
static char SccsId2[] = "R140";
#endif

#if MEIKORTE
static char SccsId2[] = "MeikoRTE";
#endif

#if UNIX386
static char SccsId2[] = "386/ix";
#endif

#if HELIOS
static char SccsId2[] = "(Helios)";
#endif

#if RS6000
static char SccsId2[] = "RS6000";
#endif

#if HP9000
static char SccsId2[] = "HP9000";
#endif


