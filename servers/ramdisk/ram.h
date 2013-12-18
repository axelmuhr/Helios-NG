/*------------------------------------------------------------------------
--                                                                      --
--                     H E L I O S   N U C L E U S                      --
--                     ---------------------------                      --
--                                                                      --
--             Copyright (C) 1990, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- ram.h								--
--                                                                      --
--	Robust Ram Disk header						--
--                                                                      --
--	Defines the ram disk block headers, used by kernel/memory1.c	--
--	and ram.c.                                                      --
--                                                                      --
--	Author: PAB 22/11/90						--
--                                                                      --
------------------------------------------------------------------------*/
/* SccsId: %W%	%G%	Copyright (C) 1987, Perihelion Software Ltd.	*/
/* RCSid: $Id: ram.h,v 1.2 1991/03/12 00:22:40 paul Exp $ */

#ifndef __rram_h
#define __rram_h

#ifndef __helios_h
#include <helios.h>
#endif
#include <memory.h>
#include <protect.h>

/* Common data to all block types */
typedef struct RRDCommon {
	word		Magic;		/* magic number & header type	     */
	Handle		Next;		/* handle of chained block	     */
	word		Checksum;	/* checksum of header + data	     */
	/* \|/ Rest of header and data is checksummed from this point on \|/ */
	word		TotalSize;	/* size of DataBlock + Header	     */
} RRDCommon;

/* define generic magic number in low short of magic word */
#define RRDCommonMagic		('R' | 'D' << 8)
#define RRDCommonMagicMask	0x0000ffff

/* File Header (One per file), also used as the dir block header at present */
typedef struct RRDFileBlock {
	RRDCommon 	c;		/* common header data		*/
	uword		FileId;		/* Id used for this file	*/
	Matrix		Matrix;		/* access matrix		*/
	Key		Key;		/* protection key		*/
	unsigned int	Date;		/* only holds update date	*/
	char		FullName[1];	/* relative from /ram/sys/	*/
} RRDFileBlock;

#define RRDFileMagic	(RRDCommonMagic | 'f' << 16 | 'i' << 24)


#define RRDDirBlock RRDFileBlock
#define RRDDirMagic	(RRDCommonMagic | 'd' << 16 | 'r' << 24)


/* File Buffer Header (One or more per file) - prefix to actual data	  */
typedef struct RRDBuffBlock {
	RRDCommon 	c;		/* common header data		  */
	uword		FileId;		/* Id used for this file	  */
	word		Pos;		/* position of block within file  */
					/* alias a non-monotonic seq. no. */
	word		DataSize;	/* amount of data in block	  */
	RPTR		DataStart;	/* point to start of data	  */
					/* The above two could be combined*/
					/* if we need to use another word */
} RRDBuffBlock;

#define RRDBuffMax	512	/* size of buffer block */
#define RRDBuffMagic	(RRDCommonMagic | 'b' << 16 | 'b' << 24)


/* Symbolic Link Block (One per symb. link) */
typedef struct RRDSymbBlock {
	RRDCommon	c;		/* common header data		*/
	Matrix		Matrix;		/* access matrix		*/
	Key		Key;		/* protection key		*/
	unsigned int	Date;		/* only holds creation date	*/
	Capability	Cap;		/* permission to access target	*/
	char		FullName[1];	/* relative from /rom/sys/	*/
	char		TargetName[1];	/* target name of link		*/
} RRDSymbBlock;

#define RRDSymbMagic	(RRDCommonMagic | 's' << 16 | 'y' << 24)

/* Do NOT restore this type of block */
#define RRDTmpMagic	0xDEADC0DE

#endif

/* end of ram.h */
