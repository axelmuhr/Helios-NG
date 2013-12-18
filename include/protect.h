/*------------------------------------------------------------------------
--                                                                      --
--                     H E L I O S   N U C L E U S                      --
--                     ---------------------------                      --
--                                                                      --
--             Copyright (C) 1987, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- protect.h								--
--                                                                      --
--	Protection data structures					--
--                                                                      --
--	Author:  NHG 18/9/87						--
--                                                                      --
------------------------------------------------------------------------*/
/* SccsId: %W% %G% Copyright (C) 1987, Perihelion Software Ltd.	*/
/* $Id: protect.h,v 1.1 90/09/05 11:07:00 nick Exp $ */

#ifndef __protect_h
#define __protect_h

#ifndef __helios_h
#include <helios.h>
#endif


typedef unsigned long	Matrix;		/* access matrix		*/

typedef UBYTE		AccMask;	/* access mask			*/

typedef word		Key;		/* encryption key		*/

/*----------------------------------------------------------------------*/
/* Access capability 							*/
/*----------------------------------------------------------------------*/

typedef struct Capability {
	AccMask		Access;		/* access mask			*/
	UBYTE 		Valid[7];	/* validation value		*/
} Capability;

/*----------------------------------------------------------------------*/
/* Access mask bits 							*/
/*----------------------------------------------------------------------*/

#define AccMask_Full	0xff		/* all bits set			*/

/* All */
#define AccMask_R	0x01		/* Read permission		*/
#define AccMask_W	0x02		/* Write permission		*/
#define AccMask_D	0x40		/* Delete permission		*/
#define AccMask_A	0x80		/* Alter permission		*/

/* Files only */
#define AccMask_E	0x04		/* Execute permission		*/
#define AccMask_F	0x08		/* unused - arbitrary letters	*/
#define AccMask_G	0x10
#define AccMask_H	0x20

/* Directories only */
#define AccMask_V	0x04		/* V access category		*/
#define AccMask_X	0x08		/* X access category		*/
#define AccMask_Y	0x10		/* Y access category		*/
#define AccMask_Z	0x20		/* Z access category		*/

/* Tasks only */
#define AccMask_K	AccMask_D	/* Kill task (== Delete)	*/

/*----------------------------------------------------------------------*/
/* Access Matrix category masks						*/
/*----------------------------------------------------------------------*/

#define AccMatrix_V	0x000000ff
#define AccMatrix_X	0x0000ff00
#define AccMatrix_Y	0x00ff0000
#define AccMatrix_Z	0xff000000

/*----------------------------------------------------------------------*/
/* Printed matrix letters						*/
/*----------------------------------------------------------------------*/

#define FileChars	"rwefghda"
#define DirChars	"rwvxyzda"
#define TaskChars	"rw????ka"
#define ModChars	"r?e???da"
#define ProgChars	"rwe???da"

/*----------------------------------------------------------------------*/
/* Default Matrices 							*/
/*----------------------------------------------------------------------*/

#define DefDirMatrix	0x21134BC7	/* darwv:drwx:rwy:rz		*/
#define DefFileMatrix	0x010343C3	/* darw:drw:rw:r		*/
#define DefLinkMatrix	0x201088C4	/* dav:dx:y:z			*/
#define DefTFMatrix	0x21134BC7	/* darwv:drwx:rwy:rz		*/
#define DefTaskMatrix	0x010343C3	/* darw:drw:rw:r		*/
#define DefModuleMatrix	0x010545C5	/* dare:dre:re:r		*/
#define DefProgMatrix	0x010545C5	/* dare:dre:re:r		*/
#define DefNameMatrix	0x21110907	/* rwv:rx:ry:rz			*/
#define DefRootMatrix	0x21130B87	/* arwv:rwx:rwy:rz		*/
#define DefRomMatrix	0x01010101	/* r:r:r:r			*/

#endif

/* -- End of protect.h */
