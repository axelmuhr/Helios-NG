/*------------------------------------------------------------------------
--                                                                      --
--                     H E L I O S   N U C L E U S                      --
--                     ---------------------------                      --
--                                                                      --
--             Copyright (C) 1987, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- procman.c								--
--                                                                      --
--	Processor Manager						--
--                                                                      --
--	Author:  NHG 16/8/87						--
--                                                                      --
------------------------------------------------------------------------*/
/* SccsId: %W%	%G% (C) 1987, Perihelion Software Ltd. 	*/
/* $Id: procman.c,v 1.68 1993/09/01 17:53:50 bart Exp $ */

#include <helios.h>	/* standard header */

#define __in_procman 1	/* flag that we are in this module */

#define FORKIOCWORKER 1	/* Fork a new thread for each IOC request	*/

#define REALSEARCH	/* set if we are using real parallel search	*/

#define TIMEOUTS 1	/* set to 1 if timeouts are to be used		*/

#define TESTSYS	0	/* test links for flow control			*/

#define PMTRACE(x) 	/* _Trace(0xbbbb0000|(x))			*/

/*--------------------------------------------------------
-- 		     Include Files			--
--------------------------------------------------------*/

#include <stdarg.h>
#include <stddef.h>
#include <string.h>
#include <queue.h>
#include <root.h>
#include <config.h>
#include <message.h>
#include <link.h>
#include <task.h>
#include <sem.h>
#include <protect.h>
#include <codes.h>
#include <gsp.h>
#include <process.h>
#include <syslib.h>
#include <servlib.h>
#include <module.h>
#include <nonansi.h>

#ifdef __TRAN
# include <asm.h>	/* for testerr_() */
#endif
#ifdef __ARM
# include <arm.h>	/* For EnterSVC/UserMode() */
#endif

#include <signal.h>	/* for SIGALRM */

extern word CallWithModTab(word arg1, word arg2, WordFnPtr fn, word *modtab);


/*--------------------------------------------------------
--		Data Definitions 			--
--------------------------------------------------------*/

typedef struct NameEntry {
	ObjNode		ObjNode;	/* node in servlib structures	*/
	Node		HashNode;	/* link in hash table		*/
	Port		Server;		/* server port			*/
	word		*LoadData;	/* auto load data		*/
	word		Confidence;	/* in aliveness of server	*/
	word		Distance;	/* distance from this processor */
} NameEntry;

typedef struct NameDir {
	DirNode		DirNode;	/* node in servlib structures	*/
	Node		HashNode;	/* link in hash table		*/
	Port		Server;		/* server port			*/
	word		*LoadData;	/* auto load data		*/
	word		Confidence;	/* in aliveness of server	*/	
	word		Distance;	/* distance from this processor */
} NameDir;

typedef struct TaskEntry {
	ObjNode		ObjNode;	/* node in servlib strucures	*/
	Task		*Task;		/* pointer to task struct	*/
	Stream		*Code;		/* Stream struct for loaded code*/
	word		UseCount;	/* number of opens		*/
	word		AlarmTime;	/* seconds until an alarm	*/
	Port		ProgInfoPort;	/* port to send prog info to 	*/
	word		ProgInfoMask;	/* mask of info bits		*/
	word		ProgInfoSent;	/* # of proginfos sent		*/
	word		TermCode;	/* termination code		*/
	bool		Running;	/* true if task is running	*/
	word		KillState;	/* task deletion state		*/
	Port		SignalPort;	/* port for signal delivery	*/
} TaskEntry;
#define Status		ObjNode.Account	/* visible status word		*/

typedef struct SockEntry {
	ObjNode		ObjNode;	/* node in servlib structures	*/
	Node		HashNode;	/* link in hash table		*/
	Port		Server;		/* reqport of server process	*/
	word		Protocol;	/* protocol type		*/
	word		Confidence;	/* in aliveness of server	*/
	word		Distance;	/* distance from this processor */
} SockEntry;
#define	BackLog		ObjNode.Size	/* pending connection backlog	*/
#define WaitQ		ObjNode.Contents/* queue of Waiters		*/
#define Users		ObjNode.Account /* number of users		*/

#define		HashSize	31		/* should be prime	*/

static Semaphore	HashLock;		/* Hash table lock	*/

static List		HashTable[HashSize];	/* the table		*/

static NameDir		*NameTableRoot;		/* name table root	*/

static NameDir		*ThisMc;		/* name table entry for me */

static DirNode		TaskTable;		/* task table root	*/

static TaskEntry	MyTaskEntry;		/* Task table entry for ProcMan */

static Port		NameTabReqPort;		/* name table server port */

static LinkInfo	*ParentLink;			/* the link which booted us */

static RootStruct	*Root;

static SockEntry	*DotSocket;		/* socket server entry	*/

static word		SockId = 1;		/* anonymous socket nos	*/

static word		TaskErr;		/* NewTask error	*/

static word		TaskId = 1;		/* task identifier counter */

#define			SearchSize	32	/* multiple of 2 */
static word		SearchTable[SearchSize];/* id's of recent searches */
static word		STOffset = 0;		/* offset into table (mod SearchSize) */
static word		IdSeed;			/* seed for Id		*/


static struct {
	word	IOCs;		/* number of IOCs		*/
	word	Servers;	/* open task servers		*/
	word	NameEntries;	/* number of NameTable entries	*/
	word	Tasks;		/* number of tasks		*/
	word	MsgBufs;	/* number of message buffers	*/
	word	Misc;		/* miscellaneous memory		*/
	word	Workers;	/* other worker processes	*/
} DbInfo;

static Semaphore KillLock;

static Semaphore SearchLock;

static Semaphore DbgLock;

#define DB_IOC1		1
#define DB_IOC2		2
#define DB_IOC3		4
#define DB_SEARCH	8
#define DB_LINK		16
#define DB_SEARCHWK	32
#define DB_MEM		64
#define DB_TASKS	128
#define DB_INFO		0x01000000
#define DB_ARM		0x10000000

#ifdef __TRAN
static word dbmask = 0;
#else
static word dbmask = 0;
#endif

#define debugging(x) (dbmask&(x))

#ifdef __TRAN
# define returnlink_(x) (((void **)(&(x)))[-2])
#else
# if defined(__C40) || defined(__ARM)
# define returnlink_(x)	NULL
# else
void *_linkreg(void);
# define returnlink_(x) (_linkreg()) /* return pointer to callers code */
# endif
#endif

/*--------------------------------------------------------
--		Stack handling				--
--------------------------------------------------------*/

#if defined(__TRAN) && !defined(PMDEBUG)
	/* Transputer Helios does not have automatic stack extension.	*/
	/* Please do not change these sizes. BLV.			*/
# define PMSTACKSIZE		1250
# define NTSTACKSIZE		1250
# define SOCKSTACKSIZE		1250
# define ClockStackSize		1000
# define IOCStackSize		1000
# define LinkIOCStack		1000
# define NSReqStackSize		1250
# define SearchStackSize	1000
# define DbgWorkStackSize	400

#else
#if defined(STACKEXTENSION)
	/* Some processors have automatic stack extension support.	*/
	/* N.B. stack checking must be used !!!				*/
	/* The sizes are a compromise between memory efficiency and	*/
	/* excessive stack extension with the fragmentation that would	*/
	/* result. N.B. stack sizes passed to the server library must	*/
	/* allow for a servinfo structure, 536 bytes plus a jump buffer,*/
	/* which will be held on the stack. NewProcess() should handle	*/
	/* the jump buffer extra. NSReqStackSize is similarly		*/
	/* affected.							*/
# define PMSTACKSIZE		1200
# define NTSTACKSIZE		1000
# define SOCKSTACKSIZE		1200

# define IOCStackSize		750
# define LinkIOCStack		750
# define NSReqStackSize		1000
# define SearchStackSize	750
# define DbgWorkStackSize	500

#ifndef STACKCHECK
#error Stack checking must be enabled when compiling this module.
#endif

#else
	/* These sizes should be used on the transputer when debugging	*/
	/* is enabled, on any processor without automatic stack		*/
	/* extension, or when porting to a new processor.		*/
# define PMSTACKSIZE		3000
# define NTSTACKSIZE		3000
# define SOCKSTACKSIZE		3000

# define IOCStackSize		4000
# define LinkIOCStack		4000
# define NSReqStackSize		4000
# define SearchStackSize	4000
# define DbgWorkStackSize	2000
#endif
#endif

#ifdef STACKCHECK
	/* Stack checking within the Processor Manager is controlled	*/
	/* by -DSTACKCHECK on the command line, not by -ps1 or similar	*/
	/* pragmas. On the transputer the stack overflow routine is	*/
	/* part of the C library, not part of the kernel, so the	*/
	/* Processor Manager must supply its own routine.		*/
#ifdef __TRAN
extern void _Trace(...);
#    pragma -s1

static void _stack_error(Proc *p)
{
	_Trace(0xaaaaaaaa,p);
	IOdebug("ProcMan stack overflow in %s at %x",p->Name,&p);	
}
#endif

	/* Enable stack checking if -DSTACKCHECK			*/
#    pragma -s0

#else

	/* Otherwise disable stack checking.				*/
# pragma -s1

#endif

/*--------------------------------------------------------
--		External References			--
--------------------------------------------------------*/

extern int splitname(char *, char, char *);
extern word KillTask(Task *);
extern _BootLink(word id, void *image, Config *conf, word confsize);
extern word _cputime(void);
extern word _ldtimer(word pri);
extern void ErrorMsg(MCB *, word );
extern void IOputs(char *s);

/*--------------------------------------------------------
--		Forward References			--
--------------------------------------------------------*/
#if defined(PMDEBUG)
# if 0 /* defined(__TRAN) */
static void NullTest(void);
void TestNull(void);
# endif
static void *PMalloc(int size);
static void PFree(void *m);
static bool PFork(int ssize, VoidFnPtr f, int asize, ... );
# undef New
# define New(_type)	(_type *)PMalloc(sizeof(_type))
#else
# define PMalloc	Malloc
# define PFree	Free
# define PFork	Fork
#endif
static void RunInit(void);
static TaskEntry *NewTask(string name,MPtr prog,Port parent,Matrix access, bool dirlocked);
static void IOC(TaskEntry *);
static void LinkIOC(LinkInfo *);
#if 0
static void DoBootLink(MsgBuf *m, LinkInfo *link, string name);
#endif
#if FORKIOCWORKER
static void CallIOCWorker(MsgBuf *m, TaskEntry *entry);
#endif
static MsgBuf *IOCWorker(MsgBuf *m, LinkInfo *link, string clientname);
static void protect(IOCCommon *req, NameEntry *n, bool local);
static void NSRequest(MsgBuf *m, DirNode *d,DispatchInfo *info);
static bool PrivateProtocol(TaskEntry *entry, MsgBuf *m);
static void LinkIOCWorker(MsgBuf *m, LinkInfo *link, NameEntry *n);
static void LinkFault(MsgBuf *m, LinkInfo *link, word code);
static MsgBuf *Search(string name, LinkInfo *link, word id);
static void SearchWorker(MsgBuf *m, LinkInfo *link);
static word HandleException(MCB *m, TaskEntry *task);
static void GenProgInfo(TaskEntry *entry);
static void DestroyTask(TaskEntry *entry);
static void LoadServer(NameEntry *n);
static Port DefaultAct( char *name, word *data );
static void dbg(char *, ... );
static void dbgworker(string dbline);
static NameEntry *NewName(NameDir *,string,word,word,Port,Matrix,word *,bool);
static word hash(char *s);
static string makename(string,string);
static void getpath(string s, DirNode *d);
static NameEntry *AddName(string name, Port port, word Flags);
static void AddHash(NameDir *n);
static void DoSignal(word signal, TaskEntry *entry);
static word NewId(void);
static void RemName(NameEntry *n);

static void do_serverinfo(ServInfo *);

static void pm_open(ServInfo *);
static void pm_create(ServInfo *);
static void pm_delete(ServInfo *);
static bool pm_private(ServInfo *);

static DispatchInfo ProcManInfo = {
	&TaskTable,
	NullPort,
	SS_ProcMan,
	NULL,
	{ (VoidFnPtr)pm_private, PMSTACKSIZE },
	{
		{ pm_open,	PMSTACKSIZE	},
		{ pm_create,	PMSTACKSIZE	},
		{ DoLocate,	0		},
		{ DoObjInfo,	0		},
		{ do_serverinfo,0		},
		{ pm_delete,	PMSTACKSIZE },
		{ DoRename,	0		},
		{ InvalidFn,	0		},
		{ DoProtect,	0		},
		{ DoSetDate,	0		},
		{ DoRefine,	0		},
		{ NullFn,	0		},
		{ DoRevoke,	0		},
		{ InvalidFn,	0		},
		{ InvalidFn,	0		}
	}
};
 
static void nt_open(ServInfo *);
static void nt_create(ServInfo *);
static void nt_delete(ServInfo *);
static void nt_rename(ServInfo *);

static DispatchInfo NameTableInfo = {
	NULL,
	NullPort,
	SS_NameTable,
	"",
	{ NULL,	0 },
	{
		{ nt_open,	NTSTACKSIZE	},
		{ nt_create,	NTSTACKSIZE	},
		{ DoLocate,	0		},
		{ DoObjInfo,	0		},
		{ do_serverinfo,0		},
		{ nt_delete,	NTSTACKSIZE },
		{ nt_rename,	NTSTACKSIZE },
		{ DoLink,	0		},
		{ DoProtect,	0		},
		{ DoSetDate,	0		},
		{ DoRefine,	0		},
		{ NullFn,	0		},
		{ DoRevoke,	0		},
		{ InvalidFn,	0		},
		{ InvalidFn,	0		}
	}
};

static word so_private(ServInfo *);

static DispatchInfo SocketInfo = {
	NULL,
	NullPort,
	SS_ProcMan,
	"",
	{ (VoidFnPtr)so_private,SOCKSTACKSIZE},
	{
		{ InvalidFn,	0		},
		{ InvalidFn,	0		},
		{ DoLocate,	0		},
		{ DoObjInfo,	0		},
		{ do_serverinfo,0		},
		{ InvalidFn,	0		},
		{ InvalidFn,	0		},
		{ DoLink,	0		},
		{ DoProtect,	0		},
		{ DoSetDate,	0		},
		{ DoRefine,	0		},
		{ NullFn,	0		},
		{ DoRevoke,	0		},
		{ InvalidFn,	0		},
		{ InvalidFn,	0		 }
	}
};

/*--------------------------------------------------------
-- main							--
--							--
-- Manager entry point					--
--							--
--------------------------------------------------------*/

int main()
{
	int i;
	Config *config = GetConfig();
	MPtr prog = MInc_(GetSysBase(),(config->FirstProg+1)*sizeof(MPtr));
	LinkNode *Parent;
	string myname, parentname;
	int len;
	char name[NameMax];

	/* as this is set before Dispatch(), incoming requests will be */
	/* handled at this priority */
	SetPriority(HighServerPri);

	Root = GetRoot();

	InitSemaphore(&DbgLock,1);

	InitSemaphore(&KillLock,1);

	InitSemaphore(&SearchLock,1);
	
#ifdef never /* defined(PMDEBUG) */
	PFork(ClockStackSize,NullTest,0);
#endif

	/* compatability ... */
	if( config->MyName == -1 ) myname = "/00";
	else myname = (string)RTOA(config->MyName);
	if( config->ParentName == -1 ) parentname = "/IO";
	else parentname = (string)RTOA(config->ParentName);

	IdSeed = *(word *)myname;
	MyTask->TaskEntry = &MyTaskEntry;	/* set before first malloc */
						/* so checktags gets taskname */

	InitSemaphore(&HashLock,1);

	for( i = 0 ; i < HashSize ; i++ ) InitList( &HashTable[i] );

	ThisMc = (NameDir *)(NameTableInfo.Root = 
				(DirNode *)(NameTableRoot = New(NameDir)));
	DbInfo.NameEntries++;
	InitNode( (ObjNode *)&NameTableRoot->DirNode, "", Type_Directory, 
		Flags_StripName|Flags_Seekable, DefNameMatrix );
 	InitList( &NameTableRoot->DirNode.Entries );
	NameTableRoot->DirNode.Key = 0;		/* all roots have same key */
	NameTableRoot->DirNode.Parent = NULL;
	NameTableRoot->DirNode.Nentries = 0;
	NameTableRoot->Confidence = 1000;
	NameTableRoot->Distance = 0;

	/* set up parent link before we crunch mcname */
	Parent = (LinkNode *)PMalloc(sizeof(LinkNode) + IOCDataMax);
	DbInfo.Misc += sizeof(LinkNode) + IOCDataMax;
	InitNode( &Parent->ObjNode, "..", Type_Link, 0, DefNameMatrix );
	NewCap(&Parent->Cap,(ObjNode *)&NameTableRoot->DirNode,AccMask_Full);
	strcpy(Parent->Link,myname);

	ProcManInfo.ParentName = Parent->Link;

	/* create initial name tree */
	while( *myname == c_dirchar ) myname++;
	while((len=splitname(name,c_dirchar,myname))!=0)
	{
		NameDir *d = ThisMc;
		ThisMc = New(NameDir);
		DbInfo.NameEntries++;		
		InitNode( (ObjNode *)&ThisMc->DirNode, name, Type_Directory, 
			Flags_StripName|Flags_Seekable, DefNameMatrix );
		InitList( &ThisMc->DirNode.Entries );
		ThisMc->DirNode.Parent = (DirNode *)d;
		ThisMc->DirNode.Nentries = 0;
		ThisMc->Confidence = 1000;
		Insert(&d->DirNode,(ObjNode *)&ThisMc->DirNode, FALSE);
		AddHash(ThisMc);
		myname+=len;
	}

	/* Processors own node is only name node which allows	*/
	/* its access matrix to be altered. And then only by a	*/
	/* local client.					*/
	ThisMc->DirNode.Matrix = DefNameMatrix|AccMask_A;
	
	/* set up /tasks directory */

	NameTabReqPort = NameTableInfo.ReqPort = NewPort();

	InitNode( (ObjNode *)&TaskTable, "tasks", Type_Directory, Flags_StripName|Flags_Seekable, DefRootMatrix );

	InitList( &TaskTable.Entries );
	TaskTable.Nentries = 0;
	TaskTable.Parent = (DirNode *)Parent;

	(void)NewName(ThisMc,"tasks",Type_Name,Flags_StripName,
		ProcManInfo.ReqPort=NewPort(),DefNameMatrix,NULL,FALSE);

	/* build a TaskEntry for myself */
	InitNode(&MyTaskEntry.ObjNode, "ProcMan.0", Type_Task, 0, DefTaskMatrix );
	Insert(&TaskTable, &MyTaskEntry.ObjNode, FALSE );
	MyTaskEntry.Task = MyTask;
	MyTaskEntry.Running = true;
	MyTaskEntry.UseCount = 0;
	MyTaskEntry.AlarmTime = 0;
	MyTaskEntry.ProgInfoPort = NullPort;
	MyTaskEntry.ProgInfoMask = 0;
	MyTaskEntry.ProgInfoSent = 0;
	MyTaskEntry.Code = 0;
	MyTaskEntry.KillState = 0;
	
	/* install an IOC port for processor manager */
	MyTask->IOCPort = NewPort();

	/* and fork an IOC for myself */
	PFork(IOCStackSize,IOC,4,&MyTaskEntry);

	ParentLink = NULL; /* if still NULL after search we must be a ROM loaded system */

	/* start up the link IOCs */
	{
		LinkInfo **linkv = Root->Links;
		while( *linkv != NULL )
		{
			if( ((*linkv)->Flags & Link_Flags_parent) != 0 )
			{
				ParentLink = *linkv;
				AddName(parentname,ParentLink->RemoteIOCPort,Flags_CacheName);
			}
			PFork(LinkIOCStack,LinkIOC,4,*linkv++);
		}

#ifdef __ABC
		if (ParentLink == NULL)
		/* (GetRoot()->Flags & Root_Flags_ROM) */
			/* default to link 0 for any possible future IO Server connection */
			AddName(parentname,Root->Links[0]->RemoteIOCPort,Flags_CacheName);
#endif
	}

	/* start up any other servers */
	while( MWord_(prog,0) )
	{
		MPtr p = MRTOA_(prog);
		if( ModuleWord_(p,Type) == T_Program )
		{
			word e;
			MCB m;
			char name[NameMax];
			Port SyncPort = NewPort();

			/*IOdebug("Starting server: %s",p->Module.Name);*/

			ModuleName_(name,p);
			(void)NewTask(makename(name,name),p,
					SyncPort,DefTaskMatrix,FALSE);

			/* && get ack message */
			InitMCB(&m,0,SyncPort,NullPort,0);

			while((e = GetMsg(&m)) != 0x456 );
			FreePort(SyncPort);

			/*IOdebug("Launched server: %s",p->Module.Name);*/
		}
		prog = MInc_(prog,sizeof(MPtr));
	}

#if 0
	/* BLV - is this causing problems ? */
	/* return acknowledge message to booter				*/
	/* except if it is an IO proc, or if we are a ROM booted system */
	/* The code returned looks like a link state change message, so	*/
	/* it will be sent to the network server by the booter.		*/
	if ( ParentLink != NULL && !(ParentLink->Flags & Link_Flags_ioproc))
	{
		word e;
		MCB m;

		InitMCB(&m,MsgHdr_Flags_preserve,
			ParentLink->RemoteIOCPort,NullPort,
				EC_Error|SS_ProcMan|EG_Broken|0x123);
		e = PutMsg(&m);
	}
#endif

#if 0
# undef Type_Auto
# define Type_Auto Type_Name
#endif

#ifndef __ABC /* these are part of the system image for ABC's Helios/ARM */
	(void)NewName(ThisMc,"ram" ,Type_Auto|Type_Directory,Flags_StripName,NullPort,DefNameMatrix,NULL,FALSE);
	(void)NewName(ThisMc,"null",Type_Auto|Type_Stream,Flags_StripName,NullPort,DefNameMatrix,NULL,FALSE);
#endif
	(void)NewName(ThisMc,"fifo",Type_Auto|Type_Directory,Flags_StripName,NullPort,DefNameMatrix,NULL,FALSE);
	(void)NewName(ThisMc,"pipe",Type_Auto|Type_Directory,Flags_StripName,NullPort,DefNameMatrix,NULL,FALSE);

	DotSocket = (SockEntry *)NewName(ThisMc,".socket",Type_Socket,0,NullPort,DefNameMatrix,NULL,FALSE);

	/* if we are a ROM loaded system or booted by IO Server, run init prog */
	/* BLV - Also if the rootnode flag is set */
	if( ParentLink == NULL || (ParentLink->Flags & Link_Flags_ioproc) ||
               (GetRoot()->Flags & Root_Flags_rootnode) )
	    	PFork(PMSTACKSIZE,RunInit,0);

	/*IOdebug("Procman dispatch - sys started");*/


#if TESTSYS
	{
		static void TestSys(void);
		PFork(PMSTACKSIZE,TestSys,0);
	}
#endif
	/* and finally drop into processor manager dispatcher */
	Dispatch(&ProcManInfo);
}

#if TESTSYS
static void TestSys(void)
{
	int xoff = 0;
	int stopped[4];
	RootStruct *root = GetRoot();
	int i;
	int dd = 0;
	
	for( i = 0; i < 4 ; i++ ) stopped[i] = 0;

	for(;;)
	{
		Delay(OneSec/4);
# if 0
		if( dd == 2 ) 
		{
			dd = 0;
			IOdebug("root %x fl %x links %x %x %x %x",root,root->Flags,
						root->Links[0]->Flags,
						root->Links[1]->Flags,
						root->Links[2]->Flags,
						root->Links[3]->Flags);
		}
		else dd++;
		
		if( root->Flags & Root_Flags_xoffed ) IOdebug("root xoff set");
		for( i = 0; root->Links[i] != NULL ; i++)
		{
			LinkInfo *link = root->Links[i];
			if( link->Flags & Link_Flags_stopped )
				IOdebug("link %d stopped",link->Id);
		}
# else
		if( root->Flags & Root_Flags_xoffed ) 
		{
			if( !xoff ) xoff=1,dbg("root xoff set");
		}
		elif( xoff ) xoff=0,dbg("root xoff cleared");
		
		for( i = 0; root->Links[i] != NULL ; i++)
		{
			LinkInfo *link = root->Links[i];
			if( link->Flags & Link_Flags_stopped )
			{
				if( !stopped[i] )
					stopped[i]=1,dbg("link %d stopped",i);
			}
			elif( stopped[i] ) 
				stopped[i]=0,dbg("link %d restarted",i);
		}
# endif
	}
}

#endif

/*--------------------------------------------------------
-- RunInit						--
--							--
-- Run init program					--
--							--
--------------------------------------------------------*/

static void RunInit(void)
{
	Object *o, *code;
	Stream *s;
	char name[NameMax];
	TaskEntry *e;
	
#ifdef __ABC
	/* ROM system startup support - only definite fs server available at boot */
	o = Locate(NULL,"/rom/sys/helios/lib/init");
#else
	o = Locate(NULL,"/helios/lib/init");
#endif

	if( o == NULL ) { IOdebug("Cannot find init"); return; }
	
	code = Load(NULL,o);
	
	if( code == NULL ) { IOdebug("Cannot load init %lx",Result2(o)); return; }
	
	s = Open(code,NULL,O_Execute);
	
	if( s == NULL ) { IOdebug("Cannot open init"); return; }
	
	e = NewTask(makename("init",name),(MPtr)s->Server,NullPort,
				DefTaskMatrix,FALSE);
				
	if( e == NULL ) { IOdebug("Cannot execute init"); return; }
	
	e->Code = s;
	
	Close( code );
	Close( o );
}

#if 0
# if defined(PMDEBUG)
#  if defined(__TRAN)

#define TESTSIZE	1024
#define TESTVAL(x)	(0xaa0000aa|((x)<<8))
/*#define TESTVAL(x)	(0)*/

static void NullTest()
{
	int i;
	word *null = NULL;
	bool firstnull = true;
	word errors = GetRoot()->Errors;
	DbInfo.Workers++;
	
	forever
	{
#  ifdef SYSDEB
		while( GetRoot()->Errors > errors )
		{
			IOdebug("%s Error Flag Set!!",ThisMc->DirNode.Name);
			errors = GetRoot()->Errors;
		}
#  endif
		if(debugging(DB_NULLTEST))
		{

			if( firstnull )
			{
				for( i = 0 ; i < TESTSIZE ; null[i++] = TESTVAL(i) );
				firstnull = false;
			}
			else 
				TestNull();
		}
		Delay(OneSec);
	}
}

void TestNull()
{
	int i;
	word *null = NULL;

	for( i = 0 ; i < TESTSIZE ; i++ )
		if( null[i] != TESTVAL(i) )
		{
			dbg("\t\t\t\t\t\t %s %x -> %x",
				ThisMc->DirNode.Name,i,null[i]);
			null[i] = TESTVAL(i);
		}
}
#  endif
# endif
#endif

/*--------------------------------------------------------
-- NewTask						--
--							--
-- Create a task and get it going			--
--							--
--------------------------------------------------------*/

static TaskEntry *NewTask(string name,MPtr prog,Port parent,Matrix access,bool dirlocked)
{
	TaskEntry **IOCArg = Null(TaskEntry *);
	word e 		 = Err_Null;
	Task *task 	 = New(Task);
	TaskEntry *entry = New(TaskEntry);

PMTRACE(0x01);
	if ( task == Null(Task) || entry == Null(TaskEntry) ) 
	{
		e = EC_Error+SS_ProcMan+EG_NoMemory+EO_Task;
		goto Fail;
	}

	task->Program	 = prog;
	InitPool(&task->MemPool);
	task->Port	 = NewPort();
	task->Parent	 = parent;
	task->IOCPort	 = NewPort();
	task->Flags	 = (dbmask>>8)&0xff;
	task->ExceptCode = (VoidFnPtr)DefaultException;
	task->ExceptData = Null(byte);
	task->TaskEntry	 = entry;

	InitNode( &entry->ObjNode, name, Type_Task, 0, access);

	entry->Task	 = task;
	entry->Code	 = NULL;
	entry->UseCount  = 0;
	entry->AlarmTime = 0;
	entry->Running   = true;
	entry->ProgInfoPort	= NullPort;
	entry->ProgInfoMask	= 0;
	entry->ProgInfoSent	= 0;
	entry->TermCode	 = 0;
	entry->KillState = 0;
	entry->SignalPort = NullPort;

	IOCArg = (TaskEntry **)NewProcess(IOCStackSize,IOC,sizeof(Task *));

	if( IOCArg == NULL ) goto Fail;

	*IOCArg = entry;
	
	if( ( e = TaskInit(task) ) != Err_Null ) goto Fail;

	Insert( &TaskTable, &entry->ObjNode, dirlocked );

#ifdef __TRAN
	RunProcess((void *)((int)IOCArg|1)); /* oring with one sets low pri */
#else
	ExecProcess((void *)IOCArg, HighServerPri);
#endif

	DbInfo.Tasks++;

	return entry;
Fail:
#ifdef SYSDEB
IOdebug("%s NewTask failed e = %x %x %x %x",ThisMc->DirNode.Name,e,IOCArg,task,entry);
#endif

	if( IOCArg != Null(TaskEntry *) ) ZapProcess((void *)IOCArg);
	if( task != Null(Task) ) PFree((void *)task);
	if( entry != Null(TaskEntry) ) PFree((void *)entry);
	TaskErr = e;
	return Null(TaskEntry);
}

/*--------------------------------------------------------
-- IOC							--
--							--
-- I/O Controller process				--
--							--
--------------------------------------------------------*/

static void
IOC(TaskEntry * entry )
{
	Task		*task = entry->Task;
	MsgBuf		*m = NULL;
	uword		starttime = 0;
	word		alarmset = false;
	word		slosh = 0;
	word		e;

PMTRACE(0x02);

	DbInfo.IOCs++;
	
	forever
	{
		alarmset = false;
		
		if( m == NULL )	{ m = New(MsgBuf); DbInfo.MsgBufs++; }

		if( m == NULL ) { Delay(OneSec); continue; }

		
		
		m->mcb.Control 	= m->control;
		m->mcb.Data 	= m->data;
		m->mcb.MsgHdr.Dest = task->IOCPort;
	
		if( entry->AlarmTime != 0 )
		{
			m->mcb.Timeout 	= (entry->AlarmTime>100)?OneSec*100:
							entry->AlarmTime*OneSec;
			m->mcb.Timeout -= slosh*10000;
			starttime = _cputime();
			alarmset = true;
		}
		else m->mcb.Timeout = IOCTimeout;	
		e = GetMsg(&m->mcb);

		if( alarmset )
		{
			uword time = (_cputime()-starttime)+slosh;

			slosh = time % 100;
			if((entry->AlarmTime -= (time/100)) <= 0)
			{
				alarmset = false;
				entry->AlarmTime = 0;
				slosh = 0;
				DoSignal(SIGALRM,entry);
			}
		}
		
		if( e == EK_Timeout ) {
			continue;
		}

		if( m->mcb.MsgHdr.FnRc < Err_Null ) 
		{
			if ( !HandleException(&m->mcb,entry) )
			{
				PFree(m);
				DbInfo.MsgBufs--;
				DbInfo.IOCs--;
				return;
			}
		}
		else {
			/* we have a message, check it is GSP protocol		*/
			if ( (m->mcb.MsgHdr.FnRc & FC_Mask) != FC_GSP )
			{
				if( !PrivateProtocol(entry,  m ) )
				{
					m->mcb.MsgHdr.FnRc = EC_Error|SS_IOC|EG_FnCode;
					ErrorMsg(&m->mcb,0);
				}
			}
#if FORKIOCWORKER
			else
			{
				if( !PFork(IOCStackSize,CallIOCWorker,8,m,entry)) 
				{
					m->mcb.MsgHdr.FnRc = EC_Error|SS_IOC|EG_NoMemory|EO_Message;
					ErrorMsg(&m->mcb,0);
				}
				else m = NULL;
			}
#else
			else m = IOCWorker(m, NULL, entry->ObjNode.Name);
#endif
		}
	}
	
}

#if FORKIOCWORKER
static void CallIOCWorker(MsgBuf *m, TaskEntry *entry)
{
	m = IOCWorker(m, NULL, entry->ObjNode.Name);
	if( m != NULL ) { PFree(m); DbInfo.MsgBufs--; }
}
#endif

/*--------------------------------------------------------
-- IOCWorker						--
--							--
-- This procedure does the actual work of the IOC. It	--
-- is called directly from IOC() and is Forked as a	--
-- process from LinkIOC().				--
--							--
--------------------------------------------------------*/

static MsgBuf *IOCWorker(MsgBuf *m, LinkInfo *link, string clientname)
{
	MCB		*mcb = &m->mcb;
	NameEntry	*n = NULL;
	word		context, pathname, next;
	IOCCommon	*req;
	byte		*dvec = mcb->Data;
	word 		len = 0;
	byte		name[NameMax];
	word		e;
	NameDir		*d = NULL;
	word		start;
	word		retries = (mcb->MsgHdr.FnRc & FR_Mask) >> FR_Shift;

	
	req = (IOCCommon *)mcb->Control;
	context = req->Context;
	pathname = req->Name;
	start = next = req->Next;

#ifdef PMDEBUG
	if(debugging(DB_IOC1))
		dbg("%s IOC: %s %F %N %N %d",ThisMc->DirNode.Name,
			clientname,
			mcb->MsgHdr.FnRc,
			context==-1?NULL:&dvec[context],
			pathname==-1?NULL:&dvec[pathname],
			next);

# if 0
	if( debugging(DB_INFO))
	{
		dbg("ProcMan: %d IOCs %d NameEntries %d Tasks",
				DbInfo.IOCs,DbInfo.NameEntries,DbInfo.Tasks);
		dbg("         %d MsgBufs %d Workers %d Misc %d Servers",
				DbInfo.MsgBufs,DbInfo.Workers,DbInfo.Misc,DbInfo.Servers);
		dbg("Heap: size %d free %d largest %d %d%% free",
				Malloc(-3),Malloc(-1),Malloc(-2),Malloc(-4));
				
		dbmask ^= DB_INFO;
	}
# endif
#endif

Again:

	if( dvec[next] == 0 && next < pathname ) next = pathname;

	while( dvec[next] == c_dirchar ) next++;

	len = splitname( name, c_dirchar, &dvec[next] );
PMTRACE(0x03);
	/* lookup name in name table		*/

	/* if the target object appears to be the name server	*/
	/* pass the request on to the server itself.		*/

	if ( len == 0 )
	{
		/* this is a bijou kludgette to allow us to use Link.n	*/
		/* to refer to neighbours				*/

	  
		if(link!=NULL && d == NULL ) d = ThisMc;
	nsreq:
		if(context == -1 || (pathname > 0 && next >= pathname) )
		{
			int Class = req->Access.Access;
			if( link == NULL ) Class = AccMask_V;
			req->Access.Access = UpdMask(Class,ThisMc->DirNode.Matrix);
		}

		req->Next = next;
		if( !PFork(NSReqStackSize,NSRequest,12,m,d,&NameTableInfo) )
			goto memerr;
		return NULL;
	}
	elif( d == NULL )  
	{
			/* if we find . and .. here, map onto / */
		if( name[0] == '.' && (name[1] == 0 ||
		    (name[1] == '.' && name[2] == 0 ))
		   ) n = (NameEntry *)NameTableRoot; 
		else
		{
			/* look it up in the hash table */
			List *l = &HashTable[hash(name)];
			Node *node = l->Head;
			Wait(&HashLock);
			while( n=NULL,node->Next != NULL )
			{
				n = (NameEntry *)((word)node - offsetof(NameEntry,HashNode));
				if( strcmp(name,n->ObjNode.Name) == 0 ) break;
				node = node->Next;
			}
			Signal(&HashLock);
		}
	}
	else n  = (NameEntry *)Lookup(&d->DirNode, name, FALSE);

	/* The following is a really nasty kludge to	*/
	/* support the installation and removal of 	*/
	/* names in the	nameserver by servers.		*/

	if(
	    (dvec[next+len] == 0) &&			/* end of name ?	*/
	    (
	      ( (mcb->MsgHdr.FnRc == FG_Create) &&	/* create request	*/
	        (n == NULL) && 				/* does not exist	*/
	        (pathname != -1) &&			/* pathname used	*/
	        (next+len > pathname)			/* entered pathname	*/
	      ) ||
	      ( (mcb->MsgHdr.FnRc == FG_Delete) &&	/* delete request	*/
	        (pathname == -1) && 			/* no path name		*/
	        (n != NULL)				/* and it exists	*/
	      )
	    )
	  )	    
	{
		if( n != NULL ) d = (NameDir *)n, next+=len;
		elif( d == NULL ) d = ThisMc;
		goto nsreq;
	}
	elif( n == Null(NameEntry) ) 
	{
		if( d == ThisMc )
		{
			mcb->MsgHdr.FnRc = EC_Error|SS_IOC|EG_Unknown|EO_Name;
			ErrorMsg(mcb, 0);
			return m;
		}
		else
		{
			MsgBuf *mm;
			string fullname = (string)PMalloc(IOCDataMax);
			if( d == NULL ) d = NameTableRoot;
			if( fullname == NULL ) 
			{
			memerr:
				mcb->MsgHdr.FnRc = EC_Error|SS_IOC|EG_NoMemory|EO_Name;
				ErrorMsg(mcb, 0);
				return m;
			}
			else DbInfo.Misc += IOCDataMax;
			getpath(fullname,&d->DirNode);
			pathcat(fullname,name);

			/* Serialize searches here with SearchLock. Once locked*/
			/* we must look again for the required node since we  */
			/* may have been blocked by another search for this   */
			/* same name.					      */

			Wait(&SearchLock);

			n = (NameEntry *)Lookup(&d->DirNode, name, FALSE);
			
			if( n == NULL )
			{
				mm = Search( fullname, link, NewId() ); 

				if( mm != NULL )
				{
					n = AddName((string)mm->data,mm->mcb.MsgHdr.Reply,Flags_CacheName|mm->control[1]);
					PFree(mm);
					DbInfo.MsgBufs--;
				}
#ifdef PMDEBUG
				else if(debugging(DB_SEARCH))
					dbg("Search for %s failed",fullname);
#endif
			}

			Signal(&SearchLock);

			PFree(fullname);
			DbInfo.Misc -= IOCDataMax;
		}
	}

	if( n == Null(NameEntry) ) 
	{
		mcb->MsgHdr.FnRc = EC_Error|SS_IOC|EG_Unknown|EO_Name;
		ErrorMsg(mcb, 0);
		return m;
	}

#ifdef PMDEBUG
	if(debugging(DB_IOC2))
		dbg("IOC: n = %s %T",n->ObjNode.Name,n->ObjNode.Type);
#endif

	switch( n->ObjNode.Type )
	{
	case Type_Name|Type_Directory:
	case Type_Name|Type_Stream:
	case Type_Name|Type_Private:
	case Type_Name:
		if( n->ObjNode.Flags & Flags_StripName ) next += len;
		break;
#if 1
	case Type_Auto|Type_Directory:
	case Type_Auto|Type_Stream:
	case Type_Auto|Type_Private:
	case Type_Auto:
		if( n->ObjNode.Flags & Flags_StripName ) next += len;
		break;
#endif
	case Type_Directory:
		next += len;
		d = (NameDir *)n;
		goto Again;

	case Type_Socket|Type_Directory:
	case Type_Socket|Type_Stream:
	case Type_Socket|Type_Private:
	case Type_Socket:
		req->Next = next+len;
		if( !PFork(NSReqStackSize,NSRequest,12,m,n,&SocketInfo) )
			goto memerr;
		return NULL;

	default:
		mcb->MsgHdr.FnRc = EC_Error|SS_IOC|EG_Invalid|EO_Name;
		ErrorMsg(mcb, 0 );
		return m;
	}

#ifdef PMDEBUG
	if(debugging(DB_IOC2))
		dbg("IOC: n = %s port = %x",n->ObjNode.Name,n->Server);
#endif
	if( n->Server == NullPort ) 
	{
		LoadServer( n );
		if( n->Server == NullPort )
		{
			mcb->MsgHdr.FnRc = EC_Error|SS_IOC|EG_Create|EO_Server;
			ErrorMsg(mcb,0);
			return m;
		}
	}
	/* see whether we have lost confidence in this server */
	if( retries == 0 ) n->Confidence = 10;
	elif( (n->Confidence -= retries) < 0 )
	{
		RemName(n);
		mcb->MsgHdr.FnRc = (retries > 8 ) ? 
					EC_Error|SS_IOC|EG_Broken|EO_Server:
					EC_Warn|SS_IOC|EG_Broken|EO_Server;
		ErrorMsg(mcb,0);
		return m;
	}

	if(context == -1 || (pathname > 0 && next >= pathname) )
		protect(req,n,link==NULL);

	mcb->MsgHdr.Flags |= MsgHdr_Flags_preserve;
	mcb->MsgHdr.Dest = n->Server;
	mcb->Timeout = IOCTimeout;
	mcb->MsgHdr.FnRc &= ~FR_Mask;
	
	/* update current pos */

	if( n->ObjNode.Flags & Flags_ResetContext ) req->Next = 1;
	else req->Next = next;
PMTRACE(0x04);
	Wait( &n->ObjNode.Lock );	/* wait for access	*/

#ifdef PMDEBUG
	if(debugging(DB_IOC2))
		dbg("IOC: Fwd %d %d %d %x",context,pathname,next,mcb->MsgHdr.Dest);

	if(debugging(DB_IOC3)) dbg("IOC: msg = %x %x %x %F",mcb->MsgHdr);
#endif

	while( (e = PutMsg(mcb)) < Err_Null && (e & EC_Mask) == EC_Recover );

#ifdef PMDEBUG
	if(debugging(DB_IOC3)) dbg("IOC: Passed On e = %E",e);
#endif

	/* if the PutMsg failed, pass the error back to client	*/
	if( e < Err_Null )
	{
		if( mcb->MsgHdr.Reply != NullPort )
			SendException(mcb->MsgHdr.Reply,e);
		if( --n->Confidence < 0 ) 
		{
			RemName(n);
			return m;
		}
	}
	
	Signal( &n->ObjNode.Lock );	/* and release		*/
	return m;
}

/* Messages without a context, or where the context has been	*/
/* passed, must have their access rights restricted.		*/
/* The exact behaviour here depends on whether the client and	*/
/* server are either local or remote. 				*/
/* When both are local we restrict access to the V class of the */
/* processor's access matrix.					*/
/* When just the server is remote the mask is retricted to a	*/
/* single class dependent on its distance.			*/
/* When just the client is remote the mask will have been	*/
/* restricted by the previous case in the client's IOC so we	*/
/* select the appropriate row from the processors matrix.	*/
/* When both are remote it is just passing through and no action*/
/* is necessary (I don't think this case arises).		*/

static void protect(IOCCommon *req, NameEntry *n, bool local)
{
	int Class = req->Access.Access;
	if( (n->ObjNode.Flags & Flags_CacheName) == 0)
	{
		/* sending to local server */
		if( local ) Class = AccMask_V;
		Class = UpdMask(Class,ThisMc->DirNode.Matrix);
	}
	else
	{
		/* sending to another processor */
		if( local ) switch( n->Distance )
		{
		case 0:  Class = AccMask_V; break; 
		case 1:  Class = AccMask_X; break; 
		case 2:  Class = AccMask_Y; break; 
		default: Class = AccMask_Z; break; 
		}
	}
	req->Access.Access = Class;
}

/*--------------------------------------------------------
-- NsRequest						--
--							--
-- Handle a request which is directed to the name server--
-- itself.						--
--							--
--------------------------------------------------------*/

static void NSRequest(MsgBuf *m, DirNode *d,DispatchInfo *info)
{
	ServInfo servinfo;
	word fncode = m->mcb.MsgHdr.FnRc;

#ifdef PMDEBUG
	if(debugging(DB_IOC2))
		dbg("NSRequest %F directory %s",
			m->mcb.MsgHdr.FnRc,d==NULL?"<>":d->Name);
#endif
PMTRACE(0x05);
	DbInfo.Workers++;

	
	if( setjmp(servinfo.Escape) != 0 )
	{
		PFree(m);
		DbInfo.MsgBufs--;
		DbInfo.Workers--;
		UnLockTarget(&servinfo);
		return;
	}

	servinfo.m = &m->mcb;
	servinfo.Context = d==NULL?info->Root:d;
	servinfo.Target = (ObjNode *)(servinfo.Context);
	servinfo.TargetLocked = FALSE;
	servinfo.FnCode = m->mcb.MsgHdr.FnRc;
	servinfo.DispatchInfo = info;
	m->mcb.MsgHdr.FnRc = info->SubSys;

	if ( m->mcb.MsgHdr.FnRc & FC_Mask != FC_GSP )
	{
		ErrorMsg(&m->mcb,EC_Error+EG_FnCode );
		PFree(m);
		DbInfo.MsgBufs--;
		DbInfo.Workers--;
		return;
	}

	if( d == NULL )	d = GetContext( &servinfo );
	else getpath(servinfo.Pathname,d);

	if( d == Null(DirNode) ) ErrorMsg(&m->mcb,0);
	else
	{
		word fn = fncode & FG_Mask;
		WordFnPtr f;

		if( fn < FG_Open || fn > FG_LastIOCFn )
		{
			f = (WordFnPtr)info->PrivateProtocol.Fn;
			if( (f == NULL) || (!f(&servinfo)) )
			{
				m->mcb.MsgHdr.FnRc = Err_Null;
				ErrorMsg(&m->mcb,EC_Error+info->SubSys+EG_FnCode );
			}
		}
		else {
			f = (WordFnPtr)info->Fntab[(fn-FG_Open) >> FG_Shift].Fn;
			(*f)(&servinfo);
		}
	}
	
	UnLockTarget(&servinfo);
	
	PFree( m );
	DbInfo.MsgBufs--;
	DbInfo.Workers--;
}

static void getpath(string s, DirNode *d)
{
	if( d != (DirNode *)NameTableRoot )
	 { if ((d->Parent == NULL) || (d->Parent == d))
	    {
#ifdef SYSDEB
	      if (d->Parent == NULL)
	 	dbg("getpath, DirNode %s, no parent", d->Name);
	      else
		dbg("getpath, DirNode %s is its own parent", d->Name);
#endif
	      *s = '\0';
	      pathcat(s, d->Name);

		/* BLV - possibly the name should be removed at this point */
	      return;	    
	    }
	   else
  	    getpath(s,d->Parent);
	 }
	else
	 *s = '\0';
	pathcat(s,d->Name);
}

/*--------------------------------------------------------
-- PrivateProtocol					--
--							--
-- Handle any private protocol messages between IOC and	--
-- task.						--
--							--
--------------------------------------------------------*/

static bool PrivateProtocol(TaskEntry *entry, MsgBuf *m)
{
	switch(m->mcb.MsgHdr.FnRc & FG_Mask )
	{
	/* the task is asking for the current name of the machine */
	case FG_MachineName:
	{
		NameDir *n = (NameDir *)(NameTableRoot->DirNode.Entries.Head);
		char *name = m->data;

		InitMCB(&m->mcb,0,m->mcb.MsgHdr.Reply,NullPort,0);

		*name = '\0';

		while( n->DirNode.Type & Type_Directory )
		{
			strcat(name,"/");
			strcat(name,n->DirNode.Name);	
			n = (NameDir *)(n->DirNode.Entries.Head);
		}

		m->mcb.MsgHdr.DataSize = strlen(name)+1;
		PutMsg(&m->mcb);

		return true;
	}
	
	/* the task is asking for an alarm signal in the given number of secs */
	case FG_Alarm:
	{
		word oldalarm = entry->AlarmTime;
		
		entry->AlarmTime = m->control[0];

		InitMCB(&m->mcb,0,m->mcb.MsgHdr.Reply,NullPort,0);

		MarshalWord(&m->mcb,oldalarm);

		PutMsg(&m->mcb);
		
		return true;
	}
	
	case FG_SetSignalPort:
	{
		Port old = entry->SignalPort;

#ifndef __TRAN	/* PAB's fix to change signal timing problem */
		entry->SignalPort = m->control[0];
#else
		entry->SignalPort = m->mcb.MsgHdr.Reply;
#endif
#ifdef PMDEBUG
	if(debugging(DB_TASKS)) dbg("%s: FG_SetSignalPort %x -> %x",entry->ObjNode.Name,old,entry->SignalPort);
#endif

		InitMCB(&m->mcb,0,m->mcb.MsgHdr.Reply,old,0);

		PutMsg(&m->mcb);
		return true;
	}

	default:
		return false;
	}
}

/*--------------------------------------------------------
-- LinkIOC						--
--							--
-- Process attached to a link which acts as agent for	--
-- remote processes.					--
--							--
--------------------------------------------------------*/

static void LinkIOC(LinkInfo *link)
{
	MsgBuf *	m;
	byte		name[NameMax];
	byte *		dbline = (byte *)PMalloc(128); /* just starting must work */
	int		dbpos = 0 ;
	NameEntry *	n;
#ifdef SYSDEB
	RootStruct *	root = GetRoot();
#endif
	word		e;
	
	DbInfo.IOCs++;
	DbInfo.Misc += 128;
	
	strcpy(name,"link.");
	if(link->Id==0) strcat(name,"0"); /* addint will not handle 0 */
	else addint(name,link->Id);
	
	n = NewName(ThisMc,name,Type_Name,Flags_StripName|Flags_LinkName,
		link->RemoteIOCPort,DefNameMatrix,NULL,FALSE);

	forever
	{
		m = New(MsgBuf);
		
		if( m == NULL ) { Delay(OneSec); continue; }
		
		DbInfo.MsgBufs++;
Label1:
		m->mcb.MsgHdr.Dest 	= link->LocalIOCPort;
		m->mcb.Timeout		= IOCTimeout;
		m->mcb.Control		= m->control;
		m->mcb.Data		= m->data;
		while( ( e = GetMsg(&m->mcb) ) == EK_Timeout );

		if( e == 0x22222222 )
		{
			int i;
			for( i = 0; i < m->mcb.MsgHdr.DataSize; i++ )
			{
				byte c = m->data[i];
				dbline[dbpos++] = c;
				if( c == '\n' || dbpos >= 127 )
				{
					dbline[dbpos] = '\0';
					PFork(DbgWorkStackSize ,dbgworker, 4, dbline);
					while( (dbline = (char *)PMalloc(128)) == NULL ) Delay(OneSec);
					DbInfo.Misc += 128;
					dbpos = 0;
				}
			}

			goto Label1;
		}
#ifdef PMDEBUG		
		if(debugging(DB_LINK))
			dbg("Link %d message %x %x %x %F",link->Id,m->mcb.MsgHdr);
#endif
		
		if( e < 0 ) 
		{
			if( (e&EG_Mask) == EG_Broken ) goto linkworker;
			else goto Label1;
		}
		else switch( e & FG_Mask )
		{
		case FG_Search:
			e = PFork(SearchStackSize,SearchWorker,8,m,link);
			if( e==0 )
			{
			forkfail:
#ifdef PMDEBUG
IOdebug("%s forkfail %d %M",ThisMc->DirNode.Name,e,&m->mcb);
#endif
				InitMCB(&m->mcb,0,m->mcb.MsgHdr.Reply,
					NullPort,EC_Error|SS_ProcMan|EG_NoMemory);
				PutMsg(&m->mcb);
				goto Label1;
			}
			break;

#if 0			
		case FG_BootLink:
			DoBootLink(m,link,name);
			break;
#endif
					
#ifdef SYSDEB
		case FG_FollowTrail:
		{
			PortInfo pi;
			Port port = m->mcb.Control[0];
			word e = GetPortInfo(port,&pi);
			Task *owner = (Task *)(pi.Owner);
			Port surr = pi.Surrogate;
			
			if( e != Err_Null ) 
			{
				dbg("%s Trail %x Invalid Port: %E",ThisMc->DirNode.Name,port,e);
			}
			else switch( pi.Type )
			{
			case T_Free:
				dbg("%s Trail %x Free !!",ThisMc->DirNode.Name,port);
				break;

			case T_Local:
				dbg("%s Trail %x Local Owner %s",ThisMc->DirNode.Name,port,
					owner==NULL?"Kernel":owner->TaskEntry->ObjNode.Name);
				break;
				
			default:
			case T_Surrogate:
			case T_Trail:
			case T_Permanent:
				dbg("%s Trail %x %s Owner %s link %d -> %x",ThisMc->DirNode.Name,port,
					(pi.Type!=T_Trail)?"Surrogate":"Trail",
					(owner==NULL)?"Kernel":owner->TaskEntry->ObjNode.Name,
					pi.Link,surr);
				InitMCB(&m->mcb,MsgHdr_Flags_preserve,
					root->Links[pi.Link]->RemoteIOCPort,
					NullPort,FG_FollowTrail);
				MarshalWord(&m->mcb,surr);
				PutMsg(&m->mcb);
			}
			goto Label1;	/* re-use mcb */
		}
#endif
		default:
		linkworker:
			e = PFork(IOCStackSize,LinkIOCWorker,12,m,link,n);
			if( e==0 ) goto forkfail;
			break;
		}
	}
}

static void LinkIOCWorker(MsgBuf *m, LinkInfo *link, NameEntry *n)
{
	word e = m->mcb.MsgHdr.FnRc;
PMTRACE(0x06);
	DbInfo.Workers++;
	
	if( e < 0 )
	{
#ifdef PMDEBUG
		if( (e & EG_Mask) == EG_Broken )
		{
			/*IOdebug("Link %d has changed state: code %E conf %x",link->Id,e,*(word *)link);*/
			LinkFault(m,link,e);
		}
		else	IOdebug("Link %d IOC got error %E",link->Id,e);

#else
		if( (e & EG_Mask) == EG_Broken ) LinkFault(m,link,e);
#endif
	}
	else m = IOCWorker(m,link,n->ObjNode.Name);

	if( m != NULL ) { PFree(m); DbInfo.MsgBufs--; }
	DbInfo.Workers--;
}

static void dbgworker(string dbline)
{
	RootStruct *root = GetRoot();

#ifdef __ARM
	EnterSVCMode();	/* as we are writing to root struct */
#endif
	DbInfo.Workers++;
	Wait(&root->IODebugLock);
	IOputs(dbline);
	Signal(&root->IODebugLock);	
	PFree(dbline); DbInfo.Misc -= 128;
	DbInfo.Workers--;
#ifdef __ARM
	EnterUserMode();	/* as we are writing to root struct */
#endif
}

#if 0
static void DoBootLink(MsgBuf *m, LinkInfo *link, string name)
{
	word e = Err_Null;
	MCB mcb;

	e = _BootLink(link->Id,NULL,(Config *)&m->data,m->mcb.MsgHdr.DataSize);

	if( e >= Err_Null )
	{
		InitMCB(&mcb,0,link->LocalIOCPort,NullPort,0);
		mcb.Data = m->data;
		mcb.Control = m->control;
		while( (e = GetMsg(&mcb)) != 0x123 );
		e = Err_Null;
	}

	name = name;
	m->mcb.MsgHdr.FnRc = e;
	ErrorMsg(&m->mcb,0);
	
	PFree(m);
	DbInfo.MsgBufs--;
}
#endif

static void LinkFault(MsgBuf *m, LinkInfo *link, word code)
{
	char mcname[100];

	getpath(mcname,&ThisMc->DirNode);

	InitMCB(&m->mcb,MsgHdr_Flags_preserve,NullPort,NullPort,
		FC_GSP|SS_NetServ|FG_NetStatus);
	
	/* fake IOCCommon */	
	MarshalWord(&m->mcb,-1);
	MarshalString(&m->mcb,"/ns");
	MarshalWord(&m->mcb,1);
	MarshalWord(&m->mcb,-1);
	MarshalWord(&m->mcb,-1);
	
	/* message parameters... */
	
	MarshalString(&m->mcb,mcname);		/* subnet/terminal name	*/
	MarshalWord(&m->mcb,link->Id);		/* link number		*/
	MarshalWord(&m->mcb,link->Mode);	/* link mode		*/
	MarshalWord(&m->mcb,link->State);	/* link state		*/
	
#if 0
	/* in case any of our cached names point through the dead link	*/
	/* remove them all here.					*/
	/* BLV - too likely to cause a deadlock				*/	
	RemName((NameEntry *)NameTableRoot);	
#endif

	SendIOC(&m->mcb);		/* send to ns but ignore reply */ 
	
	code=code;
}

/*--------------------------------------------------------
-- Search						--
--							--
-- Distributed search code.				--
--							--
--------------------------------------------------------*/

static MsgBuf *Search(string name, LinkInfo *srclink, word id)
{
	MsgBuf *m = Null(MsgBuf);
	Port reply;
	word e = -1;
	word i;
	word nlinks = 0;
	word maxlinks = GetConfig()->NLinks;
	word timeouts = 3;
#ifdef __TRAN
	bool used[4];	/* We know Tranny only has 4 links */
#else
	bool *used = (bool *)PMalloc(sizeof(bool)*((int)maxlinks));
#endif
#ifdef PMDEBUG
	if(debugging(DB_SEARCH))
		dbg("Search(%s,link.%d,%x)",name,srclink==NULL?-1:srclink->Id,id);
#endif
PMTRACE(0x07);
#ifndef __TRAN
	if( used == NULL ) return NULL;
#endif

	m = New(MsgBuf);

	if( m == Null(MsgBuf) ) 
	{
#ifndef __TRAN
		PFree(used);
#endif
		return NULL;
	}

	DbInfo.MsgBufs++;
	
	m->mcb.Control = m->control;
	m->mcb.Data    = m->data;

	reply = NewPort();

	/* fire off a search request to each active link */
	for( i = 0; i < maxlinks ; i++ )
	{
		LinkInfo *link = Root->Links[i];

		used[i] = FALSE;
		
		if( link==srclink ) continue;
		
		if( (link->Mode  != Link_Mode_Intelligent) ||
		    (link->State != Link_State_Running) ) continue;
			
		nlinks++;
		used[i] = TRUE;

		InitMCB(&m->mcb,MsgHdr_Flags_preserve,
			link->RemoteIOCPort,reply,FC_Private+FG_Search);

		MarshalString(&m->mcb,name);
		MarshalWord(&m->mcb,id);
		MarshalWord(&m->mcb,i);
#ifdef PMDEBUG
		if(debugging(DB_SEARCH))
			dbg("Search: link %d for %s id = %x",link->Id,m->data,id);
#endif

		if( (e = PutMsg(&m->mcb)) < Err_Null) nlinks--,used[i]=FALSE;
	}

	/* while we wait for replies we must release the lock	*/
	/* in case they loop back.				*/
	Signal(&SearchLock);

	while( nlinks )
	{
		m->mcb.MsgHdr.ContSize = 0;
		m->mcb.MsgHdr.Dest = reply;
		m->mcb.Timeout = OneSec;
		m->control[0] = 0;
		m->control[1] = 0;		/* default flags	*/
		m->control[2] = -1;

		e = GetMsg(&m->mcb);

		/* no matter what the result, ensure that the control	*/
		/* vector has 3 words in it, and control[2] is source	*/
		/* link or -1.						*/
		if( m->mcb.MsgHdr.ContSize == 0 ) MarshalWord(&m->mcb,-1);
		if( m->mcb.MsgHdr.ContSize == 1 ) MarshalWord(&m->mcb,0);
		if( m->mcb.MsgHdr.ContSize == 2 )
		{
			for( i = 0; i < maxlinks; i++ )
			{
				LinkInfo *link = Root->Links[i];
				if( used[i] && (link->Flags & Link_Flags_ioproc ) )
				{
					MarshalWord(&m->mcb,i);
					break;
				}
			}
			if( i == maxlinks ) MarshalWord(&m->mcb,-1);
		}
		
		if( e >= Err_Null ) 
		{
#ifdef PMDEBUG			
			if(debugging(DB_SEARCH))
				dbg("Search: %s found, port: %x flags %x id: %x",
					m->data,m->mcb.MsgHdr.Reply,m->control[1],id);
#endif
			break;	/* throw all other replies away */
		}
		elif( (e&SS_Mask) == SS_Kernel )
		{
			if( e == EK_Timeout )
				if( timeouts-- == 0 ) break;
			/* Re-check the links. If a link has stopped working */
			/* decrement nlinks.				     */
			for( i = 0; i < maxlinks; i++ )
			{
				LinkInfo *link = Root->Links[i];
				if( !used[i] ) continue;
				if( (link->Mode  != Link_Mode_Intelligent) ||
				    (link->State != Link_State_Running) )
				    used[i] = FALSE, nlinks--;
			}
		}
		elif( (e&(EC_Mask|EG_Mask|EO_Mask)) == EC_Error|EG_Unknown|EO_Server )
		{
			/* Not found reply.				*/
			int mylink = (int)(m->control[2]);
			if( mylink >= 0 ) used[mylink] = FALSE,nlinks--;
		}
	}

	Wait(&SearchLock);

	FreePort(reply);
#ifndef __TRAN
	PFree(used);
#endif
	if( e >= 0 ) return m;

	PFree(m);
	DbInfo.MsgBufs--;
	
	return NULL;
}

/*--------------------------------------------------------
-- SearchWorker						--
--							--
-- Distributed search code. run from Link IOC.		--
--							--
--------------------------------------------------------*/

static void SearchWorker(MsgBuf *m, LinkInfo *link)
{
	word i, id;
	char *sname = m->data;
	NameDir *d = NULL;
	NameEntry *n = NULL;
	int len;
	char name[NameMax];
	MsgBuf *mm;
	word hislink;
		
	Wait(&SearchLock);

	DbInfo.Workers++;
PMTRACE(0x08);	
#ifdef PMDEBUG
	if(debugging(DB_SEARCHWK))
		dbg("SearchWorker for %s id = %x from link %d[%d]",
			sname,m->control[1],link->Id,m->control[2]);
#endif

	hislink = m->control[2];

	/* check we have not already seen this search */
	
	id = m->control[1];
	for( i = 0; i < SearchSize ; i++ ) 
		if( SearchTable[i] == id ) goto failed;

	/* enter id into table */
	SearchTable[ (STOffset++) & (SearchSize - 1) ] = id;
	
	/* see if we have it here */
	/* first hash into name table */

	while( *sname == c_dirchar ) sname++;

	len = splitname(name,c_dirchar,sname);

	if( len != 0 )
	{
		List *l = &HashTable[hash(name)];
		Node *node = l->Head;
		Wait(&HashLock);
		while( n=NULL,node->Next != NULL )
		{
			n = (NameEntry *)((word)node - offsetof(NameEntry,HashNode));
			if( strcmp(name,n->ObjNode.Name) == 0 ) break;
			node = node->Next;
		}
		Signal(&HashLock);
	}

	/* if the initial hash fails to find anything, search for it.	*/
	/* if it is server name we have found something, otherwise we	*/
	/* need to search down the name tree from here.			*/
	if( n == NULL ) goto search;
	else {
		if( (n->ObjNode.Type & ~Type_Flags) == Type_Name ) goto found;
		else d = (NameDir *)n, sname += len;
	}

	/* now run down directories */
	while((len=splitname(name,c_dirchar,sname))!=0)
	{
		n = (NameEntry *)Lookup(&d->DirNode,name,FALSE);
		if( n == NULL ) goto search;
		sname += len;
		d = (NameDir *)n;
	}
	goto found;

search:
	/* pass request on to neighbours */
	
	mm = Search( m->data, link, id );
#ifdef PMDEBUG
	if(mm == NULL && debugging(DB_SEARCH))
		dbg("Search in SearchWork for %s failed",m->data);
#endif

	if( mm == NULL ) goto failed;

#ifdef PMDEBUG	
	if(debugging(DB_SEARCHWK))
		dbg("Found in search %s port: %x flags: %x",mm->data,mm->mcb.MsgHdr.Reply,mm->control[1]);
#endif

	mm->mcb.MsgHdr.Dest = m->mcb.MsgHdr.Reply;
	mm->control[2] = hislink;
	PFree(m); DbInfo.MsgBufs--;
	m = mm;
	goto done;
	
found:
	/* We have found an entry for this name, but only respond if it	*/
	/* is one of ours, otherwise pass search on.			*/
	if( (n->ObjNode.Flags & Flags_CacheName) != 0 )
	{
#ifdef PMDEBUG		
		if(debugging(DB_SEARCHWK))		
			dbg("Found CacheName %s, searching",n->ObjNode.Name);
#endif
		goto search;
	}
	/* found it locally , pass back link IOC port. */
	InitMCB(&m->mcb,
#if 1
		0,
#else
		/* This is a bit of a kludge to stop undelivered messages */
		/* destroying their reply trail. 			  */
		MsgHdr_Flags_exception,
#endif
		m->mcb.MsgHdr.Reply,
		(((n->ObjNode.Type&~Type_Flags)==Type_Socket) && (n != (NameEntry *)DotSocket)) 
			? n->Server : link->LocalIOCPort,
		0);

	MarshalOffset(&m->mcb);
	getpath(m->data,(DirNode *)&n->ObjNode);
	MarshalWord(&m->mcb,Flags_ResetContext);
	MarshalWord(&m->mcb,hislink);

	m->mcb.MsgHdr.DataSize = strlen(m->data)+1;

#ifdef PMDEBUG	
	if(debugging(DB_SEARCHWK))
		dbg("Found locally %s port = %x",m->data,m->mcb.MsgHdr.Reply);
#endif
	goto done;

failed:
#ifdef PMDEBUG
	if(debugging(DB_SEARCHWK)) dbg("Search for %s failed",m->data);
#endif
	InitMCB(&m->mcb,0,m->mcb.MsgHdr.Reply,NullPort,
		EC_Error|SS_ProcMan|EG_Unknown|EO_Server);
	MarshalWord(&m->mcb,-1);
	MarshalWord(&m->mcb,0);
	MarshalWord(&m->mcb,hislink);

done:
	PutMsg(&m->mcb);
	PFree(m); DbInfo.MsgBufs--;
	DbInfo.Workers--;
	Signal(&SearchLock);
}

/*--------------------------------------------------------
-- HandleException					--
--							--
-- Handle any exception/error messages sent to the IOC	--
-- by its Task.						--
-- Returns true if the IOC is to continue and false if	--
-- it is to terminate.					--
--							--
--------------------------------------------------------*/

static word HandleException(MCB *m, TaskEntry *entry)
{
	Task *task = entry->Task;
	word e = m->MsgHdr.FnRc;
	word code = (m->Control)[0];

#ifdef PMDEBUG
	if(debugging(DB_TASKS))
		dbg("ProcMan: %s Exception e %E code %x",entry->ObjNode.Name,e,code);
#endif
	if( (e & (EC_Mask|EG_Mask)) != (EC_Error|EG_Exception) )
		return true;

PMTRACE(0x09);
	switch( e & EE_Mask )
	{
	default:
		return true;

	case EE_Kill:
#ifdef PMDEBUG	
		if(debugging(DB_TASKS))
			dbg("Kill %s parent %x",entry->ObjNode.Name,
						task->Parent);
#endif
		Wait(&entry->ObjNode.Lock);
		entry->Status += 1000;

		Wait(&KillLock);
		e = KillTask(task);
		Signal(&KillLock);

		entry->Status += 1000;

		entry->Running = false;

		entry->TermCode = code;

		GenProgInfo(entry);

		entry->Status += 1000;

		Close(entry->Code);
		entry->Code = NULL;

		if( entry->UseCount == 0 ) DestroyTask(entry);
		else Signal(&entry->ObjNode.Lock);

		return false;
	}
}

static void GenProgInfo(TaskEntry *entry)
{		
	MCB m;
	word e = 0;
#ifdef __TRAN
	Task *task = entry->Task;
#endif
	
#ifdef PMDEBUG			
		if(debugging(DB_TASKS))
			dbg("Generating %s proginfo %x",
				entry->ObjNode.Name,entry->TermCode);
#endif

	if( (entry->ProgInfoMask != 0) && 
		(entry->ProgInfoPort != NullPort) )
	{
		word f = MsgHdr_Flags_preserve;
		if( entry->ProgInfoSent ) f |= MsgHdr_Flags_sacrifice;

		InitMCB(&m,(byte)f,entry->ProgInfoPort,NullPort,entry->TermCode);
			
		entry->Status += 100;
		e = PutMsg(&m);
		entry->Status += 100;
		entry->ProgInfoSent++;
	}

#ifdef __TRAN /* required for Charlies netserver, should not be needed for Barts */
# if 1
	/* the following will vanish eventually */
	elif( task->Parent != NullPort )
	{
		InitMCB(&m,MsgHdr_Flags_preserve,
			task->Parent,NullPort,entry->TermCode);
		entry->Status += 100;
		e = PutMsg(&m);
	}
# endif
#endif
#ifdef PMDEBUG
		if(e != 0 && debugging(DB_TASKS))
			dbg("%s ProgInfo PutMsg e = %x",entry->ObjNode.Name,e);
#endif
}

/*--------------------------------------------------------
-- DestroyTask						--
--							--
-- Delete the task data structures. This will only	--
-- happen if the task has exited and there are no open	--
-- streams to it.					--
--							--
--------------------------------------------------------*/

static void DestroyTask(TaskEntry *entry)
{
	Task *task = entry->Task;
	char  name[NameMax];

	if( entry->Running || entry->UseCount > 0 ) return;
PMTRACE(0x0a);
#ifdef PMDEBUG
	if(debugging(DB_TASKS)) dbg("Destroy Task %s",entry->ObjNode.Name);
#endif

	/* To avoid deadlocks we must release the lock on the task here	*/
	/* get the root lock, and then re-acquire the task lock. By	*/
	/* getting the root lock we can be sure that no-one else is	*/
	/* waiting for this task. Note that we must re-test the conditions*/
	/* after getting the locks to ensure that nothing has changed.	*/
 	strcpy(name, entry->ObjNode.Name);
	Signal(&entry->ObjNode.Lock);
	Wait(&TaskTable.Lock);
	/* BLV - check that the task has not disappeared in the meantime. */
	entry = (TaskEntry *) Lookup(&TaskTable, name,TRUE);
	if (entry == NULL)
	 { Signal(&TaskTable.Lock);
	   return;
	 }
	Wait(&entry->ObjNode.Lock);
	
	if( entry->Running || entry->UseCount > 0 ) 
	{
		Signal(&entry->ObjNode.Lock);
		Signal(&TaskTable.Lock);
		return;
	}
	
	FreePort(task->Port);

	FreePort(task->IOCPort);

	FreePort(task->Parent);

	FreePool(&task->MemPool);

	Unlink(&entry->ObjNode, TRUE);

	if( entry->Code != NULL ) Close(entry->Code);

	PFree(task);

	FreePort(entry->ProgInfoPort);

	PFree(entry);

	DbInfo.Tasks--;
	
	Signal(&TaskTable.Lock);
}


#if 0
/*--------------------------------------------------------
-- AsyncException					--
--							--
-- Call exception code asyncronously. 			--
-- Used if there is no standard signal port to send	--
-- signal to.						--
--							--
-- This should be called via CallWithModTab() so that	--
-- the signal stack that is used comes from the target	--
-- tasks own memory.					--
--							--
--------------------------------------------------------*/

static void AsyncException(Task *task, word signal)
{
	PFork(PMSTACKSIZE, CallException, 8, task, signal);
}
#endif

/*--------------------------------------------------------
-- DoSignal						--
--							--
-- Send a signal to the task.				--
--							--
--------------------------------------------------------*/

static void DoSignal(word signal, TaskEntry *entry)
{
	if( entry->SignalPort != NullPort )
	{
		/* deliver the signal via a message to the task's signal port */
		SendException(entry->SignalPort,EC_Recover|SS_ProcMan|EG_Exception|EE_Signal|signal);
	}
	else
	{
#if 0
	/* Exception code is now only called in a syncronous manner */
	/* There should be no need to fork this thread. */

		/*
		 * Signal port has not been set yet, so call its exception
		 * code. As syncronous signals also use CallException, we
		 * have to fork off a thread here (it used to be done in
		 * individual language runtime's). The CallWithModTab() is to
		 * force the signal stack to be allocated from the receiving
		 * tasks memory pool.
		 */

		CallWithModTab(	(word)entry->Task, signal,
			(WordFnPtr) AsyncException,
			entry->Task->ModTab);
#else
		/* Call low level exception handler, This will either call */
		/* Exit, or when CLib is loaded, call raise() (not on TRAN). */
		/* It is used to support hardware exceptions. */
		Task *task = entry->Task;
		CallException(task,signal);
#endif
	}
}


/*--------------------------------------------------------
-- LoadServer						--
--							--
-- The given server is not loaded, do so.		--
-- This a achieved by causing the autoload code to be   --
-- executed.						--
--							--
--------------------------------------------------------*/

static void LoadServer(NameEntry *n)
{
PMTRACE(0x0b);
	n->Server = DefaultAct( n->ObjNode.Name, n->LoadData );
	/* change type to Type_Name */
	n->ObjNode.Type = (n->ObjNode.Type & Type_Flags) | Type_Name;
	
}

static Port DefaultAct( char *name, word *data )
{
	Object *libdir= NULL;
	Object *servercode = NULL;
	Object *server = NULL;
	Stream *s = NULL;
	Port result = NullPort;
	MPtr p;
	char tname[NameMax];
	TaskEntry *t;

	data = data;		/* keep compiler happy */

	libdir = Locate(NULL,"/helios/lib");

	if( libdir == NULL ) goto done;

	servercode = Locate(libdir,name);

	if( servercode == NULL ) goto done;

	server = Load(NULL,servercode);

	if( server == NULL ) goto done;

	s = Open(server,NULL,O_Execute);

	p = (MPtr)(s->Server);

	ModuleName_(tname,p);
	
	t = NewTask(makename(tname,tname),p,
		NullPort,DefTaskMatrix,FALSE);

	if( t == NULL ) { Close(s); goto done; }

	t->Code = s;

	result = t->Task->Port;

done:
	Close(libdir);
	Close(servercode);
	Close(server);

	return result;
}

/*--------------------------------------------------------
-- Action procedures					--
--							--
-- Process individual packet actions			--
-- These are common to both name server and processor	--
-- manager.						--
--							--
--------------------------------------------------------*/

static void do_serverinfo(ServInfo *servinfo)
{
	RootStruct *root = GetRoot();
	MCB *m = servinfo->m;
	int i;
	int NLinks = (int)(GetConfig()->NLinks);
	int mctype = (int)MachineType();
	char *name = (char *)PMalloc(IOCDataMax);
	
	InitMCB(m,0,m->MsgHdr.Reply,NullPort,Err_Null);

	if( name == NULL )
	{
		m->MsgHdr.FnRc = EC_Error|SS_IOC|EG_NoMemory|EO_Name;
		ErrorMsg(m, 0);
		return;
	}
	
	getpath(name,&ThisMc->DirNode);
	
	/* build a ProcStats structure in the data vector */
	
	MarshalData(m,4,(byte *)&mctype);
	MarshalData(m,4,(byte *)&root->LoadAverage);
	MarshalData(m,4,(byte *)&root->Latency);
	MarshalData(m,4,(byte *)&root->MaxLatency);
	MarshalData(m,4,(byte *)&root->FreePool->Max);
	MarshalData(m,4,(byte *)&root->FreePool->Size);
	MarshalData(m,4,(byte *)&root->LocalMsgs);
	MarshalData(m,4,(byte *)&root->BufferedMsgs);	
	MarshalData(m,4,(byte *)&root->Errors);
	MarshalData(m,4,(byte *)&root->EventCount);		
	MarshalData(m,4,(byte *)&NLinks);		
	
	for( i = 0; root->Links[i] != NULL ; i++ )
	{
		LinkInfo *link = root->Links[i];
		MarshalData(m,4,(byte *)link);
		MarshalData(m,4,(byte *)&link->MsgsIn);
		MarshalData(m,4,(byte *)&link->MsgsOut);
		MarshalData(m,4,(byte *)&link->MsgsLost);		
	}
	
	MarshalString(m,name);
	
	PFree(name);

	PutMsg(m);
}

/*--------------------------------------------------------
-- Processor Manager Specific Actions.			--
--							--
--------------------------------------------------------*/

static void pm_open(ServInfo *servinfo)
{
	MCB *m = servinfo->m;
	MsgBuf *r;
	TaskEntry *t;
	IOCMsg2 *req = (IOCMsg2 *)(m->Control);
	Port reqport;
	char *pathname = servinfo->Pathname;
	int mode = (int)(req->Arg.Mode & O_Mask);
	Port proginfo_port	= NullPort;
	int  proginfo_count	= 0;

PMTRACE(0x0c);
#ifdef PMDEBUG
	if(debugging(DB_TASKS))
		dbg("PM Open from %x %s",m->MsgHdr.Reply,m->Data);
#endif

	t = (TaskEntry *)GetTarget(servinfo);

	if( t == Null(TaskEntry) )
	{
		ErrorMsg(m,EO_Task);
		return;
	}

	unless( CheckMask(req->Common.Access.Access,mode) ) 
	{
		ErrorMsg(m,EC_Error+EG_Protected);
		return;
	}

	r = New(MsgBuf);

	if( r == Null(MsgBuf) )
	{
		ErrorMsg(m,EC_Error+EG_NoMemory);
		return;
	}

	reqport = NewPort();
	FormOpenReply(r,m,&t->ObjNode,Flags_Closeable, pathname);
	r->mcb.MsgHdr.Reply = reqport;
	PutMsg(&r->mcb);
	PFree(r);

	if( t->ObjNode.Type == Type_Directory )
	{
		DirServer(servinfo,m,reqport);
		FreePort(reqport);
		return;
	}

	t->UseCount++;
	t->Status += 10000;
	
	UnLockTarget(servinfo);
	
	DbInfo.Servers++;
	
	forever
	{
		word e;
		m->MsgHdr.Dest = reqport;
		m->Timeout = OneSec*5;
		e = GetMsg(m);
	
		if( e == EK_Timeout )
		{
			/* BLV - if a processor crashes then a stream connection	*/
			/* to a running task may be broken. The TFM etc. will		*/
			/* automatically ReOpen() the connection and install another	*/
			/* ProgramInfo port on the new stream. However the old stream	*/
			/* must have a way of exiting. Hence the routine keeps track	*/
			/* of the last ProgInfo port installed on this stream and	*/
			/* shuts down when appropriate. Also, if the parent has		*/
			/* completely lost track of this child without reopening	*/
			/* the entry should eventually disappear (as if the processor	*/
			/* had crashed...						*/
			if( !t->Running ) {
				if ((proginfo_port == NullPort) || 
				    (proginfo_port != t->ProgInfoPort) ||
				    (++proginfo_count > 16))
					break;
				else
					GenProgInfo(t);	
			}
			continue;
		}

		if( e < Err_Null ) continue;

#ifdef PMDEBUG
		if(debugging(DB_TASKS))
			dbg("Procman Stream message %F from %x",
				m->MsgHdr.FnRc,m->MsgHdr.Reply);
#endif
PMTRACE(0x0d);
		Wait(&t->ObjNode.Lock);

		switch( m->MsgHdr.FnRc & FG_Mask )
		{
		case FG_SendEnv:
			unless( mode & O_WriteOnly ) goto badmode;

			m->MsgHdr.Dest = t->Task->Port;
			m->Timeout = IOCTimeout;
			t->Status += 1;
			PutMsg(m);
			t->Status += 1;
			break;

		case FG_Signal:
			unless( mode & O_WriteOnly ) goto badmode;
			/* BLV - do not send signals to tasks that have already died	*/
			unless(t->Running) {
				ErrorMsg(m, EC_Error + EG_Broken + EO_Program);
				break;
			}
			t->Status += 10;
			m->MsgHdr.FnRc = SS_ProcMan;
			
			ErrorMsg(m,0);

			DoSignal(m->Control[0],t);
			
#ifdef PMDEBUG
			if(debugging(DB_TASKS)) dbg("Signal of %s done",t->ObjNode.Name);
#endif
			t->Status += 10;

			break;

		case FG_ProgramInfo:
			unless( mode & O_ReadOnly ) goto badmode;
		{
			Port port = m->MsgHdr.Reply;
			word mask = m->Control[0];
			mask &= PS_Terminate;

			FreePort(t->ProgInfoPort);

			t->Status += 200;			
			t->ProgInfoPort = (mask==0)?NullPort:port;
			proginfo_port = t->ProgInfoPort;
			t->ProgInfoMask = mask;
			InitMCB(m,(mask==0)?0:MsgHdr_Flags_preserve,
					port,NullPort,Err_Null);
			MarshalWord(m,mask);
			MarshalWord(m,1);

			PutMsg(m);

			t->Status -= 100;
			if( !t->Running ) GenProgInfo(t);
			break;
		}

			
		case FG_Close:
#ifdef PMDEBUG		
			if(debugging(DB_TASKS))
				dbg("Task %s closed",t->ObjNode.Name);
#endif
			if( m->MsgHdr.Reply != NullPort ) ErrorMsg(m,0);
			m->MsgHdr.FnRc = SS_ProcMan;
			goto done;

		default:
			ErrorMsg(m,EC_Error+EG_FnCode+EO_Task);
			break;
		badmode:
			ErrorMsg(m,EC_Error+EG_Protected+EO_Task);
			break;
		}
		Signal(&t->ObjNode.Lock);
	}

	Wait(&t->ObjNode.Lock);
done:
	DbInfo.Servers--;
	t->UseCount--;
	t->Status -= 10000;
	if( !t->Running && t->UseCount == 0 ) DestroyTask(t);
	else Signal(&t->ObjNode.Lock);
	
	FreePort(reqport);
}

static void pm_create( ServInfo * servinfo )
{
	MCB *		m = servinfo->m;
	MsgBuf * 	r;
	DirNode *	d;
	TaskEntry *	t;
	IOCCreate *	req  = (IOCCreate *)(m->Control);
	TaskInfo *	info = (TaskInfo *)&(m->Data[req->Info]);
	Object *	o;
	Stream *	s;
	char		name[ NameMax ];
	char *		pathname = servinfo->Pathname;
	
PMTRACE(0x0e);
	
#ifdef PMDEBUG
	if(debugging(DB_TASKS))
		dbg("PM Create from %x",m->MsgHdr.Reply);
#endif

	d = (DirNode *)GetTarget(servinfo);

	if( d == Null(DirNode) )
	{
		ErrorMsg(m,EO_Directory);
		return;
	}

	unless( CheckMask(req->Common.Access.Access,AccMask_W) ) 
	{
		ErrorMsg(m,EC_Error+EG_Protected+EO_Directory);
		return;
	}

	o = NewObject(RTOA(info->Name),&info->Cap);

	if( o == Null(Object) )
	{
		ErrorMsg(m,EC_Error+EG_Invalid+EO_Program);
		return;
	}

	if( ReLocate(o) < Err_Null )
	{
		m->MsgHdr.FnRc = Result2(o);
		ErrorMsg(m,0);
		return;
	}
	
	/* if the object is not a program, or is in a remote	*/
	/* loader, it must be loaded locally.			*/
	if( (o->Type != Type_Program) || (o->Flags & Flags_Remote) )
	{
		Object *o1;
		word e = 0;
		
		o1 = Load(Null(Object),o);
		e = Result2(o);
		
		if( o1 != NULL && o1->Type != Type_Program ) 
		{
			e = EC_Error|SS_ProcMan|EG_Invalid|EO_Program;
			Close(o1);
		}
			
		if( o1 == NULL || e < Err_Null )
		{
			m->MsgHdr.FnRc = e;
			ErrorMsg(m,0);
			Close(o);
			return;
		}
		Close(o);
		o = o1;
	}

	s = Open(o,NULL,O_Execute);

	if( s == Null(Stream) )
	{
		m->MsgHdr.FnRc = Result2(o);
		ErrorMsg(m,0);
		Close(o);
		return;
	}

	r = New(MsgBuf);

	if( r == Null(MsgBuf) )
	{
		ErrorMsg(m,EC_Error+EG_NoMemory);
		Close(o);Close(s);
		return;
	}
	DbInfo.MsgBufs++;
	
	t = NewTask(makename(o->Name,name),(MPtr)s->Server,
			m->MsgHdr.Reply,info->Matrix&DefTaskMatrix,TRUE);

	if( t == Null(TaskEntry) )
	{
		ErrorMsg(m,TaskErr);
		PFree(r); DbInfo.MsgBufs--;
		Close(s);Close(o);
		return;
	}

	t->Code = s;

	pathcat(pathname,name);

	/* give creator full access rights */
	req->Common.Access.Access = AccMask_Full;
	
	FormOpenReply(r,m,&t->ObjNode, 0, pathname);
	
	r->mcb.MsgHdr.Flags |= MsgHdr_Flags_preserve;

#ifdef PMDEBUG
	if(debugging(DB_TASKS))
		dbg("%s Parent = %x",t->ObjNode.Name,t->Task->Parent);
#endif
	PutMsg(&r->mcb);

	Close(o);

	PFree(r); DbInfo.MsgBufs--;
	
	return;	
}

static void pm_delete(ServInfo *servinfo)
{
	MCB *m = servinfo->m;
	TaskEntry *t;
	IOCCommon *req = (IOCCommon *)(m->Control);
PMTRACE(0x0f);
	t = (TaskEntry *)GetTarget(servinfo);

	if( t == NULL )
	{
		ErrorMsg(m,EO_Task);
		return;
	}

#ifdef PMDEBUG
	if(debugging(DB_TASKS)) dbg("pm_delete %s",t->ObjNode.Name);
#endif
	
	unless( CheckMask(req->Access.Access,AccMask_D) ) 
	{
		ErrorMsg(m,EC_Error+EG_Protected+EO_Task);
		return;
	}

	if( t->ObjNode.Type != Type_Task )
	{
		ErrorMsg(m,EC_Error+EG_WrongFn+EO_Directory);
		return;
	}

	ErrorMsg(m,Err_Null);

	/* BLV - do not send signals to tasks that have already	*/
	/* died.						*/
	unless(t->Running) return;
	
	t->KillState += (servinfo->FnCode & FF_Mask);

	switch( t->KillState++ )
	{
	case 0:
		DoSignal(SIGINT,t);
		break;
		
	case 1:
		DoSignal(SIGKILL,t);
		break;
	
	default:
		InitMCB(m,0,t->Task->IOCPort,NullPort,
			EC_Error|SS_ProcMan|EG_Exception|EE_Kill);
		MarshalWord(m,0xFF80|SIGKILL);
		PutMsg(m);
	}
}

#ifdef PMDEBUG
static bool dbprocrunning = FALSE;
static void dbinfoproc(void);
#endif

static bool pm_private(ServInfo *servinfo)
{
	switch( servinfo->FnCode & FG_Mask )
	{
	default:
		dbg("ProcMan unknown function in pm_private: %lx",servinfo->FnCode);
		return false;
		
	case FG_Debug:
		dbmask ^= servinfo->m->Control[5];
#ifdef PMDEBUG
		if( servinfo->m->Control[5] & DB_INFO )
		{
			if( dbprocrunning ) dbprocrunning = FALSE;
			else Fork(2000,dbinfoproc,0);
		}
		
#endif
		return true;
		
	case FG_SetFlags:
	{
		MCB *m = servinfo->m;
		TaskEntry *t = (TaskEntry *)GetTarget(servinfo);
		IOCCommon *req = (IOCCommon *)m->Control;
		
		if( t == Null(TaskEntry) ) { ErrorMsg(m,EO_Task); return true; }

		unless( CheckMask(req->Access.Access,AccMask_W) ) 
		{ ErrorMsg(m,EC_Error+EG_Protected); return true; }

		t->Task->Flags ^= m->Control[5];	
		break;
	}

	case FG_Reconfigure:
	{
		NameDir *d = ThisMc;
		while( (d = (NameDir *)d->DirNode.Parent) != NULL )
		{
			WalkList(&d->DirNode.Entries,(WordFnPtr)RemName);
		}
		return true;
	}

	}

	return false;
}

/*--------------------------------------------------------
-- Name Server Specific Actions.			--
--							--
--------------------------------------------------------*/

static void nt_open(ServInfo *servinfo)
{
	MCB *m = servinfo->m;
	MsgBuf *r;
	NameDir *d;
	IOCMsg2 *req = (IOCMsg2 *)(m->Control);
	Port reqport;
	char *pathname = servinfo->Pathname;

PMTRACE(0x11);

	d = (NameDir *)GetTarget(servinfo);

	if( d == NULL )
	{
		ErrorMsg(m,EO_Directory);
		return;
	}

	/* only a directory may be opened for listing		*/
	unless( d->DirNode.Type & Type_Directory )
	{
		ErrorMsg(m,EC_Error+EG_WrongFn+EO_Name);
		return;
	}

	unless( CheckMask(req->Common.Access.Access,(int)(req->Arg.Mode & O_Mask)) ) 
	{
		ErrorMsg(m,EC_Error+EG_Protected+EO_Directory);
		return;
	}

	r = New(MsgBuf);

	if( r == Null(MsgBuf) )
	{
		ErrorMsg(m,EC_Error+EG_NoMemory);
		return;
	}
	DbInfo.MsgBufs++;
	
	FormOpenReply(r,m,(ObjNode *)&d->DirNode,Flags_Closeable, pathname);

	reqport = NewPort();
	r->mcb.MsgHdr.Reply = reqport;

	PutMsg(&(r->mcb));

	PFree(r); DbInfo.MsgBufs--;

	DirServer(servinfo,m,reqport);

	FreePort(reqport);
}

static void nt_create(ServInfo *servinfo)
{
	MCB *m = servinfo->m;
	MsgBuf *r;
	NameEntry *n;
	NameDir *d;
	IOCCreate *req = (IOCCreate *)(m->Control);
	NameInfo *info = (NameInfo *)&(m->Data[req->Info]);
	char *name;
	char *pathname = servinfo->Pathname;
	word e;
PMTRACE(0x12);

	d = (NameDir *)GetTargetDir(servinfo);

	if( d == Null(NameDir) )
	{
		ErrorMsg(m,EO_Name);

		return;
	}

	unless( CheckMask(req->Common.Access.Access,AccMask_W) ) 
	{
		ErrorMsg(m,EC_Error+EG_Protected+EO_Directory);
		return;
	}

	n = (NameEntry *)GetTargetObj(servinfo);

	if( n != NULL )
	{
		ErrorMsg(m,EC_Error|EG_Create|EO_Name);

		return;
	}
	
	r = New(MsgBuf);

	if( r == Null(MsgBuf) )
	{
		ErrorMsg(m,EC_Error+EG_NoMemory);
		return;
	}
	DbInfo.MsgBufs++;
	
	name = objname(pathname);

	n = NewName(d,name,req->Type,info->Flags,info->Port,info->Matrix&DefNameMatrix,
		info->LoadData,TRUE);

	/* Only the capability returned as a result of this	*/
	/* request contains the right to delete the name.	*/
	
	req->Common.Access.Access = AccMask_D;
	
	FormOpenReply(r,m,&n->ObjNode, 0, pathname);

	e = PutMsg(&r->mcb);

	PFree(r); DbInfo.MsgBufs--;

	return;
}

static void nt_rename(ServInfo *servinfo)
{
	IOCMsg2 *req = (IOCMsg2 *)(servinfo->m->Control);
	byte *data = servinfo->m->Data;
	NameDir *d;
	char *name;
	char *toname = &data[req->Arg.ToName];
	char mcname[100];
	int mpos, tpos;
PMTRACE(0x13);

	d = (NameDir *)GetTarget(servinfo);

	if( d != ThisMc )
	{
		ErrorMsg(servinfo->m,EC_Error|EG_WrongFn|EO_Name);
		return;
	}

	if( *toname != c_dirchar ) goto badname;

	getpath(mcname,&ThisMc->DirNode);

	mpos = strlen(mcname);
	tpos = strlen(toname);

	until( mpos == 0 )
		if( mcname[--mpos] != toname[--tpos] ) goto badname;


	Wait(&HashLock);

	d = NameTableRoot;
	
	while(tpos != 0)
	{
		NameDir *n = New(NameDir);

		if( n == NULL ) 
		{
			ErrorMsg(servinfo->m,EC_Error|EG_NoMemory);
			Signal(&HashLock);
			return;
		}
		DbInfo.NameEntries++;
		
		toname[tpos] = '\0';

		while( toname[--tpos] != c_dirchar );

		name = &toname[tpos+1];

		/* add new root */
		InitNode((ObjNode *)&n->DirNode,"",Type_Directory,
			Flags_StripName|Flags_Seekable, DefNameMatrix );
		InitList(&n->DirNode.Entries);
		n->DirNode.Parent = NULL;
		n->DirNode.Nentries = 0;
		NameTableInfo.Root = (DirNode *)(NameTableRoot = n);
		n->DirNode.Key = d->DirNode.Key;
		n->Server = NullPort;
		n->LoadData = NULL;
		n->Confidence = 1000;
		n->Distance = 0;
		strcpy(d->DirNode.Name,name);
		Insert(&n->DirNode,(ObjNode *)&d->DirNode, FALSE);
		AddHash(d);
		d = n;
	}

	getpath(ProcManInfo.ParentName,&ThisMc->DirNode);
	
	Signal(&HashLock);

	ErrorMsg(servinfo->m,Err_Null);
	return;

badname:
	ErrorMsg(servinfo->m,EC_Error|EG_Invalid|EO_Name);
	return;

}

static void nt_delete(ServInfo *servinfo)
{
	MCB *m = servinfo->m;
	NameEntry *n;
	IOCCommon *req = (IOCCommon *)(m->Control);
PMTRACE(0x14);

	n = (NameEntry *)GetTarget(servinfo);

	if( n == NULL )
	{
		ErrorMsg(m,EO_Name);
		return;
	}

	unless( CheckMask(req->Access.Access,AccMask_D) ) 
	{
		ErrorMsg(m,EC_Error+EG_Protected+EO_Name);
		return;
	}

	if( (NameDir *)n == ThisMc )
	{
		NameDir *d = ThisMc;
		while( (d = (NameDir *)d->DirNode.Parent) != NULL )
		{
			WalkList(&d->DirNode.Entries,(WordFnPtr)RemName);
		}	
		ErrorMsg(m,Err_Null);	
		return;
	}
	
	if( ((n->ObjNode.Type & ~Type_Flags) != Type_Name) ||
	    ((n->ObjNode.Flags & Flags_CacheName) != 0) )
	{
		ErrorMsg(m,EC_Error+EG_WrongFn+EO_Directory);
		return;
	}

	Wait(&HashLock);
	Unlink(&n->ObjNode, FALSE);
	Remove(&n->HashNode);
	Signal(&HashLock);

	PFree(n); DbInfo.NameEntries--;
	
	ErrorMsg(m,Err_Null);
	
	servinfo->TargetLocked = FALSE;
}

/*--------------------------------------------------------
-- NewName						--
--							--
-- Install a new entry into the name table.		--
--							--
--------------------------------------------------------*/

static NameEntry *
NewName(
	NameDir *	d,
	string		name,
	word		type,
	word		flags,
	Port		port,
	Matrix		matrix,
	word *		loaddata,
	bool		dirlocked )
{
	NameEntry *n;

PMTRACE(0x15);

	/* BLV - only insert name if the server is not already part	*/
	/* of the nucleus.						*/
	if (Lookup(&d->DirNode, name, dirlocked) != NULL)
	 return(NULL);

	n = New(NameEntry);
	if( n == Null(NameEntry) ) return n;
	
	DbInfo.NameEntries++;

	InitNode( &n->ObjNode, name, (int)type, (int)flags, matrix );
	n->ObjNode.Size = 0;
	InitList(&n->ObjNode.Contents); /* just in case */
	n->Server = port;
	n->LoadData = loaddata;
	n->Confidence = 10;		/* start with some confidence */
	Insert(&d->DirNode,&n->ObjNode,dirlocked);
	Wait(&HashLock);
	AddHash((NameDir *)n);
	Signal(&HashLock);

	return n;
}

static int distance(NameDir *n)
{
	int dist = 0;
	NameDir *t = ThisMc;

	unless( n->DirNode.Flags & Flags_CacheName ) return 0;
	
	do {
		n = (NameDir *)n->DirNode.Parent;
	} while( n->DirNode.Flags & Flags_CacheName );

	while( t != n ) 
	{
		dist++;
		t = (NameDir *)t->DirNode.Parent;
	}

	return dist;
}

static void AddHash(NameDir *n)
{
	NameDir *x = n;
	List *l = &HashTable[hash(n->DirNode.Name)];
	Node *node;

PMTRACE(0x16);
	x->Distance = distance(n);

	node = l->Head;	

	until( node->Next == NULL )
	{
		n = (NameDir *)((word)node - offsetof(NameEntry,HashNode));
		if( n->Distance > x->Distance ) break;
		node = node->Next;
	}

	PreInsert(node,&x->HashNode);
}

static NameEntry *AddName(string name, Port port, word Flags)
{
	word len;
	char part[NameMax];
	NameEntry *n = NULL;
	NameDir *d = NameTableRoot;
	NameDir *nd;

	while( *name == c_dirchar ) name++;

	while( (len = splitname(part, c_dirchar, name )) != 0 )
	{
PMTRACE(0x17);
		n = (NameEntry *)Lookup(&d->DirNode,part,FALSE);

		if( n == NULL )	
		{
			if( name[len] == 0 )
			{
				n = NewName(d,part,(word)Type_Name,Flags,
					port,(Matrix)DefNameMatrix,(word *)NULL,FALSE);
				break;
			}
			else {
				nd = New(NameDir);

				if( nd == NULL ) return NULL;
				DbInfo.NameEntries++;
				
				Wait(&HashLock);

				InitNode( (ObjNode *)&nd->DirNode, 
					part, 
					Type_Directory,
					Flags_CacheName|Flags_StripName|Flags_Seekable, 
					DefNameMatrix 
					);

				InitList(&nd->DirNode.Entries);
				nd->DirNode.Nentries = 0;
				nd->Server = NullPort;
				nd->LoadData = NULL;

				Insert(&d->DirNode,(ObjNode *)&nd->DirNode,FALSE);

				AddHash(nd);

				Signal(&HashLock);

				name += len;
				d = nd;
			}
		}
		else {
			if( name[len] == 0 ) break;

			if( (n->ObjNode.Type & ~Type_Flags) == Type_Name )
			{
				nd = (NameDir *)n;
				Wait(&nd->DirNode.Lock);
				/* convert name entry to a directory */
				nd->DirNode.Type = Type_Directory;
				nd->DirNode.Flags |= Flags_StripName|Flags_Seekable;
				InitList(&nd->DirNode.Entries);
				nd->DirNode.Nentries = 0;
				Signal(&nd->DirNode.Lock);
			}
			name += len;
			d = (NameDir *)n;
		}
	}

	return n;
}


static void RemName1(NameEntry *n)
{
	/* check that node is still on list, if not then we have just	*/
	/* waited for another process to do the same job as us		*/
	if( n->ObjNode.Node.Next == &n->ObjNode.Node ) return;
PMTRACE(0x18);
	/* if it is a directory, remove its children first */
	if( (n->ObjNode.Type & Type_Flags) == Type_Directory )
	{
		NameDir *d = (NameDir *)n;

		WalkList(&d->DirNode.Entries,(WordFnPtr)RemName1);
		if( d->DirNode.Nentries != 0 ) return;
	}

	if( (n->ObjNode.Flags & Flags_CacheName) == 0 ) return;
		
	/* now remove name entry from hash table and parent directory	*/
	Remove(&n->HashNode);
	Unlink(&n->ObjNode,FALSE);
	if( n->Server != NullPort ) FreePort(n->Server);
	PFree(n); DbInfo.NameEntries--;
}

static void RemName(NameEntry *n)
{
	Wait(&HashLock);
	RemName1(n);
	Signal(&HashLock);
}

static word hash(char *s)
{
        char *p;
        unsigned long h = 0, g;
        for( p = s ; *p != 0 ; p++ )
        {
                h = (h << 4) + *p;
                if( (g = h & 0xf0000000L) != 0 )
                {
                        h = h ^ (g >> 24);
                        h = h ^ g;
                }
        }
        return (word)(h % HashSize);
}

static word NewId(void)
{
	return SearchTable[ (STOffset++) & (SearchSize - 1) ] = 
					NewKey()+(IdSeed++)+_ldtimer(0);
}

static string makename(string obj,string name)
{
	string s = objname(obj);

	if( s != name ) strcpy(name,s);
	strcat(name,".");
	(void)addint(name,TaskId++);

	return name;
}

static char dbg_msg[128];
static int dbg_args[10];

static void dbg(char *str, ... )
{
	int i;
	va_list a;
	va_start(a,str);
	Wait(&DbgLock);
	getpath(dbg_msg,&ThisMc->DirNode);
	strcat(dbg_msg,": ");
	strcat(dbg_msg,str);
	for( i = 0 ; i < 10 ; i++ ) dbg_args[i] = va_arg(a,int);
	IOdebug(dbg_msg,dbg_args[0],dbg_args[1],dbg_args[2],dbg_args[3],
			dbg_args[4],dbg_args[5],dbg_args[6],dbg_args[7],
			dbg_args[8],dbg_args[9]);
	Signal(&DbgLock);
	va_end(a);
}

/* AF_HELIOS socket domain handling */

#include <sys/types.h>
#include <sys/socket.h>

#define RemoteBit	8

typedef struct Waiter
{
	Node		Node;
	word		EndTime;
	SockEntry	*Socket;
	word		Type;
	Port		Reply;
	void		*Addr;		/* used only by Connect & SendMessage */
	word		Size;		/* used only by RecvMessage	*/
} Waiter;

#define FreeWaiter(www) if(www->Addr!=NULL) PFree(www->Addr); PFree(www);

static Waiter *NewWaiter(word type, Port reply, SockEntry *s)
{
	Waiter *w = New(Waiter);
	
	if( w == NULL ) return NULL;
	
	w->Type = type;
	w->Reply = reply;
	w->EndTime = (word)GetDate()+10;
	w->Socket = s;
	w->Addr = NULL;
	w->Size = 0;
	
	return w;
}

static bool findtype(Waiter *w, word type) { return w->Type==type; }

static word replyselect(Waiter *w, int flags)
{
	if( ((w->Type & FG_Mask) == FG_Select) && (w->Type & flags) )
	{
		MCB mcb;
		Remove(&w->Node);
		InitMCB(&mcb,0,w->Reply,NullPort,flags);
		PutMsg(&mcb);
		FreeWaiter(w);
		return 1;
	}
	return 0;
}

static word killselect(Waiter *w)
{
	if( (w->Type & FG_Mask) == FG_Select)
	{
		Remove(&w->Node);
		FreePort(w->Reply);
		FreeWaiter(w);
		return 1;
	}
	return 0;
}

static void *copystruct(word size, void *str)
{
	void *v;

	if( size == 0 || str == NULL ) return NULL;
	
	v = PMalloc((int)size);
	if( v == NULL ) return NULL;
	
	memcpy(v,str,(int)size);
	
	return v;
}

static void MarshalAddr(MCB *mcb, int family, void *addr, word addrsize)
{
	int dsize;
	byte *data;
	int sasize = 2 + (int)addrsize;
	
	if( addr == NULL )
	{ 
		MarshalWord(mcb,-1);
		return;
	}
	
	MarshalOffset(mcb);
	
	MarshalData(mcb, sizeof(word), (byte *)&sasize );

	dsize = mcb->MsgHdr.DataSize;
	data = mcb->Data + dsize;
	
	*data++ = family & 0xff;
	*data++ = (family>>8) & 0xff;
	
	memcpy(data,addr,(int)addrsize);
	
	mcb->MsgHdr.DataSize = dsize + sasize;
}

static void MakeConnection(Waiter *acc, Waiter *conn)
{
	SockEntry *s = acc->Socket;
	MsgBuf *m;
	MCB *mcb;
	word proto = s->Protocol & 0xff;
	byte *addr = (byte *)conn->Addr;
	
	m = New(MsgBuf);
	
	if( m == NULL )
	{
		AddTail(&s->WaitQ,&acc->Node);
		AddTail(&s->WaitQ,&conn->Node);
		return;
	}
	
	mcb = &m->mcb;
	mcb->Control = m->control;
	mcb->Data = m->data;
	
	if( proto == 1 ) /* stream socket, make a pipe */
	{
		Object *pipe;
		Object *pipeman;
		char *name = m->data;	/* temp */
		word count = 1;
		NameEntry *n;
	
		/* force the pipe server to be loaded */	
		n = (NameEntry *)Lookup(&ThisMc->DirNode,"pipe",FALSE);
		if( n->Server == NullPort ) LoadServer(n);
			
		Signal(&s->ObjNode.Lock);
		pipeman = Locate(NULL,"/pipe");
		Wait(&s->ObjNode.Lock);

		if( pipeman == NULL ) goto fail;

		forever
		{
			count %= 1000;
			
			strcpy(name,"stream.socket.");
			addint(name,count);
			Signal(&s->ObjNode.Lock);
			pipe = Create(pipeman,name,Type_Pipe,0,0);
			Wait(&s->ObjNode.Lock);
			if( pipe != NULL ) break;

			count++;
		}

		InitMCB(mcb,0,conn->Reply,NullPort,Err_Null);
		
		MarshalWord(mcb,Type_Pseudo);
		MarshalWord(mcb,pipe->Flags|O_ReadWrite);
		MarshalCap(mcb,&pipe->Access);
		MarshalString(mcb,pipe->Name);
		MarshalWord(mcb, NullPort);
				
		PutMsg(mcb);
		
		mcb->MsgHdr.Dest = acc->Reply;

			/* BLV - fix 4.3.93, ProcMan and the system	  */
			/* library disagreed about the structure returned */
		mcb->MsgHdr.ContSize--;
		
		MarshalAddr(mcb, AF_HELIOS, addr+2, (word)strlen(addr+2)+1);
		
		PutMsg(mcb);
		
		Close(pipe);
		Close(pipeman);
	}
	elif( proto == 3 )	/* raw protocol interface 	*/
	{
		word e;
		/* This generates replies just like a stream socket */
		/* except that the two ports are simply exchanged   */		

		InitMCB(mcb,MsgHdr_Flags_preserve,
				conn->Reply,acc->Reply,Err_Null);
		
		MarshalWord(mcb,Type_Socket);
		MarshalWord(mcb,O_ReadWrite);
		MarshalWord(mcb,-1);
		MarshalWord(mcb,-1);
		MarshalOffset(mcb);
		getpath(mcb->Data+mcb->MsgHdr.DataSize,(DirNode *)&s->ObjNode);
		mcb->MsgHdr.DataSize += strlen(mcb->Data+mcb->MsgHdr.DataSize)+1;

		e = PutMsg(mcb);

		mcb->MsgHdr.Dest = acc->Reply;
		mcb->MsgHdr.Reply = conn->Reply;

		MarshalAddr(mcb, AF_HELIOS, addr+2, (word)strlen(addr+2)+1);
		
		e = PutMsg(mcb);
	}

	FreeWaiter(acc); FreeWaiter(conn);
	PFree(m);	/* BLV - fix memory leak	*/
	return;				

fail:
	InitMCB(mcb,0,acc->Reply,NullPort,EC_Error|SS_ProcMan|EG_WrongFn|EO_Socket);
	PutMsg(mcb);
	mcb->MsgHdr.Dest = conn->Reply;	
	PutMsg(mcb);
	FreeWaiter(acc); FreeWaiter(conn);
	PFree(m);	/* BLV - fix memory leak	*/
}

static void DoAccept(MCB *m, SockEntry *s)
{
	Waiter *w, *w1;
	
	if( s->BackLog == 0 )
	{
		ErrorMsg(m,EC_Error|EG_WrongFn|EO_Socket);
		return;
	}
	
	w = NewWaiter(FG_Accept,m->MsgHdr.Reply,s);

	if( w == NULL ) 
	{
		ErrorMsg(m,EC_Error|EG_NoMemory|EO_Socket);
		return;
	}
		
	if( (w1 = (Waiter *)SearchList(&s->WaitQ,findtype,FG_Connect)) == NULL )
		AddTail(&s->WaitQ,&w->Node);
	else 
	{
		Remove(&w1->Node);
		MakeConnection(w,w1);
	}
}

static bool ForwardRequest(MCB *m, SockEntry *s, word fn, char *name)
{
	MsgBuf *mm;
	SockEntry *n;
	Port dest = NullPort;

	n = (SockEntry *)Lookup(&ThisMc->DirNode,name,FALSE);

	if( n != NULL ) dest = n->Server;
	else {
		/* do a Search for the required socket */
		Wait(&SearchLock);
		mm = Search( name, NULL, NewId() ); 
		Signal(&SearchLock);
		if( mm != NULL ) dest = mm->mcb.MsgHdr.Reply;
		PFree(mm);
	}
	
	if( dest == NullPort )
	{
		/* return an error if it is not found */
		ErrorMsg(m,EC_Error|EG_Unknown|EO_Socket);
		return FALSE;
	}
	else
	{
		word e;

		/* pass request on if it was found, adding the name of	*/
		/* this socket to the message.				*/
		
		MarshalAddr(m, AF_HELIOS, s->ObjNode.Name, (word)strlen(s->ObjNode.Name)+1);		
		m->MsgHdr.Flags = 0;
		m->MsgHdr.Dest = dest;
		m->MsgHdr.FnRc = fn | RemoteBit;
		
		e = PutMsg(m);
	}

	return TRUE;
}

static void DoConnectIn(MCB *m, SockEntry *s)
{
	Waiter *w1, *w;
	
	if( (s->Protocol & 0xf) == 2 )	/* DataGram connect	*/
	{
		Capability New;
		InitMCB(m,0,m->MsgHdr.Reply,s->Server,SS_ProcMan|RemoteBit);
		MarshalWord(m,Type_Socket);
		MarshalWord(m,Flags_Server|Flags_Closeable|Flags_Selectable|O_ReadWrite);
		NewCap(&New,&s->ObjNode,AccMask_Full);
		MarshalCap(m,&New);
		getpath(m->Data+m->MsgHdr.DataSize,(DirNode *)&s->ObjNode);
		m->MsgHdr.DataSize += strlen(m->Data+m->MsgHdr.DataSize)+1;

		PutMsg(m);

		s->Users++;

		return;
	}
	
	w = NewWaiter(FG_Connect,m->MsgHdr.Reply,s);

	if( w == NULL ) 
	{
		ErrorMsg(m,EC_Error|EG_NoMemory|EO_Socket);
		return;
	}

	w->Addr = copystruct(*(word *)(m->Data + m->Control[1]),m->Data + m->Control[1] + 4);
	
	if( (w1 = (Waiter *)SearchList(&s->WaitQ,findtype,FG_Accept)) == NULL )
	{
		AddTail(&s->WaitQ,&w->Node);
		WalkList(&s->WaitQ,replyselect,O_ReadOnly);
	}
	else 
	{
		Remove(&w1->Node);
		MakeConnection(w1,w);
	}
}

static void DoSelect(MCB *m, SockEntry *s, word fn)
{
	Waiter *w1, *w;
	word flags = fn & FF_Mask;

	if( (s->Protocol & 0xf) == 1 || (s->Protocol & 0xf) == 3 )
	{
		/* stream/raw select, look for a Connect */
		if( (w1 = (Waiter *)SearchList(&s->WaitQ,findtype,FG_Connect)) != NULL )
		{
			m->MsgHdr.FnRc = O_ReadOnly;
			ErrorMsg(m,0);
			return;
		}
	}
	elif( (s->Protocol & 0xf) == 2 )
	{
		/* DataGram select, look for either recv or send or an	*/
		/* opposite select.					*/

		int result = 0;

		if( flags & O_ReadOnly )
		{
			if( SearchList(&s->WaitQ,findtype,FG_SendMessage) ||
			    WalkList(&s->WaitQ,replyselect,O_WriteOnly)	   )
				result |= O_ReadOnly;
		}

		if( flags & O_WriteOnly )
		{
			if( SearchList(&s->WaitQ,findtype,FG_RecvMessage) ||
			    WalkList(&s->WaitQ,replyselect,O_ReadOnly)	   )
				result |= O_WriteOnly;
		}

		if( result )
		{
			m->MsgHdr.FnRc = result;
			ErrorMsg(m,0);
			return;
		}
	}

	/* no immediate match found, add a waiter, first get rid of any */
	/* outstanding selects.						*/
	
	WalkList(&s->WaitQ,killselect);
		
	w  = NewWaiter(FG_Select|flags,m->MsgHdr.Reply,s);

	if( w == NULL ) 
	{
		ErrorMsg(m,EC_Error|EG_NoMemory|EO_Socket);
		return;
	}

	w->EndTime = -1;	/* wait forever */
			
	AddTail(&s->WaitQ,&w->Node);
}

static void SwapDataGram(Waiter *send, Waiter *recv)
{
	MsgBuf *m;
	MCB *mcb;
	byte *addr   = (byte *)send->Addr;
	SockEntry *s = send->Socket;

	m = New(MsgBuf);
	
	if( m == NULL )
	{
		AddTail(&s->WaitQ,&send->Node);
		AddTail(&s->WaitQ,&recv->Node);
		return;
	}
	
	mcb = &m->mcb;
	mcb->Control = m->control;
	mcb->Data = m->data;

	InitMCB(mcb,0,send->Reply,recv->Reply,Err_Null);
	
	MarshalWord(mcb,0);
	MarshalWord(mcb,recv->Size);
	MarshalWord(mcb,0);
	MarshalWord(mcb,-1);
	MarshalAddr(mcb,AF_HELIOS,s->ObjNode.Name,(word)strlen(s->ObjNode.Name)+1);
	MarshalAddr(mcb,AF_HELIOS,addr+2,(word)strlen(addr+2)+1);

	PutMsg(mcb);
	
	FreeWaiter(send);
	FreeWaiter(recv);
	PFree(m);
}

static void DoSendMessage(MCB *m, SockEntry *s)
{
	Waiter *w1, *w;
	DataGram *dg = (DataGram *)m->Control;

	w = NewWaiter(FG_SendMessage,m->MsgHdr.Reply,s);
	w->EndTime = dg->Timeout==-1?-1:GetDate()+(dg->Timeout/OneSec);
	
	if( w == NULL ) 
	{
		ErrorMsg(m,EC_Error|EG_NoMemory|EO_Socket);
		return;
	}

	w->Addr = copystruct(*(word *)(m->Data + dg->SourceAddr),m->Data + dg->SourceAddr + 4);
	w->Size = dg->DataSize;

	if( (w1 = (Waiter *)SearchList(&s->WaitQ,findtype,FG_RecvMessage)) == NULL )
	{
		AddTail(&s->WaitQ,&w->Node);
		WalkList(&s->WaitQ,replyselect,O_ReadOnly);
	}
	else 
	{
		Remove(&w1->Node);
		SwapDataGram(w,w1);
	}	
}

static void DoRecvMessage(MCB *m, SockEntry *s)
{
	Waiter *w, *w1;
	DataGram *dg = (DataGram *)m->Control;

	if( (s->Protocol & 0xf) != 2 )
	{
		ErrorMsg(m,EC_Error|EG_Invalid|EO_Socket);
		return;
	}
	
	w = NewWaiter(FG_RecvMessage,m->MsgHdr.Reply,s);
	w->EndTime = dg->Timeout==-1?-1:GetDate()+(dg->Timeout/OneSec);

	if( w == NULL ) 
	{
		ErrorMsg(m,EC_Error|EG_NoMemory|EO_Socket);
		return;
	}

	w->Size = dg->DataSize;
			
	if( (w1 = (Waiter *)SearchList(&s->WaitQ,findtype,FG_SendMessage)) == NULL )
	{
		AddTail(&s->WaitQ,&w->Node);
		WalkList(&s->WaitQ,replyselect,O_WriteOnly);
		
	}
	else 
	{
		Remove(&w1->Node);
		SwapDataGram(w1,w);
	}
}

static void DoGetInfo(MCB *mcb, SockEntry *s, word fn)
{
	word level = mcb->Control[0];
	word option = mcb->Control[1];

/* IOdebug("DoGetInfo %s %x",s->ObjNode.Name,fn); */
	switch( level )
	{
	case SOL_SYSTEM:		/* system	*/
		switch( option )
		{
		default: goto badopt;
		case SO_HOSTID:
		case SO_HOSTNAME:
			goto badopt;
		}
		break;
		
	case SOL_SOCKET:		/* socket	*/
		switch( option )
		{
		case SO_SOCKNAME:
			InitMCB(mcb,0,mcb->MsgHdr.Reply,NullPort,Err_Null);
			MarshalWord(mcb, level );
			MarshalWord(mcb, option );
			MarshalAddr(mcb,AF_HELIOS,s->ObjNode.Name,(word)strlen(s->ObjNode.Name)+1);
			PutMsg(mcb);
			break;
			
		case SO_PEERNAME:
			default:
		badopt:
			ErrorMsg(mcb,EC_Error|EG_Parameter|2);
			break;
		}
		break;
	
	default:
		ErrorMsg(mcb,EC_Error|EG_Parameter|1);
		break;
	}
}

static void DoSetInfo(MCB *mcb, SockEntry *s, word fn)
{
	word level = mcb->Control[0];
	word option = mcb->Control[1];

/* IOdebug("DoSetInfo %s %x",s->ObjNode.Name,fn); */
	switch( level )
	{
	case SOL_SYSTEM:		/* system	*/
		switch( option )
		{
		default: goto badopt;
		case SO_HOSTID:
		case SO_HOSTNAME:
			goto badopt;
		}
		break;
		
	case SOL_SOCKET:		/* socket	*/
		switch( option )
		{
		case SO_PEERNAME:
		case SO_SOCKNAME:
		default:
		badopt:
			ErrorMsg(mcb,EC_Error|EG_Parameter|2);
			break;
		}
		break;
	
	default:
		ErrorMsg(mcb,EC_Error|EG_Parameter|1);
		break;
	}
}

static word dotimeout(Waiter *w)
{
	if ( w->EndTime > 0 && GetDate() >= w->EndTime )
	{
		MCB mcb;
		Remove(&w->Node);
		InitMCB(&mcb,0,w->Reply,NullPort,
			EC_Recover|SS_ProcMan|EG_Timeout|EO_Socket);
		PutMsg(&mcb);
		FreeWaiter(w);
	}
	return 1;
}

static void SocketServer(SockEntry *s)
{
	MsgBuf *m;
	MCB *mcb;
		
	while( (m = New(MsgBuf)) == NULL ) Delay(OneSec);

	mcb = &m->mcb;
	mcb->Control = m->control;
	mcb->Data = m->data;
	
	forever
	{
		word e;

		mcb->MsgHdr.Dest = s->Server;
		mcb->Timeout = OneSec*2;

		e = GetMsg(mcb);

		if( e == EK_Timeout )
		{
			Wait(&s->ObjNode.Lock);
			WalkList(&s->WaitQ,dotimeout,s);
			Signal(&s->ObjNode.Lock);
			continue;
		}	
		
		if( e < Err_Null ) continue;

		Wait(&s->ObjNode.Lock);

/*		IOdebug("SocketServer %s msg %M",s->ObjNode.Name,mcb);*/
		
		mcb->MsgHdr.FnRc = SS_ProcMan;

		switch( e & (FG_Mask|FF_Mask) )
		{
		case FG_Listen:
			if( (s->Protocol & 0xf) == 1 || 
			    (s->Protocol & 0xf) == 3 ) 
			{
				s->BackLog = mcb->Control[0];
				ErrorMsg(mcb,0);
			}
			else goto badfn;
			break;
			
		case FG_Accept:		DoAccept(mcb,s);	break;
		case FG_Connect|RemoteBit:	
					DoConnectIn(mcb,s);	break;
		
		case FG_Select|O_ReadOnly:
		case FG_Select|O_WriteOnly:
		case FG_Select|O_ReadWrite:
					DoSelect(mcb,s,e);	break;
		
		case FG_SendMessage|RemoteBit:
					DoSendMessage(mcb,s);	break;
		case FG_RecvMessage:	DoRecvMessage(mcb,s);	break;
			
		case FG_SendMessage:
		{
			DataGram *dg = (DataGram *)mcb->Control;
			ForwardRequest(mcb,s,e,mcb->Data+dg->DestAddr+6);
		}
					break;
		case FG_GetInfo:
		case FG_GetInfo|RemoteBit:
					DoGetInfo(mcb,s,e);	break;
					
		case FG_SetInfo:
		case FG_SetInfo|RemoteBit:
					DoSetInfo(mcb,s,e);	break;
					
		case FG_Connect:	
		{
			ConnectRequest *req = (ConnectRequest *)mcb->Control;
			if( !ForwardRequest(mcb,s,e,mcb->Data+req->DestAddr+6) )
				break;
		}
		case FG_Close:
		case FG_Close|RemoteBit:
			s->Users--;
			if( s->Users == 0 ) goto done;
			break;

		badfn:
		default:
			ErrorMsg(mcb,EC_Error|EG_WrongFn|EO_Socket);
			break;
		}
		
		Signal(&s->ObjNode.Lock);
	}
	
done:
	Signal(&s->ObjNode.Lock);
	Wait(&HashLock);
	Remove(&s->HashNode);
	Unlink(&s->ObjNode,FALSE);
	if( s->Server != NullPort ) FreePort(s->Server);
	Signal(&HashLock);
	PFree(s); DbInfo.NameEntries--;
	PFree(m);	/* BLV - fix memory leak	*/
}

static word DoBind(ServInfo *servinfo) 
{
	SockEntry *s = (SockEntry *)servinfo->Context;
	SockEntry *n;
	MCB *m = servinfo->m;
	MsgBuf *r;
	char *name;
	char *pathname = servinfo->Pathname;
		
	if( s != DotSocket )
	{
		ErrorMsg(m,EC_Error|EG_WrongFn|EO_Socket);
		return TRUE;
	}

	r = New(MsgBuf);	
	
	if( r == NULL )
	{
		ErrorMsg(m,EC_Error+EG_NoMemory);
		return TRUE;		
	}

	if( m->Control[6] == -1 )
	{
		strcat(pathname,".");
		addint(pathname,SockId++);
		name = objname(pathname);
	}
	else 
	{
		char *p;
		struct sockaddr { short so_family; char so_name[1]; } *addr;
		
		addr = (struct sockaddr *)&m->Data[m->Control[6]+4];
		name = addr->so_name;

		for( p = name; *p ; p++ )
			if( *p == c_dirchar )
			{
				ErrorMsg(m,EC_Error|EG_Invalid|EO_Name);
				PFree(r);	/* BLV - fix memory leak */
				return TRUE;
			}
		getpath(pathname,&ThisMc->DirNode);
		pathcat(pathname,name);
	}

	UnLockTarget(servinfo);
	
	n = (SockEntry *)Lookup(&ThisMc->DirNode,name,FALSE);
		
	if ( n == NULL )
	{
		n = (SockEntry *)NewName(ThisMc,name,(word)Type_Socket,0L,NullPort,
				(Matrix)DefNameMatrix,(word *)NULL,FALSE);
	
		if( n == NULL ) goto nomem;
		n->Protocol = m->Control[5];
		n->BackLog = 0;
		n->Users = 1;
		InitList(&n->WaitQ);
		n->Server = NewPort();
		if( !PFork(SOCKSTACKSIZE,SocketServer,sizeof(n),n) )
		{
		nomem:
			ErrorMsg(m,EC_Error|EG_NoMemory|EO_Socket);
			PFree(r);	/* BLV - fix memory leak	*/
			return TRUE;
		}
	}
	else
	{
		if( n->ObjNode.Type != Type_Socket )
		{
			ErrorMsg(m,EC_Error|EG_Invalid|EO_Socket);
			PFree(r);	/* BLV - fix memory leak	*/
			return TRUE;
		}
		if( n->Protocol != m->Control[5] )
		{
			ErrorMsg(m,EC_Error|EG_Parameter|4);
			PFree(r);	/* BLV - fix memory leak	*/
			return TRUE;			
		}
		n->Users++;
	}
	
	FormOpenReply(r,m,&n->ObjNode, Flags_Closeable|Flags_Selectable, pathname);
	r->mcb.MsgHdr.Reply = n->Server;
	PutMsg(&r->mcb);
	PFree(r);
	return TRUE;
}

static word so_private(ServInfo *servinfo)
{
	MCB *mcb = servinfo->m;
	
	if( (servinfo->FnCode & FG_Mask) == FG_Bind ) return DoBind(servinfo);
	else ErrorMsg(mcb,EC_Error|SS_ProcMan|EG_WrongFn|EO_Socket);
	
	return FALSE;
}


#ifdef PMDEBUG

static int MemAlloced = 0;
static int NForks = 0;


# if 0
static int MemSize(void *v)
{
	int *vv = (int *)v;
	int sz = vv[-2];
	if( sz < 0 ) sz = -sz;
	return sz;
}
# endif

static void *PMalloc(int size)
{
	void *v = Malloc(size);
	
	if( v ) MemAlloced += (int)MemSize(v);
	
	if( debugging(DB_MEM) )
		IOdebug("PMalloc(%d)=%x[%d] from %s",
		size, v, MemSize(v), procname(returnlink_(size)));

	if( MemAlloced > 500000 )
	{
/*		if( !dbprocrunning ) Fork(2000,dbinfoproc,0); */
		dbmask |= DB_IOC1|DB_MEM;
	}

	return v;
}

static void PFree(void *v)
{
	if( v ) MemAlloced -= (int)MemSize(v);
	if( debugging(DB_MEM) )
		IOdebug("PFree(%x[%d]) from %s",v,MemSize(v),
		procname(returnlink_(v)));
		
	Free(v);
}

static bool PFork(int ssize, VoidFnPtr f, int asize, ... )
{
#ifdef __TRAN
	int *args = (&asize)+1;
#else
	/* currently there is no Fork for > 3 args in  Procman */
	word args[4];
	va_list	argp;
	int i;
#endif
	bool res;

#ifndef __TRAN
	va_start(argp, asize);

	for (i = 0;  i < (asize / sizeof(word)) ; i++)
		args[i] = va_arg(argp, word); /* get args */

	va_end(argp);
#endif

	res = Fork(ssize,f,asize,args[0],args[1],args[2]);

	if( res ) NForks++;
	
	if( debugging(DB_MEM) ) 
	{
#ifdef __TRAN
		fncast fc;
		fc.vfn = f;
		IOdebug("PFork %s(%x,%x,%x) from %s %s",
			procname(fc.vp),
			args[0],args[1],args[2],
			procname(returnlink_(ssize)),
			res?"OK":"FAILED");
#else
	  IOdebug("PFork %s(%x,%x,%x) from %s %s",
		  procname(f),
		  args[ 0 ], args[ 1 ], args[ 2 ],
		  procname(NULL),
		  res ? "OK" : "FAILED" );	  
#endif
	}
	
	return res;
}

static word dbtask(TaskEntry *t)
{
	char mode = 'C';
	
	if( t->UseCount > 0 ) mode = 'E';
	if( (t->Status % 10) != 0 ) mode = 'R';
	if( !t->Running ) mode = 'D';
	if( t->ProgInfoSent != 0 ) mode = 'P';
	
	IOdebug("%s[%d,%c] %",t->ObjNode.Name,t->UseCount,mode);
	return 1;
}

static void dbinfoproc(void)
{
	RootStruct *root = GetRoot();
	Config *config = GetConfig();
	int i;
	
	dbprocrunning = TRUE;
	
	while( dbprocrunning )
	{
		dbg("INFO: ------------------------------------------------------------");
		dbg("INFO: FreePool Max %d Size %d",root->FreePool->Max,root->FreePool->Size);
		dbg("INFO: Buffers Max %d Used %d Pool %d",root->MaxBuffers,root->BufferCount,root->BuffPoolSize);
		dbg("INFO: heap size %d, free %d, largest block %d", Malloc(-3), Malloc(-1), Malloc(-2));
		dbg("INFO: Tasks:%"); WalkList(&TaskTable.Entries,dbtask,0); IOdebug("");
		dbg("INFO: Load Av %d Latency %d Max %d",root->LoadAverage,root->Latency,root->MaxLatency);
		root->MaxLatency = 0;
		dbg("INFO: Local Traffic %d Events %d Errors %d",root->LocalMsgs,root->EventCount,root->Errors);
		for( i = 0; i < config->NLinks; i++ )
		{
			LinkInfo *l = root->Links[i];
			dbg("INFO: Link %d St %d Md %d Fl %x Msgs: In %d Out %d Lost %d",
				l->Id,l->State,l->Mode,l->Flags,
				l->MsgsIn,l->MsgsOut,l->MsgsLost);
		}
		dbg("INFO: IOCs %d Servers %d NameEntries %d Tasks %d",
			DbInfo.IOCs,DbInfo.Servers,DbInfo.NameEntries,DbInfo.Tasks);
		dbg("INFO: MsgBufs %d Misc %d Workers %d",
			DbInfo.MsgBufs,DbInfo.Misc,DbInfo.Workers);
		Delay(OneSec*5);
	}
}

#endif

/* -- End of procman.c */

