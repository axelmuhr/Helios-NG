/*------------------------------------------------------------------------
--                                                                      --
--                     H E L I O S   N U C L E U S                      --
--                     ---------------------------                      --
--                                                                      --
--             Copyright (C) 1987, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- memory.h								--
--                                                                      --
--	Kernel memory primitives					--
--                                                                      --
--	Author:  NHG 16/8/87						--
--                                                                      --
------------------------------------------------------------------------*/
/* SccsId: %W% %G% Copyright (C) 1987, Perihelion Software Ltd.		*/
/* RcsId: $Id: memory.h,v 1.16 1993/08/05 17:03:19 nickc Exp $ */

#ifndef __memory_h
#define __memory_h

#ifndef __helios_h
# include <helios.h>
#endif

#include <queue.h>

/* Memory block header */

typedef struct Memory {
	Node		Node;		/* node in current pool		*/
	WORD		Size;		/* block size + alloc bits	*/
	struct Pool	*Pool;		/* pointer to owning pool	*/
} Memory;

#define Memory_Size_BitMask	0xF	/* mask for alloc bits		*/
#define Memory_Size_BwdBit	0x2	/* alloc state of predecessor	*/
#define Memory_Size_FwdBit	0x1	/* alloc state of this block	*/
#define Memory_Size_Carrier	0x4	/* marks a RAM carrier		*/
#define Memory_Size_Reloc	0x8	/* marks relocatable block	*/

/* Memory pool structure */
typedef struct Pool {
	Node		Node;		/* queuing node			*/
	List		Memory;		/* list of blocks owned		*/
	WORD		Blocks;		/* number of blocks in pool	*/
	WORD		Size;		/* total size of pool		*/
	WORD		Max;		/* initial size of pool		*/
} Pool;

#ifdef __MI
typedef struct MIInfo {
	bool		MIWriteProtect;	/* True if memory is WP'ed	*/
	word		**MITable;	/* Memory Indirect(ion) table	*/
	word		MIFreeSlots;	/* Free slots in MI table	*/
	word		MITableSize;	/* Total slots in MI Table	*/
	Pool		*FreePool;	/* Allocate from this free pool */
	Pool		*DstPool;	/* Into this pool		*/
} MIInfo;

typedef word	Handle;
#endif

/* Memory allocation procedures */

PUBLIC void InitPool(Pool *);
PUBLIC void *AllocMem(WORD , Pool *);
PUBLIC word FreeMem(void *);
PUBLIC void FreeMemStop(void *);
PUBLIC void FreePool(Pool *);
PUBLIC bool InPool(void *addr, Pool *pool);

#if defined(__ARM) || defined(__C40)
PUBLIC void *LowAllocMem(word size, Pool *);
PUBLIC bool ReAllocMem(word newsize, void *addr);
PUBLIC word StatMem(word *totalsize, word *totalfree, word *largestfree, word *percfree);
#endif

#ifdef __ABC
/* Support for memory cards */
PUBLIC Pool *BuildPool(byte *base, word size, bool scavenge);
PUBLIC word RRDPoolInit(MIInfo *mi, Memory *magicbase, Memory *magictop);
/* Memory Indirect (relocatable block) operations */
PUBLIC void MIInit(MIInfo *mi, Pool *free, Pool *dst);
PUBLIC Handle MIAlloc(MIInfo *mi, WORD size);
PUBLIC void MIFree(MIInfo *mi, Handle handle);
PUBLIC void MICompact(MIInfo *mi);
PUBLIC void *MILock(MIInfo *mi, Handle handle);
PUBLIC void MIUnLock(MIInfo *mi, Handle handle);
PUBLIC bool MITrim(MIInfo *mi, Handle handle, word amount);
#endif

/* RAM Carrier. This is used to describe a block of memory where the	*/
/* memory is managed in a "hands-off" manner. This is used for precious	*/
/* memory like On-Chip RAM in the Tranny and C40, or for memory which is*/
/* outside the C address range, like the global memory on a C40.	*/

typedef struct Carrier {
	MPtr		Addr;		/* address of RAM block		*/
	word		Size;		/* size of block in WORDS on C40 in bytes otherwise */
	word		Type;		/* type of RAM			*/
} Carrier;

PUBLIC Carrier *AllocFast(word, Pool *);

PUBLIC Carrier *AllocSpecial(word size, word type, Pool *dest);

#define RAMType_Speed		0x01	/* RAM speed property		*/
#define	RAMType_Dynamic		0x00	/* Slower dynamic RAM		*/
#define	RAMType_Static		0x01	/* Fast on-chip or static RAM	*/

#define RAMType_Addressing	0x02	/* RAM addressability		*/
#define	RAMType_Direct		0x00	/* Directly addressable	from C	*/
#define RAMType_Indirect	0x02	/* Machine addressable only	*/

#define RAMType_Location	0x04	/* RAM location			*/
#define RAMType_Local		0x00	/* Local processor Bus		*/
#define RAMType_Global		0x04	/* Global processor Bus		*/

#define RAMTypeIndex(x)		((x)&0x7)

#endif


/* -- End of memory.h */
