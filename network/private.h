/*------------------------------------------------------------------------
--                                                                      --
--           H E L I O S   N E T W O R K I N G   S O F T W A R E	--
--           ---------------------------------------------------	--
--                                                                      --
--             Copyright (C) 1990, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- private.h								--
--                                                                      --
--	Header file defining Resource Management library facilities	--
--	that are not publicly available. In addition it contains some	--
--	useful macros.							--
--                                                                      --
--	N.B. This header file must be included before RmLib.h		--
--									--
--	Author:  BLV 1/5/90						--
--                                                                      --
------------------------------------------------------------------------*/

/* $Header: /hsrc/network/RCS/private.h,v 1.13 1993/08/06 10:20:11 nickc Exp $ */

#if defined(__SUN4) || defined (RS6000)
/*
 * /usr/include does not have an ampp directory, so this hack forces
 * the compiler to pick up the Helios version of memory.h
 */
#include "ampp/../memory.h"
#endif

#ifndef __helios_h
#include <helios.h>
#endif

#ifndef __syslib_h
#include <syslib.h>
#endif

#ifndef __sem_h
#include <sem.h>
#endif

#ifndef __servlib_h
#include <servlib.h>
#endif

#ifndef __device_h
#include <device.h>
#endif

#ifdef __rmlib_h
#error The header file private.h must be included BEFORE rmlib.h
#endif

#define eq ==
#define ne !=
#define Clear(a) memset((BYTE *) &a, 0, sizeof(a))
#define abend	goto
#define endcase	break

#if defined(__SUN4)
#define EXIT_FAILURE 1
#define EXIT_SUCCESS 0
#endif

/**
*** The four basic data structures, RmProcessor, RmNetwork, RmTask and
*** RmTaskforce. These are pointers to structures RmProcessorStruct etc.
**/

#define __RmStructs 1	/* prevent redefinition in RmLib. h */

/**
*** First, every processor in a network and every task in a taskforce
*** has a unique identifier or uid. The connections between these
*** objects are stored using uid's, so that there is less work to do
*** when shipping objects from program to program. 
*** The Root object maintains a table of tables,
*** mapping uid's to pointers, using a similar mechanism to the
*** kernel's message port tables. Life is slightly easier for the kernel
*** because ports are global for the whole processor. In the Resource
*** Management library the root network changes, for example when a
*** subnet is added to another network a merging of uid's has to take
*** place.
***
*** The uid's maintained by the Network Server are global. If processor
*** 02 has uid x inside the Network Server and has been allocated to
*** the Task Force Manager for user Tom, then processor 02 inside the
*** user's domain will also have uid x. This may give problems when
*** merging networks and when using notifyns or joinnet, but nothing
*** too serious.
***
*** Only leaf objects, i.e. processors and tasks, have uid's. There is
*** no point in giving subnets uids.
***
*** The table of tables of Uid's consists of UidTableEntry structures.
*** The top byte is a cycle number, used in the same way as port cycle
*** numbers. Changing the size of the network with notifyns etc. may
*** cause table entries to be reallocated, so a cycle number could be
*** useful. For a processor in the global network the cycle number
*** will always remain the same. The middle two bytes are not currently
*** used, because I cannot think of anything useful to put into them.
*** The whole of the bottom byte is used as a flag field, to indicate
*** whether or not the slot in the table is free. The second word of
*** the UidTableEntry structure is a pointer to the appropriate object,
*** either a processor or a task, and this has to be changed when shipping
*** objects between programs.
***
*** A Uid itself is just one word. The top byte is a cycle number, as
*** before. The middle two bytes are a table index, an offset in the
*** table of tables. The bottom byte is an index into the resulting
*** table. At present each table has 64 entries, requiring 512 bytes.
*** Hence two bits of the bottom byte are unused. Using all
*** 8 bits would mean tables of 2K and not needing any expansion until
*** the network expands past 256 processors, which is too much for
*** small PC networks.
**/

typedef struct RmUidTableEntry {
	byte	Cycle;
	byte	Spare1;
	byte	Spare2;
	byte	Free;
	void	*Target;	/* pointer to RmProcessor or RmTask */
} RmUidTableEntry;

typedef int RmUid;

#define RmL_SlotsPerTable	64
#define RmL_NoObject		((RmObject)NULL)
#define RmL_ExtObject		((RmObject) 1)
#define RmL_ExternalObject	((RmObject) 1)
#define RmL_NoUid		0	/* guaranteed invalid */
#define RmL_ExtUid		1	/* ditto */
#define RmGetCycle(Uid) ((BYTE)((uid >> 24) & 0x0FF))

/**
*** Processors are connected by links. The link structure
*** consists of four bytes and two word. The bytes are a flags field,
*** and currently three spare fields. These spare fields may be used
*** by individual programs such as the Network Server. The next word
*** is a destination link, i.e. which link on the other processor is
*** connected to this one. Then there is a Uid. There are two special Uids,
*** RmL_NoUid and RmL_ExtUid, and the first two
*** slots in any table are reserved for these with contents RmM_NoObject
*** and RmM_ExternalObject.
**/ 

typedef struct RmLink {
	byte	Flags;
	byte	Spare1;
	byte	Spare2;
	byte	Spare3;
	int	Destination;
	RmUid	Target;
} RmLink;

/**
*** Tasks communicate over channels. Basically these are the same as
*** links. The channel and link structures must be kept the same,
*** so that code can be shared for the two. Channels are bidirectional.
**/

typedef struct RmChannel {
	byte	Flags;
	byte	Spare1;
	byte	Spare2;
	byte	Spare3;
	int	Destination;
	RmUid	Target;
} RmChannel;


/**
*** In the Resource Management library it is important to keep memory
*** requirements down. Hence the library routines share code for many
*** operations. For example, RmAddTailProcessor() and RmAddTailTask()
*** both end up calling RmAddTailObject(), private to the library,
*** after performing the necessary validation checks. There are two
*** private types RmSet and RmObject, and the four main data types are
*** supersets of these.
***
*** The RmSet structure is the base for the RmNetwork and RmTaskforce
*** structures. It contains a server library DirNode, which has much of
*** the information that is required. In particular the DirNode contains
*** a count of the number of entries, a list for these entries, a pointer
*** to the parent, a name, and protection support. Following the
*** DirNode is a pointer back to the root of the current structure, set
*** to NULL if this is the root. Since following links requires access
*** to the root, it is important that this can be done quickly. If the
*** set is not the root, then NoTables will be set to 0 and the pointer
*** will be NULL. Otherwise these fields give details of the table of
*** tables for mapping uid's to objects.
**/

typedef struct RmSetStruct {
	DirNode			DirNode;
	struct RmSetStruct	*Root;
	int			Private;
	byte			StructType;
	byte			NoSubsets;
	byte			Spare1;
	byte			Spare2;
	int			NoTables;
	RmUidTableEntry		**Tables;
} RmSetStruct;
typedef RmSetStruct *RmSet;
	
/**
*** An RmNetwork structure is a superset of RmSet. It contains
*** additional information for reset and configuration support.
*** Also, it contains a StructType field indicating how the Network
*** came into existence: a call to RmNewNetwork() for a new structure;
*** or to RmObtainNetwork(); or to RmGetRootNetwork().
**/

typedef struct RmNetworkStruct {
	DirNode			DirNode;
	RmSet			Root;
	int			Private;
	byte			StructType;
	byte			NoSubnets;
	byte			Spare1;
	byte			Spare2;
	int			NoTables;
	RmUidTableEntry		**Tables;
	List			Hardware;
	int			Private2;
	int			Errno;
	int			Spares[8];
} RmNetworkStruct;
typedef RmNetworkStruct *RmNetwork;

#define RmL_New			1	/* for processors and tasks	*/
#define RmL_Obtained		2	/* processors only		*/
#define RmL_Existing		3	/* processors only ?		*/
#define RmL_Executing		2	/* tasks only 			*/
#define RmL_Terminated		4	/* tasks only			*/
#define RmL_Done		4	/* alias			*/

/**
*** A Taskforce is another superset of RmSet. At present I cannot think
*** of anything else useful to put into it.
**/

typedef struct RmTaskforceStruct {
	DirNode			DirNode;
	RmSet			Root;
	int			Private;
	BYTE			StructType;	/* RmL_New or RmL_Executing */
	BYTE			NoSubsets;
	BYTE			Spare1;
	BYTE			Spare2;
	int			NoTables;
	RmUidTableEntry		**Tables;
	int			Private2;
	int			Errno;
	int			ReturnCode;
	int			State;
	Capability		TfmCap;	
	int			Spares[4];
} RmTaskforceStruct;
typedef RmTaskforceStruct *RmTaskforce;

/**
*** RmObject is the base for RmProcessor and RmTask. It contains an
*** ObjNode structure as might be expected, a pointer to the root which
*** is NULL if the object is not currently in a set, a Uid, an integer
*** reserved for use by the library, a count of the number of connections,
*** space for a default of four connections, and a pointer to a table
*** of additional connections if required. Four connections seems like
*** a reasonable compromise for the default. Transputers have four links.
*** C programs have three channels stdin, stdout, and stderr. If fewer
*** than four connections are needed, the cost is only eight bytes per
*** wasted slot. If more are needed the structures are expandable.
**/

typedef struct RmObjectStruct {
	ObjNode		ObjNode;
	RmSet		Root;
	int		Private;	
	byte		StructType;	/* New, Obtained, Existing... */
	byte		Spare1;
	byte		Spare2;
	byte		Spare3;
	RmUid		Uid;
	int		Connections;
	RmLink		DefaultLinks[4];
	RmLink		*OtherLinks;
	int		AttribSize;
	int		AttribFree;
	char		*AttribData;
	int		PAttribSize;
	int		PAttribFree;
	char		*PAttribData;
} RmObjectStruct;
typedef RmObjectStruct *RmObject;

/**
*** A processor is a superset of RmObject. The size field is used to
*** hold the state of the processor as well as the current purpose,
*** and the account field is the owner.
*** There are lots of additional fields, to be decided.
**/

typedef struct RmProcessorStruct {
	ObjNode		ObjNode;
	RmNetwork	Root;
	int		Private;	
	byte		StructType;	/* New, Obtained, Existing... */
	byte		AllocationFlags;
	byte		Control;
	byte		Purpose;	/* Purpose used to be in Size */
	RmUid		Uid;
	int		Connections;
	RmLink		DefaultLinks[4];
	RmLink		*OtherLinks;
	int		AttribSize;
	int		AttribFree;
	char		*AttribData;
	int		PAttribSize;
	int		PAttribFree;
	char		*PAttribData;
	Capability	RealCap;	/* for /Cluster/00	*/
	Capability	NsCap;		/* for /Cluster/ns/00	*/
	Capability	ReadOnlyCap;
	unsigned long	MemorySize;	/* in bytes */
	int		Type;		/* T800, H1, ... */
	int		MappedTo;	/* see RmObtainNetwork() */
	List		MappedTasks;
	int		Private2;
	int		Errno;
	int		SessionId;
	int		ApplicationId;
	int		Spares[2];
	/* ... */
} RmProcessorStruct;
typedef RmProcessorStruct *RmProcessor;

/**
*** By a strange coincidence the RmTask structure is also a superset
*** of RmObject. The account and size fields are not currently used.
**/
typedef struct RmTaskStruct {
	ObjNode		ObjNode;
	RmTaskforce	Root;
	int		Private;	
	byte		StructType;	/* New, Executing */
	byte		IsNative;	/* TRUE or FALSE */
	byte		Spare1;
	byte		Spare2;
	RmUid		Uid;
	int		Connections;
	RmChannel	DefaultChannels[4];
	RmChannel	*OtherChannels;
	int		AttribSize;
	int		AttribFree;
	char		*AttribData;
	int		PAttribSize;
	int		PAttribFree;
	char		*PAttribData;
	unsigned long	MemorySize;	/* requirements in bytes */
	int		Type;		/* target processor */
	int		*ArgIndex;
	int		MaxArgIndex;
	int		NextArgIndex;
	char		*ArgStrings;
	int		MaxArgStrings;
	int		NextArgStrings;
	RmUid		MappedTo;
	Node		MappedNode;
	int		Private2;
	int		Errno;
	int		ReturnCode;
	Capability	TfmCap;
	int		Spares[2];
} RmTaskStruct;	
typedef RmTaskStruct *RmTask;

/**
*** These macros are useful for diagnostic messages, without the considerable
*** memory overhead of invoking RmGetProcessorId() etc.
**/
#define Procname(a)		((a)->ObjNode.Name)
#define Netname(a)		((a)->DirNode.Name)
#define Taskname(a)		((a)->ObjNode.Name)
#define Taskforcename(a)	((a)->DirNode.Name)

/**
*** These are shorthands used within the TFM and Network Server
**/
#define GetProcEntry(a)		((ProcessorEntry *)(a->Private))
#define GetTaskEntry(a)		((TaskEntry *)(a->Private))

/**
*** The library contains a hidden variable RmProgram, specifying the
*** current program. By default this is set to user, but network
*** programs that know about it can set it to something else. The variable
*** controls which program to contact for certain operations such as
*** obtaining a processor. For user application such requests should
*** go via the Task Force Manager, but for system software the request
*** may go directly to the Network Server. A capability is provided to
*** validate this. The capability is returned to startns by the network
*** server, and is passed on to the session manager and hence to
*** taskforce managers.
**/

extern int		RmProgram;
#define Rm_Netserv	0x01
#define Rm_Session	0x02
#define Rm_TFM		0x03
#define Rm_Startns	0x04
#define Rm_User		0x7F
extern Capability	RmLib_Cap;

/**
*** Yet another undocumented variable. This one may be set to the 
*** Uid of a particular processor in order to start a search at
*** that processor, when using RmObtainNetwork
**/
extern int		RmStartSearchHere;

/**
*** Another secret variable, giving the version number of the
*** Resource Management library.
**/
extern char		*RmVersionNumber;

/**
*** And another, an exception handler for special messages.
**/
extern	VoidFnPtr	RmExceptionHandler;
extern	int		RmExceptionStack;

/**
*** Flags for processor allocation
**/
#define	RmF_Exclusive		0x01
#define RmF_Permanent		0x02
#define RmF_TfmProcessor	0x04
#define RmF_Booked		0x08

/**
*** These are function codes exchanged between applications and the networking
*** servers. At present these are written down pipes, but this may change.
*** Function codes start with RmC_, and replies with RmR_. N.B. the codes
*** do not clash with any nucleus codes, and fit within the right bits.
**/
#define	RmC_Init			0x00010010
#define RmC_Startns			0x00010020
#define	RmC_GetNetwork			0x00010030
#define RmC_GetHierarchy		0x00010040
#define	RmC_ObtainProcessor		0x00010050
#define	RmC_ReleaseProcessor		0x00010060
#define	RmC_ProcessorPermanent		0x00010070
#define RmC_ProcessorTemporary		0x00010080
#define RmC_ProcessorExclusive		0x00010090
#define RmC_ProcessorShareable		0x000100A0
#define RmC_ObtainExactNetwork		0x000100B0
#define	RmC_ObtainNetwork		0x000100C0
#define RmC_ReleaseNetwork		0x000100D0
#define RmC_LastChange			0x000100E0
#define RmC_GetNetworkHardware		0x000100F0
#define RmC_GetLinkMode			0x00010100
#define RmC_SetLinkMode			0x00010110
#define RmC_Lock			0x00010120
#define RmC_Unlock			0x00010130
#define	RmC_AcceptNetwork		0x00010140	/* Used for joinnet */
#define RmC_JoinNetwork			0x00010150
#define RmC_IsProcessorFree		0x00010160
#define RmC_ObtainProcessors		0x00010170
#define RmC_ObtainExactProcessors	0x00010180
#define RmC_Reboot			0x00010190
#define RmC_ResetProcessors		0x000101A0
#define RmC_SetNative			0x000101B0
#define RmC_Revert			0x000101C0
#define	RmC_TestConnections		0x000101D0
#define RmC_MakeConnections		0x000101E0
#define RmC_ProcessorBooked		0x000101F0
#define RmC_ProcessorCancelled		0x00010200
#define	RmC_GetId			0x00010210
#define RmC_RegisterTfm			0x00010220
#define RmC_ReportProcessor		0x00010230
#define	RmC_GetTaskforce		0x00010240
#define RmC_GetTask			0x00010250
#define RmC_Execute			0x00010260
#define RmC_Update			0x00010270
#define RmC_SendSignal			0x00010280
#define RmC_Leave			0x00010290
#define RmC_Wait			0x000102A0

	/* This flag means that RmTx should leave the stream locked on	*/
	/* success. It is used to enforce locking within the client	*/
	/* during complicated transactions.				*/
#define RmC_KeepLocked			0x40000000

#define RmR_Private		0x80000000
#define	RmR_Terminate		0x00020010
#define RmR_Crashed		0x00020020
#define RmR_Synch		0x00020030
#define RmR_Reclaimed		0x00020040
#define RmR_Suspicious		0x00020050
#define RmR_Dup2		0x00020060
#define	ConnectMagic		0x26ab4582

/**
*** The filter data structure is used when writing data down a stream, mostly
*** to strip out sensitive information such as the keys used to create
*** capabilities. The relevant routines can all take a NULL pointer
*** if no filtering is required. Unless the SendHardware flag is set,
*** the routines will not send any details of the reset and configuration
*** drivers.
**/
typedef struct RmFilterStruct {
	int  (*Network)(RmNetwork real, RmNetwork copy);
	int  (*Processor)(RmProcessor real, RmProcessor copy);
	int  (*Taskforce)(RmTaskforce real, RmTaskforce copy);
	int  (*Task)(RmTask real, RmTask copy);
	bool SendHardware;
} RmFilterStruct;
typedef RmFilterStruct	*RmFilter;

/**
*** The RmServer structure is used inside the Resource Management library,
*** to hold the basic details of a stream connection to a networking server.
*** Typically there is one of these for the applications connection to the
*** Network Server, one for the connection to the Session Manager, and one
*** for the connection to the parent (either the Taskforce Manager or the
*** Network Server, depending on the program).
***
*** The structure contains two streams, one for each direction, at the Helios
*** Stream level. Use of system calls is necessary to get timeouts, and use
*** of two fields for each socket helps to avoid race conditions/deadlocks.
*** There is a semaphore to control write access to the server. Read locking
*** is handled by a semaphore inside the RmJob structure. When the pipe
*** guardian detects a message for a particular job, it walks down the list 
*** of jobs looking for the appropriate structure. The MaxId field is used
*** to keep track of the next job Id, and the StructLock semaphore is used by
*** RmNewJob() and RmFinishedJob().
**/
typedef struct RmServerStruct {
	Semaphore	WriteLock;
	int		MaxId;
	Semaphore	StructLock;
	Semaphore	PipeGuardian;
	List		Jobs;
	Stream		*Socket_ctos;
	Stream		*Socket_stoc;
	Stream		*Pipe_ctos;
	Stream		*Pipe_stoc;
} RmServerStruct;
typedef RmServerStruct	*RmServer;

/**
*** The RmJob data structure is used for communication between the various
*** clients and the various servers. These structures are held in a linked
*** list inside the RmServer structure. Every Job has its own id,
*** used by the pipe guardian to figure out where a reply should go.
*** The semaphore is used by RmLockRead() to wait for a reply.
**/
typedef	struct	RmJobStruct {
	Node		Node;
	RmServer	Server;
	int		Id;
	Semaphore	Wait;
	bool		WriteLocked;
	bool		ReadLocked;
	bool		KeepLocked;
} RmJobStruct;

typedef RmJobStruct	*RmJob;

/**
*** This structure is used to transfer details of obtained processors
*** around. It incorporates the Uid and capability, which is always
*** enough to uniquely identify obtained processors.
**/
typedef struct ProcessorDetails {
	int		Uid;
	Capability	Cap;
} ProcessorDetails;

/**
*** This is a similar structure for tasks and taskforces. The name refers
*** either to a top-level task, in which case the Uid is RmL_NoUid, or
*** to a taskforce. When examining a component task the Uid will be set
*** appropriately.
**/
typedef struct TaskDetails {
	char		Name[NameMax];
	int		Uid;
	Capability	Cap;
} TaskDetails;
	
typedef struct TaskUpdate {
	int		Errno;
	int		StructType;
	int		ReturnCode;
} TaskUpdate;

/**
*** This structure is used for link information
**/
typedef struct LinkDetails {
	int		Uid;
	Capability	Cap;
	int		Link;
	int		Mode;
} LinkDetails;

/**
*** This is used for testing and making connections, i.e. reconfiguring the
*** network.
**/
typedef struct LinkConnection {
	int		SourceUid;
	Capability	SourceCap;
	int		SourceLink;
	int		DestUid;
	Capability	DestCap;
	int		DestLink;
} LinkConnection;

/**
*** This is used with the native network calls to update various odds
*** and ends.
**/
typedef struct	ProcessorUpdate {
	int		Uid;
	int		State;
	int		Purpose;
} ProcessorUpdate;

/**
*** This is used for the RmR_Dup2 function
**/
typedef struct Dup2Details {
	int		FileDescriptor;
	char		Name[IOCDataMax];
} Dup2Details;

/**
*** RmRequest and RmReply are the main structures transmitted between
*** the Resource Management library and the various servers.
**/
typedef struct RmRequest {
	int		FnRc;	/* Request code */
	int		Uid;	/* Processor identifier */
	Capability	Cap;	/* For Processor */
	int		Arg1;
	int		Arg2;
	int		Arg3;
	int		Arg4;
	int		Arg5;
	RmNetwork	Network;
	RmTaskforce	Taskforce;
	RmProcessor	Processor;
	RmTask		Task;
	RmFilter	Filter;
	int		VariableSize;
	BYTE		*VariableData;
} RmRequest;

typedef struct	RmReply {
	int		FnRc;
	int		Reply1;
	int		Reply2;
	int		Reply3;
	int		Reply4;
	int		Reply5;
	RmNetwork	Network;
	RmTaskforce	Taskforce;
	RmProcessor	Processor;
	RmTask		Task;
	RmFilter	Filter;
	int		VariableSize;
	BYTE		*VariableData;
} RmReply;

/**
*** Undocumented functions
**/
extern RmUidTableEntry	*RmFindTableEntry(RmSet, RmUid);
extern RmObject		RmFindUid(RmSet, RmUid);
#define RmFindProcessor(a, b) (RmProcessor) RmFindUid((RmSet) a, b)
#define RmFindTask(a, b) (RmTask) RmFindUid((RmSet) a, b)
extern bool		RmNextFreeUid(RmSet, RmObject);
extern bool		RmReleaseUid(RmObject);
extern bool		RmObtainUid(RmSet, RmObject);
extern bool		RmExtendFreeQ(RmSet);
extern RmLink		*RmFindLink(RmProcessor, int);
extern int		RmReadStream(Stream	*, RmNetwork *, RmTaskforce *);
extern int		RmWriteStream(Stream	*, RmNetwork, RmTaskforce, 
					RmFilter);
extern int		RmReadNetwork(Stream	*, RmNetwork *, bool);
extern int		RmWriteNetwork(Stream	*, RmNetwork, RmFilter);
extern int		RmReadProcessor(Stream	*, RmProcessor *, bool);
extern int		RmWriteProcessor(Stream	*, RmProcessor, RmFilter);
extern int		RmReadTaskforce(Stream	*, RmTaskforce *, bool);
extern int		RmWriteTaskforce(Stream	*, RmTaskforce, RmFilter);
extern int		RmReadTask(Stream	*, RmTask *, bool);
extern int		RmWriteTask(Stream	*, RmTask, RmFilter);
extern int		RmNewJob(RmServer *,	RmJob *);
extern void		RmFinishedJob(RmJob	Job);
extern Stream		*RmLockWrite(RmJob	Job);
extern void		RmUnlockWrite(RmJob	Job);
extern Stream		*RmLockRead(RmJob	Job);
extern Stream		*RmSwitchLockRead(RmJob	Job);
extern void		RmUnlockRead(RmJob	Job);
extern int		RmOpenServer(char *server, char *name, Capability *cap, RmServer *);
extern int		RmCloseServer(RmServer Server);
extern RmServer		RmNetworkServer;
extern RmServer		RmSessionManager;
extern RmServer		RmParent;
extern RmUid		RmGetProcessorUid(RmProcessor);
extern RmUid		RmGetTaskUid(RmTask);
extern Capability	*RmGetProcessorCapability(RmProcessor, bool real);
extern char		*RmGetObjectAttribute(RmObject Obj, char *, bool);
extern int		RmAddObjectAttribute(RmObject Obj, char *, bool);
extern int		RmRemoveObjectAttribute(RmObject, char *, bool);
extern int		RmTx(RmJob	Job, RmRequest *);
extern int		RmRx(RmJob	Job, RmReply *);
extern int		RmXch(RmServer	*Server, RmRequest *, RmReply *);
extern char 		*RmRootName;

/**
*** These tests are useful inside the Resource Management library modules.
*** They ensure that arguments passed to the library routines are sensible.
*** Unfortunately they are rather expensive, at the time of writing they
*** add 5K out of a library size of 31/26K, i.e. almost 20 percent.
**/
#ifdef in_rmlib
extern	char	*my_objname(char *);
#define objname my_objname
extern	int	FullRead(Stream *pipe, BYTE *buffer, int amount, word timeout);

#if 1
#define	CheckProcessor(p) \
  if ((p eq (RmProcessor) NULL) || (p->ObjNode.Type ne Type_Processor)) \
	return(RmErrno = RmE_NotProcessor);
  
#define	CheckNetwork(n) \
  if ((n eq (RmNetwork) NULL) || (n->DirNode.Type ne Type_Network)) \
	return(RmErrno = RmE_NotNetwork);
  
#define CheckTask(t) \
  if ((t eq (RmTask) NULL) || (t->ObjNode.Type ne Type_Task)) \
	return(RmErrno = RmE_NotTask);
  
#define CheckTaskforce(t) \
  if ((t eq (RmTaskforce) NULL) || (t->DirNode.Type ne Type_Taskforce)) \
	return(RmErrno = RmE_NotTask);
  
#define	CheckProcessorFail(p, e) \
  if ((p eq (RmProcessor) NULL) || (p->ObjNode.Type ne Type_Processor)) \
	{ RmErrno = RmE_NotProcessor; return e; }

#define	CheckNetworkFail(n, e) \
  if ((n eq (RmNetwork) NULL) || (n->DirNode.Type ne Type_Network)) \
	{ RmErrno = RmE_NotNetwork; return e; }
  
#define CheckTaskFail(t, e) \
  if ((t eq (RmTask) NULL) || (t->ObjNode.Type ne Type_Task)) \
	{ RmErrno = RmE_NotTask; return e; }
  
#define CheckTaskforceFail(t, e) \
  if ((t eq (RmTaskforce) NULL) || (t->DirNode.Type ne Type_Taskforce)) \
	{ RmErrno = RmE_NotTaskforce; return e; }

#else
#define	CheckProcessor(a)
#define CheckTask(b)
#define CheckNetwork(c)
#define CheckTaskforce(d)
#define CheckProcessorFail(e,f)
#define CheckTaskFail(g,h)
#define CheckNetworkFail(i,j)
#define CheckTaskforceFail(k,l)
#endif

#endif
	 
