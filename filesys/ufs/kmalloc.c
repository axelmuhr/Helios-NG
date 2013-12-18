/* (C)1992 Perihelion Software Limited                                */
/* Author: Alex Schuilenburg                                          */
/* Date: 29 July 1992                                                 */
/* File: kmalloc.c                                                    */
/*                                                                    */
/*  Kernal malloc, free and associated init routines to fake the      */
/*  UNIX kernel calls, using the helios malloc, but keeping the       */
/*  kernel's statistics.                                              */
/*                                                                    */

#define __IN_KMALLOC__

#include "param.h"
#include "stdlib.h"	/* Normal malloc routine definitions */
#ifdef KERNEL
#undef KERNEL		/* For this include only */
#include "malloc.h"
#define KERNEL
#else
#include "malloc.h"
#endif

struct kmemstats kmemstats[M_LAST];
u_int kmemwanted = 0;

/*
 * Display Memory Statistics
 */
void showmemdata(char *str, struct kmemstats *ksp)
{
	printf("%s\t\t%d\t%d\t%d\t%d\n",
	str,ksp->ks_memuse,ksp->ks_maxused,ksp->ks_calls,ksp->ks_inuse);
}

void showmemstats()
{
	printf("UNIX DYNAMIC MEMORY USAGE\n");
	printf("Type\t\tIn Use\tMaximum\tTotal Calls\tCurrent Calls\n");

	showmemdata("M_FREE",&kmemstats[M_FREE]);
	showmemdata("M_MBUF",&kmemstats[M_MBUF]);
	showmemdata("M_DEVBUF",&kmemstats[M_DEVBUF]);
	showmemdata("M_SOCKET",&kmemstats[M_SOCKET]);
	showmemdata("M_PCB",&kmemstats[M_PCB]);
	showmemdata("M_RTABLE",&kmemstats[M_RTABLE]);
	showmemdata("M_HTABLE",&kmemstats[M_HTABLE]);
	showmemdata("M_FTABLE",&kmemstats[M_FTABLE]);
	showmemdata("M_ZOMBIE",&kmemstats[M_ZOMBIE]);
	showmemdata("M_IFADDR",&kmemstats[M_IFADDR]);
	showmemdata("M_SOOPTS",&kmemstats[M_SOOPTS]);
	showmemdata("M_SONAME",&kmemstats[M_SONAME]);
	showmemdata("M_NAMEI",&kmemstats[M_NAMEI]);
	showmemdata("M_GPROF",&kmemstats[M_GPROF]);
	showmemdata("M_IOCTLOPS",&kmemstats[M_IOCTLOPS]);
	showmemdata("M_SUPERBLK",&kmemstats[M_SUPERBLK]);
	showmemdata("M_CRED",&kmemstats[M_CRED]);
	showmemdata("M_PGRP",&kmemstats[M_PGRP]);
	showmemdata("M_SESSION",&kmemstats[M_SESSION]);
	showmemdata("M_IOV",&kmemstats[M_IOV]);
	showmemdata("M_MOUNT",&kmemstats[M_MOUNT]);
	showmemdata("M_FHANDLE",&kmemstats[M_FHANDLE]);
	showmemdata("M_NFSREQ",&kmemstats[M_NFSREQ]);
	showmemdata("M_NFSMNT",&kmemstats[M_NFSMNT]);
	showmemdata("M_VNODE",&kmemstats[M_VNODE]);
	showmemdata("M_CACHE",&kmemstats[M_CACHE]);
	showmemdata("M_DQUOT",&kmemstats[M_DQUOT]);
	showmemdata("M_UFSMNT",&kmemstats[M_UFSMNT]);
	showmemdata("M_MAPMEM",&kmemstats[M_MAPMEM]);
	showmemdata("M_SHM",&kmemstats[M_SHM]);
	showmemdata("M_TEMP",&kmemstats[M_TEMP]);
}

/*
 * Allocate a block of memory
 */
qaddr_t KMalloc(size, type, flags)
	unsigned long size;
	int type, flags;
{
	caddr_t	va;
#ifdef KMEMSTATS
	register struct kmemstats *ksp = &kmemstats[type];
	
	if (((unsigned long)type) > M_LAST)
		panic("KMalloc - bogus type");
#endif
	
	/* Get memory */
	while ((va = (caddr_t)Malloc(size)) == NULL) {
		if ((flags & M_NOWAIT) == M_NOWAIT) {
			printf("KMalloc - failed to allocate %d memory of type %d\n",
				size,type);
			return(NULL);
		}
		kmemwanted = 1;
		sleep((caddr_t)&kmemwanted,PWAIT);	/* Wait for memory to be freed */
	}
	
#ifdef KMEMSTATS
	/* Helios comes in 8 byte blocks */
	ksp->ks_memuse += MemSize(va);

	ksp->ks_calls++;
	ksp->ks_inuse++;
	if (ksp->ks_memuse > ksp->ks_maxused)
		ksp->ks_maxused = ksp->ks_memuse;
#endif
	return((qaddr_t)va);
}

/*
 * Free a block of memory allocated by KMalloc
 */
void KFree(addr, type)
	caddr_t addr;
	int type;
{
#ifdef KMEMSTATS
	register struct kmemstats *ksp = &kmemstats[type];
	
	if (((unsigned long)type) > M_LAST)
		panic("KFree - bogus type");
	
	/* Helios comes in 8 byte blocks */
	ksp->ks_memuse -= MemSize(addr);

	ksp->ks_inuse--;
	
#endif
	Free(addr);
	if (kmemwanted) {
		kmemwanted = 0;
		wakeup(&kmemwanted);
	}
}

KMeminit()
{
	int indx;
	for (indx=0; indx < M_LAST; indx++) {
		kmemstats[indx].ks_inuse = 0;	/* # of packets of this type currently in use */
		kmemstats[indx].ks_calls = 0;	/* total packets of this type ever allocated */
		kmemstats[indx].ks_memuse = 0;	/* total memory held in bytes */
		kmemstats[indx].ks_limblocks = 0;	/* number of times blocked for hitting limit */
		kmemstats[indx].ks_mapblocks = 0;	/* number of times blocked for kernel map */
		kmemstats[indx].ks_maxused = 0;	/* maximum number ever used */
		kmemstats[indx].ks_limit = 0;	/* most that are allowed to exist */
	}
}

