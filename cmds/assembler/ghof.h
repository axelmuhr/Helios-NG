/*
 * File:	ghof.h
 * Subsystem:	Generic Assembler
 * Author:	P.A.Beskeen
 * Date:	Sept '91
 *
 * Description: GHOF (Generic Helios Object Format) directive and patch
 *		definitions.
 *
 * RcsId: $Id: ghof.h,v 1.3 1993/01/29 17:56:29 paul Exp $
 *
 * (C) Copyright 1991 Perihelion Software Ltd.
 * 
 * $RcsLog$
 *
 */


#ifndef __ghof_h
#define __ghof_h


/* defines the maximum size of an individual CODE directive */
#define CODEBUFFERSIZE	1024


#define GHOF_CODE	0x01
#define GHOF_SPACE	0x02
#define GHOF_INIT	0x03
#define GHOF_BYTE	0x09
#define GHOF_SHORT	0x0a
#define GHOF_WORD	0x0c
#define GHOF_CODESYMB	0x0d	/* split module table support */
#define GHOF_LABELREF	0x0f
#define GHOF_MODULE	0x20
#define GHOF_EXPORT	0x22
#define GHOF_LABEL	0x23
#define GHOF_DATA	0x24
#define GHOF_COMMON	0x25
#define GHOF_CODETABLE	0x26	/* split module table support */
#define GHOF_REF	0x27
#define GHOF_CODESTUB	0x28
#define GHOF_ADDRSTUB	0x29
#define GHOF_DATASYMB	0x10
#define GHOF_DATAMODULE	0x11
#define GHOF_MODSIZE	0x0e
#define GHOF_MODNUM	0x12

#define PATCHMIN	0x13
#define PATCHMAX	0x1f


/* General Patches */

#define PATCH_ADD	0x013
#define PATCH_SHIFT	0x014
#define PATCH_BYTESWAP	0x01e
#define PATCH_OR	0x01f


/* Processor Specific Patches */

#ifdef __C40TARGET
# define PATCH_C40DATAMODULE1	0x15
# define PATCH_C40DATAMODULE2	0x16
# define PATCH_C40DATAMODULE3	0x17
# define PATCH_C40DATAMODULE4	0x18
# define PATCH_C40DATAMODULE5	0x19
# define PATCH_C40MASK24ADD	0x1a
# define PATCH_C40MASK16ADD	0x1b
# define PATCH_C40MASK8ADD	0x1c
#endif

#ifdef __ARMTARGET
# define PATCH_ARM_DT	  0x015  /* Data transfer instruction patch (bits 0-11) */
# define PATCH_ARM_DP	  0x016  /* Data processing instruction patch (bits 0-8) */
# define PATCH_ARM_JP	  0x017  /* Branch instruction patch (bits 0-23 rshift 2) */
# define PATCH_ARM_DPLSB  0x018  /* Data processing instruction patch lsbyte */
# define PATCH_ARM_DPREST 0x019  /* Data processing instruction patch residue from above */
# define PATCH_ARM_DPMID  0x01A  /* Data processing instruction patch lsb+1 byte */
#endif

#if 0
# define PATCH_PGC1	0x01f
#endif


/* GHOF encoding definitions */

#define ENC_MORE 0x80	/* number encoding - another byte to follow */
#define ENC_NEG  0x40	/* number encoding - number is neg */
#define BOT6BITS 0x3f
#define BOT7BITS 0x7f


/* Exported Data: */

extern int codesize;


/* Exported Functions: */

void FlushCodeBuffer(void);		/* flush buffer before GHOFEncode's */
void GHOFEncode(int n);			/* GHOF encode and output a number */
void ObjWriteByte(char b);		/* write a byte to the object file */
void ObjWrite(char *buf, int size);	/* write a block to the object file */



#endif	/* ghof.h */
