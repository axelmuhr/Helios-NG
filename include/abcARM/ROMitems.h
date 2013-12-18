/*> ROMitems/h <*/
/*----------------------------------------------------------------------*/
/*									*/
/*				ROMitems.h				*/
/*				----------				*/
/* Copyright (c) 1990, Active Book Company, Cambridge, United Kingdom.	*/
/*									*/
/* ITEM definitions							*/
/* This file describes the "ITEMs" held in the ROM image. This can be	*/
/* extended to deal with "ITEMs" on external ROMS, external		*/
/* non-volatile RAM and as the identifying field on volatile media.	*/
/*									*/
/* Note: All "ITEMs" are files. This is a function of the ROM/device	*/
/*       filing system. The system ROM (and expansion ROMs) are viewed	*/
/*       as fixed size, read only file systems by Helios and NOT as	*/
/*       system specific black-holes.					*/
/*									*/
/* Author:	James G Smith						*/
/*									*/
/************************************************************************/
/* NOTE: If this file is updated, then the command			*/
/*       "genhdr -p /hsrc/include/abcARM/asm" should be executed.	*/
/* 	 The "genhdr" binary may need to be remade to include any new   */
/*       definitions.							*/
/*----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*/
/*
 * An ITEM describes a "file" unit. The initial section of the structure will
 * be the same for ROM and RAM ITEMs:
 *
 *       name            length  description
 *       ===============================================================
 *       ITEMID          4       Magic number (identification)
 *       ITEMLength      4       Word-aligned length of complete ITEM
 *       OBJECTOffset    4       Offset to OBJECT data
 *       OBJECTLength    4       Length of the OBJECT
 *       ITEMAccess      4       Helios Access Matrix for the OBJECT
 *       ITEMDate        8       Encoded timestamp
 *       ITEMExtensions  4       Extensions bitmask
 *       ITEMNameLength  1       Length of the following field
 *       ITEMName        n       NULL terminated ASCII identifier
 *       ===============================================================
 *
 * ITEMID
 * ------
 * This is currently fixed as &6D657469 ("item").
 *
 * ITEMLength
 * ----------
 * This is the word-aligned length of the complete ITEM, INCLUDING
 * the header structure.
 *
 * OBJECTOffset
 * ------------
 * This is a relative pointer to the start of the OBJECT data.
 *
 * OBJECTLength
 * ------------
 * This is the byte-aligned length of the OBJECT data.
 *
 * ITEMAccess
 * ----------
 * This is a standard Helios Access Matrix. The default used in ROM
 * ITEMs is 0x01010101.
 *
 * ITEMDate
 * --------
 * Encoded as follows:
 *       word 0 (internal) : micro-seconds into the second
 *       word 1 (external) : seconds since "00:00:00 1 Jan 1970"
 *
 * The external word is identical to the Unix timestamp value, and is
 * used for external timestamp storage. The internal word is used to
 * store the number of micro-seconds into the second. This is only
 * really relevant for process timing or for information that can
 * be created and modified within a second. Normally the internal word
 * will be &00000000.
 *
 * The second resolution timing will be from the RTC hardware. Problems
 * may occur if reading the clock on an event boundary, ie. the
 * incrementing of the minute counter could occur between the reading
 * of the second value and the reading of the minute value.
 * Reading <hi><lo><hi> and comparing the two <hi> values, repeating
 * the procedure if they are different, should ensure accurate reading.
 * The internal micro-second timer must be synchronised to the RTC,
 * since we never want to return a smaller microsecond value for a
 * later event within the same second.
 *
 * ITEMExtensions
 * --------------
 * This is currently defined as:
 *	bit 0       ROM ITEM header - follows "ITEMName"
 *      bit 1       RAM ITEM header - follows "ITEMName"
 *      bits 2..7   Unused (0)
 *	bit 8	    Executive RESET BRANCH instruction follows ITEM header
 *	bits 9..31  Unused (0)
 *
 * Bits 0 and 1 are mutually exclusive.
 *
 * ITEMNameLength
 * --------------
 * This field contains the size of the "ITEMName" text, INCLUDING the
 * terminating and padding NULLs. This is used as a quick means of
 * reaching the over-loaded header sections.
 *
 * ITEMName
 * --------
 * This is a NULL terminated ASCII identifier. It may contain any 
 * printable 7bit ASCII character (&20->&7E inclusive). It is padded
 * to the next word-boundary with NULLs. When used in searches it
 * is treated in a non-case-specific manner.
 *
 * The remainder of the structure then depends on the media:
 *
 * ROM ITEM
 * ========
 *       name            length  description
 *       ===============================================================
 *       OBJECTInit      4       Offset to OBJECT initialisation code
 *       ITEMVersion     4       BCD ASCII version number (0000hh.ll)
 *       ===============================================================
 *
 * OBJECTInit
 * ----------
 * This field is used during the initialisation of special programs
 * (eg. FPEmulator). If non-zero, it is assumed to be a relative
 * pointer to a function contained with the ITEM. This function is
 * called during the reset/initialisation of the Executive.
 *
 * ITEMVersion
 * -----------
 * This field is a number used to identify different versions of the
 * same named ITEM. It is an extension to the global ROM/OS version
 * number and could be checked explicitely by PATCHes.
 *
 * RAM ITEM
 * ========
 *       name            length  description
 *       ===============================================================
 *       ITEMCheck       3       Checksum/CRC on attached data
 *       ITEMHdrSeq1     1       First sequence number
 *       OBJECTUsed      3       Number of bytes used in attached data
 *       OBJECTSize      1       Encoded data size (2^n)
 *       OBJECTRef       3       Offset to the next data block header
 *       ITEMHdrSeq2     1       Second sequence number
 *       ===============================================================
 *
 * The "attached data" follows immediately after the header structure.
 *
 * ITEMCheck
 * ---------
 * The exact use of this field is to be decided: Full CRC checks would
 * be the ideal solution, but they are probably too time consuming
 * (~5 instructions per BIT of data). A simple 32bit additive sum or
 * EOR would probably suffice for spotting the majority of data errors,
 * since if the RAMFS memory is being corrupted, it is likely the
 * rest of the system RAM is also damaged.
 *
 * ITEMHdrSeq1 and ITEMHdrSeq2
 * -----------     -----------
 * These bytes are used as an update validity check on the RAM ITEM
 * header information. The majority of the time these bytes should
 * contain the same value. The only exception to this rule is when the
 * header is being updated. Before the update occurs "ITEMHdrSeq1" is
 * incremented. When the update has completed "ITEMHdrSeq2" is
 * incremented. When the validity of RAM ITEMs is checked, the
 * difference in the bytes can be used to flag incomplete ITEM updates.
 *
 * OBJECTUsed
 * ----------
 * This is the number of bytes actually used in the "attached data". It
 * is required to deal with the EOF case where a complete block may
 * not be used.
 *
 * OBJECTSize
 * ----------
 * This is an encoded block size. 
 *
 * OBJECTRef
 * ---------
 * This is a relative pointer to the header of the next file data block.
 * A RAM file is constructed from a linked list of these structures.
 *
 *       name            length  description
 *       ===============================================================
 *       ITEMCheck       3       Checksum/CRC on attached data
 *       ITEMHdrSeq1     1       First sequence number
 *       OBJECTUsed      3       Number of bytes used in attached data
 *       OBJECTSize      1       Encoded data size (2^n)
 *       OBJECTRef       3       Offset to the next data block header
 *       ITEMHdrSeq2     1       Second sequence number
 *       OBJECTRefLast   3       Offset to the previous data block
 *       ITEMSpare1      1       Unused (should be zero)
 *       ITEMNumber      4       Unique block number for this RAM ITEM
 *       ===============================================================
 *
 * The initial section of this structure is identical to the RAM ITEM
 * specfic header structure described above. It does however contain
 * some new information:
 *
 * OBJECTRefLast
 * -------------
 * The "OBJECTRef" and "OBJECTRefLast" pointers could be merged using
 * the standard EOR technique. This saves space, but loses a possible
 * validity check (ie. look through the "next" and check that the
 * "last" references this header).
 *
 * ITEMSpare1
 * ----------
 * This is an unused byte and should be zero for the moment.
 *
 * ITEMNumber
 * ----------
 * The RAM ITEM header and attached data is assumed to be file block 1.
 * All the other file blocks contain their position in this field. This
 * should be a monotonically increasing number.
 *
 * For more information refer to the RAMFS documentation.
 */
/*----------------------------------------------------------------------*/

#ifndef __ROMitems_h
#define __ROMitems_h

/*----------------------------------------------------------------------*/
/* This is the MAGIC word used to identify ITEM headers ("ITEMID") */

#define ITEMMagic	('i' | ('t' << 8) | ('e' << 16) | ('m' << 24))

/*----------------------------------------------------------------------*/
/* This is a standard Helios access matrix. */

#define defaultITEMaccess       (0x01010101)

/*----------------------------------------------------------------------*/
/* ITEM extension bitmask */

#define ITEMhdrROM	(1 << 0)   /* ROM ITEM header follows ITEMName */
#define ITEMhdrRAM      (1 << 1)   /* RAM ITEM header follows ITEMName */
#define ITEMhdrBRANCH	(1 << 8)   /* BRANCH instruction follows header */
/* all other bits are currently undefined (and should be 0) */

/*----------------------------------------------------------------------*/
/* ITEM structure */

typedef struct ITEMstructure {
	word ITEMID ;         /* magic number (identification) */
	word ITEMLength ;     /* word-aligned length of the complete ITEM */
	word OBJECTOffset ;   /* offset to the OBJECT data */
	word OBJECTLength ;   /* length of the OBJECT data */
	word ITEMAccess ;     /* Helios access matrix for the OBJECT */
	word ITEMDate[2] ;    /* encoded timestamp */
	word ITEMExtensions ; /* extensions bitmask */
	byte ITEMNameLength ; /* length of the following field */
	byte ITEMName[1] ;    /* NULL terminated ASCII identifier for OBJECT */
                             } ITEMstructure ;

/*----------------------------------------------------------------------*/
/* ROM ITEM structure */

typedef struct ROMITEMstructure {
	/* offset from end of "ITEMName" */
	word OBJECTInit ;	/* offset to OBJECT initialisation function */
	word ITEMVersion ;	/* BCD ASCII version number (0000hh.ll) */
                                } ROMITEMstructure ;

#define ITEMROMend (sizeof(ROMITEMstructure))	/* end of the ROM header */

/*----------------------------------------------------------------------*/
/* RAM ITEM structure */

typedef struct RAMITEMstructure {
        /* offset from end of "ITEMName" */
	byte ITEMCheck[3] ;     /* checksum/CRC on the attached data */
	byte ITEMHdrSeq1 ;      /* first header sequence number */
	byte OBJECTUsed[3] ;    /* number of bytes used in the attached data */
	byte OBJECTSize ;       /* encoded data size (2^n) */
	byte OBJECTRef[3] ;     /* offset to the next extended RAM ITEM hdr */
	byte ITEMHdrSeq2 ;      /* second sequence number */
	byte OBJECTRefLast[3] ; /* offset to the previous RAM ITEM header */
	byte ITEMSpare1 ;       /* unused (should be zero) */
	word ITEMNumber ;       /* unique index number for this RAMITEM hdr */
                                } RAMITEMstructure ;

/* end of the ITEM structure attached header */
#define ITEMRAMhdrend OBJECTRefLast

/* end of the extended RAM ITEM header */
#define ITEMRAMend (sizeof(RAMITEMstructure))

/*----------------------------------------------------------------------*/
/* ITEM access functions */

/* "GetROMItem" returns information about the ROM ITEM at "index" */
extern int GetROMItem(word location,word *index,ITEMstructure **item) ;

/*----------------------------------------------------------------------*/

#endif /* __ROMitems_h */

/*----------------------------------------------------------------------*/
/*> EOF ROMitems/h <*/
