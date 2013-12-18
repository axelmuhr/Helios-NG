/*------------------------------------------------------------------------
--                                                                      --
--           H E L I O S   N E T W O R K I N G   S O F T W A R E	--
--           ---------------------------------------------------	--
--                                                                      --
--             Copyright (C) 1990, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- netaux.h								--
--                                                                      --
--	Additional odds and ends needed by the Network Server and	--
--	related programs/drivers.					--
--                                                                      --
--	Author:  BLV 18/8/90						--
--                                                                      --
------------------------------------------------------------------------*/
/* $Header: /hsrc/network/RCS/netaux.h,v 1.21 1993/08/11 10:32:52 bart Exp $ */

#ifndef __config_h
#include <config.h>
#endif
#ifndef __c40_h
#include <c40.h>
#endif

#ifdef __NetworkServer

#if defined(__TRAN)
#define Native_Supported	1
#else
#define Native_Supported	0
#endif
#if !defined(__C40)
#define Joinnet_Supported	1
#else
#define Joinnet_Supported	0
#endif

	/* This is used for requests sent by the resource management library */
typedef struct	NsConnStruct {
	Semaphore	WriteLock;
	int		Program;
	int		FullAccess;
	List		Processors;
	int		Id;	/* Unique identifier for this session */
	int		Socket_ctos;
	int		Socket_stoc;
	Stream		*Pipe_ctos;
	Stream		*Pipe_stoc;
} NsConnStruct;
typedef NsConnStruct *NsConn;

	/* This is used for new bootstrap jobs */
typedef struct	BootstrapJob {
	int		Sequence;
	int		NumberProcessors;
	int		Next;
	int		MaxProcessors;
	int		SafetyCheck;
	RmProcessor	*Table;
	RmProcessor	*Progress;
	Semaphore	Lock;
	Semaphore	Finished;
	int		NumberProcesses;
	Semaphore	ProcessesFinished;
	Semaphore	LinksPending;
	Semaphore	LinksEnabled;
	Semaphore	ClearNames;
	bool		JobStarted;
} BootstrapJob;

	/* this structure is used for external network connections */
typedef	struct	ExternalNetwork {
	RmProcessor	Connector;
	int		Link;
	bool		Reported;
} ExternalNetwork;


#define Debug(a, b) if (DebugOptions & a) debug b
extern		WORD	DebugOptions;
#endif

/**
*** Debugging options. N.B. the top bit must always be clear, or
*** there will be confusion between sending masks around and
*** error messages. dbg_Inquire is exempt because this is passed only
*** in the control vector.
**/
#define		dbg_Inquire		-1
#define		dbg_Redirect		-2
#define		dbg_Revert		-3
#define		dbg_Boot		0x0001
#define		dbg_Execute		0x0002
#define		dbg_Allocate		0x0004
#define		dbg_Release		0x0008
#define		dbg_Monitor		0x0010
#define		dbg_Problem		0x0020
#define		dbg_Links		0x0040
#define		dbg_Initialise		0x0080
#define		dbg_Memory		0x0100
#define		dbg_Native		0x0200
#define		dbg_Comms		0x0400
#define		dbg_Lock		0x0800
#define		dbg_IOC			0x1000

/**
*** Protocols used between the Network Server and the Netagent program.
**/
#define	NetAgentCode		"/helios/lib/netagent"

typedef struct	NA_Message {
	WORD	FnRc;
	WORD	Arg1;
	WORD	Arg2;
	WORD	Size;
	BYTE	*Data;
} NA_Message;

#define NA_Quit			0x001
#define NA_TransputerBoot	0x002	/* Normal transputer bootstrap	*/
#define NA_ParsytecBoot		0x003	/* same, but with parsytec reset*/
#define NA_SetLinkMode		0x004	/* Link control			*/
#define NA_Protect		0x005	/* set protection		*/
#define NA_Revoke		0x006
#define NA_Cupdate		0x007	/* Change name			*/
#define NA_Clean		0x008	/* Clean out any garbage	*/
#define NA_ClearNames		0x009	/* Clean out name tables	*/
#define NA_ParsytecReset	0x00A	/* perform a Parsytec reset	*/
#define NA_UpdateIO		0x00B	/* update an I/O processor name	*/
#define NA_GetLinkMode		0x00C	/* get current link state	*/
#define NA_Terminate		0x00D	/* Terminate processor		*/
#define NA_Noop			0x00E	/* Keep running please		*/
#define NA_C40Boot		0x00F	/* Normal C40 bootstrap		*/
#define NA_i860Boot		0x010	/* Normal i860 bootstrap	*/

/**
*** These manifests are used in conjunction with processor bootstraps.
*** They are not documented anywhere, but see the source file kernel/link1.c.
*** BootLink() and My_BootLink() can fail in various different places.
**/
#define	Boot_SoftResetSize	 1
#define Boot_SoftResetCode	 2
#define Boot_BootstrapSize	 3
#define Boot_BootstrapCode	 4
#define Boot_Clear1		 5
#define Boot_Clear2		 6
#define Boot_Clear3		 7
#define Boot_ControlByte	 8
#define Boot_Image		 9
#define Boot_ConfigSize		10
#define Boot_ConfigVector	11
#define Boot_ProtocolTail	12
#define Boot_Acknowledgement	13
#define Boot_Acknowledgement2	14
#define Boot_Hwconfig		15
#define Boot_Idrom		16

/**
*** This structure is used for C40 bootstrap requests. It is an extended
*** version of the configuration vector.
**/
typedef struct C40_Bootstrap {
	RPTR	Nucleus;
	RPTR	Bootfile;
	WORD	Hwconfig;
	IDROM	Idrom;
	WORD	ConfigSize;
	Config	Config;
} C40_Bootstrap;

/**
*** These structures are used inside the Network Server and in device drivers
*** loaded by the Network Server. Associated with every processor are the
*** following.
***
*** 1) Three capabilities. One for owner access, which allows
***    everything accept changing the access. One for the network server
***    which includes alter access. This means that the Network Server is the
***    only program in the entire system that can change the access matrix,
***    by passing it onto the network agent. And one read-only capability.
*** 2) A count of the number of device drivers and commands that have some
***    control over this processor.
*** 3) A table of structures, one for every device driver. This table is
***    actually at the end of the data structure so that it gets freed
***    at the same time, but is of variable size. For now I allow two
***    words of information per device driver per processor, which should
***    be enough.
*** 4) A table of the standard links. This allows the Network Server to
***    reconfigure processors back to a sensible state when they are returned
***    to the system pool, if users have been fiddling with link switches.
***    N.B. this table should not be copied until after the device drivers
***    have been initialised, to allow for the Telmat configuration driver
***    to change the connections. Also, various other bits of state.
*** 5) Details of the Netagent if it is currently running on that processor.
***    When the count drops to zero the program is terminated.
**/
typedef struct	DriverEntry {
	RmHardwareFacility	*Hardware;
	word			Flags;
	word			Aux1;
	word			Aux2;
} DriverEntry;

#define	DriverFlags_ResetDriver		0x00000001
#define DriverFlags_ConfigureDriver	0x00000002
#define DriverFlags_ResetCommand	0x00000004
#define DriverFlags_DefiniteReset	0x00000010
#define DriverFlags_PossibleReset	0x00000020
#define DriverFlags_SpecialBootstrap	0x00000040
#define DriverFlags_NativePossible	0x00000080
#define DriverFlags_MappingFlexible	0x00000100
#define DriverFlags_Reclaim		0x00000200

typedef struct	ProcessorEntry {
	Node		NetagentNode;
	Stream		*Netagent;
	Semaphore	NetagentLock;
	word		NetagentCount;
	word		NetagentDate;
	RmProcessor	Processor;	/* pointer back to real processor */
	Node		Connection;	/* entry in connection list */
	Node		Cleaners;	/* node for the cleaners list */
	Capability	Owner;
	Capability	General;
	Capability	Full;
	int		Incarnation;
	word		NumberDrivers;
	DriverEntry	*DriverEntry;
	word		Purpose;
	RmLink		*StandardLinks;
	bool		BeingBooted;
	word		CommandDate;
	Object		*WindowServer;
	Object		*ConsoleWindow;
} ProcessorEntry;

/**
*** These data structures are used in the network device drivers. 
*** 1) NetworkFuncs is a simple table to let the device drivers access useful
***    routines within the Network Server.
*** 2) NetworkDCB is the device control block, as per the usual device drivers.
***    I find this structure not very useful, so in addition...
*** 3) DriverRequest is the most common structure seen by people working on
***    device drivers. DriverConfRequest is used for certain link configuration
***    requests which are more complex.
***    
**/
typedef struct NetworkFuncs {
	VoidFnPtr	report;
	VoidFnPtr	fatal;
	WordFnPtr	LookupProcessor;
	WordFnPtr	rexec;
	WordFnPtr	BuildConfig;
	WordFnPtr	StartNetworkAgent;
	WordFnPtr	StopNetworkAgent;
	WordFnPtr	XchNetworkAgent;
} NetworkFuncs;

typedef struct NetworkDCB {
	DCB		DCB;
	char		*NetworkName;
	RmNetwork	Net;
	void		*HardwareFacility;
	RmProcessor	RootProcessor;
	NetworkFuncs	*Functions;
	int		Spare[20];	/* BLV - used to be 5, but not enough */
} NetworkDCB;

typedef struct DriverRequest {
	int		FnRc;
	int		NumberProcessors;
	RmProcessor	Processor[1];
} DriverRequest;


typedef struct	DriverConnection {
	RmProcessor	Source;
	int		SourceLink;
	RmProcessor	Dest;
	int		DestLink;
} DriverConnection;

typedef struct DriverConfRequest {
	int			FnRc;
	int			NumberConnections;
	bool			Exact;
	bool			Preserve;
	DriverConnection	*Connections;
} DriverConfRequest;

#define ND_Initialise		0x01	/* For reset and config drivers	*/

#define ND_Reset		0x02	/* Reset drivers only		*/
#define ND_Analyse		0x03
#define ND_TestReset		0x04
#define ND_Boot			0x05
#define ND_ConditionalReset	0x06

#define	ND_MakeLinks		0x10	/* Configuration drivers only	*/
#define ND_TestLinks		0x11
#define ND_ObtainProcessors	0x12
#define ND_MakeInitialLinks	0x13
#define ND_FreeProcessors	0x14

/**
*** These flags make use of the nibble 0x00F00000, reserved for
*** applications.
***
*** This flag is used to indicate whether or not other users should get
*** read-only access to processors.
**/
#define	NsFlags_DenyReadOnly		0x00100000

/**
*** This flag is used for processors not specified in the resource map.
*** When external networks are added to the main machine, every processor
*** inside the external network will have this flag set.
**/
#define	NsFlags_NotInResourceMap	0x00200000

/**
*** This flag is used for processors allocated to a TFM. If one of them
*** goes then things are seriously wrong.
**/
#define NsFlags_TfmProcessor		0x00400000

/**
*** Stack sizes for various threads spawned within the Network Server
*** 1) BootstrapStack. Every bootstrap job involves one master thread
***    plus an additional thread for every processor booted in this job.
**/
#ifndef STACKEXTENSION
#define	Bootstrap_Stack			2000
#define	Monitor_Stack			1500
#define Problem_Stack			2500
#define ProblemAux3_Stack		1500
#define AcceptRmLib_Stack		2000
#define ConnectionGuardian_Stack	2000
#define	AgentMonitor_Stack		1024
#define Cleaning_Stack			1024
#define NS_Stack			2000
#else
#define	Bootstrap_Stack			1000
#define	Monitor_Stack			 750
#define Problem_Stack			1000
#define ProblemAux3_Stack		1000
#define AcceptRmLib_Stack		1000
#define ConnectionGuardian_Stack	1000
#define	AgentMonitor_Stack		 750
#define Cleaning_Stack			 750
#define	NS_Stack			1000
#endif

/**
*** Function prototypes within the Network Server
**/
#ifdef __NetworkServer
			/* Module netserv.c */
extern	void		fatal	(char *, ...);
extern	void		report	(char *, ...);
extern	void		debug	(char *, ...);
extern  char		ProcessorName[IOCDataMax];
extern	Object		*ThisProcessor;
extern	RmNetwork	Net;
extern	RmTaskforce	DefaultTaskforce;
extern	RmProcessor	RootProcessor;
extern	RmProcessor	BootIOProcessor;
extern	char		NetworkName[NameMax];
extern	bool		FullReset;
extern	int		NumberProcessors;
extern	Object		*NetAgent;
extern	bool		SilentMode;
extern	WORD		LastChange;
extern	void		TerminateNetworkServer(void);
extern  RmProcessor	LastBooted;
extern	int		ReplyRmLib(NsConn, int, RmReply *);
extern	void		RemConnection(RmProcessor, int reason);
extern	void		InformConnection(RmProcessor, int reason);
extern	void		AbortConnection(NsConn);
extern	void		KickSessionManager(void);
extern	char		**nsrc_environ;

			/* Module netboot.c */
extern	void		InitBootstrap(void);
extern	void		StartBootstrap(void);
extern	BootstrapJob	*NewBootstrapJob(void);
extern	bool		AddProcessorToBootstrapJob(BootstrapJob *, RmProcessor);
extern	bool		StartBootstrapJob(BootstrapJob *);
extern	void		WaitBootstrapJob(BootstrapJob *);
extern	void		AbortBootstrapJob(BootstrapJob *);
extern	bool		BuildTRANConfig(RmProcessor source,
	RmProcessor dest, int destlink, Config **config_vec, word *confsize);
extern	void		UpdateProcessor(RmProcessor, bool name_only);
extern	void		UpdateIOProcessor(RmProcessor, bool name_only);
extern	void		HandleGetLinkMode(NsConn, int, RmRequest *, RmReply *);
extern	void		HandleSetLinkMode(NsConn, int, RmRequest *, RmReply *);
extern	int		SetLinkMode(RmProcessor Processor, int link, int mode);
extern	int		GetLinkMode(RmProcessor Processor, int link);
extern	void		ClearNames(RmProcessor);
extern	int		ResetProcessors(int, RmProcessor *);
	
			/* Module netmon.c */
extern	int		Monitor_Delay;
extern	void		InitMonitor(void);
extern	void		MarkProcessor(RmProcessor, bool locate_failed);
extern	void		MarkLink(RmProcessor, int);
extern	bool		CheckProcessor(RmProcessor);
extern	void		HandleReportProcessor(NsConn, int, RmRequest *, RmReply *);
extern	void		HandleLinkChange(ServInfo *);
extern	void		HandleAcceptNetwork(NsConn, int, RmRequest *, RmReply *);
extern	void		HandleJoinNetwork(NsConn, int, RmRequest *, RmReply *);

			/* Module netalloc.c */
extern	void		InitAlloc(void);
extern	void		HandleGetNetwork(NsConn, int, RmRequest *, RmReply *);
extern	void		HandleLastChange(NsConn, int, RmRequest *, RmReply *);
extern	void		HandleIsProcessorFree(NsConn, int, RmRequest *, RmReply *);
extern	void		HandleObtainProcessor(NsConn, int, RmRequest *, RmReply *);
extern	void		HandleReleaseProcessor(NsConn, int, RmRequest *, RmReply *);
extern	void		HandleObtainNetwork(NsConn, int, RmRequest *, RmReply *);
extern	void		HandleReleaseNetwork(NsConn, int, RmRequest *, RmReply *);
extern	word		AutomaticRelease(Node *);

			/* Module netnativ.c */
extern	void		InitNative(void);
extern	void		HandleNative(NsConn, int, RmRequest *, RmReply *);
extern	void		HandleConnections(NsConn, int, RmRequest *, RmReply *);
extern	void		CleanNative(RmProcessor);

			/* Module netmisc.c, utility routines.		*/
extern	void		InitMisc(void);
extern	bool		StartNetworkAgent(RmProcessor);
extern	bool		StopNetworkAgent(RmProcessor);
extern	int		XchNetworkAgent(RmProcessor, NA_Message *, bool rc,
			int rsize, BYTE *rdata);
extern	void		RemNetworkAgent(RmProcessor);
extern	void		TerminateProcessor(RmProcessor);
extern	word		rexec(RmProcessor, Object *, Environ *, word delay);
extern	word		rexec_task(RmProcessor, RmTask, Environ *, word delay);
extern	Object		*NetMapProcessorToObject(RmProcessor);
extern	char		*BuildName(char *buffer, RmProcessor);
extern	word		FullRead(Stream *, BYTE *, word , word);
extern	RmProcessor	LookupProcessor(RmNetwork, char *);

#endif /* __NetworkServer */

