/*------------------------------------------------------------------------
--                                                                      --
--           H E L I O S   N E T W O R K I N G   S O F T W A R E	--
--           ---------------------------------------------------	--
--                                                                      --
--             Copyright (C) 1990, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- tfmaux.h								--
--                                                                      --
--	Odds and ends needed by the Taskforce Manager and related	--
--	programs.							--
--                                                                      --
--	Author:  BLV 18/8/90						--
--                                                                      --
------------------------------------------------------------------------*/
/* $Header: /hsrc/network/RCS/tfmaux.h,v 1.10 1993/12/20 13:23:41 nickc Exp $ */

/**
*** compile-time flags, mainly for debugging
**/
#define	ParallelStartup	1

#if defined(__TRAN)
#define Native_Supported	1
#else
#define Native_Supported	0
#endif

/**
*** This structure is used to keep track of various useful thingies,
*** like allocated processors and running tasks
**/
typedef struct	TfmConnStruct {
	Semaphore	WriteLock;
	int		Program;
	List		Processors;
	List		Tasks;
	List		Taskforces;
	bool		FullAccess;
	int		Id;	/* Unique identifier for this application */
	int		Socket_ctos;
	int		Socket_stoc;
	Stream		*Pipe_ctos;
	Stream		*Pipe_stoc;
} TfmConnStruct;
typedef TfmConnStruct *TfmConn;

typedef void TfmRequestHandler(TfmConn, int, RmRequest *, RmReply *);

/**
*** This structure is similar in purpose to the ProcessorEntry structure
*** in the network server.
**/
#define	MaxUsersPerProcessor		32
#define MaxComponentsPerProcessor	32

typedef struct	AllocationInfo {
	Node		Node;		/* in TfmConn processor list */
	int		Id;
	TfmConn		Connection;
	RmProcessor	Processor;
} AllocationInfo;

typedef struct	DomainEntry {
	RmProcessor	Processor;
	int		NumberUsers;
	AllocationInfo	AllocationTable[MaxUsersPerProcessor];
} DomainEntry;

/**
*** Short-hand to reduce the number of RmLib calls
**/
#define GetDomainEntry(a)	((DomainEntry *)(a->Private))

/**
*** This structure is used to hold details of the binary executable and
*** the various libraries when starting up taskforce components. The TFM
*** selects one or more component tasks as the starting places for
*** taskforce loading, taking into account the different processor types
*** and the actual programs. All other components will point at the
*** appropriate starting point.
**/
#define MaxLibrariesPerComponent	8

typedef struct ComponentCode {
	Node		Node;
	List		List;
	RmTask		StartingPoint;
	Semaphore	Ready;
	Semaphore	StructLock;
	Object		*Libraries[MaxLibrariesPerComponent];
} ComponentCode;

/**		
*** The tfm is based on the processor manager where possible. This
*** structure is very similar to one in the processor manager. However,
*** in procman TaskEntry is a superset of ObjNode whereis in the TFM
*** TaskEntry is a separate data structure pointed at by the RmLib field
*** of the RmTask and RmTaskforce structure. Also it contains additional
*** information such as a table of the standard streams.
**/

typedef struct TaskEntry {
	Object		*Program;	/* Actual program object	     */
	Object		*LoadedCode;	/* Entry in the /loader directory    */
	Object		*ProgramObject;	/* Entry in the /tasks directory     */
	Stream		*ProgramStream;	/* stream to the /tasks entry	     */
	int		ProgramSize;	/* program size for mapping	     */
	RmTask		Task;		/* Pointer back to task.	     */
	int		UseCount;	/* Number of threads accessing this  */
					/* object (task or taskforce)	     */
					/* +1 for RmLib connection	     */
	Port		ProgInfoPort;	/* Reply port for ProgInfo	     */
	int		ProgInfoMask;	/* and a mask			     */
	int		KillState;	/* For exterminator		     */
	Semaphore	Finished;	/* Signalled by components	     */
	int		Mapped;		/* Has taskforce been mapped ?	     */
	TfmConn		Connection;	/* RmLib connection responsible      */
	Node		ConnectionNode; /* associated list node		     */
	List		Waiting;	/* Thingies waiting for this task    */
	ComponentCode	ComponentCode;	/* Location of various binaries	     */
	Stream		*Streams[1];	/* Table of pipes 		     */
} TaskEntry;

#define		DefaultStreams		4	/* stdin stdout stdout stddbg */
#define		TaskRetries		3
#define		TaskforceRetries	1
#define		TfmFlags_FirstTask	0x20000000
#define		TfmFlags_GotEnviron	0x10000000
#define		TfmFlags_Special	0x08000000

	/* Flags bit for internal streams, 0x00F00000 nibble reserved */
#define		TfmFlags_InternalStream	0x00100000

/**
*** Stack sizes
**/
#ifndef STACKEXTENSION
#define	Monitor_Stack			2000
#define	ConnectionGuardian_Stack	2000
#define	GuardianAux_Stack		2000
#define	Terminate_Stack			5000
#define	ParallelEnv_Stack		2000
#define LostProcessor_Stack		2000
#define	CheckWindow_Stack		1000
#define AcceptConnections_Stack		2000
#define	TFM_Stack			2000
#else
#define	Monitor_Stack			1000
#define	ConnectionGuardian_Stack	1000
#define	GuardianAux_Stack		1000
#define	Terminate_Stack			1000
#define	ParallelEnv_Stack		1000
#define LostProcessor_Stack		1000
#define	CheckWindow_Stack		 700
#define AcceptConnections_Stack		1000
#define	TFM_Stack			1000
#endif

/**
*** This is used to wait for termination
**/
typedef struct TaskWaiter {
	Node		Node;
	Semaphore	*Sem;
} TaskWaiter;

/**
*** Debugging options. N.B. the top bit must always be clear, or
*** there will be confusion between sending masks around and
*** error messages. dbg_Inquire is exempt because this is passed only
*** in the control vector.
**/
#define		dbg_Inquire		-1
#define		dbg_Redirect		-2
#define		dbg_Revert		-3
#define		dbg_Create		0x0001
#define		dbg_Mapping		0x0002
#define		dbg_Monitor		0x0004
#define		dbg_Environ		0x0008
#define		dbg_Delete		0x0010
#define		dbg_Signal		0x0020
#define		dbg_Memory		0x0040
#define		dbg_Allocate		0x0080
#define		dbg_Release		0x0100
#define		dbg_Comms		0x0200
#define		dbg_Lock		0x0400
#define		dbg_IOC			0x0800

#define Debug(a, b) if (DebugOptions & a) report b

		/* Module tfm.c */
extern void	   fatal(char *, ...);
extern void	   report(char *, ...);
extern DirNode	   Root;
extern DirNode	   TFM;
extern RmNetwork   Domain;
extern char	   NetworkName[];
extern int	   TaskSequenceNumber;
extern WORD	   DebugOptions;
extern WORD	   LastChange;
extern RmProcessor TfmProcessor;
extern Semaphore   LibraryLock;
extern void	   TerminateTFM(void);
extern char	   ProcessorName[];
extern Object	  *PipeCode;

		/* Module TfmJobs.c */
extern void	   InitJobs(void);
extern void	   AcceptConnections(void);
extern int	   Socket_stoc;
extern int	   Socket_ctos;
extern bool	   DomainLocked;
extern int	   ReplyRmLib(TfmConn, int, RmReply *);
extern bool        CreateDup2(TfmConn, int, Stream *);

		/* Module readcdl.c */
extern RmTaskforce RmReadCDL(Stream *, ImageHdr *, int);

		/* Module tfmmap.c */
extern void	   InitMap(void);
extern bool	   domain_MapTask(RmTask);
extern bool	   domain_FillInTaskMapping(RmTask, RmProcessor);
extern void	   domain_UnmapTask(RmTask);
extern bool	   domain_MapTaskforce(RmNetwork, RmTaskforce);
extern void	   domain_UnmapTaskforce(RmTaskforce);
extern void	   HandleObtainProcessor(TfmConn, int, RmRequest *, RmReply *);
extern void	   HandleReleaseProcessor(TfmConn, int, RmRequest *, RmReply *);
extern void	   HandleObtainNetwork(TfmConn, int, RmRequest *, RmReply *);
extern void	   HandleReleaseNetwork(TfmConn, int, RmRequest *, RmReply *);
extern void	   HandleObtainProcessors(TfmConn, int, RmRequest *, RmReply *);
extern word	   AutomaticRelease(Node *Node);
extern void	   ReturnProcessorToPool(RmProcessor);
extern bool	   AddDomainEntry(RmProcessor);

		/* Module tfmrun.c */
extern int	   MonitorDelay;
extern void	   InitRun(void);
extern int	   task_AddTaskEntry(RmTask, ...);
extern word	   task_Run(RmProcessor processor, RmTask task, Object *code);
extern void	   task_Monitor(RmTask);
extern void 	   task_GenProgInfo(RmTask);
extern void 	   task_Destroy(RmTask);
extern void 	   task_DoSignal(RmTask, word);
extern word	   task_HandleEnv(RmTask, Environ *);
extern bool	   task_FilterArgs(RmTask, Environ *sending, char **received, char **default_args);
extern void	   task_Exterminate(RmTask);

extern word	   taskforce_Start(RmTaskforce);
extern void	   taskforce_Monitor(RmTaskforce);
extern void	   taskforce_GenProgInfo(RmTaskforce);
extern void	   taskforce_Destroy(RmTaskforce);
extern void	   taskforce_DoSignal(RmTaskforce, word);
extern word	   taskforce_HandleEnv(RmTaskforce, Environ *);

		/* Module tfmwoe.c  */
extern void	   Tfm_ExceptionHandler(int fnrc, int size, BYTE *data);
extern void	   LostProcessor(int);
extern void	   SuspiciousProcessor(int);
extern void	   MarkProcessor(RmProcessor);
extern void	   HandleReportProcessor(TfmConn, int, RmRequest *, RmReply *);

		/* Module tfmmisc.c */
extern void	   InitMisc(void);
extern word	   FullRead(Stream *, BYTE *, word, word);		
extern char	  *BuildName(char *buffer, RmProcessor);
extern RmProcessor LookupProcessor(RmNetwork, char *);
extern bool	   MatchProcessor(RmProcessor real, RmProcessor Template);
extern bool	   MatchTask(RmProcessor real, RmTask Template);
extern Object	  *TfmMapProcessorToObject( RmProcessor);
extern Port	   tfm_GetEnv(Port reqport, MCB *m, Environ *env);
extern void	   tfm_FreeEnv(Environ *env);
