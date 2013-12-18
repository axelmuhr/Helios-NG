/*------------------------------------------------------------------------
--                                                                      --
--                     H E L I O S   N U C L E U S                      --
--                     ---------------------------                      --
--                                                                      --
--             Copyright (C) 1987, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- servlib.h								--
--                                                                      --
--	Server support library header					--
--                                                                      --
--	Author:  NHG 06/10/87						--
--                                                                      --
------------------------------------------------------------------------*/
/* SccsId: %W%	%G%	Copyright (C) 1987, Perihelion Software Ltd.	*/
/* $Id: servlib.h,v 1.2 1993/08/18 16:16:05 nickc Exp $ */

#ifndef __servlib_h
#define __servlib_h

#ifndef __helios_h
#include <helios.h>
#endif

#include <setjmp.h>
#include <sem.h>
#include <queue.h>
#include <message.h>
#include <protect.h>
#include <gsp.h>
#include <codes.h>

/*----------------------------------------------------------------------*/
/* Directory Node structure 						*/
/*----------------------------------------------------------------------*/

struct DirNode 
{
	Node		Node;		/* link in directory list	*/
	char		Name[NameMax];	/* entry name			*/
	word		Type;		/* entry type			*/
	word		Flags;		/* flag word			*/
	Matrix		Matrix;		/* access matrix		*/
	Semaphore	Lock;		/* locking semaphore		*/
	Key		Key;		/* protection key		*/
	struct DirNode  *Parent;	/* parent directory		*/
	DateSet		Dates;		/* dates of object		*/
	word		Account;	/* owning account		*/
	word		Nentries;	/* number of entries in dir	*/
	List		Entries;	/* directory entries		*/
}; 

#ifndef __cplusplus
typedef struct DirNode  DirNode ;
#endif

/*----------------------------------------------------------------------*/
/* Object Node structure 						*/
/*----------------------------------------------------------------------*/

struct ObjNode 
{
	Node		Node;		/* link in directory list	*/
	char		Name[NameMax];	/* entry name			*/
	word		Type;		/* entry type			*/
	word		Flags;		/* flag word			*/
	Matrix		Matrix;		/* access matrix		*/
	Semaphore	Lock;		/* locking semaphore		*/
	Key		Key;		/* protection key		*/
	struct DirNode  *Parent;	/* parent directory		*/
	DateSet		Dates;		/* dates of object		*/
	word		Account;	/* owning account		*/
	word		Size;		/* object size			*/
	List		Contents;	/* whatever this object contains*/
					/* may be cast to something else*/
}; 

#ifndef __cplusplus
typedef struct ObjNode ObjNode ;
typedef struct ObjNode * ObjNodePtr;
#endif

/* Note that the above two structures should be the same size and layout*/

/*----------------------------------------------------------------------*/
/* Dispatch Info Structure						*/
/*----------------------------------------------------------------------*/

struct DispatchEntry 
{
	VoidFnPtr 	Fn;		/* function			*/
	word		StackSize;	/* stack size			*/
}; 

#ifndef __cplusplus
typedef struct DispatchEntry DispatchEntry;
#endif

struct DispatchInfo 
{
	DirNode		*Root;		/* root of filing system	*/
	Port		ReqPort;	/* request port			*/
	word		SubSys;		/* subsystem code		*/
	char		*ParentName;	/* name of root's parent dir	*/
	DispatchEntry	PrivateProtocol;/* escape for non-GSP functions	*/
	DispatchEntry	Fntab[IOCFns];	/* function jump table		*/
};

#ifndef __cplusplus
typedef struct DispatchInfo DispatchInfo;
#endif

/*----------------------------------------------------------------------*/
/* Server Info structure						*/
/*----------------------------------------------------------------------*/

struct ServInfo 
{
	DispatchInfo	*DispatchInfo;		/* DispatchInfo struct	   */
	MCB		*m;			/* MCB of request	   */
	DirNode		*Context;		/* original context object */
	ObjNode		*Target;		/* the target object	   */
	bool		TargetLocked;		/* Target locked ?	   */
	word		FnCode;			/* request fn code	   */
	jmp_buf		Escape;			/* error/esacpe jump buffer*/
	char		Pathname[IOCDataMax];	/* current object pathname */
};

#ifndef __cplusplus
typedef struct ServInfo ServInfo;
#endif

/*----------------------------------------------------------------------*/
/* Link Node								*/
/*----------------------------------------------------------------------*/

struct LinkNode 
{
	ObjNode		ObjNode;	/* object node			*/
	Capability	Cap;		/* permissions			*/
	char		Link[4];	/* linked name string		*/
}; 

#ifndef __cplusplus
typedef struct LinkNode LinkNode;
#endif

/*----------------------------------------------------------------------*/
/* Dynamic message buffer 						*/
/*----------------------------------------------------------------------*/

struct MsgBuf 
{
	Node		Node;			/* for queuing		*/
	MCB		mcb;			/* message control block*/
	word		control[IOCMsgMax];	/* control vector	*/
	byte		data[IOCDataMax]; 	/* data vector		*/
}; 

#ifndef __cplusplus
typedef struct MsgBuf MsgBuf;
#endif

/*----------------------------------------------------------------------*/
/* Data buffer								*/
/*----------------------------------------------------------------------*/

struct Buffer 
{
	Node		Node;			/* queueing node	*/
	word		Pos;			/* buffer pos in list	*/
	word		Size;			/* bytes held in buffer	*/
	word		Max;			/* max size of buffer	*/
	byte		*Data;			/* actual data buffer	*/
}; 

#ifndef __cplusplus
typedef struct Buffer Buffer;
#endif

/*----------------------------------------------------------------------*/
/* Name Creation Info structure						*/
/*----------------------------------------------------------------------*/

struct NameInfo 
{
	Port		Port;		/* server port			*/
	word		Flags;		/* flag word			*/
	Matrix		Matrix;		/* initial access matrix	*/
	word		*LoadData;	/* auto-load data		*/
}; 

#ifndef __cplusplus
typedef struct NameInfo NameInfo;
#endif

/*----------------------------------------------------------------------*/
/* Function Prototypes  						*/
/*----------------------------------------------------------------------*/

extern void InitNode(ObjNode * o, string name, int type, int flags, Matrix matrix);
extern void Dispatch(DispatchInfo *info);
extern DirNode *GetContext(ServInfo *servinfo);
extern ObjNode * GetTarget(ServInfo *servinfo);
extern DirNode *GetTargetDir(ServInfo *servinfo);
extern ObjNode * GetTargetObj(ServInfo *servinfo);
extern void pathcat(string, string);
extern string objname(string);
extern word addint(char *, word);
extern ObjNode * Lookup(DirNode *d, string name, bool dirlocked);
extern void Insert(DirNode *dir, ObjNode *obj, bool dirlocked);
extern void Unlink(ObjNode * obj, bool dirlocked);
extern void DirServer(ServInfo *servinfo, MCB *m, Port reqport);
extern void MarshalInfo(MCB *m, ObjNode * o);

extern void DoLocate(ServInfo *servinfo);
extern void DoRename(ServInfo *servinfo);
extern void DoLink(ServInfo *servinfo);
extern void DoProtect(ServInfo *servinfo);
extern void DoRevoke(ServInfo *servinfo);
extern void DoObjInfo(ServInfo *servinfo);
extern void DoSetDate(ServInfo *servinfo);
extern void DoRefine(ServInfo *servinfo);
extern void InvalidFn(ServInfo *servinfo);
extern void NullFn(ServInfo *servinfo);

extern void ErrorMsg(MCB *mcb,word err);
extern AccMask UpdMask(AccMask mask, Matrix matrix);
extern int CheckMask(AccMask mask, AccMask access);
extern word GetAccess(Capability *cap, Key key);
extern void Crypt(bool, Key, byte *, word);
extern Key NewKey(void);
extern void FormOpenReply(MsgBuf *, MCB *, ObjNode *, word, char *);
extern void NewCap(Capability *cap, ObjNode *obj, AccMask mask);

extern bool AdjustBuffers(List *list, word start, word end, word bufsize);
extern word DoRead(MCB *m, Buffer *(*GetBuffer)(), void *info);
extern word DoWrite(MCB *m, Buffer *(*GetBuffer)(), void *info);
extern Buffer *GetReadBuffer(word pos, List *list);
extern Buffer *GetWriteBuffer(word pos, List *list);

extern void *ServMalloc(word size);

/* map Malloc() into ServMalloc */
#define Malloc ServMalloc

#define LockTarget(s) \
	if(!(s)->TargetLocked) {Wait(&(s)->Target->Lock);(s)->TargetLocked=TRUE;}

#define UnLockTarget(s) \
	if((s)->TargetLocked) {Signal(&(s)->Target->Lock);(s)->TargetLocked=FALSE;}


#endif

/* -- End of servlib.h */
