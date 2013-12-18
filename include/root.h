/*------------------------------------------------------------------------
--                                                                      --
--                     H E L I O S   N U C L E U S                      --
--                     ---------------------------                      --
--                                                                      --
--             Copyright (C) 1987, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- root.h								--
--                                                                      --
--	Root data structure						--
--                                                                      --
--	Author:  NHG 16/8/87						--
--                                                                      --
------------------------------------------------------------------------*/
/* SccsId: %W% 	%G% Copyright (C) 1987, Perihelion Software Ltd.	*/
/* RcsId: $Id: root.h,v 1.29 1993/08/17 11:33:29 bart Exp $ */

#ifndef __root_h
#define __root_h

#ifndef __helios_h
# include <helios.h>
#endif

#include <queue.h>
#include <memory.h>
#include <link.h>
#include <sem.h>

#ifndef in_kernel
struct PTE { word secret; };		/* keep secret outside kernel	*/
#endif

#ifdef __ARM
# include <arm.h>	/* for "[Interrupt|User]Vectors" sizes	*/
#endif
#ifdef __C40
# include <c40.h>
#endif

typedef struct RootStruct {
        word    	ATWFlags;	/* used to fix bugs in ATW h/w	*/
        struct PTE	**PortTable;    /* pointer to port table	*/
        word    	PTSize;         /* number of ports in table	*/
        word    	PTFreeq;        /* port table free queue	*/
	LinkInfo	**Links;	/* pointer to link table	*/
	Pool		SysPool;	/* allocated system memory	*/
	Pool		*FreePool;	/* free memory list		*/
	word		Incarnation;	/* our incarnation number	*/
	List		BufferPool;	/* pending delivery buffer pool	*/
	word		BuffPoolSize;	/* number of free slots in pool	*/
	word		LoadAverage;	/* low pri load average		*/
	word		Latency;	/* hi pri scheduling latency	*/
	word		*TraceVec;	/* pointer to trace vector	*/
#ifdef __TRAN
	List		EventList;	/* list of event routines	*/
#else					/* ditto for each vector	*/
	List		EventList[InterruptVectors];
# ifdef __ABC				/* lists of user event handlers */
	List		UserEventList[UserVectors];
# endif
#endif
	/* Event count only counts interrupts passed on to user handlers. */
	word		EventCount;	/* number of events seen	*/
	word		Time;		/* current system time (secs)	*/
	Pool		FastPool;	/* fast memory pool		*/
	word		MaxLatency;	/* maximum latency		*/
	Semaphore	IODebugLock;	/* lock for all IOdebugs	*/
	word		MachineType;	/* processor type code		*/
	word		BufferCount;	/* number of kernel buffers used*/
	word		MaxBuffers;	/* max number of buffers allowed*/
	word		Timer;		/* system timer value (MHz)	*/
	/* Errors are currently only used for transputers error line.	*/
	word		Errors;		/* number of errors seen	*/
	word		LocalMsgs;	/* local message traffic	*/
	word		BufferedMsgs;	/* messages buffered by kernel	*/
	word		Flags;		/* system flags			*/
	Pool		*LoaderPool;	/* pointer to loader pool	*/
	word		*Configuration;	/* pointer to config structure	*/
	word		*ErrorCodes;	/* array of kernel error codes	*/
	Port		IODebugPort;	/* intercept on IOdebug messages*/
	word		GCControl;	/* control of port garbage collector */
#ifndef __TRAN
	byte		IODBuffer[80];	/* static buffer for IOdebug()s	*/
	word		IODBufPos;	/* Position in buffer		*/
#endif
#ifdef __SMT
	word		*cpi;		/* SMT code pointer index	*/
	word		cpislots;	/* number of slots in cpi	*/
	Semaphore	cpi_op;		/* atomic ops on cpi		*/
#endif
#ifdef __ABC
# ifdef __MI
	MIInfo		*MISysMem;	/* Memory Indirect(ion) table 	*/
# endif
# ifdef __RRD
	word		RRDScavenged;	/* Number of blocks found	*/
	Pool		*RRDPool;	/* Robust Ram Disk pool		*/
# endif
#endif
#ifdef __C40
# ifdef ALLOCDMA
	DMAReq		*DMAReqQhead;	/* outstanding DMA engine alloc req */
	DMAReq		*DMAReqQtail;	/* outstanding DMA engine alloc req */
	DMAFree		*DMAFreeQhead;	/* first free DMA engine in DMAFreeQ */
	DMAFree		DMAFreeQ[6];	/* alloc list of free DMA engines */
# endif
#endif

/* Spare slots to be used when adding new fields to the root structure.
 * If the root structure grows beyond this size, then all the (shared
 * memory) bootstraps that download the config structure. will have to
 * be re-assembled, and distributed with the new nucleus.
 */

#if 1
	Pool		**SpecialPools;	/* pools for fast/global etc. RAM */
#else
	word		spare1;
#endif
#if defined(__ARM) && defined(__VY86PID)
	/* VLSI PID INTC interrupt mask is write only so its contents */
	/* should always be read from here and written here and to INTC. */
	word		IRQM_softcopy;
#else
	word		spare2;
#endif
	word		spare3;
	word		spare4;
	word		spare5;
	word		spare6;
	word		spare7;
	word		spare8;
} RootStruct;

/* flags set in Flags word (init from config.Flags)			*/

#define Root_Flags_rootnode	0x00000001	/* set if rootnode */
#define Root_Flags_special	0x00000002	/* set if special nucleus */
#define Root_Flags_ROM		0x00000004	/* set if ROMm'ed nucleus */
#define Root_Flags_xoffed	0x00000100	/* set if links xoffed	*/
# define Root_Flags_CacheOff	0x00000200	/* Dont enable cache */


MPtr GetNucleusBase(void);
RootStruct *GetRootBase(void);

#if (defined(__ARM) || defined(__C40)) && ! defined(in_kernel)
/* Use macros internally in the kernel */
# define GetSysBase() ((MPtr)GetNucleusBase())
# define GetRoot() (GetRootBase())
#endif
#ifdef __TRAN
/* For the transputer version we cannot afford the luxury of procedure	*/
/* calls to do this.							*/
# define GetSysBase() ((MPtr)0x80001000)
# define GetRoot() ((RootStruct *)((word)GetSysBase()+(word)(*(word *)GetSysBase())))
#endif

/* Processor Statistics structure returned by ServerInfo of /tasks */

typedef struct ProcStats {
	word	Type;			/* processor type		*/
	word	Load;			/* usec/process av. cpu usage	*/
	word	Latency;		/* average interrupt latency	*/
	word	MaxLatency;		/* max interrupt latency seen	*/
	word	MemMax;			/* memory available on processor*/
	word	MemFree;		/* amount currently free	*/
	word	LocalTraffic;		/* bytes sent in local messages	*/
	word	Buffered;		/* data buffered by kernel	*/
	word	Errors;			/* number of errors raised	*/
	word	Events;			/* number of events		*/
	word	NLinks;			/* number of links following	*/
	struct {
		LinkConf	Conf;	/* link configuration		*/
		word		In;	/* bytes transmitted thru link	*/
		word		Out;	/* bytes received through link	*/
		word		Lost;	/* messages lost or sunk	*/
	} Link[4 /* NLinks */];		/* for each link		*/
	char	Name[Variable];		/* current network name		*/
					/* note that the exact position */
					/* of the Name field depends on	*/
					/* the value of NLinks.		*/
} ProcStats;

#endif /* __root_h */


/* -- End of root.h */

