/*------------------------------------------------------------------------
--                                                                      --
--                     H E L I O S   N U C L E U S                      --
--                     ---------------------------                      --
--                                                                      --
--             Copyright (C) 1987, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- loader.c								--
--                                                                      --
--	Server to manage all loaded code and libraries.			--
--                                                                      --
--	Author:  NHG 16/8/87						--
--                                                                      --
------------------------------------------------------------------------*/
/* $Id: loader.c,v 1.37 1993/09/01 17:55:33 bart Exp $ */
/* Copyright (C) 1987, Perihelion Software Ltd.				*/


#include <helios.h>	/* standard header */

#define __in_loader 1	/* flag that we are in this module */

#define TIMEOUTS 0

#define SEARCHCODE	0

#ifdef __C40
#define	READBUFFER	1
#else
#define	READBUFFER	0
#endif

	/* BLV - option to check all loaded code every ten seconds */
#define CONTINUOUS_CHECKING 0

/* #define debug(x)	if( MyTask->Flags & (Task_Flags_servlib)) IOdebug x */
#define debug(x)	/* IOdebug("%s: %",mcname);IOdebug x */

/*--------------------------------------------------------
-- 		     Include Files			--
--------------------------------------------------------*/

#include <string.h>
#include <codes.h>
#include <config.h>
#include <module.h>
#include <syslib.h>
#include <servlib.h>
#include <root.h>
#include <link.h>
#include <process.h>
#include <task.h>
#ifdef __TRAN
#include <asm.h>
#endif

/*----------------------------------------------------------------
--	Stack Handling						--
----------------------------------------------------------------*/

#if defined(__TRAN) && !defined(SYSDEB)
	/* Transputer Helios does not have automatic stack extension.	*/
	/* Please do not change these sizes. Some bits of the loader,	*/
	/* particularly the resident library stuff, involves recursion.	*/
#  define bigstack 3000
#  define defstack 1250
#else
#if defined(STACKEXTENSION)
	/* Some processors have automatic stack extension support.	*/
	/* N.B. stack checking must be used !!!				*/
	/* The sizes are a compromise between memory efficiency and	*/
	/* excessive stack extension with the fragmentation that would	*/
	/* result. N.B. stack sizes passed to the server library must	*/
	/* allow for a servinfo structure, 536 bytes plus a jump buffer,*/
	/* which will be held on the stack. NewProcess() should handle	*/
	/* the jump buffer extra.					*/
#define bigstack	1000
#define defstack	1000

#ifndef STACKCHECK
#error Stack checking must be enabled when compiling this module.
#endif

#else
	/* These sizes should be used on the transputer when debugging	*/
	/* is enabled, on any processor without automatic stack		*/
	/* extension, or when porting to a new processor.		*/
#  define bigstack 4000
#  define defstack 2000
# endif
#endif

#ifdef STACKCHECK
	/* Stack checking within the Loader is controlled		*/
	/* by -DSTACKCHECK on the command line, not by -ps1 or similar	*/
	/* pragmas. On the transputer the stack overflow routine is	*/
	/* part of the C library, not part of the kernel, so the	*/
	/* Loader must supply its own routine.				*/
#ifdef __TRAN
extern void _Trace(...);
#    pragma -s1

static void _stack_error(Proc *p)
{
	_Trace(0xaaaaaaaa,p);
	IOdebug("Loader stack overflow in %s at %x",p->Name,&p);	
}
#endif

	/* Enable stack checking if -DSTACKCHECK			*/
#    pragma -s0

#else

	/* Otherwise disable stack checking.				*/
# pragma -s1

#endif

/*--------------------------------------------------------
--		Private Data Definitions 		--
--------------------------------------------------------*/

typedef struct Image  {
	ObjNode		ObjNode;	/* node for directory struct	*/
#define UseCount 	ObjNode.Account	/* number of users		*/
	word		Retain;		/* true if to remain loaded despite no users */
	word		Closing;	/* true if being closed		*/
	MPtr		Image;		/* pointer to actual code	*/
	Carrier		*ImageCarrier;	/* Carrier for code		*/
	Object		*Object;	/* Object for image source	*/
	word		Flags;		/* image header flag word	*/
	word		CheckSum;	/* image checksum		*/
	Buffer		Buf;		/* structure for DoRead		*/
	ImageHdr	Hdr;		/* image header			*/
} Image;

char		mcname[100];

word		*CodePoolType = NULL;	/* Type of pools for code	*/

DirNode		Root;			/* root of loader directory	*/

Pool		CodePool;		/* Memory pool for loader code */

Object		*LibDir = NULL;		/* library directory		*/

NameInfo	LoaderInfo =
		{
			NullPort,
			Flags_StripName,
			DefNameMatrix,
			(word *)NULL
		};

#if CONTINUOUS_CHECKING
static void checker_process(void);
#endif

#ifdef SYSDEB
#define CheckLock(p,x)\
if( TestSemaphore(&((DirNode *)(x))->Lock) > 0 ) IOdebug("%s: %s: %s not locked",mcname,p,((DirNode *)(x))->Name);
# if 0
#define WaitObj(o) WaitLock(&(o)->Lock,(o)->Name)
void WaitLock(Semaphore *s,char *name)
{
	int i;
	for( i = 0; i < 10; i++ ) 
	{
		if( TestWait(s) ) return;
		Delay(OneSec/100);
	}
	while( !TestWait(s) )
	{
		IOdebug("%s: Loader %s locked",mcname,name);
		Delay(OneSec*2);
	}
}
# else
#  define WaitLock(s,n) Wait(s)
#  define WaitObj(o) Wait(&(o)->Lock)
# endif
# define SEMDBG 0
#else
#define CheckLock(p,x)
#define WaitLock(s,n) Wait(s)
#define WaitObj(o) Wait(&(o)->Lock)
# define SEMDBG 0
#endif

# if !SEMDBG
#define InitSemDbg()
#define NameSem(s,n)
# endif

static void do_open(ServInfo *);
static void do_create(ServInfo *);
static void do_delete(ServInfo *);
static void do_link(ServInfo *);
static void do_closeobj(ServInfo *);

/* private protocol functions */
#if SEARCHCODE
static bool do_search(ServInfo *servinfo);
#endif
static bool do_private(ServInfo *);
	bool RomCreate(ServInfo *);

static DispatchInfo LoaderDInfo = {
	&Root,
	NullPort,
	SS_Loader,
	NULL,
	{ (VoidFnPtr)do_private,bigstack },
	{
		{ do_open,	bigstack },
		{ do_create,	bigstack },
		{ DoLocate,	0	 },
		{ DoObjInfo,	0	 },
		{ NullFn,       0	 },
		{ do_delete,	defstack },
		{ DoRename,	0	 },
		{ do_link,	defstack },
		{ DoProtect,	0	 },
		{ DoSetDate,	0	 },
		{ DoRefine,	0	 },
		{ do_closeobj,	defstack },
		{ DoRevoke,	0	 },
		{ InvalidFn,	0	 },
		{ InvalidFn,	0	 }
	}
};

Image *NewImage(DirNode *, Carrier *, string, word, word, Matrix, word size);
word bindfn(Image *i);
word progsize(MPtr);
word checksum(MPtr, word size);
void check(Image *);
Image *makelink(DirNode *d, string lname, string name, Capability *cap);
Carrier *LoadImage(string name, Capability *cap, word pos, int bindp,
				Object **objp, word *size);
Carrier *LoadObject(Object *obj, word pos, bool bindp, word *size);
Image *LoadLibrary(string name);
Carrier *AllocCode(word size);
void FreeCode(Carrier *code);
word BindLibraries(MPtr image, bool readonly);
void UnbindLibraries(MPtr image);
Carrier *ReadStream(Stream *s, word *size);
void ReadImage(MCB *, Image *);
void WriteImage(MCB *, Image *);
word LoadCache(Image *i);
Buffer *ImageReadBuffer(word pos, Image *i);
#if SEARCHCODE
Image *SearchLoad(char *name);
#endif
void DestroyImage(Image *i);
bool AllocSMT(MPtr image);

Capability fplibcap = { 0xff, { 0, 0, 0, 0, 0, 0, 0 } };
Capability posixcap = { 0xff, { 0, 0, 0, 0, 0, 0, 0 } };
Capability clibcap  = { 0xff, { 0, 0, 0, 0, 0, 0, 0 } };

#if SEARCHCODE
Semaphore SearchLock;
Semaphore SearchTabLock;
#endif

#define MagicKey 0x87654321;

#define READSIZE	(16*1024)	/* size for data reads	*/
#define MINREADSIZE	(4*1024)	/* minimum buffer size	*/

/*--------------------------------------------------------
-- main							--
--							--
-- Entry point of loader				--
--							--
--------------------------------------------------------*/

int main()
{
	/* for romable Helios */
	MPtr ivec = GetSysBase();	/* from root.h */

	Image *i;
	MPtr m;
	Object *nte;
	word j;

	InitSemDbg();

#if SEARCHCODE
	InitSemaphore(&SearchLock,1); 
	NameSem(&SearchLock,"SearchLock");

	InitSemaphore(&SearchTabLock,1); 
	NameSem(&SearchTabLock,"SearchTabLock");
#endif
	
	/* as this is set before Dispatch(), incoming requests will be */
	/* handled at this priority */
	SetPriority(HighServerPri);

	InitPool(&CodePool);
#ifdef __ARM
	EnterSVCMode();		/* Writing to root struct, so enter SVC mode. */
#endif
	GetRoot()->LoaderPool = &CodePool;
#ifdef __ARM
	EnterUserMode();	/* Finished writing to root struct. */
#endif
	InitNode( (ObjNode *)&Root, "loader", Type_Directory, 0, DefRootMatrix );
	InitList( &(Root.Entries) );
	Root.Nentries = 0;
	Root.Key = MagicKey;
	NameSem(&Root.Lock,"RootLock");
	LoaderDInfo.ReqPort = LoaderInfo.Port = NewPort();

	/* .. of root is a link to our machine root	*/
	{
		Object *o;
		LinkNode *Parent;

		MachineName(mcname);
		o = Locate(NULL,mcname);

		Parent = (LinkNode *)Malloc(sizeof(LinkNode) + (word)strlen(mcname));	
		InitNode( &Parent->ObjNode, "..", Type_Link, 0, DefLinkMatrix );
		NameSem(&Parent->ObjNode.Lock,"Parent");
		Parent->Cap = o->Access;
		strcpy(Parent->Link,mcname);
		Root.Parent = (DirNode *)Parent;

		nte = Create(o,"loader",Type_Name,sizeof(NameInfo),
			(byte *)&LoaderInfo);

		Close(o);
	}

	WaitLock(&Root.Lock,"Root,Lock");

	/* scan image vector and install all recognisable modules */
	for( j = 1; MWord_(ivec,j*sizeof(MPtr)) != 0; j++ )
	{
		word type;
		MPtr mm = MInc_(ivec,j*sizeof(MPtr));
		Carrier *c;
		char name[32];
	        m = MRTOA_(mm);

	        switch ( ModuleWord_(m,Type) )
	        {
	        case T_Module:  type = Type_Module;  break;
	        case T_Program: type = Type_Program; break;
	        case T_Device:  type = Type_Device;  break;
	        case T_DevInfo:	type = Type_File;    break;
	        default: continue;
	        }

		ModuleName_(name,m);

		/* @@@ note that any programs included in the nucleus */
		/* will have slightly erroneous sizes displayed by the */
		/* loader as any code stubs generated by the linker will */
		/* not be counted by procsize. */

		c = (Carrier *)AllocMem(sizeof(Carrier),&CodePool);
		c->Addr = m;
		c->Size = progsize(m)/sizeof(word);
		c->Type = 0;

		i = NewImage(&Root,c,name,type,0,-1, 0);

		if( type == Type_Program ) i->UseCount=1;
		i->Retain = true;
		i->ObjNode.Matrix &= ~(0x40404040);
	}


	/* use BindLibraries to set use counts */
	WalkList(&Root.Entries,bindfn);

	Signal(&Root.Lock);

	/* reply to procman that we have started */
	{
		MCB m;
		word e;
		InitMCB(&m,0,MyTask->Parent,NullPort,0x456);
		e = PutMsg(&m);
	}

#if CONTINUOUS_CHECKING
	Fork(2000, &checker_processor, 0);
#endif

#ifdef __C40
	/* The following is a rather grubby way of deciding whether	*/
	/* the loader should try to put the code in the global bus.	*/
	/* If ivec, which is the start of the Nucleus, is C addressable	*/
	/* then we put the code into the default memory area, otherwise	*/
	/* we put it in global memory.					*/

	if( C40CAddress(ivec) == 0 )
	{
		CodePoolType    = (word *)Malloc(sizeof(word)*4);
		CodePoolType[0] = RAMType_Indirect|RAMType_Global|RAMType_Dynamic;
		CodePoolType[1] = RAMType_Indirect|RAMType_Global|RAMType_Static;
		CodePoolType[2] = RAMType_Indirect|RAMType_Local|RAMType_Dynamic;
		CodePoolType[3] = 0;
		
	}
#endif


	Dispatch(&LoaderDInfo);

	Delete(nte,NULL);

	Close(nte);
}

word bindfn(Image *i)
{
	BindLibraries(i->Image,TRUE);
	return 0;
}

/*--------------------------------------------------------
-- BLV, a thread which checks every piece of loaded	--
-- code at regular intervals.				--
--------------------------------------------------------*/

#if CONTINUOUS_CHECKING
static void checker_process()
{ ObjNode *f;

  forever
   { Delay(10 * OneSec);
     Wait(&Root.Lock);
     for (f = Head_(ObjNode, Root.Entries); 
     	  !EndOfList_(f);
     	  f = Next_(ObjNode, f) )
      check((Image *) f);
     Signal(&Root.Lock);
   }
}
#endif

/*--------------------------------------------------------
-- NewImage						--
--							--
-- Add an image entry to the directory			--
--							--
--------------------------------------------------------*/

Image *NewImage(DirNode *dir, Carrier *image, string name, word type, word flags,
	Matrix matrix, word size)
{
	Image *i = New(Image);
debug(("NewImage %s i %x[%x] s %d",name,image,image->Addr,size));
	if( i == Null(Image) ) return Null(Image);

	matrix &= (type==Type_Module)?DefModuleMatrix:DefProgMatrix;

	InitNode(&i->ObjNode,name,(int)type,(int)flags,matrix);
	NameSem(&i->ObjNode.Lock,i->ObjNode.Name);
	i->ObjNode.Key = MagicKey;
	
	i->UseCount 	  = 0;
	i->Retain 	  = false;
	i->Closing 	  = false;
	i->ImageCarrier   = image;
	i->Image	  = image->Addr;
	
	/* if we have no image header, work out the size by hand */
	i->ObjNode.Size = (size == 0) ? progsize(i->Image) : size;
	i->ObjNode.Size += sizeof(ImageHdr);

	i->Flags = 0;
	i->CheckSum = checksum(i->Image, i->ObjNode.Size - sizeof(ImageHdr));	
	i->Object = NULL;
	
	Insert( dir, &i->ObjNode, TRUE );

	return i;
}

word progsize(MPtr m)
{
	word size = 0;

	while( MWord_(m,0) != 0 )
	{
		size += ModuleWord_(m,Size);
		m = ModuleNext_(m);
	}

	return size + 4;
}

word checksum(MPtr p, word size)
{
	word csum = 0;
	word i;
#ifdef __TRAN
	word *pp = (word *)p;
	for( i = 0 ; i < size/sizeof(word) ; i++ ) csum = sum_(csum,*pp++);
#else
	for( i = 0 ; i < size ; i+=4 )
		csum = (word)((unsigned long)csum + (unsigned long)MWord_(p,i));
#endif
	return csum;
}

void check(Image *i)
{
/* return; */
	if( i->ObjNode.Type == Type_CacheName ) return;
	if( i->CheckSum != checksum(i->Image, i->ObjNode.Size - sizeof(ImageHdr)) )
	{
		IOdebug("WARNING: Corruption of %s detected in %s",i->ObjNode.Name,mcname);
		i->Retain = false;
	}
}	

Image *makelink(DirNode *d, string lname, string name, Capability *cap)
{
	Image *i;
	Object *o;

	o = NewObject(name,cap);

	if( o == NULL ) return NULL;
		
	i = New(Image);

	if( i == NULL ) { Close(o); return NULL; }
	
	memset(i,0,sizeof(Image));
	
	InitNode(&i->ObjNode,lname,Type_CacheName,0,DefProgMatrix);
	NameSem(&i->ObjNode.Lock,i->ObjNode.Name);	
	i->ObjNode.Key = MagicKey;
	
	i->Object = o;

	Insert( d, &i->ObjNode, TRUE );

	return i;
}

static int strlcmp(char *s1, char *s2)
{
	forever
	{
		char c1 = *s1;
		char c2 = *s2;
		if( 'A' <= c1 && c1 <= 'Z' ) c1 += ('a'-'A');
		if( 'A' <= c2 && c2 <= 'Z' ) c2 += ('a'-'A');		
		if( c1 != c2 ) return c1-c2;
		if( c1 == 0 ) return 0;
		s1++,s2++;
	}
}

static bool FindImage(Image *i, char *name )
{
#if 1
	char *oname = objname(name);
	return (strlcmp(oname,i->ObjNode.Name)==0);
#else
	int nlen;
	char *oname;
	int olen;

	/* if there is no Object, compare last item with node name */
	
	if( i->Object != NULL ) oname = i->Object->Name;
	else return strcmp(i->ObjNode.Name,objname(name))==0;

	nlen = strlen(name);
	olen = strlen(oname);
	
	/* one string is a complete substring of the other, then we	*/
	/* assume they reference the same object.			*/
	
	while( olen && nlen )
		if( oname[olen--] != name[nlen--] ) return false;
		
	return true;
#endif
}

Image *LookupImage( char *name )
{
	return (Image *)SearchList(&Root.Entries,FindImage,name);
}

extern ObjNode *GetTargetObj1(ServInfo *servinfo)
{
	DirNode *d = (DirNode *)servinfo->Target;
	ObjNode *o = Null(ObjNode);
	MCB *m = servinfo->m;
	IOCCommon *req = (IOCCommon *)(m->Control);
	byte *data = m->Data;
	word next = req->Next;
	word len = 0;
	char name[NameMax];
	char *pathname = servinfo->Pathname;

	/* If the context is the target we have artificially stepped	*/
	/* back to its parent. Step back down here, except if the	*/
	/* context is the root, when we have not stepped back.		*/
	if( next == -1 || data[next] == '\0' )
	{
		/* if target is root then context == target here */
		if( servinfo->Context == (DirNode *)servinfo->Target ) 
		{ LockTarget(servinfo); o = servinfo->Target; goto found; }

		/* else step on down to context */
		WaitObj(servinfo->Context);

		o = servinfo->Target = (ObjNode *)servinfo->Context;
		goto found;
	}

	if( (len = splitname(name, c_dirchar, &data[next] )) == 0 )
	{
		m->MsgHdr.FnRc |= EC_Error+EG_Name;
		return NULL;
	}

	/* special case . and .. */
	if( name[0] == '.' && name[1] == '\0' )
	{
		req->Next = next+len;		
		return (ObjNode *)d;
	}
	elif( name[0] == '.' && name[1] == '.' && name[2] == '\0') 
	{
		int l = strlen(pathname);
		while( pathname[l--] != c_dirchar );
		pathname[l+1] = '\0';
		o = (ObjNode *)(d->Parent);
		servinfo->Target = (ObjNode *)o;
		WaitObj(o);
		req->Access.Access = UpdMask(req->Access.Access,o->Matrix);
		goto found;
	}
	else 
	{
		o = (ObjNode *)Lookup( d, name, TRUE );
		pathcat(pathname, name );
	}

	if( o == NULL )
	{
		/* if the required object is not found, we keep its	*/
		/* parent dir locked and set as the target.		*/
		m->MsgHdr.FnRc |= EC_Error+EG_Name;
		return NULL;
	}

	req->Access.Access = UpdMask(req->Access.Access,o->Matrix);

	/* lock o and set as target */
	WaitObj( o );
	servinfo->Target = o;

	/* This is general code for all above variations. We come here	*/
	/* with o locked.						*/
found:
	req->Next = next+len;
	
	return servinfo->Target = o;
}

/*--------------------------------------------------------
-- Action Procedures					--
--							--
--------------------------------------------------------*/

static void do_open(ServInfo *servinfo)
{
	MCB *m = servinfo->m;
	MsgBuf *r;
	Image *i;
	DirNode *d;
	IOCMsg2 *req = (IOCMsg2 *)(m->Control);
	Port reqport;
	byte *data;
	word mode = req->Arg.Mode;
	string pathname = servinfo->Pathname;
	word e;

	r = New(MsgBuf);

	if( r == Null(MsgBuf) )
	{
		ErrorMsg(m,EC_Error+EG_NoMemory);
		return;
	}

	d = GetTargetDir(servinfo);

	if( d == Null(DirNode) )
	{
		ErrorMsg(m,Err_Null);
		return;
	}
		
	i = (Image *)GetTargetObj1(servinfo);

	if( i == Null(Image) )
	{
		ErrorMsg(m,Err_Null);
		Free(r);
		return;
	}

	CheckLock("do_open",&Root);
	CheckLock("do_open",i);
	
	/* here we have i locked */		

	unless( CheckMask(req->Common.Access.Access,(AccMask)(req->Arg.Mode & Flags_Mode)) ) 
	{
		ErrorMsg(m,EC_Error+EG_Protected+EO_Program);
		return;
	}


	if( i->ObjNode.Type == Type_Directory )
	{
		/* the only directory we have is the root, hence must not*/
		/* unlock it here					 */
		reqport = NewPort();
		FormOpenReply(r,m,&i->ObjNode,Flags_Closeable, pathname);
		r->mcb.MsgHdr.Reply = reqport;
		PutMsg(&r->mcb);
		Free((void *)r);
		DirServer(servinfo,m,reqport);
		FreePort(reqport);
		return;
	}
	elif( i->ObjNode.Type == Type_CacheName )
	{
		word r;

		/* We must do this with the root locked since we may	*/
		/* need to load some more libraries & do not want to	*/
		/* clash with other loads.				*/

		r = LoadCache(i);

		Signal(&Root.Lock);
		
		if( !r )
		{
			ErrorMsg(m,EC_Error+EG_Create+EO_Program);
			return;
		}
	}
	else Signal(&Root.Lock);

	switch( req->Arg.Mode & Flags_Mode )
	{
	case O_Execute:
		FormOpenReply(r,m,&i->ObjNode,Flags_Closeable,pathname);
		MarshalWord(&r->mcb,(word)i->Image);
		i->UseCount++;
		e = PutMsg(&r->mcb);
		Free((void *)r);
		return;

	case O_ReadOnly:
	case O_Create|O_WriteOnly:
		reqport = NewPort();
		FormOpenReply(r,m,&i->ObjNode,Flags_Closeable,pathname);
		r->mcb.MsgHdr.Reply = reqport;
		i->UseCount++;
		PutMsg(&r->mcb);
		Free((void *)r);
		break;


	default:
		ErrorMsg(m,EC_Error+EG_WrongFn);
		Free((void *)r);
		return;
	}

	/* Here we are reading the module/program, possibly for	*/
	/* shipment to another processor. This file may only be	*/
	/* read serially, and it is prepended by an image header*/
	/* to allow it to be read in by another loader.		*/
	
	data = m->Data;

	UnLockTarget(servinfo);
	
	forever
	{
		m->MsgHdr.Dest = reqport;
		m->Timeout = StreamTimeout;
		m->Data = data;

		e = GetMsg(m);
#if TIMEOUTS
		if( e == EK_Timeout ) break;
#endif
		if( e < Err_Null ) continue;

		WaitObj( &i->ObjNode );

		i->ObjNode.Dates.Access = GetDate();

		switch( m->MsgHdr.FnRc & FG_Mask )
		{
		case FG_Read:
			if( mode != O_ReadOnly ) goto moderror;
			ReadImage(m,i);
			break;
#if 0		
		case FG_Write:
			if( mode != O_WriteOnly ) goto moderror;
			WriteImage(m,i);
			break;
		
#endif		
		case FG_Close:
			if( m->MsgHdr.Reply != NULL ) ErrorMsg(m,Err_Null);
			FreePort(reqport);
			i->UseCount--;
			Signal( &i->ObjNode.Lock );
			return;

		case FG_GetSize:
		{
			InitMCB(m,0,m->MsgHdr.Reply,NullPort,Err_Null);
			MarshalWord(m,i->ObjNode.Size);
			PutMsg(m);
			break;
		}

		case FG_Seek:
		case FG_SetSize:
			ErrorMsg(m,EC_Error+EG_WrongFn+EO_Module);
			break;
		default:
		moderror:
			ErrorMsg(m,EC_Error+EG_FnCode+EO_Module);
			break;
		}
		Signal( &i->ObjNode.Lock );
	}
}

static void do_create(ServInfo *servinfo)
{
	MCB *m = servinfo->m;
	MsgBuf *r;
	DirNode *d;
	Image *i;
	MPtr p;
	Carrier *c;
	IOCCreate *req = (IOCCreate *)(m->Control);
	LoadInfo *info = (LoadInfo *)&(m->Data[req->Info]);
	string name;
	char *pathname = servinfo->Pathname;

	d = (DirNode *)GetTarget(servinfo);

	if( d != &Root )
	{
		ErrorMsg(m,EO_Program);
		return;
	}

	/* here we have d (==Root) locked */

	CheckLock("do_create",&Root);

	unless( CheckMask(req->Common.Access.Access,AccMask_W) ) 
	{
		ErrorMsg(m,EC_Error+EG_Protected+EO_Directory);
		return;
	}

	name = objname( info->Name );

	i = (Image *)LookupImage( info->Name );

	if( i != NULL ) 
	{
		/* lock both object and root to avoid clashes	*/
		
		WaitObj( &i->ObjNode );
		if( i->ObjNode.Type == Type_CacheName )
		{
			word r;
			
			r = LoadCache(i);
			
			if( !r )
			{
				ErrorMsg(m,EC_Error+EG_Create+EO_Program);
				Signal(&i->ObjNode.Lock);
				return;
			}
		}

		check(i);
		
		p = i->Image;
		Signal(&i->ObjNode.Lock);
	}
	else
	{
		Object *o = NULL;
		word type;
		word size;

#if SEARCHCODE		
		i = SearchLoad(objname(info->Name));
		if( i != NULL )
		{
			p = i->Image;
			goto gotimage;
		}
#endif		
		c = LoadImage(info->Name, &info->Cap, info->Pos, true, &o,
			&size);

		if( c == NULL ) 
		{
			ErrorMsg(m,EC_Error+EG_Create+EO_Program);
			if (o != NULL) Close(o);
			return;
		}
		p = (MPtr)c->Addr;

		switch( ModuleWord_(p,Type) )
		{
		case T_Module	: type = Type_Module; break;
		case T_Program	: type = Type_Program; break;
		case T_Device	: type = Type_Device; break;
		default:
			ErrorMsg(m,EC_Error+EG_Invalid+EO_Program);
			FreeCode(c);
			return;
		}
		i = NewImage(d,c,name,type,0,info->Matrix, size);
		i->Object = o;
	}
	
#if SEARCHCODE
gotimage:
#endif
	pathcat( pathname, name );

	if( i == Null(Image) )
	{
		ErrorMsg(m,EC_Error+EG_Create+EO_Program);
		return;
	}

	r = New(MsgBuf);

	if( r == Null(MsgBuf) )
	{
		ErrorMsg(m,EC_Error+EG_NoMemory);
		return;
	}

	/* give creator full rights */
	req->Common.Access.Access = AccMask_Full;
	
	FormOpenReply(r,m,&i->ObjNode, 0, pathname);

	PutMsg(&r->mcb);

	i->ObjNode.Dates.Access = GetDate();

	Free(r);
}

static void do_delete(ServInfo *servinfo)
{
	MCB *m = servinfo->m;
	IOCCommon *req = (IOCCommon *)(m->Control);
	Image *i;
	

	i = (Image *)GetTarget(servinfo);
	
	if( i == NULL )
	{
		ErrorMsg(m,EO_Module);
		return;
	}
	
	unless( CheckMask(req->Access.Access,AccMask_D) ) 
	{
		ErrorMsg(m,EC_Error+EG_Protected+EO_Module);
		return;
	}

	if( i->ObjNode.Type == Type_Directory )
	{
		ErrorMsg(m,EC_Error+EG_Delete);
		return;
	}

	i->Retain = false;

	/* Reply to user now, so rest is done in parallel		*/
	
	ErrorMsg(m,Err_Null);

	check(i);
	
	servinfo->TargetLocked = false;
	
	DestroyImage(i);		
}

static void do_link(ServInfo *servinfo)
{
	MCB *m = servinfo->m;
	IOCMsg3 *req = (IOCMsg3 *)(m->Control);
	byte *data = m->Data;
	DirNode *d;
	ObjNode *o;
	Image *i;
	char *pathname = servinfo->Pathname;

	
	d = GetTargetDir(servinfo);

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

	o = (ObjNode *)GetTargetObj(servinfo);

	if( o != NULL )
	{
		if( o->Type == Type_CacheName )
		{
			ErrorMsg(m,EC_Error+EG_Create);
			return;
		}
		else {
			/* If it exists and is not a CacheName, simply make */
			/* sure it stays loaded!!			    */
			/* This only usually happens if we are trying to    */
			/* cache the ln command!!			    */
			((Image *)o)->Retain = true;
			ErrorMsg(m,Err_Null);
			return;
		}
	}
	else m->MsgHdr.FnRc = servinfo->DispatchInfo->SubSys;
	
	/* We now know that there is not an existing entry with the	*/
	/* desired name. Install the link.				*/

	i = makelink(d,objname(pathname),&data[req->Name],&req->Cap);

	if( i == NULL )
	{
		ErrorMsg(m,EC_Error+EG_NoMemory);
		return;
	}

	ErrorMsg(m, Err_Null);	
}

static void do_closeobj(ServInfo *servinfo)
{
	MCB *m = servinfo->m;
	Image *i;

	i = (Image *)GetTarget(servinfo);

	if( i == NULL )
	{
		ErrorMsg(m,EC_Error|EO_Program);
		return;
	}
	else ErrorMsg(m,Err_Null);

	check(i);
	
	i->UseCount--;

	servinfo->TargetLocked = false;

	DestroyImage(i);
}

void DestroyImage(Image *i)
{
	if( i->Closing ) goto done;
debug(("DestroyImage %s",i->ObjNode.Name));
	while( i->UseCount == 0 && !i->Retain )
	{

	/* We ensure that we do not delete the object until at least 5	*/
	/* seconds after it was last touched. This avoids the problem	*/
	/* of locating a program in the loader, only to fail on the open*/

		int time = GetDate()-i->ObjNode.Dates.Access;

		i->Closing = true;
		
		if( time < 5 )
		{
			time = 5 - time;
			Signal(&i->ObjNode.Lock);
			Delay(OneSec*time);
			WaitObj( &i->ObjNode );
			i->Closing = false;
			continue;
		}

		/* To avoid deadlocks, we must relinquish the object	*/
		/* lock to get the root, then the object again. After	*/
		/* this we must re-test the conditions on which we have	*/
		/* decided to kill the image.				*/
		
		Signal(&i->ObjNode.Lock);
		WaitLock(&Root.Lock,"Root.Lock");
		WaitObj(&i->ObjNode);
		
		if( i->UseCount > 0 || i->Retain || 
		    (GetDate()-i->ObjNode.Dates.Access) < 5)
		{
			Signal(&Root.Lock);
			i->Closing = false;
			continue;
		}	

		if( i->ObjNode.Type == Type_CacheName )
		{
			Unlink(&i->ObjNode,TRUE);
			Signal(&Root.Lock);
			Close(i->Object);
			Free(i);
			return;
		}

		UnbindLibraries(i->Image);

#ifdef __SMT
		/* If removing resident library, remove its shared code */
		/* pointer table as well. */
		if( ModuleWord_(i->Image,Type) == T_Module ) {
			RootStruct *root = GetRoot();

# ifdef __ARM
			/* Writing to root struct, so enter SVC mode. */
			EnterSVCMode();
# endif
			Wait(&root->cpi_op);
			/* free modules shared code pointer table */
# ifdef __C40
			FreeMem((void*)(C40CAddress(root->cpi[ModuleWord_(i->Image,Id)])));
# else
			FreeMem((void*)(root->cpi[ModuleWord_(i->Image,Id)]));
# endif
			root->cpi[ModuleWord_(i->Image,Id)] = NULL;
			Signal(&root->cpi_op);
# ifdef __ARM
			/* Finished writing to root struct, so re-enter */
			/* user mode. */
			EnterUserMode();
# endif
		}
#endif

		FreeCode(i->ImageCarrier);
		Unlink(&i->ObjNode,TRUE);
		Signal(&Root.Lock);
		Close(i->Object);
		Free(i);
		return;		
	}
done:
	Signal(&i->ObjNode.Lock);
}

/*--------------------------------------------------------
-- ReadImage						--
--							--
-- Process a request to read data from a loaded image.	--
--							--
--------------------------------------------------------*/

void ReadImage(MCB *m, Image *i)
{
	ReadWrite *r = (ReadWrite *)(m->Control);
	word pos = r->Pos;
	word size = r->Size;
	word isize = i->ObjNode.Size;
#if READBUFFER
	char *buffer;
	int readsize;

	for
	(
		readsize = READSIZE, buffer = NULL;
		readsize > MINREADSIZE && buffer == NULL;
		readsize -= MINREADSIZE
	) buffer = (char *)Malloc(readsize);

	if( buffer == NULL ) goto done;

	i->Buf.Data = buffer;
	i->Buf.Max = readsize;
#endif
debug(("ReadImage %s pos %d size %d isize %d",i->ObjNode.Name,pos,size,isize));
	if( pos < 0 || pos > isize )
	{
		ErrorMsg(m,EC_Error+EG_Parameter+1);
		goto done;
	}
	if( pos == isize )
	{
		m->MsgHdr.FnRc = ReadRc_EOF;
		ErrorMsg(m,0);
		goto done;
	}

	if( pos + size > isize ) r->Size = isize - pos;

	DoRead(m,ImageReadBuffer,i);

done:

#if READBUFFER
	if( buffer ) Free(buffer);
#endif
	return;
}

#if READBUFFER
Buffer *ImageReadBuffer(word pos, Image *i)
{
	Buffer *b = &i->Buf;
	word dsize;
debug(("ImageReadBuffer pos %d",pos));	

	if( pos == 0 )
	{
		ImageHdr *h = (ImageHdr *)b->Data;
		b->Pos = 0;
		b->Size = sizeof(ImageHdr);
		h->Magic = Image_Magic;
		h->Flags = i->Flags;
		h->Size = i->ObjNode.Size-sizeof(ImageHdr);
		return b;
	}

	if( pos < sizeof(ImageHdr) || pos >= i->ObjNode.Size ) return NULL;

	dsize = i->ObjNode.Size - pos;
	if( dsize > b->Max ) dsize = b->Max;
debug(("ReadBuffer pos %d dsize %d",pos,dsize));
	b->Pos = pos;
	b->Size = dsize;

	MData_(b->Data,i->Image,pos-sizeof(ImageHdr),dsize);

	return b;
}	
#else
Buffer *ImageReadBuffer(word pos, Image *i)
{
	Buffer *b = &i->Buf;
debug(("ImageReadBuffer pos %d",pos));	
	if( pos == 0 )
	{
		ImageHdr *h = &i->Hdr;
		b->Pos = 0;
		b->Size = b->Max = sizeof(ImageHdr);
		b->Data = (byte *)h;
		h->Magic = Image_Magic;
		h->Flags = i->Flags;
		h->Size = i->ObjNode.Size-sizeof(ImageHdr);
		return b;
	}
	
	if( pos < sizeof(ImageHdr) || pos >= i->ObjNode.Size ) return NULL;

	b->Pos = sizeof(ImageHdr);
	b->Size = b->Max = i->ObjNode.Size - sizeof(ImageHdr);
	b->Data = (char *)i->Image;
	return b;		
}
#endif

#if 0
/*--------------------------------------------------------
-- WriteImage						--
--							--
-- Write an image file into memory. 			--
--							--
--------------------------------------------------------*/

void WriteImage(MCB *m, Image *i)
{
	ReadWrite *r = (ReadWrite *)m->Control;
	word pos = r->Pos;
	word size = r->Size;
	
	if( pos < 0 )
	{
		ErrorMsg(m,EC_Error|EG_Parameter|1);
		return;
	}

	DoWrite(m,ImageWriteBuffer,i);
}

Buffer *ImageWriteBuffer(word pos, Image *i)
{
	Buffer *b = &i->Buf;
	
	if( pos == 0 )
	{
		ImageHdr *h = &i->Hdr;
		b->Pos = 0;
		b->Size = sizeof(ImageHdr);
		b->Max = 4096;
		b->Data = (byte *)h;
		return b;
		
	}
}
#endif
/*--------------------------------------------------------
-- LoadImage						--
--							--
-- Read an image file into memory. 			--
--							--
--------------------------------------------------------*/

Carrier *LoadImage(string name, Capability *cap, word pos, int bindp,
	Object **objp, word *size)
{
	Object *o = NULL;
debug(("LoadImage %s",name));
	CheckLock("LoadImage",&Root);

	o = NewObject(name,cap);
	
	if( o == NULL ) return NULL;

	*objp = o;
	
	return LoadObject(o, pos, bindp, size);
}

Carrier *LoadObject(Object *o, word pos, bool bindp, word *size)
{
	Stream *s;
	Carrier *image = NULL;
debug(("LoadObject %O",o));
	CheckLock("LoadObject",&Root);

	s = Open(o,NULL,O_ReadOnly);

	if( s == NULL ) return NULL;

	/* if necessary, seek to given file posn */
	if( pos != 0 ) Seek(s,S_Beginning,pos);

	if( (image = ReadStream(s, size)) == NULL ) goto done;

#ifdef __SMT
	if( ModuleWord_(image->Addr,Type) == T_Module )
		if( !AllocSMT(image->Addr) )
		{
			FreeCode(image);
			return NULL;
		}
#endif
	
	if( bindp && !BindLibraries((MPtr)image->Addr,FALSE) )
	{
		FreeCode(image);
		image = NULL;
	}

done:
	Close(s);

	return image;
}

Carrier *ReadStream(Stream *s, word *retsize)
{
	ImageHdr hdr;
	Carrier *c = NULL;
	word size;
	char *buffer = NULL;
	word readsize = READSIZE;
#if READBUFFER
	MPtr image;
#endif

debug(("ReadStream %S",s));
#if READBUFFER
	for
	(
		readsize = READSIZE;
		readsize > MINREADSIZE && buffer == NULL;
		readsize -= MINREADSIZE
	) buffer = (char *)Malloc(readsize);

	if( buffer == NULL ) goto done;
#endif
	
	size = Read(s,(byte *)&hdr,sizeof(ImageHdr),-1);

	if( size != sizeof(ImageHdr) || hdr.Magic != Image_Magic ) 
		goto done;

	c = AllocCode(hdr.Size);

	if( c == NULL ) goto done;
	
#if !READBUFFER
	/* If we are not using a read buffer, then the code must be	*/
	/* directly addressable, so the following assignment is OK.	*/
	
	buffer = c->Addr;
#else
	/* Otherwise, get the MPtr so we can copy to it		*/

	image = c->Addr;
#endif

	/* return true size of image to caller */
	*retsize = hdr.Size;

	{
		word got = 0;
		word dsize = readsize - sizeof(hdr);
		while( got != hdr.Size )
		{
			if( dsize > hdr.Size - got ) dsize = (int)hdr.Size - got;
			size = Read(s,buffer,dsize,-1);
			if( size <= 0 ) break;

#if READBUFFER
			SetMData_(image,got,buffer,dsize);
#else
			buffer += dsize;
#endif

			got += dsize;

			/* The first read adjusts us to a READSIZE boundary */
			/* READSIZE should be a multiple of the disc block  */
			/* size (a multiple of 4k should usually be OK)	    */
			dsize = readsize;
		}
		size = got;
	}

	if( size != hdr.Size ) 
	{
debug(("ReadStream failed"));
		FreeCode(c);
		c = NULL;
	}

done:
#if READBUFFER
	if( buffer != NULL ) Free(buffer);
#endif
	return c;
}

/*--------------------------------------------------------
-- AllocCode						--
-- FreeCode						--
--							--
-- Code space management. On some systems the code is	--
-- kept in a seperate pool to the data. This is handled	--
-- here.						--
--							--
--------------------------------------------------------*/

word InitCodeSpace()
{
	return 1;
}

Carrier *AllocCode(word size)
{
	Carrier *image = NULL;

#ifdef __C40
	if( CodePoolType != NULL )
	{
		int i;
		word wsize = (size + sizeof(word)-1)/sizeof(word);
		for( i = 0; CodePoolType[i] != 0; i++ )
		{
			image = AllocSpecial(wsize,CodePoolType[i],&CodePool);
			if( image != NULL ) return image;
		}
	}
	if( image == NULL )
#endif
	{
		void *p = AllocMem(size,&CodePool);
		if( p == NULL ) return NULL;

		image = (Carrier *)AllocMem(sizeof(Carrier),&CodePool);
		if( image == NULL ) { FreeMem(p); return NULL; }

		image->Addr = CtoM_(p);
		image->Size = (size + sizeof(word)-1)/sizeof(word);
		image->Type = 0;

	}

	return image;

}

void FreeCode(Carrier *c)
{
#ifdef __C40
	Memory *mem = (Memory *)(((char *)c)-sizeof(Memory));

	if( mem->Size & Memory_Size_Carrier )
	{
		FreeMem(c);
		return;
	}
#endif

	FreeMem(MtoC_(c->Addr));
	FreeMem(c);
}

/*--------------------------------------------------------
-- BindLibraries					--
--							--
-- Bind any ResRef library references to the named	--
-- library if it exists.				--
--							--
--------------------------------------------------------*/

word BindLibraries(MPtr image, bool readonly)
{
	MPtr	m			= image;
	bool	resident_library	= (ModuleWord_(m,Type) == T_Module);
	
debug(("BindLibraries %x",image));	
	CheckLock("BindLibraries",&Root);

	while( ModuleWord_(m,Type) != 0 )
	{
		if( ModuleWord_(m,Type) == T_ResRef )
		{
			bool loaded = false;
			char name[32];
			Image *i;

			ModuleName_(name,m);

			i = (Image *)Lookup(&Root,name,TRUE);

#if SEARCHCODE			
			if( i == NULL )	i = SearchLoad(name);
#endif
			if( i == NULL ) i = LoadLibrary(name);
			if( i == NULL ) return false;		
			
			WaitObj(&i->ObjNode);
			
			if ( i->ObjNode.Type != Type_CacheName ) loaded = true;
			else loaded = LoadCache(i);

			if( !readonly )
{
				SetResRefModule_(m,(word)i->Image);
/* #define SetResRefModule_(m,v) SetMWord_(m,offsetof(ResRef,Module),v)
#define SetMWord_(mp,offset,val)	((*((word *)((mp)+(offset)))) = val)
*/
if ((*((word *)((m)+(offsetof(ResRef, Module))))) != (word)i->Image)
  IOdebug("SetResRefModule failed");
}

			/* BLV - checking loaded resident libraries when binding	*/
			/* a resident library is expensive. The kernel gets checked	*/
			/* many times for every taskforce component. Instead the check	*/
			/* should only be done for real user programs.			*/
			if (!resident_library)
				check(i);
			
			i->UseCount++;
				
			Signal(&i->ObjNode.Lock);
			if( !loaded ) return false;
		}
		m = ModuleNext_(m);
	}

	return true;
}

void UnbindLibraries(MPtr image)
{
	MPtr	m			= image;
	bool	resident_library	= (ModuleWord_(m,Type) == T_Module);

debug(("UnbindLibraries %x",image));	
	CheckLock("UnbindLibraries",&Root);
	
	while( ModuleWord_(m,Type) != 0 )
	{
		if( ModuleWord_(m,Type) == T_ResRef )
		{
			char name[32];
			Image *i;

			ModuleName_(name,m);

			i = (Image *)Lookup(&Root,name,TRUE);
			
			WaitObj(&i->ObjNode);

			i->UseCount--;

			/* BLV - UnbindLibraries is recursive and can lead to a	*/
			/* the kernel and syslib being checked many times. This	*/
			/* test avoids some of the overheads.			*/
			if (!resident_library)			
				check(i);

			if( i->UseCount == 0 && !i->Retain )
			{
#ifdef __SMT
				RootStruct *root = GetRoot();

# ifdef __ARM
				/* Writing to root struct, so enter SVC mode. */
				EnterSVCMode();
# endif
				Wait(&root->cpi_op);
				/* free modules shared code pointer table */
# ifdef __C40
				FreeMem((void*)(C40CAddress(root->cpi[ModuleWord_(m,Id)])));
# else
				FreeMem((void*)(root->cpi[ModuleWord_(m,Id)]));
# endif
				root->cpi[ModuleWord_(m,Id)] = NULL;
				Signal(&root->cpi_op);
# ifdef __ARM
				/* Finished writing to root struct, so */
				/* re-enter user mode. */
				EnterUserMode();
# endif
#endif
				UnbindLibraries(i->Image);
				FreeCode(i->ImageCarrier);
				Unlink(&i->ObjNode,TRUE);
				Close(i->Object);
				Free(i);		
			}
			Signal(&i->ObjNode.Lock);
			
		}
		m = ModuleNext_(m);
	}
}

#ifdef __SMT
#define CPISLOTINC	32	/* minimum granularity of CPI size increases */

/* Enlarge the size of the shared code pointer table index */
bool EnlargeCPI(int maxid)
{
	RootStruct *root = GetRoot();
	word newsize = (word)maxid + CPISLOTINC;
	word *newCPI = (word *)AllocMem(newsize * sizeof(word), &root->SysPool);

#ifdef SYSDEB
	IOdebug("Enlarging SMT Code Pointer Index table (%x->%x)",root->cpislots,newsize);
#endif
	if(newCPI == NULL)
		return FALSE;

#ifndef __TRAN
	/* Tran alloc automatically zeros memory */
	{
		word *newslotp = newCPI + root->cpislots;
		word i;

		for(i = newsize - root->cpislots; i > 0; i--)
			newslotp[i-1] = NULL;
	}
#endif

	/* root->cpi_op semaphore is assumed to be in use! */
	memcpy((void *)newCPI, (void *)(root->cpi), (int)root->cpislots * sizeof(word));
	FreeMem((void *)(root->cpi));

	root->cpislots = newsize;
	root->cpi = newCPI;

	return TRUE;
}

bool AllocSMT(MPtr image)
{
	RootStruct *root = GetRoot();
	word id = ModuleWord_(image,Id);

#ifdef __ARM
	/* Writing to root struct, so enter SVC mode. */
	EnterSVCMode();
#endif

	Wait(&root->cpi_op);

	if(id >= root->cpislots)
		if (!EnlargeCPI( (int)id)) {
			Signal(&root->cpi_op);
#ifdef __ARM
			/* Finished writing to root struct, so re-enter user mode. */
			EnterUserMode();
#endif
			return FALSE;
		}

	if (root->cpi[id] != 0)
#ifdef SYSDEB
		IOdebug("SMT error: Duplicate Shared code pointer index (%d, %s)", id, ((Module *)image)->Name);
#else
		IOdebug("ERROR: internal error in loader");
#endif			

#ifdef __C40
	if ((root->cpi[id] = C40WordAddress(AllocMem(ModuleWord_(image,MaxCodeP), &root->SysPool))) == NULL) {
		Signal(&root->cpi_op);
		return FALSE;
	}
#else
	if ((root->cpi[id] = (word)AllocMem(ModuleWord_(image,MaxCodeP), &root->SysPool)) == NULL) {
		Signal(&root->cpi_op);
# ifdef __ARM
		/* Finished writing to root struct, so re-enter user mode. */
		EnterUserMode();
# endif
		return FALSE;
	}
#endif

	/* Note that this shared code pointer table needs initialising */
	root->cpi[id] |= 1;
		
	Signal(&root->cpi_op);

#ifdef __ARM
	/* Finished writing to root struct, so re-enter user mode. */
	EnterUserMode();
#endif
	return TRUE;
}

#endif

Image *LoadLibrary(string name)
{
	Image *i = NULL;
	MPtr image;
	Carrier *c;
	Stream *s;
	Object *o = NULL;
#ifdef __TRAN
	char xname[100];
#endif
	char lname[NameMax];
	bool lcdone = FALSE;
	word size;
debug(("LoadLibrary %s",name));
	CheckLock("LoadLibrary",&Root);

	strcpy(lname,name);
	
	if( LibDir == NULL ) LibDir = Locate(NULL,"/helios/lib");

again:
	if( o == NULL ) o = Locate(LibDir,lname);

#ifdef __TRAN
	if( o == NULL )
	{
		char *suffix;
		int t = MachineType();

		strcpy(xname,lname);

		switch( t )
		{
		case 801:
		case 805:
		case 800: suffix = ".t8"; break;
		default:  IOdebug("Unknown processor type %d, T4 assumed",t);
		case 400:
		case 414:
		case 425: suffix = ".t4"; break;		
		}

		strcat(xname,suffix);

		o = Locate(LibDir,xname);
	}
#endif

	if( o == NULL && !lcdone)
	{
		char *p;
		for( p = lname; *p; p++ )
			if( 'A' <= *p && *p <= 'Z' ) *p = *p - 'A' + 'a';
		lcdone = TRUE;
		goto again;
	}

	if( o == NULL ) return NULL;

	s = Open(o,NULL,O_ReadOnly);
	
	if( s == NULL ) { Close(o); return NULL; }
	
	if( (c = ReadStream(s, &size)) == NULL ) goto done;

	image = (MPtr)c->Addr;
	
#ifdef __SMT
	if( !AllocSMT(image) )
	{
		FreeCode(c);
		return NULL;
	}
#endif
	i = NewImage(&Root, c, name, Type_Module, 0, -1, size);

	WaitObj(&i->ObjNode);
	BindLibraries(image,FALSE);
	
	i->Retain = true;

	i->CheckSum = checksum(image, i->ObjNode.Size - sizeof(ImageHdr));
	i->Object = o;
			
	Signal(&i->ObjNode.Lock);
done:
	Close(s);

	return i;

}

word LoadCache(Image *i)
{
	word type;
	Matrix matrix;
	Carrier *c;
	MPtr p;
	Object *o = i->Object;
	word size;
debug(("LoadCache %s",i->ObjNode.Name));
	CheckLock("LoadCache",i);
	CheckLock("LoadCache",&Root);
	
	c = LoadObject(o, 0, true, &size);

	if( c == NULL ) return false;

	p = (MPtr)c->Addr;
	
	switch( ModuleWord_(p,Type) )
	{
	case T_Module:
		type = Type_Module;
		matrix = DefModuleMatrix;
		break;
	case T_Program:
		type = Type_Program;
		matrix = DefProgMatrix;
		break;
	case T_Device:
		type = Type_Device;
		matrix = DefProgMatrix;
		break;
	default:
		return false;
	}

	i->ObjNode.Type = type;
	i->ObjNode.Matrix = matrix;
	i->UseCount = 0;
	i->Retain = true;
	i->Image = p;
	i->Flags = 0;

	i->ObjNode.Size = size + sizeof(ImageHdr);

	i->CheckSum = checksum(p, size);
	
	return true;
}


static bool do_private(ServInfo *servinfo)
{
	switch (servinfo->FnCode & FG_Mask)
	{
#if SEARCHCODE
		case FG_ServerSearch:
			return (do_search(servinfo));
			break;
#endif
		case FG_RomCreate:
			return (RomCreate(servinfo));
			break;

		default:
			return FALSE;
	}
}

#if SEARCHCODE
word SearchTab[32];
word STPos = 0;

word NewSeq()
{
  extern word _ldtimer( int );
  
	word v = GetDate()+_ldtimer(0);
	WaitLock(&SearchTabLock,"SearchTabLock");
	SearchTab[(STPos++)&0x1f] = v;
	Signal(&SearchTabLock);	
	return v;
}

void MarshalDataWord(MCB *mcb, word w)
{
	MarshalData(mcb,sizeof(word),(byte *)&w);
}

Object *SearchCode1(char *name, word id, word fromlink);

Image *SearchLoad(char *name)
{
	Object *o = NULL;
	Image *i = NULL;
	byte *p = NULL;
	word type;
	word size;

/*IOdebug("%s: SearchLoad %s",mcname,name);*/
	
	o = SearchCode1(name,NewSeq(),-1);

	if( o == NULL ) goto done;

	i = Lookup(&Root,name,TRUE);
if( i ) IOdebug("%s: SearchLoad %s already loaded",mcname,name);	
	if( i != NULL ) 
	{
		Close(o);
		goto done;
	}

	p = LoadObject(o, 0, FALSE, &size);
	
	if( p == NULL ) goto done;

	switch( *(word *)p )
	{
	default:
	case T_Module	: type = Type_Module; break;
	case T_Program	: type = Type_Program; break;
	case T_Device	: type = Type_Device; break;
	}

	i = NewImage(&Root,(void *)p,name,type,0,-1, size);
	i->Object = o;
/*if( i ) IOdebug("%s: SearchLoad got %s from %O",mcname,i->ObjNode.Name,o);*/
	
	WaitObj( &i->ObjNode );
	
	if( !BindLibraries(p,FALSE) )
	{
		Unlink(&i->ObjNode,TRUE);
		FreeMem(p);
		Close(o);
		Free(i);
		i = NULL;
		
	}
	else 
	{
		i->CheckSum = checksum(p, i->ObjNode.Size - sizeof(ImageHdr));
		Signal( &i->ObjNode.Lock );
	}

done:	
	return i;


}

Object *SearchCode1(char *name, word id, word fromlink)
{
	MsgBuf *m;
	Port reply;	
	int i;
	word e = 0;
	Object *o = NULL;
	Capability cap;
	MCB *mcb;
	int nlinks = (int)(GetConfig()->NLinks);
	int nreplies = 0;

	m = New(MsgBuf);
	
	if( m == NULL ) return NULL;

	mcb = &m->mcb;
	mcb->Control = m->control;
	mcb->Data = m->data;
	
	reply = NewPort();
	
	InitMCB(mcb,MsgHdr_Flags_preserve,NullPort,reply,FC_GSP|FG_ServerSearch);
	
	MarshalString(mcb,"/link.X/loader");
	MarshalString(mcb,name);
	MarshalWord(mcb,1);
	NewCap(&cap,(ObjNode *)&Root,AccMask_R);
	MarshalCap(mcb,&cap);
	MarshalWord(mcb,id);
	MarshalWord(mcb,MachineType());

	for( i = 0; i < nlinks; i++ )
	{
		if( i != fromlink )
		{
			static LinkInfo link;
			LinkData(i,&link);
			if( (link.Mode  != Link_Mode_Intelligent) ||
			    (link.State != Link_State_Running)    ||
			    (link.Flags & Link_Flags_stopped) ) continue;
			mcb->Data[6] = '0'+i;
			SendIOC(mcb);
			nreplies++;
		}
	}

	InitMCB(mcb,0,reply,NullPort,0);
	mcb->Timeout = 2*OneSec;

	/* must unlock root while we wait since we will get loop-backs	*/
	/* note that searchlock is kept to serialize all searches	*/
	
	Signal(&Root.Lock);

	for( i = 0; i < nreplies; i++ )
	{
		e = GetMsg(mcb);
		if( e == EK_Timeout ) break;
		if( e > 0 ) break;
	}
	/* Order of waits is important here to avoid deadlock	*/
	WaitLock(&Root.Lock,"Root.Lock");

	FreePort(reply);
	
	if( e >= 0 )
	{
		StrDesc *sd = (StrDesc *)mcb->Data;
		o = NewObject(sd->Name,&sd->Cap);
		o->Flags = sd->Mode;
	}

	Free(m);
	return o;
}

static bool do_search(ServInfo *servinfo)
{
	int i;
	Image *image;
	Capability cap;
	MCB *mcb = servinfo->m;	
	char *pathname = servinfo->Pathname;
	word mode;
	Object *o = NULL;
	word id = mcb->Control[5];
	word type = mcb->Control[6];
	
	WaitLock(&SearchTabLock,"SearchTabLock");
			
	for( i = 0; i < 32 ; i++ ) if( id == SearchTab[i] ) 
	{
		Signal(&SearchTabLock);		
		goto fail;
	}

	SearchTab[(STPos++)&0x1f] = id;
	
	Signal(&SearchTabLock);
	
	image = (Image *)GetTarget(servinfo);

	/* Fail if we know nothing about this image, or if the requester  */
	/* is a different machine type to us, and there is some suspicion */
	/* that this is a machine-dependant library.			  */
	
	if( image == NULL || 
#ifdef __TRAN
		( type/100 != MachineType()/100 &&
#else
		(type != MachineType() &&
#endif
		 strlcmp(objname(image->Object->Name),objname(pathname))!=0)
	  ) 
	{
#if 1
		goto fail;
#else
		PortInfo pi;
		GetPortInfo(mcb->MsgHdr.Reply,&pi);
		o = SearchCode1(objname(servinfo->Pathname),id,pi.Link);
		if( o == NULL ) goto fail;
		pathname = o->Name;
		cap = o->Access;
		mode = o->Flags;	
#endif
	}
#if 1
	else if( image->ObjNode.Type == Type_CacheName )
	{
		/* if this is a cache name, return the referenced object */
		pathname = image->Object->Name;
		cap = image->Object->Access;
		mode = image->Object->Flags;
	}
#endif
	else
	{
		pathname = servinfo->Pathname;
		NewCap(&cap,&image->ObjNode,AccMask_R);
		mode = image->ObjNode.Flags|Flags_Closeable|O_ReadOnly;
	}
	
	InitMCB(mcb,0,mcb->MsgHdr.Reply,NullPort,0);
	MarshalOffset(mcb);
	MarshalDataWord(mcb,mode);
	MarshalDataWord(mcb,0);
	MarshalData(mcb,sizeof(cap),(byte *)&cap);
	MarshalData(mcb,(word)strlen(pathname)+1,pathname);
	goto done;

fail:
	InitMCB(mcb,0,mcb->MsgHdr.Reply,NullPort,EC_Error);

done:
	if( o != NULL ) Close(o);	
	PutMsg(mcb);
	return TRUE;
}
#endif	/* SEARCHCODE */

/* Private protocol message */
typedef struct ROMCreate {
	IOCCommon	Common;
	Offset		Name;
	word		Addr;
} ROMCreate;

bool RomCreate(ServInfo *servinfo)
{
	MCB *m = servinfo->m;
	ROMCreate *req = (ROMCreate *)(m->Control);
	Carrier *c;
	MPtr mod;
	Image *i;
	word type;
	MPtr addr = (MPtr)req->Addr;		/* Get image's addr from msg */
	char *name = &(m->Data[req->Name]);	/* Get name from msg */

	/*IOdebug("Loader:Rom Create: %s @ %x",name,addr);*/

	/* check magic */
	if (MWord_(addr,0) != Image_Magic)
		return FALSE;

	/* get module address here */
	mod = MInc_(addr,sizeof(ImageHdr));

        switch ( ModuleWord_(mod,Type) )
        {
        case T_Module:  type = Type_Module;  break;
        case T_Program: type = Type_Program; break;
        case T_Device:  type = Type_Device;  break;
        case T_DevInfo:	type = Type_File;    break;
        default:	return FALSE;
        }

	c = (Carrier *)AllocMem(sizeof(Carrier),&CodePool);
	c->Addr = mod;
	c->Size = 0;
	
        
	i = NewImage(&Root, c, name, type, 0, -1, 0);
#if 0
	if( type == Type_Program ) i->UseCount=1;
#endif
	i->Retain = true;
	
	return TRUE;
}


/* -- End of loader.c */



