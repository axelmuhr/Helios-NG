/* $Header: /hsrc/filesys/pfs_v2.1/src/fs/RCS/fservlib.h,v 1.1 1992/07/13 16:17:41 craig Exp $ */

/* $Log: fservlib.h,v $
 * Revision 1.1  1992/07/13  16:17:41  craig
 * Initial revision
 *
 * Revision 2.1  90/08/31  11:03:18  guenter
 * first multivolume/multipartition PFS with tape
 * 
 * Revision 1.2  90/08/08  08:05:20  guenter
 * minor changes
 * 
 * Revision 1.1  90/01/02  19:03:40  chris
 * Initial revision
 * 
 */

/*------------------------------------------------------------------------
--                                                                      --
--                     H E L I O S   N U C L E U S                      --
--                     ---------------------------                      --
--                                                                      --
--             Copyright (C) 1987, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- fservlib.h								--
--                                                                      --
--	Server support library header					--
--                                                                      --
--	Author:  NHG 06/10/87						--
--      Modified: AI 19/04/89                                           --
------------------------------------------------------------------------*/
/* SccsId: @(#)servlib.h	1.4	16/1/89 Copyright (C) 1987, Perihelion Software Ltd.	*/

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


typedef jmp_buf JB;

/*----------------------------------------------------------------------*/
/* Directory Node structure 						*/
/*----------------------------------------------------------------------*/

typedef struct DirNode {
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
} DirNode ;

/*----------------------------------------------------------------------*/
/* Object Node structure 						*/
/*----------------------------------------------------------------------*/

typedef struct ObjNode {
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
} ObjNode ;

/* Note that the above two structures should be the same size and layout*/

/*----------------------------------------------------------------------*/
/* Dispatch Info Structure						*/
/*----------------------------------------------------------------------*/

typedef struct DispatchEntry {
	VoidFnPtr 	Fn;		/* function			*/
	word		StackSize;	/* stack size			*/
} DispatchEntry;

typedef struct DispatchInfo {
	DirNode		*root;		/* root of filing system	*/
	Port		reqport;	/* request port			*/
	word		subsys;		/* subsystem code		*/
	WordFnPtr	PrivateProtocol;/* escape for non-GSP functions	*/
	VoidFnPtr	fntab[IOCFns];	/* function jump table		*/
} DispatchInfo;

/*----------------------------------------------------------------------*/
/* Server Info structure						*/
/*----------------------------------------------------------------------*/

typedef struct ServInfo {
	MCB		*m;			/* MCB of request	   */
/*
	DirNode		*Context;	
	ObjNode		*Target;	
*/
	word		FnCode;			/* request fn code	   */
	jmp_buf		Escape;			/* error/esacpe jump buffer*/
	char		Pathname[IOCDataMax];	/* current object pathname */
	char		Context[IOCDataMax];	/* context object's pathname */
	char		Target[IOCDataMax];	/* target object's pathname */
	bool		ParentOfRoot;
} ServInfo;

/*----------------------------------------------------------------------*/
/* Link Node								*/
/*----------------------------------------------------------------------*/

typedef struct LinkNode {
	ObjNode		ObjNode;	/* object node			*/
	Capability	Cap;		/* permissions			*/
	char		Link[4];	/* linked name string		*/
} LinkNode;

/*----------------------------------------------------------------------*/
/* Dynamic message buffer 						*/
/*----------------------------------------------------------------------*/

typedef struct MsgBuf {
	Node		Node;			/* for queuing		*/
	MCB		mcb;			/* message control block*/
	word		control[IOCMsgMax];	/* control vector	*/
	byte		data[IOCDataMax]; 	/* data vector		*/
} MsgBuf;

/*----------------------------------------------------------------------*/
/* Data buffer								*/
/*----------------------------------------------------------------------*/

typedef struct Buffer {
	Node		Node;			/* queueing node	*/
	word		Pos;			/* buffer pos in list	*/
	word		Size;			/* bytes held in buffer	*/
	word		Max;			/* max size of buffer	*/
	byte		*Data;			/* actual data buffer	*/
} Buffer;


/*----------------------------------------------------------------------*/
/* Name Creation Info structure						*/
/*----------------------------------------------------------------------*/

typedef struct NameInfo {
	Port		Port;		/* server port			*/
	word		Flags;		/* flag word			*/
	Matrix		Matrix;		/* initial access matrix	*/
	word		*LoadData;	/* auto-load data		*/
} NameInfo;

/*----------------------------------------------------------------------*/
/* Function Prototypes  						*/
/*----------------------------------------------------------------------*/

extern void InitNode(ObjNode *o, string name, int type, int flags, Matrix matrix);
extern void Dispatch(DispatchInfo *info);
extern DirNode *GetContext(ServInfo *servinfo);
extern ObjNode *GetTarget(ServInfo *servinfo);
extern DirNode *GetTargetDir(ServInfo *servinfo);
extern ObjNode *GetTargetObj(ServInfo *servinfo);
extern bool GetName(MCB *m, string name, string pathname);
extern void pathcat(string, string);
extern string objname(string);
extern word addint(char *, word);
extern ObjNode *Lookup(DirNode *d, string name, bool dirlocked);
extern void Insert(DirNode *dir, ObjNode *obj, bool dirlocked);
extern void Unlink(ObjNode *obj, bool dirlocked);
extern void DirServer(ServInfo *servinfo, MCB *m, Port reqport);
extern void MarshalInfo(MCB *m, ObjNode *o);

extern void DoLocate(ServInfo *servinfo);
extern void DoRename(ServInfo *servinfo);
extern void DoLink(ServInfo *servinfo);
extern void DoProtect(ServInfo *servinfo);
extern void DoObjInfo(ServInfo *servinfo);
extern void DoSetDate(ServInfo *servinfo);
extern void DoRefine(ServInfo *servinfo);
extern void InvalidFn(ServInfo *servinfo);
extern void NullFn(ServInfo *servinfo);

extern void ErrorMsg(MCB *mcb,word err);
extern AccMask UpdMask(AccMask mask, Matrix matrix);
extern int CheckMask(AccMask mask, AccMask access);
extern word GetAccess(Capability *cap, Key key);
extern void crypt(Key, byte *, word);
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
/* #define Malloc ServMalloc */

#define LockTarget(s) \
	if(!(s)->TargetLocked) {Wait(&(s)->Target->Lock);(s)->TargetLocked=TRUE;}

#define UnLockTarget(s) \
	if((s)->TargetLocked) {Signal(&(s)->Target->Lock);(s)->TargetLocked=FALSE;}


#endif

/* -- End of servlib.h */
