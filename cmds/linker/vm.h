/************************************************************************/
/* Helios Linker							*/
/*                                                                      */
/* File: vm.c                                                           */
/*                                                                      */
/*                                                                      */
/* Copyright (c) 1987, Perihelion Software Ltd. All Rights Reserved.    */
/************************************************************************/
/* RcsId: $Id: vm.h,v 1.2 1992/04/16 10:35:30 nickc Exp $ */

#define VMmask		0xf
#define VMswapped	1
#define VMdirty		2
#define VMlocked	4
#define VMtouched	8
#define VMrefcnt	0x7ffffff0
#define VMrefinc	16

typedef struct VMpage {
	int		status;
	char		*addr;
	int		filepage;
	int		left;
} VMpage;

#if 0
/*#ifdef __STDC__*/

typedef struct VMRef {
	unsigned 	offset : 16, page   : 16;
} VMRef;

#define offset_(x)	((x).offset)
#define page_(x)	((x).page)

#define VMSame(x,y) ((page_(x) == page_(y)) && (offset_(x) == offset_(y)))

#define MakeVMRef(r,p,o) ((r).page = p, (r).offset = o)

/*#else*/
#endif

typedef unsigned long	VMRef;

#define offset_(x)	((x)&0xffff)
#define page_(x)	((x)>>16)

#define VMSame(x,y)	((x) == (y))

#define MakeVMRef(r,p,o) ((r) = ((p)<<16) | (o))

/*#endif */

#ifdef VM_DEBUG

#define VMAddr(type,x) ((type *)VMswap(x))

#else

#define VMAddr(type,x) \
((type *)(((VMTable[page_(x)].status|=VMtouched)&VMswapped)==VMswapped? \
VMswap(x):VMTable[page_(x)].addr+offset_(x)))

#endif
		
#define VMDirty(x) VMTable[page_(x)].status|=VMdirty;

#define NullRef(x) (*(int *)&(x) == -1)

extern VMpage *VMTable;
extern char *VMfilename;
extern VMRef NullVMRef;

#ifdef __STDC__
extern APTR VMswap(VMRef v);
extern VMRef VMalloc(int size, VMRef v);
extern int VMInit(char *vmfile,int pagesize,int maxpages);
extern VMRef VMPage(void);
extern int VMleft(VMRef v);
extern void VMlock(VMRef v);
extern void VMunlock(VMRef v);
extern VMRef VMnext(VMRef v);
extern void VMLRU(void);
extern void VMTidy(void);
extern void VMStats(void);
#else
extern APTR VMswap();
extern VMRef VMalloc();
extern int VMInit();
extern VMRef VMPage();
extern int VMleft();
extern void VMlock();
extern void VMunlock();
extern VMRef VMnext();
extern void VMLRU();
extern void VMTidy();
extern void VMStats();
#endif
