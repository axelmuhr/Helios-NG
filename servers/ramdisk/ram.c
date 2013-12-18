/*------------------------------------------------------------------------
--                                                                      --
--                     H E L I O S   N U C L E U S                      --
--                     ---------------------------                      --
--                                                                      --
--             Copyright (C) 1987, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- ram.c								--
--                                                                      --
--	RAM disc handler						--
--                                                                      --
--	Robust version of RAM disk server.				--
--	Restores contents of RAM disk after a system reset.		--
--	It uses relocatable memory blocks (__MI) to enable a reduction	--
--	of memory fragmentation, making best use of the available	--
--	memory. The performance has also been improved			--
--	(Requires __RRD and __MI flags to be defined in kernel make)	--
--                                                                      --
--	Author:  NHG 22/11/87						--
--      Robust version: PAB Nov 90					--
--	CardCode: PAB/JGS Dec 90					--
--									--
--                                                                      --
------------------------------------------------------------------------*/
/* SccsId: %W% %G% Copyright (C) 1987, Perihelion Software Ltd. */
/* $Id: ram.c,v 1.8 1993/04/05 17:26:26 paul Exp $ */

/*
 * TODO: 
 *
 * File Writes should change fileblock date. Alteration of protection matrix
 * should be reflected in file/symb/dirblock 
 *
 * do_serverinfo() should return relevant sizes. This would allow the 'du'
 * program to be used on the RAMFS. - Should also create a kernel StatMem() fn
 * to return the amount of available memory. 
 *
 * Check that if a prog crashes with files open, that a files .users is
 * decremented after a timeout. - Must be able to delete temporary files! 
 *
 * Need check for EOF on validation of a file - file can be truncated without us
 * being able to detect it. 
 *
 * Need to take account of JEIDA card write protect tab! 
 *
 *
 * Possible speed improvements via: 
 *
 * Cacheing the file pointer - GetBuffer uses this first to test for correct
 * position in current or next block (shouldn't rely on this - server should
 * be stateless). 
 *
 * AdjustBuffers could be speeded up by keeping a tail handle in the spare
 * Objnode.Contents.Tail This would speed up extensions to the buffer chain. 
 *
 * AdjustBuffers could also be speeded up by coding a tight loop between while
 * (pos > start) && (pos < end). 
 *
 * Write custom buffer chain trim/extend/delete functions rather than use the
 * general purpose AdjustBuffers. 
 *
 * Create RRDWriteWord/ReadWord() functions so we dont have to lock blocks for
 * simple block updates. 
 *
 * Turn off read checksumming! Increase the block size - Remember the larger the
 * block the less accurate the checksum! 
 *
 *
 * Possible space utilisation improvements via: 
 *
 * #filename# should not have any magic so they are not saved over reset 
 *
 * Recode all ServLib functions that require a fixed style Obj/DirNode, to use
 * the MI/lock/unlock + reduced structure contents. We will not need to have
 * separate transient and reset resistent structures then. - This is a big job
 * though. 
 *
 * Try to bodge the Obj/Dir node structures by appending the reset resilient
 * structure to them - where is a fixed size mandated? - stops the reset
 * resilient parts being relocatable. 
 *
 * If another word is needed in RRDBuffBlock struct, .DataSize and .DataStart
 * could be combined into a single word. This is not needed at present as all
 * Memory pool allocations have a 16 byte granularity. 
 *
 * Depending on the average size of files: Small files: Decrease the blocksize
 * (to reduce wasted block space). Large files: Increase the blocksize (to
 * reduce management info overhead). 
 *
 * Remember that the smaller the blocks the more leway we will have in
 * RMCompact() to defragment memory. 
 *
 * Implement a Truncate/Read_Only mode (Read once/read destructively). This
 * allows users to read temporary files without having to have the data in two
 * places at once. Refine this system to the sub-block level by reading the
 * data, moving the header up, fixing the handle chaining and use of the
 * MITrimAlloc() fn to reduce the blocksize from the bottom. Reads of less
 * than and/or with a remainder of 16 bytes will have to be catered for by
 * altering RRDBuffBlock.DataStart RPTR. 
 *
 * Implement a Close/Realloc. When a file is closed, its last block should be
 * reallocated to its *actual* size requirements. This has only one problem,
 * FileWrite and Adjustbuffers assume a fixed blocksize. The Helios GSP
 * Protocol also assumes this. If this is implemented, then the std. block
 * size should be made as large as feasable. The MIRealloc() fn is always
 * guaranteed to succeed. 
 *
 * Background compression of RAMFS blocks, file is decompressed on Open/fly. To
 * keep the memory utilisation low when decompressing, extend the files
 * buffers to their original size and then starting from the end decompress to
 * the new end gradually moving upwards and overwriting the old compressed
 * blocks. This would require some form of reversable compression algorithm. I
 * don't know of any. Block compression on the fly could reallocate blocks to
 * reduce memory space requirements. 
 *
 *
 * Integrity checks could be improved by: 
 *
 * Use of CRC's - these require ~8 instructions per BIT of data, they would slow
 * the access rate to a crawl. 
 *
 * Increase the size of the Checksum / use two checksums one ADD, one XOR. 
 *
 * Use some other technique. 
 *
 *
 * Other Things: 
 *
 *
 */
/*---------------------------------------------------------------------------*/

#include <helios.h>		/* standard header */

#define __in_ram 1		/* flag that we are in this module */

/*---------------------------------------------------------------------------*/
/* Debugging control */

#ifdef SYSDEB
# define DEBUG(x) IOdebug(x)
#else
# define DEBUG(x)
#endif

/*---------------------------------------------------------------------------*/

/* @@@ May wish to increase speed by removing next two defines		 */
/* @@@ Maybe allow SetInfo to choose...					 */
#define READCHECK		/* read checksum each buffer block		 */
#define READCHECK2		/* read checksum fileblock headers		 */

/* The following MUST be defined					 */
#define WRITECHECK		/* write checksum each file & buffer block	 */

/*---------------------------------------------------------------------------*/
/*-- Include Files ----------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

#include <string.h>
#include <codes.h>
#include <syslib.h>
#include <servlib.h>
#include <task.h>
#include <process.h>
#include <stddef.h>
#include <event.h>
#include <root.h>
#include "ram.h"		/* RAM header block information */
#ifdef __CARD
# include <abcARM/manifest.h>	/* useful Executive manifests */
# include <abcARM/PCcard.h>	/* external CARD information */
# include <abcARM/ROMitems.h>	/* ROM ITEM information and access functions */
# include <abcARM/ABClib.h>	/* CARD interface functions */
# include <event.h>
#endif

/*---------------------------------------------------------------------------*/
/*-- Type definitions -------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

typedef struct File {
	ObjNode         ObjNode;/* node for directory struct */
}               File;

#ifdef __CARD
typedef struct CardData {
	int             slot;	/* slot we are attached to */
	int             insert;	/* TRUE if card is inserted */
	Semaphore       event;	/* hard semaphore to handler */
	MIInfo         *mi;	/* memory table for ramdisk */
	char            cardname[NameMax];	/* card link name */
}               CardData;

#endif				/* __CARD */

/* server info structure used by 'df' program */
typedef struct serv_info {
	WORD	type;
	WORD	size;
	WORD	available;
	WORD	alloc;
} serv_info;


/*---------------------------------------------------------------------------*/
/*-- Function prototypes ----------------------------------------------------*/
/*---------------------------------------------------------------------------*/

static bool     RRDExtendBuffers(File *f, word end, word pos, RRDFileBlock *fileblock, MIInfo * mypool);
static bool     RRDAdjustBuffers(File *f, word start, word end, MIInfo * mypool);
static RRDBuffBlock *RRDGetWriteBuffer(word pos, Handle hint, Handle nextb, Handle * bh, MIInfo * mypool);
static RRDBuffBlock *RRDGetReadBuffer(word pos, Handle hint, Handle nextb, Handle * bh, MIInfo * mypool);
static void     RRDPutWriteBuffer(RRDBuffBlock * buf, Handle h, MIInfo * mypool);
static Handle   InitFileBlock(ObjNode * o, char *pathname, bool tempfile, MIInfo * mypool);
static Handle   InitDirBlock(ObjNode * o, char *pathname, MIInfo * mypool);
static Handle   InitSymbBlock(LinkNode * l, char *pathname, MIInfo * mypool);
static word     Checksum(RRDBuffBlock * buf);
static uword    CreateFileId(void);
static char    *shortname(char *);
static DirNode *NewDir(DirNode * dir, char *name, word flags, Matrix matrix, bool save, MIInfo * mypool);
static void     ResurrectRRD(MIInfo * mypool, word savedblocks);
static void     ReCreateRRD(MIInfo * mypool);
static DirNode *MakeHierarchy1(char *path, bool save, MIInfo * mypool);
static void     MakeHierarchy2(RRDDirBlock * db, Handle dirhandle, bool save, MIInfo * mypool);
static void     RestoreSymbLn(RRDSymbBlock * sb, Handle handle);
static void     RestoreFile(RRDFileBlock * fb, Handle fhandle);
static void     RestoreBlock(RRDFileBlock * f, RRDBuffBlock * nb, Handle bhandle, MIInfo * mypool);
static bool     ValidateROFile(RRDFileBlock * fb, Handle fhandle, word * size, MIInfo * mypool);
static bool     ValidateFile(RRDFileBlock * fb, Handle fhandle, word * size, MIInfo * mypool);
static void     DeleteBuffBlocks(RRDFileBlock * fb, MIInfo * mypool);
static ObjNode *FindObj(char *name);
static int      basename(char *base, char *str);
static MIInfo  *GetRootPool(DirNode *d);
static File    *CreateNode(MCB * m, DirNode * d, char * pathname, MIInfo * mypool);
static void     FileRead(MCB * m, File * f, MIInfo * mypool);
static void     FileWrite(MCB * m, File * f, MIInfo * mypool);
static void     FileSeek(MCB * m, File * f);

# ifdef __CARD
static void     CardInsertHandler(int slot);
static void     CardRemovalHandler(int slot);
static word     RamCardHandler(bool insert, int slot, word type);
static void     CardHandlerProcess(int slot);
static void     InitCard(void);
# endif				/* __CARD */

/* Server Action functions */
static void     do_open(ServInfo *);
static void     do_create(ServInfo *);
static void     do_rename(ServInfo *);
static void     do_delete(ServInfo *);
static void     do_serverinfo(ServInfo *);
static void     do_link(ServInfo * servinfo);
static void	do_setdate(ServInfo * servinfo);
static void	do_protect(ServInfo *servinfo);
static void	do_revoke(ServInfo *servinfo);

/*---------------------------------------------------------------------------*/
/*-- MACROs -----------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

#ifdef New			/* change helios.h's definition of New() macro */
# undef New
# define New(_type) ((_type *)ServMalloc(sizeof(_type)))
#endif

/*---------------------------------------------------------------------------*/
/*-- Private Data Definitions -----------------------------------------------*/
/*---------------------------------------------------------------------------*/

/* Block and transfer size constants */

/* BlockMax must be <= 64k and a 16 byte multiple! */
# define BlockMax		RRDBuffMax	/* bytes in each block	  */
# define TransferMax		((BlockMax)/1)	/* bytes in each message  */
/* This MUST be a product of a division of BlockMax that has no remainder */

/* Protection constants - default to "protect.h" */

#define RootMatrix 	0x21110985	/* arv:rx:ry:rz			 */
#define BaseDirMatrix	0x21130B87	/* arwv:rwx:rwy:rz		 */

/* open Mode constants */

#define WriteModes	(O_WriteOnly|O_Create|O_Truncate|O_Append)

/* Redefinitions of existing structure elements */
/* objects (files) */
#define Upb		ObjNode.Size	/* use ObjNode size field	 */
#define Users		ObjNode.Account	/* number of opens		 */
#define Buffers         ObjNode.Contents.Head	/* handle to RRDFileBlock */
#define TailBuffer	ObjNode.Contents.Tail	/* handle to last buffer */
#define BufPosHint	ObjNode.Contents.Earth	/* hint as to current pos */
#define SymbBlockH	ObjNode.Contents.Head	/* handle to RRDSymbBlock */
/* directories */
#define DirBlockH	Account	/* handle to RRDDirBlock	 	*/
#define MemBlockH	Account	/* handle to MIINfo	 		*/

/* If the overloading of Account is no-go, we will have to embed the	 */
/* DirNode struct inside another that just defines "Handle DirBlockH"	 */
/* or "MIInfo *MemBlockH".						 */

/* local static data */

DirNode         Root;		/* root of /ram directory system */

DirNode        *SysRoot;	/* root of /ram/sys directory */

/* Data used to calculate next unique file Id */
static Semaphore FIDSem;
static uword    LastFID;

#ifdef __CARD
#define defStack (0x0400)	/* 1K default process stack	 */

static CardData *CardVec;	/* vector of slot descriptors	 */
static CardEvent cardinfo;	/* CARD handler information	 */

#endif				/* __CARD */

#ifndef DEMANDLOADED
NameInfo        Info = {
			NullPort,
			Flags_StripName,
			DefNameMatrix,
			(word *) NULL
};

#endif

static DispatchInfo RamInfo = {
       &Root,
       NullPort,
       SS_RamDisk,
       NULL,
       {NULL, 0},
       {
		{do_open, 2000},	/* FG_Open	 */
		{do_create, 2000},	/* FG_Create	 */
		{DoLocate, 2000},	/* FG_Locate	 */
		{DoObjInfo, 2000},	/* FG_ObjectInfo */
		{do_serverinfo, 2000},	/* FG_ServerInfo */
		{do_delete, 2000},	/* FG_Delete	 */
		{do_rename, 2000},	/* FG_Rename	 */
		{do_link, 2000},	/* FG_Link	 */
		{do_protect, 2000},	/* FG_Protect	 */
		{do_setdate, 2000},	/* FG_SetDate	 */
		{DoRefine, 2000},	/* FG_Refine	 */
		{NullFn, 2000},		/* FG_CloseObj	 */
		{do_revoke, 2000},	/* FG_Revoke	 */
		{InvalidFn, 2000},	/* Reserved 	 */
		{InvalidFn, 2000}	/* Reserved	 */
	}
};

/*---------------------------------------------------------------------------*/
/*-- main : entry point of RAM handler --------------------------------------*/
/*---------------------------------------------------------------------------*/

int     main(void)
{
#ifndef DEMANDLOADED
	Object         *nte;
#endif
	char            mcname[100];	/* nasty (large stack allocation)
					 * constant */
	RootStruct     *root = GetRoot();
	MIInfo         *mypool = root->MISysMem;	/* standard system
							 * memory pool */

	/* initialise unique file Id system */
	InitSemaphore(&FIDSem, 1);
	LastFID = 0;

#ifdef STANDALONE
	Environ         env;

	GetEnv(MyTask->Port, &env);	/* posix exec send's env ! */
	/* if executed by procman alone, we shouldn't read this */
#endif

	MachineName(mcname);

	RamInfo.ParentName = mcname;

	InitNode((ObjNode *) & Root, "ram", Type_Directory, 0, RootMatrix);
	InitList(&Root.Entries);
	Root.Nentries = 0;

#ifdef DEMANDLOADED
	RamInfo.ReqPort = MyTask->Port;
#else
	Info.Port = RamInfo.ReqPort = NewPort();
#endif

	/* .. of root is a link to our machine root	 */
	{
		Object         *o;
		LinkNode       *Parent;

		o = Locate(NULL, mcname);

		Parent = (LinkNode *) ServMalloc(sizeof(LinkNode) + (word) strlen(mcname));
		InitNode(&Parent->ObjNode, "..", Type_Link, 0, DefLinkMatrix);
		Parent->Cap = o->Access;
		strcpy(Parent->Link, mcname);
		Root.Parent = (DirNode *) Parent;

#ifndef DEMANDLOADED
		/* demand loaded servers already have name entry */
		nte = Create(o, "ram", Type_Name, sizeof(NameInfo), (byte *) & Info);
#endif
		Close(o);
	}

	/*
	 * The new RAM world has a single RAM server root directory "/ram",
	 * under which directories will appear for each of the RAM blocks
	 * provided to the system. The directory "/ram/sys" is permanently
	 * present, providing an interface to the internal RAM. 
	 */
	/* create "/ram/sys" directory */
	SysRoot = NewDir(&Root, "sys", 0, BaseDirMatrix, FALSE, mypool);

	/*
	 * root memory directory "/ram/sys", so remember memory pool for this
	 * dir 
	 */
	SysRoot->MemBlockH = (word) mypool;	/* internal memory */

	/* Recreate any filesystem that may have existed before reset */
	ResurrectRRD(mypool, GetRoot()->RRDScavenged);

#ifdef __CARD
	/* initialise card insert/extract handlers */
	/* and pseudo insert any cards that are already present */
	InitCard();
#endif

#ifdef INSYSTEMIMAGE
	/* reply to procman that we have started */
	/* if we are part of system image 0x456 is expect to be returned */
	{
		MCB             m;
		word            e;

		InitMCB(&m, 0, MyTask->Parent, NullPort, 0x456);
		e = PutMsg(&m);
	}
#endif				/* INSYSTEMIMAGE */

	Dispatch(&RamInfo);

#ifndef DEMANDLOADED
	Delete(nte, NULL);
#endif				/* DEMANDLOADED */
}

/*---------------------------------------------------------------------------*/
/*-- NewFile : Add a new file to the directory ------------------------------*/
/*---------------------------------------------------------------------------*/

File           *NewFile(DirNode * dir, char * name, word flags, Matrix matrix, MIInfo * mypool)
{
	char           *oname = objname(name);
	File           *f = New(File);

	if (f == NULL)
		return (NULL);

	InitNode(&f->ObjNode, oname, Type_File, (int) flags, matrix);

	/* Get file block that will stay over reset */
	/* Buffers just holds handle to it */
	if ((f->Buffers = (Node *) InitFileBlock(&f->ObjNode,shortname(name), (((*oname == '#') && (oname[strlen(oname) - 1] == '#')) ? TRUE : FALSE), mypool)) == NULL) {
		Free(f);
		return (NULL);
	}
	f->Upb = 0;
	f->Users = 0;
	f->BufPosHint = NULL;
	f->TailBuffer = NULL;

	Insert(dir, &f->ObjNode, TRUE);

	return (f);
}

/*---------------------------------------------------------------------------*/

static DirNode *NewDir(DirNode * dir, char * name, word flags, Matrix matrix, bool save, MIInfo * mypool)
{
	DirNode        *d = New(DirNode);

#ifdef ALLDEBUG
	IOdebug("newdir: %s (%s)", name, objname(name));
#endif

	if (d == NULL)
		return (NULL);

	InitNode((ObjNode *) d, objname(name), Type_Directory, (int) flags, matrix);

	if (save) {
		/* Get dir block that will stay over reset. Note that we	   */
		/*
		 * overload the Account (DirBlockH) with its handle. Account
		 * is 
		 */
		/* currently not used by the system				   */
		if ((d->DirBlockH = InitDirBlock((ObjNode *) d, shortname(name), mypool)) == NULL) {
			Free(d);
			return (NULL);
		}
	}
	else
		d->DirBlockH = NULL;

	InitList(&d->Entries);
	d->Nentries = 0;
	d->Parent = dir;

	Insert(dir, (ObjNode *) d, TRUE);

	return (d);
}

/*---------------------------------------------------------------------------*/

static File    *CreateNode(MCB * m, DirNode * d, char * pathname, MIInfo * mypool)
{
	File           *f;
	IOCCreate      *req = (IOCCreate *) (m->Control);

	if (req->Type == Type_Directory)
		f = (File *) NewDir(d, pathname, 0, DefDirMatrix, TRUE, mypool);
	else
		f = NewFile(d, pathname, Flags_Selectable, DefFileMatrix, mypool);

	return (f);
}

/*---------------------------------------------------------------------------*/
/*-- Server Action procedures -----------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*
 * The lowest ram (root) subdirectory should be found to get the MIInfo
 * pointer out of its MemBlockH (Account) field. This should be passed to all
 * subsequent functions to get and return memory to the AREA associated with
 * that directory. This is only done for root directories, the MemBlockH field
 * is over-loaded with DirBlockH, which holds a Handle to a reset resilent
 * dirblock structure in all other cases. 
 */

static void     do_open(ServInfo * servinfo)
{
	MCB            *m = servinfo->m;
	MsgBuf         *r;
	DirNode        *d;
	File           *f;
	IOCMsg2        *req = (IOCMsg2 *) (m->Control);
	Port            reqport;
	byte           *data = m->Data;
	char           *pathname = servinfo->Pathname;
	word            mode = req->Arg.Mode;
	MIInfo         *mypool;

#ifdef ALLDEBUG
	IOdebug("ram: do_open: %s", servinfo->Pathname);
#endif
	/*
	 * We need to work from "Root" until we find the relevant directory.
	 * This is a real problem... since we cannot simply look at the
	 * MemBlockH field of the target directory due to the fact that the
	 * location is over-loaded with DirBlockH for sub-directories. 
	 *
	 * We need to set "mypool" to the relevant "DirNode * ->MemBlockH" 
	 *
	 * However, we do know that the pool root directory will be of the form:
	 * "/ram/directory"
	 */

	d = (DirNode *) GetTargetDir(servinfo);

	if (d == NULL) {
		ErrorMsg(m, Err_Null);
		return;
	}

	r = New(MsgBuf);

	if (r == NULL) {
		ErrorMsg(m, (EC_Error + EG_NoMemory));
		return;
	}
	f = (File *) GetTargetObj(servinfo);

	mypool = GetRootPool(f == NULL ? d : (DirNode *)f);

	if (mypool->MIWriteProtect && (mode & WriteModes)) {
		m->MsgHdr.FnRc &= SS_Mask;
		ErrorMsg(m, (EC_Error + EG_WriteProtected + EO_Directory));
		return;
	}

	if ((f == NULL) && (mode & O_Create)) {
		m->MsgHdr.FnRc &= SS_Mask;	/* PAB: clear any error set by
						 * GetContextObj */

		/* if file does not exist, see whether we are allowed to */
		/* create a file here.				    */
		unless(CheckMask(req->Common.Access.Access, AccMask_W)) {
			ErrorMsg(m, (EC_Error + EG_Protected + EO_Directory));
			Free(r);
			return;
		}
		f = CreateNode(m, d, pathname, mypool);
	}
	if (f == NULL) {
		ErrorMsg(m, Err_Null);
		Free(r);
		return;
	}
	unless(CheckMask(req->Common.Access.Access, (int) mode & Flags_Mode)) {
		ErrorMsg(m, (EC_Error + EG_Protected + EO_File));
		Free(r);
		return;
	}

	FormOpenReply(r, m, &f->ObjNode, (Flags_Server | Flags_Closeable), pathname);

	reqport = NewPort();
	r->mcb.MsgHdr.Reply = reqport;
	PutMsg(&r->mcb);
	Free(r);

	if (f->ObjNode.Type == Type_Directory) {
		DirServer(servinfo, m, reqport);
		FreePort(reqport);
		return;
	}
	if ((mode & O_Truncate) && (mode & O_WriteOnly)) {
		f->Upb = 0;
		RRDAdjustBuffers(f, 0, 0, mypool);
	}
	f->Users++;

	UnLockTarget(servinfo);

	forever
	{
		word            e;

		m->MsgHdr.Dest = reqport;
		m->Timeout = StreamTimeout;
		m->Data = data;

		e = GetMsg(m);

		if (e == EK_Timeout)
			break;

		if (e < Err_Null)
			continue;

		Wait(&f->ObjNode.Lock);

		m->MsgHdr.FnRc = SS_RamDisk;

		switch (e & FG_Mask) {
		case FG_Read:
			unless(mode & O_ReadOnly)
				goto badmode;

			FileRead(m, f, mypool);
			break;

		case FG_Write:
			unless(mode & O_WriteOnly)
				goto badmode;

			FileWrite(m, f, mypool);
			break;

		case FG_Close:
			{
				RRDFileBlock *fb = MILock(mypool, (Handle)f->Buffers);

				/* make hint point back to first block */
				f->BufPosHint = (Node *)fb->c.Next;
				MIUnLock(mypool, (Handle)f->Buffers);
			
				if (m->MsgHdr.Reply != NullPort)
					ErrorMsg(m, Err_Null);
				FreePort(reqport);
				f->Users--;
				Signal(&f->ObjNode.Lock);
				return;
			}

		case FG_GetSize:
			InitMCB(m, 0, m->MsgHdr.Reply, NullPort, Err_Null);
			MarshalWord(m, f->Upb);
			PutMsg(m);
			break;

		case FG_Seek:
			FileSeek(m, f);
			break;

		case FG_SetSize:
			unless(mode & O_WriteOnly)
				goto badmode;
			else
			{
				word            newsize = m->Control[0];

				if (newsize > f->Upb)
					ErrorMsg(m, (EC_Error + EG_Parameter + 1));
				else {
					if (newsize < f->Upb)
						f->Upb = newsize;
					if (!RRDAdjustBuffers(f, 0, f->Upb, mypool)) {
						DEBUG("FG_SetSize:Error");

						ErrorMsg(m, (EC_Error | EG_NoMemory));
						break;
					}
					InitMCB(m, 0, m->MsgHdr.Reply, NullPort, Err_Null);
					MarshalWord(m, f->Upb);
					PutMsg(m);
				}
				break;
			}

		case FG_Select:
			/* a file is always ready for I/O so reply now */
			ErrorMsg(m, (e & Flags_Mode));
			break;

		default:
			ErrorMsg(m, (EC_Error + EG_FnCode + EO_File));
			break;
	badmode:
			ErrorMsg(m, (EC_Error + EG_WrongFn + EO_File));
			break;
		}
		Signal(&f->ObjNode.Lock);
	}

	f->Users--;
	FreePort(reqport);
	return;
}

/*---------------------------------------------------------------------------*/

static void     do_create(ServInfo * servinfo)
{
	MCB            *m = servinfo->m;
	MsgBuf         *r;
	DirNode        *d;
	File           *f;
	IOCCreate      *req = (IOCCreate *) (m->Control);
	char           *pathname = servinfo->Pathname;
	MIInfo         *mypool;

	d = GetTargetDir(servinfo);

	if (d == NULL) {
		ErrorMsg(m, EO_Directory);
		return;
	}

	mypool = GetRootPool(d);

	if (mypool->MIWriteProtect) {
		ErrorMsg(m, (EC_Error + EG_WriteProtected + EO_Directory));
		return;
	}

	f = (File *) GetTargetObj(servinfo);

	if (f != NULL) {
		ErrorMsg(m, (EC_Error + EG_Create + EO_File));
		return;
	}
	unless(CheckMask(req->Common.Access.Access, AccMask_W)) {
		ErrorMsg(m, (EC_Error + EG_Protected + EO_Directory));
		return;
	}

	r = New(MsgBuf);

	if (r == NULL) {
		ErrorMsg(m, (EC_Error + EG_NoMemory));
		return;
	}
	f = CreateNode(m, d, pathname, mypool);

	if (f == NULL) {
		ErrorMsg(m, 0);
		Free(r);
		return;
	}
	FormOpenReply(r, m, &f->ObjNode, 0, pathname);

	PutMsg(&r->mcb);

	Free(r);
	return;
}

/*---------------------------------------------------------------------------*/

/*--------------------------------------------------------
-- do_rename						--
--							--
-- General support for an internal rename operation	--
--							--
--------------------------------------------------------*/
static void	do_rename(ServInfo * servinfo)
{
	MCB *m = servinfo->m;
	IOCMsg2 *req = (IOCMsg2 *)(m->Control);
	byte *data = m->Data;
	word hdr = *(word *)m;
	AccMask mask = req->Common.Access.Access;
	DirNode *src, *dest;
	ObjNode *o, *o1;
	char *pathname = servinfo->Pathname;
	MIInfo *mypool;
	Handle fh2;
	RRDFileBlock *fb1, *fb2;
	int namesize;

	src = GetTargetDir(servinfo);

	if( src == Null(DirNode) ) {
		ErrorMsg(m,EO_Directory);
		return;
	}

	unless( CheckMask(req->Common.Access.Access,AccMask_W) ) {
		ErrorMsg(m,EC_Error+EG_Protected+EO_Directory);
		return;
	}

	if( (o = GetTargetObj(servinfo)) == NULL ) {
		ErrorMsg(m,EC_Error+EG_Unknown);
		return;
	}

	if( o == (ObjNode *)&Root ) {
		ErrorMsg(m,EC_Error+EG_WrongFn+EO_Directory);
		return;
	}

	/* we now have the source object, find the dest dir	*/
	/* restore state to context dir				*/
	/* at this point we have the target locked		*/
	
	*(word *)m = hdr;
	req->Common.Access.Access = mask;
	req->Common.Next = req->Arg.ToName;
	servinfo->Target = (ObjNode *)(servinfo->Context);
	Wait(&servinfo->Target->Lock);		/* relock context	*/

	dest = GetTargetDir( servinfo );

	if( dest == NULL ) {
		ErrorMsg(m,EO_Directory);
		Signal(&o->Lock);		/* unlock object	*/
		return;
	}

	unless( CheckMask(req->Common.Access.Access,AccMask_W) ) {
		ErrorMsg(m,EC_Error+EG_Protected+EO_Directory);
		Signal(&o->Lock);		/* unlock object	*/
		return;
	}

	o1 = GetTargetObj(servinfo);

	if( o1 != NULL ) {
		ErrorMsg(m,EC_Error|EG_Create);
		Signal(&o->Lock);		/* unlock object	*/
		return;
	}
	else m->MsgHdr.FnRc = Err_Null;
		
	/* if we get here we have the source and dest directories	*/
	/* the object itself, and the new name is objname(pathname).	*/
	/* Now do the rename.						*/

	if ((mypool = GetRootPool(src)) != GetRootPool(dest)) {
		ErrorMsg(m,EC_Error|EG_WrongFn|EO_Object);
		Signal(&o->Lock);		/* unlock object	*/
	}

	if (mypool->MIWriteProtect) {
		ErrorMsg(m, (EC_Error + EG_WriteProtected + EO_Directory));
		return;
	}

	/* rename permanent fileblock */
	fb1 = MILock(mypool, (Handle)((File *)o)->Buffers);

	/* work out extra space required for name */
	namesize = strlen(shortname(&data[req->Common.Context]));
	if (namesize != 0)
		namesize++; /* incase we need to add "/" */
	namesize += strlen(&data[req->Arg.ToName]);
	
	/* allocate fileblock of the correct new size */
	if ( (fh2 = MIAlloc(mypool, sizeof(RRDFileBlock) + namesize)) == NULL) {
		ErrorMsg(m,EC_Error|EG_NoMemory|EO_File);
		Signal(&o->Lock);		/* unlock object	*/
	}

	/* copy original contents, but with new name and date */
	fb2 = MILock(mypool, fh2);
	memcpy(fb2, fb1, sizeof(RRDFileBlock));

	/* concatenate the relative name (to /ram/sys) from the context */
	/* and target strings */
	strcpy(fb2->FullName, shortname(&data[req->Common.Context]));
	if (*fb2->FullName != '\0')
		strcat(fb2->FullName, "/");
	strcat(fb2->FullName, &data[req->Arg.ToName]);

	fb2->Date = o->Dates.Modified = o->Dates.Access = GetDate();
	fb2->c.Checksum = Checksum((RRDBuffBlock *) fb2);
	MIUnLock(mypool, fh2);

#if 0
	IOdebug("Rename old: %s, new %s, context %s, shortcont %s, ToName %s",
	o->Name,
	fb2->FullName,
	&data[req->Common.Context],
	shortname(&data[req->Common.Context]),
	&data[req->Arg.ToName]);
#endif

	/* get rid of original fileblock */
	fb1->c.Magic = 0;
	MIFree(mypool, (Handle)((File *)o)->Buffers);
	/* replacing with new file block */
	((File *)o)->Buffers = (Node *)fh2;

	/* rename nonpersistent object */
	strcpy(o->Name,objname(pathname));

	if( src != dest ) {
		/* if different location within servers hierarchy, then 'mv' */
		Unlink(o, false);
		Insert((DirNode *)servinfo->Target, o, servinfo->TargetLocked);
	}

	Signal(&o->Lock);		/* unlock object */
	UnLockTarget( servinfo );	/* unlock dest dir */

	ErrorMsg(m, Err_Null);	
}

/*---------------------------------------------------------------------------*/

/* Handle GSP file deletion requests */
static void     do_delete(ServInfo * servinfo)
{
	MCB            *m = servinfo->m;
	DirNode        *d;
	File           *f;
	IOCCommon      *req = (IOCCommon *) (m->Control);
	MIInfo         *mypool;

	d = GetTargetDir(servinfo);

	if (d == NULL) {
		ErrorMsg(m, EO_Directory);
		return;
	}

	mypool = GetRootPool(d);

	if (mypool->MIWriteProtect) {
		ErrorMsg(m, (EC_Error + EG_WriteProtected + EO_Directory));
		return;
	}

	f = (File *) GetTargetObj(servinfo);

	if (f == NULL) {
		ErrorMsg(m, EO_File);
		return;
	}
	unless(CheckMask(req->Access.Access, AccMask_D)) {
		ErrorMsg(m, (EC_Error + EG_Protected + EO_File));
		return;
	}

	if (f->ObjNode.Type == Type_Directory) {
		if (((DirNode *) f)->Nentries != 0) {
			ErrorMsg(m, (EC_Error + EG_Delete + EO_Directory));
			return;
		}
		else {
			RRDDirBlock    *dir;

			dir = MILock(mypool, (Handle) ((DirNode *) f)->DirBlockH);
			/* don't allow dir be resurrected */
			dir->c.Magic = 0;
			/* Free dir block header */
			MIFree(mypool, (Handle) ((DirNode *) f)->DirBlockH);
		}
	}
	else if (f->ObjNode.Type == Type_File) {
		RRDBuffBlock   *buf;

		if (f->Users != 0) {
			ErrorMsg(m, (EC_Error + EG_InUse + EO_File));
			return;
		}
		f->Upb = 0;
		RRDAdjustBuffers(f, 0, 0, mypool);
		buf = MILock(mypool, (Handle) f->Buffers);
		buf->c.Magic = 0;	/* don't let file be resurrected */
		MIFree(mypool, (Handle) f->Buffers);	/* Free file block
							 * header */
	}
	else if (f->ObjNode.Type == Type_Link) {
		RRDSymbBlock   *sb;

		sb = MILock(mypool, (Handle) ((LinkNode *) f)->SymbBlockH);
		/* don't allow link be resurrected */
		sb->c.Magic = 0;
		/* Free link block header */
		MIFree(mypool, (Handle) ((LinkNode *) f)->SymbBlockH);
	}

	Unlink(&f->ObjNode, FALSE);
	servinfo->TargetLocked = FALSE;	/* to stop ServLib worker indirection through Free'd target on unlock */
	Free(f);

	ErrorMsg(m, Err_Null);
}

/*---------------------------------------------------------------------------*/

/* Std fileserver server_info protocol to return filesystem stats */

static void     do_serverinfo(ServInfo * servinfo)
{
	MCB		* mcb = servinfo->m;
	DirNode		* d;
	ObjNode		* f;
	serv_info	* info = (serv_info *) mcb->Data;
	MIInfo		* mypool;

	if ((d = GetTargetDir(servinfo)) == NULL) {
		ErrorMsg(mcb, EO_Directory);
		return;
	}

	f = GetTargetObj(servinfo);

	mypool = GetRootPool(f == NULL ? d : (DirNode *)f);

	info->type = Type_Directory;
	info->alloc = BlockMax;

#ifdef ALLDEBUG
	IOdebug("Servinfo: freeMax %d, FreeSize %d, AllocMax %d, AllocSize %d",mypool->FreePool->Max,mypool->FreePool->Size,mypool->DstPool->Max,mypool->DstPool->Size);
#endif
	/* max size */
	info->size = mypool->FreePool->Max;
	/* whats left of max size to use */
	info->available = mypool->FreePool->Size;
	/* space used =  size - available */

	InitMCB(mcb,0,mcb->MsgHdr.Reply,NullPort,Err_Null);
	MarshalData(mcb, sizeof(serv_info), (byte *)info);
	PutMsg(mcb);
	return;
}

/*---------------------------------------------------------------------------*/

/* Handle GSP request for creating symbolic links */

static void     do_link(ServInfo * servinfo)
{
	MCB            *m = servinfo->m;
	IOCMsg3        *req = (IOCMsg3 *) (m->Control);
	byte           *data = m->Data;
	DirNode        *d;
	ObjNode        *o;
	LinkNode       *l;
	MIInfo         *mypool;

	d = GetTargetDir(servinfo);

	if (d == Null(DirNode)) {
		ErrorMsg(m, EO_Directory);
		return;
	}

	mypool = GetRootPool(d);

	if (mypool->MIWriteProtect) {
		ErrorMsg(m, (EC_Error + EG_WriteProtected + EO_Directory));
		return;
	}

	unless(CheckMask(req->Common.Access.Access, AccMask_W)) {
		ErrorMsg(m, (EC_Error + EG_Protected + EO_Directory));
		return;
	}

	o = GetTargetObj(servinfo);

	if (o != Null(ObjNode)) {
		ErrorMsg(m, (EC_Error + EG_Create));
		return;
	}
	else
		m->MsgHdr.FnRc = 0;

	/* We now know that there is not an existing entry with the	 */
	/* desired name. Install the link.				 */

	l = (LinkNode *) ServMalloc((word) sizeof(LinkNode) + (word) strlen(&data[req->Name]));

	if (l == Null(LinkNode)) {
		ErrorMsg(m, (EC_Error + EG_NoMemory + EO_Link));
		return;
	}
	InitNode(&l->ObjNode, objname(servinfo->Pathname), Type_Link, 0, DefLinkMatrix);
	l->Cap = req->Cap;
	strcpy(l->Link, &data[req->Name]);

	if ((l->SymbBlockH = (Node *) InitSymbBlock(l, shortname(servinfo->Pathname), mypool)) == NULL) {
		Free(l);
		ErrorMsg(m, (EC_Error + EG_NoMemory + EO_Link));
		return;
	}
	Insert((DirNode *) servinfo->Target, &l->ObjNode, servinfo->TargetLocked);

	ErrorMsg(m, Err_Null);
	return;
}


/*--------------------------------------------------------
-- do_setdate						--
--							--
-- Alter the date on an object				--
--							--
--------------------------------------------------------*/

static void do_setdate(ServInfo *servinfo)
{
	MCB     *m = servinfo->m;
	IOCMsg4 *req = (IOCMsg4 *)(m->Control);
	ObjNode *o;
	DateSet *dates = &req->Dates;
	word     e = Err_Null;
	MIInfo  *mypool;
	RRDFileBlock *fb;

	
#ifdef ALLDEBUG
       IOdebug("DoSetDate %x %x %s",m,context,servinfo->pathname);
#endif

	o = GetTarget(servinfo);

	if( o == Null(ObjNode) )
	{
		ErrorMsg(m,0);
		return;
	}

	mypool = GetRootPool((DirNode *)o);

	if (mypool->MIWriteProtect) {
		ErrorMsg(m, (EC_Error + EG_WriteProtected + EO_Directory));
		return;
	}

	/* only allow the user to set the date on an object if he	*/
	/* could write to it.						*/
	unless( CheckMask(req->Common.Access.Access,AccMask_W) ) 
	{
		ErrorMsg(m,EC_Error+EG_Protected);
		return;
	}

	if ( dates->Creation != 0 ) o->Dates.Creation = dates->Creation;
	if ( dates->Access != 0 ) o->Dates.Access = dates->Access;
	if ( dates->Modified != 0 ) o->Dates.Modified = dates->Modified;

	/* set date in reset proof block */
	fb = MILock(mypool, (Handle) ((File *)o)->Buffers);
	if ( dates->Creation != 0 ) fb->Date = dates->Creation;
	if ( dates->Access != 0 ) fb->Date = dates->Access;
	if ( dates->Modified != 0 ) fb->Date = dates->Modified;
	MIUnLock(mypool, (Handle) ((File *)o)->Buffers);

	ErrorMsg(m,e);
}


/*--------------------------------------------------------
-- do_protect						--
--							--
-- Alter the matrix on an object			--
--							--
--------------------------------------------------------*/

static void do_protect(ServInfo *servinfo)
{
	MCB *m = servinfo->m;
	IOCMsg2 *req = (IOCMsg2 *)(m->Control);
	ObjNode *o;
	Matrix newmatrix = req->Arg.Matrix;
	MIInfo  *mypool;
	RRDFileBlock *fb;

	o = GetTarget(servinfo);

	if( o == Null(ObjNode) )
	{
		ErrorMsg(m,0);
		return;
	}

	mypool = GetRootPool((DirNode *)o);

	if (mypool->MIWriteProtect) {
		ErrorMsg(m, (EC_Error + EG_WriteProtected + EO_Directory));
		return;
	}

	unless( CheckMask(req->Common.Access.Access,AccMask_A) ) 
	{
		ErrorMsg(m,EC_Error+EG_Protected);
		return;
	}

	/* We are allowed to alter the matrix, ensure that it is 	*/
	/* resonable, and that someone still has delete or alter access */

	if( (UpdMask(AccMask_Full, newmatrix) & (AccMask_A|AccMask_D)) == 0 )
	{
		ErrorMsg(m,EC_Error+EG_Invalid+EO_Matrix);
		return;
	}

	/* set new matrix and modified date on both dynamic and permanent blocks */
	fb = MILock(mypool, (Handle) ((File *)o)->Buffers);
	fb->Matrix = o->Matrix = newmatrix;
	fb->Date = o->Dates.Modified = o->Dates.Access = GetDate();
	MIUnLock(mypool, (Handle) ((File *)o)->Buffers);
	
	ErrorMsg(m,0);
}

/*--------------------------------------------------------
-- do_revoke						--
--							--
-- Alter the encryption key on an object, invalidating	--
-- all existing capabilities.				--
--							--
--------------------------------------------------------*/

static void do_revoke(ServInfo *servinfo)
{
	MCB *m = servinfo->m;
	IOCCommon *req = (IOCCommon *)m->Control;
	ObjNode *o;
	Capability cap;
	MIInfo  *mypool;
	RRDFileBlock *fb;
		
	o = GetTarget(servinfo);

	if( o == Null(ObjNode) )
	{
		ErrorMsg(m,0);
		return;
	}

	mypool = GetRootPool((DirNode *)o);

	if (mypool->MIWriteProtect) {
		ErrorMsg(m, (EC_Error + EG_WriteProtected + EO_Directory));
		return;
	}

	/* only allow revocation if the target is the context object 	*/
	/* and the client has alter rights 				*/
	unless( o == (ObjNode *)servinfo->Context && 
		CheckMask(req->Access.Access,AccMask_A) ) 
	{
		ErrorMsg(m,EC_Error+EG_Protected);
		return;
	}

	/* get out all the rights encoded in the capability	*/
	req->Access.Access = AccMask_Full;
	GetAccess(&req->Access, o->Key);
	
	fb = MILock(mypool, (Handle) ((File *)o)->Buffers);
	/* change the key */
	fb->Key = o->Key = NewKey();
	fb->Date = o->Dates.Modified = o->Dates.Access = GetDate();
	MIUnLock(mypool, (Handle) ((File *)o)->Buffers);

	/* pass back a new capability with all the original rights */
	NewCap(&cap, o, req->Access.Access);
	InitMCB(m,0,m->MsgHdr.Reply,NullPort,Err_Null);
	MarshalCap(m,&cap);
	PutMsg(m);
}

/* End of GSP interface functions */


/*---------------------------------------------------------------------------*/
/*-- FileRead : Read data from the File and return it to the client ---------*/
/*---------------------------------------------------------------------------*/

static void     FileRead(MCB * m, File * f, MIInfo * mypool)
{
	ReadWrite      *rw = (ReadWrite *) m->Control;
	word            pos = rw->Pos;
	word            size = rw->Size;
	Port            reply = m->MsgHdr.Reply;
	word            offset = 0;
	word            sent = 0;
	word            seq = 0;
	word            e = Err_Null;
	Handle		starth = (Handle)f->Buffers;
	RRDFileBlock   *fileblock = MILock(mypool, starth);
	Handle          bufh = NULL;
	RRDBuffBlock   *buf;

	f->ObjNode.Dates.Access = GetDate();

	if (pos < 0) {
		ErrorMsg(m, (EC_Error | EG_Parameter | 1));
		return;
	}
	if (size < 0) {
		ErrorMsg(m, (EC_Error | EG_Parameter | 2));
		return;
	}
	if (pos >= f->Upb) {
		m->MsgHdr.FnRc = ReadRc_EOF;
		ErrorMsg(m, 0);
		return;
	}
	if ((pos + size) > f->Upb)
		size = rw->Size = f->Upb - pos;

	InitMCB(m, MsgHdr_Flags_preserve, reply, NullPort, ReadRc_More);

#ifdef READCHECK2
	/*
	 * Check fileblock - cannot delete as pointers may have been corrupted 
	 */
	if (fileblock->c.Checksum != Checksum((RRDBuffBlock *) fileblock)) {
		DEBUG("FileRead: Checksum error on fileblock!!!");

		goto readerror;
	}
#endif

	if ((buf = RRDGetReadBuffer(pos, (Handle)f->BufPosHint, fileblock->c.Next, &bufh, mypool)) == NULL) {
		/* if there are no buffers, EOF */
		DEBUG("FileRead: no buffers1 - should never happen!!!");

		goto readerror;
	}

	/* start read at correct place in buffer */
	offset = pos - buf->Pos;

	do {
		word            dsize;

#ifdef SYSDEB
		/* if there are no buffers, EOF */
		if (buf == NULL) {
			DEBUG("FileRead: no buffers2!!!! - should never happen");
			goto readerror;
		}
#endif				/* SYSDEB */

#ifdef READCHECK
		if (buf->c.Checksum != Checksum(buf)) {
#ifdef SYSDEB
			IOdebug("RamDisk: ReadFile: buffer block CHECKSUM failure!, handle %x", bufh);
#endif
			goto readerror;
		}
#endif
		dsize = buf->DataSize - offset;

		if ((dsize + sent) >= size) {
			dsize = size - sent;
			m->MsgHdr.FnRc = (ReadRc_EOD | seq);
			m->MsgHdr.Flags = 0;
		}
		m->Data = RTOA(buf->DataStart) + offset;	/* &buf->Data[offset] ; */
		m->MsgHdr.DataSize = (unsigned short) dsize;

		if ((e = PutMsg(m)) < Err_Null) {
			DEBUG("Ram Disk: FileRead: PutMsg() error");

			MIUnLock(mypool, starth);
			MIUnLock(mypool, bufh);
			return;
		}
		offset += dsize;
		sent += dsize;
		seq += ReadRc_SeqInc;
		m->MsgHdr.FnRc = (ReadRc_More | seq);

		if (offset >= buf->DataSize) {
			Handle          next = buf->c.Next;

			MIUnLock(mypool, bufh);
			if ((bufh = next) == NULL) {
#ifdef SYSDEB
				if(sent<size)
					DEBUG("Ram Disk: FileRead: EOF before requested amount sent");
#endif
				break; /*EOF reached*/
			}

			buf = MILock(mypool, next);
			offset = 0;
		}
	} while (sent < size);

	f->BufPosHint = (Node *)bufh;
	if (bufh != NULL)
		MIUnLock(mypool, bufh); /* if not @ EOF */
	MIUnLock(mypool, starth);

/*	rw->Size = sent; ??? */

	return;

readerror:
	MIUnLock(mypool, starth);
	m->MsgHdr.Flags = 0;
	m->MsgHdr.FnRc = ReadRc_EOF;
	m->MsgHdr.DataSize = 0;
	PutMsg(m);

	return;
}

/*--------------------------------------------------------
-- FileWrite						--
-- 							--
-- Write data to the File.				--
-- To preserve retryability we must allow the client	--
-- to over-write existing data.				--
--							--
--------------------------------------------------------*/

static void     FileWrite(MCB * m, File * f, MIInfo * mypool)
{
	ReadWrite      *rw = (ReadWrite *) m->Control;
	word            pos = rw->Pos;
	word            size = rw->Size;
	Port            request = m->MsgHdr.Dest;
	Port            reply = m->MsgHdr.Reply;
	word            msgdata = m->MsgHdr.DataSize;
	word            offset = 0;
	word            got = 0;
	word            e = Err_Null;
	Handle		starth = (Handle) f->Buffers;
#if 0
	RRDFileBlock   *fileblock = MILock(mypool, starth);
#else
	RRDFileBlock   *fileblock;
#endif
	Handle          bufh = NULL;
	RRDBuffBlock   *buf;

#if 1
Output("FileWrite:");WriteHex8(starth);Output("\n");
Output("FileWrite:lock 1\n");
fileblock = MILock(mypool, starth);
#endif

	if (pos > f->Upb) {
		ErrorMsg(m, (EC_Error | EG_Parameter | 1));
		return;
	}

	/* will write enlarge the file? */
	if (f->Upb < (pos + size)) {

		f->Upb = pos + size;

		if (!RRDExtendBuffers(f, f->Upb, pos, fileblock, mypool)) {
			/* work out size actually alloc'ed and return err */
			if (f->TailBuffer != NULL) {
#if 1
Output("FileWrite:lock 2\n");
#endif
				buf = MILock(mypool, (Handle) f->TailBuffer);
				f->Upb = buf->Pos + BlockMax;
				MIUnLock(mypool, (Handle) f->TailBuffer);
			}
			else {
				f->Upb = 0;
			}
			MIUnLock(mypool, (Handle) f->Buffers);
			m->MsgHdr.FnRc = 0;
			ErrorMsg(m, (EC_Error | SS_RamDisk | EG_NoMemory | EO_File));
			DEBUG("Filewrite: extend failed: return");

			return;
		}
	}

#ifdef READCHECK2
	/*
	 * Check fileblock - cannot delete as pointers may have been corrupted 
	 */
	if (fileblock->c.Checksum != Checksum((RRDBuffBlock *) fileblock)) {
		DEBUG("FileWrite: Checksum error on fileblock!!!");

		e = (EC_Error | EG_Broken);
		goto done;
	}
#endif
	if ((buf = RRDGetWriteBuffer(pos, (Handle)f->BufPosHint, fileblock->c.Next, &bufh, mypool)) == NULL) {
		DEBUG("FileWrite:Err nomem1");

		e = (EC_Error | EG_NoMemory);
		goto done;
	}

/*	DEBUG("WR pos %d, size %d, hintb %x, realb %x",pos,size,(word)f->BufPosHint,(word)bufh);*/

	offset = pos - buf->Pos;

	/* If the user has supplied any data in the request, deal with	 */
	/* it here and adjust things for later.			 */
	if (msgdata > 0) {
		while (got < msgdata) {
			word            dsize = BlockMax - offset;

			/* if buffer is bigger than msg adjust to msg size */
			if (dsize > (size - got))
				dsize = size - got;

			memcpy(RTOA(buf->DataStart) + offset, &m->Data[got], (int) dsize);

			if ((offset + dsize) > buf->DataSize)
				buf->DataSize = offset + dsize;
			got += dsize;

			if ((offset + dsize) >= BlockMax) {
				Handle          next = buf->c.Next;

				RRDPutWriteBuffer(buf, bufh, mypool);
#if 1
Output("FileWrite:lock 3\n");
#endif
				buf = MILock(mypool, next);
				bufh = next;

#ifdef SYSDEB
				if (buf == NULL) {
					IOdebug("FileWrite:Err nonmem 3 - poss happen.");
					e = (EC_Error | EG_NoMemory);
					goto done;
				}
#endif
			}
			offset = 0;
		}

		pos += got;
		offset = pos - buf->Pos;
		if (got == size)
			goto done;
	}
	InitMCB(m, MsgHdr_Flags_preserve, reply, NullPort, WriteRc_Sizes);

	/* GSP protocol: size of first data block to send */
	/* BlockMax == buf->c.TotalSize - sizeof(RRDBuffBlock) */
	MarshalWord(m, (BlockMax - offset));
	/* size of subsequent data blocks to send */
	MarshalWord(m, TransferMax);

	if ((e = PutMsg(m)) < Err_Null) {
		RRDPutWriteBuffer(buf, bufh, mypool);
		MIUnLock(mypool, starth);
		DEBUG("FileWrite:error in putmsg");

		return;
	}
	m->MsgHdr.Dest = request;
	m->Timeout = WriteTimeout;

	while (got < size) {
		word            dsize;

#ifdef SYSDEB
		if (buf == NULL) {
			IOdebug("FileWrite:err no mem - should never happen!!!");
			e = (EC_Error | EG_NoMemory);
			goto done;
		}
#endif

		m->Data = RTOA(buf->DataStart) + offset;

		if ((e = GetMsg(m)) < Err_Null) {
			e &= ~SS_Mask;
			e |= SS_SysLib;
			DEBUG("FileWrite:getmsg<ErrNull");

			break;
		}
		dsize = m->MsgHdr.DataSize;

		if ((offset + dsize) > buf->DataSize)
			buf->DataSize = offset + dsize;

		got += dsize;
		if (got >= size)
			break; /* quick fix to stop MILock error */

		offset += dsize;
		if (offset >= BlockMax) {
			Handle          next = buf->c.Next;

			RRDPutWriteBuffer(buf, bufh, mypool);
#if 1
Output("FileWrite:lock 4: ");WriteHex8(next);Output("\n");
#endif
			bufh = next;
			offset = 0;

			buf = MILock(mypool, next);
#if 1
Output("FileWrite:after lock 4: ");WriteHex8((word)buf);Output("\n");
#endif
		}
#ifdef SYSDEB
		if (offset > BlockMax)
			IOdebug("FileWrite: ARGHH = Written over end of buffer!!!");
#endif
	}

done:
	/* all done, tell user how much data we got */
	if (buf != NULL) {
		f->BufPosHint = (Node *)bufh;
		RRDPutWriteBuffer(buf, bufh, mypool);
	}

	InitMCB(m, 0, reply, NullPort, ((e < 0) ? e : WriteRc_Done));
	MarshalWord(m, got);
	PutMsg(m);

	fileblock->Date = f->ObjNode.Dates.Access = f->ObjNode.Dates.Modified = GetDate();
	fileblock->c.Checksum = Checksum((RRDBuffBlock *) fileblock);
	MIUnLock(mypool, starth);

	rw->Size = got;

	return;
}

/*--------------------------------------------------------
-- FileSeek						--
-- 							--
-- Seek to a new file position. This is merely a hint	--
-- which is useful to 'real' file servers but is of no	--
-- use to us. Perform the necessary calculation in any	--
-- case.						--
--							--
--------------------------------------------------------*/

static void     FileSeek(MCB * m, File * f)
{
	SeekRequest    *req = (SeekRequest *) m->Control;
	word            curpos = req->CurPos;
	word            mode = req->Mode;
	word            newpos = req->NewPos;

	switch (mode) {
	case S_Beginning:
		break;

	case S_Relative:
		newpos += curpos;
		break;

	case S_End:
		newpos += f->Upb;
		break;
	}

	InitMCB(m, 0, m->MsgHdr.Reply, NullPort, Err_Null);

	MarshalWord(m, newpos);

	PutMsg(m);
	return;
}

/*---------------------------------------------------------------------------*/

/* Some of these functions are stolen from ServLib and converted to use */
/* relocatable blocks provided by the MI (Memory Indirection) functions */

/*--------------------------------------------------------
-- RRDGetReadBuffer					--
-- RRDGetWriteBuffer					--
--							--
-- When a buffer list is being maintained using 	--
-- AdjustBuffers, These are used by FileRead and 	--
-- FileWrite to calculate the correct MI BuffBlock to	--
-- use for a given file pointer.			--
--							--
-- If they find the correct block it is locked down and	--
-- a pointer to it is passed back. It is the		--
-- responsibility of the caller to unlock the buffer	--
-- after it has been used, or alternatively for Write	--
-- buffers, call PutWriteBuffer().			--
--							--
-- The hint is the last recorded buffer that a stream	--
-- wrote to. keeping the server stateless and allowing	--
-- randomly positioned writes means that it must be	--
-- checked for consistency though.			--
--							--
--------------------------------------------------------*/

static RRDBuffBlock *RRDGetReadBuffer(word pos, Handle hint, Handle nextb, Handle * bh, MIInfo * mypool)
{
	RRDBuffBlock   *buf;
	Handle          thisb;

	/* check hint */
	if (hint != NULL) {
		if ((buf = MILock(mypool, hint)) != NULL) {
			if ((buf->Pos <= pos) && (pos < (buf->Pos + buf->DataSize))) {
				*bh = hint;
				return (buf);
			}
			MIUnLock(mypool, hint);
		}
	}

	/* hint is not correct - find out the labourious way */
	while (nextb != NULL) {
		thisb = nextb;
		buf = MILock(mypool, thisb);
		nextb = buf->c.Next;

		if ((buf->Pos <= pos) && (pos < (buf->Pos + buf->DataSize))) {
			*bh = thisb;
			return (buf);
		}
		MIUnLock(mypool, thisb);
	}

	*bh = NULL;
	return (NULL);
}

/*---------------------------------------------------------------------------*/

static RRDBuffBlock *RRDGetWriteBuffer(word pos, Handle hint, Handle nextb, Handle * bh, MIInfo * mypool)
{
	RRDBuffBlock   *buf;
	Handle          thisb;

	/* check hint */
	if (hint != NULL) {
#if 1
Output("GetWriteBuffer:lock 1\n");
#endif
		if ((buf = MILock(mypool, hint)) != NULL) {
			if ((buf->Pos <= pos) && (pos < (buf->Pos + BlockMax))) {
				*bh = hint;
				return (buf);
			}
			MIUnLock(mypool, hint);
		}
	}

	/* hint is not correct - find out the labourious way */
	while (nextb != NULL) {
		thisb = nextb;
#if 1
Output("GetWriteBuffer:lock 2\n");
#endif
		buf = MILock(mypool, thisb);
		nextb = buf->c.Next;

		if ((buf->Pos <= pos) && (pos < (buf->Pos + BlockMax)))
		/* BlockMax == buf->c.TotalSize - sizeof(RRDBuffBlock))  */
		{
			*bh = thisb;
			return (buf);
		}
		MIUnLock(mypool, thisb);
	}

	*bh = NULL;
	return (NULL);
}

/*--------------------------------------------------------
-- RRDPutWriteBuffer					--
--							--
-- Used to update a block's checksum after it has been	--
-- written. The block is then unlocked.			--
--							--
--------------------------------------------------------*/
static void     RRDPutWriteBuffer(RRDBuffBlock * buf, Handle h, MIInfo * mypool)
{
#ifdef WRITECHECK
        buf->c.Checksum = Checksum(buf);
#endif
	MIUnLock(mypool, h);
	return;
}

/*--------------------------------------------------------
-- RRDExtendBuffers					--
-- 							--
-- Expands the given buffer list to the new size	--
-- An optimisation of Adjust buffers for large file	--
-- writes.						--
--							--
-- If the current pos lies within an existing file the	--
-- hint isn't updated, otherwise it is set at the start	--
-- of the first block allocated.			--
--							--
-- Only the ExtendBuffers and AdjustBuffers functions	--
-- resize the buffer list.				--
--							--
--------------------------------------------------------*/

static bool	RRDExtendBuffers(File *f, word end, word pos, RRDFileBlock *fileblock, MIInfo * mypool)
{
	word            currentend;	/* pos of 1st byte beyond end of list */
	Handle          thisb = NULL;
	Handle          nextb = fileblock->c.Next;
	RRDBuffBlock   *buf;
	bool            tmpfile = (fileblock->c.Magic == RRDTmpMagic) ? TRUE : FALSE;
	bool		recordhintblock = TRUE;

#ifdef ALLDEBUG
	IOdebug("ExtendBuffers: end:%d", end);
#endif
	/* only record new hint block if positioned on a block boundary */
	if (pos % BlockMax)
		recordhintblock = FALSE;

	/* adjust to block boundaries */
	end += BlockMax - 1;	/* desired value for last */
	end -= end % BlockMax;	/* points to start of next buf */

	/* At end of blocks */
	/* See if we need to append new blocks to satisfy request */

	if (fileblock->c.Next == NULL) { /* empty file */
		currentend = 0;
		buf = (RRDBuffBlock *) fileblock;
		thisb = NULL;
	}
	else { /* extend existing file */
#ifdef SYSDEB /* @@@ I DONT THINK THIS IS REALLY NECESSARY ????????????????? */
		if (f->TailBuffer == NULL) {
			DEBUG("ExtendBuffers: Damn!!! inconsistent tail buffer");
			
			/* recover here by trudging to the end of list */
			nextb = fileblock->c.Next;
			while (nextb != NULL) {
				f->TailBuffer = (Node *) (thisb = nextb);
#if 1
Output("ExtendBuffers:lock 1\n");
#endif
				buf = MILock(mypool, thisb);
				nextb = buf->c.Next;
				MIUnLock(mypool, thisb);
			}
		}
#endif
		thisb = (Handle)f->TailBuffer;
#if 1
Output("ExtendBuffers:lock 2\n");
#endif
		buf = MILock(mypool, thisb);
		currentend = buf->Pos + BlockMax;
	}

	while (currentend < end) {
		/* add new blocks */

		nextb = buf->c.Next = MIAlloc(mypool, sizeof(RRDBuffBlock) + BlockMax);

		if (thisb != NULL)
			MIUnLock(mypool, thisb);

		if (nextb == NULL) {
			/* if error in alloc */
#ifdef SYSDEB
			IOdebug("ExtendBuffers: couldn't alloc block! (%d)", sizeof(RRDBuffBlock) + BlockMax);
#endif
			f->TailBuffer = (Node *)thisb;

			return (false);
		}

		if (recordhintblock) {
			f->BufPosHint = (Node *)nextb;
			recordhintblock = FALSE; /* record only the first block */
		}
		
		/* initialise a new buffer block */
		thisb = nextb;
#if 1
Output("ExtendBuffers:lock 3\n");
#endif
		buf = MILock(mypool, thisb);

		buf->c.Magic = tmpfile ? RRDTmpMagic : RRDBuffMagic;
		buf->c.TotalSize = sizeof(RRDBuffBlock) + BlockMax;
		buf->Pos = currentend;
		buf->FileId = fileblock->FileId;
		buf->DataSize = 0;
		buf->DataStart = 4;	/* buf->DataStart = &((char *)buf->DataStart) + 4;ATOR(buf->DataStart); */

#ifdef WRITECHECK
		buf->c.Checksum = Checksum(buf);
#endif
		currentend += BlockMax;
	}

	f->TailBuffer = (Node *)thisb;
	buf->c.Next = NULL;

	MIUnLock(mypool, thisb);

	return (true);
}


/*--------------------------------------------------------
-- RRDAdjustBuffers					--
-- 							--
-- RRD buffer block support. 				--
-- Trims or expands the given buffer list so that the	--
-- given bounds lie within the buffers.			--
-- This routine assumes that the list has been allocated--
-- here.						--
--							--
-- Different from the ServLib AdjustBuffers, in that	--
-- It uses a const. blocksize and it is passed a handle	--
-- (single MI linked list) rather than a List pointer.	--
--							--
--------------------------------------------------------*/

static bool     RRDAdjustBuffers(File *f, word start, word end, MIInfo * mypool)
{
	word            currentend;	/* pos of 1st byte beyond end of list */
	Handle		startb = (Handle)f->Buffers;
	RRDFileBlock   *fileblock = MILock(mypool, startb);
	Handle          prevb = NULL;
	Handle          thisb = NULL;
	Handle          nextb = fileblock->c.Next;
	RRDBuffBlock   *buf;
	bool            tmpfile = (fileblock->c.Magic == RRDTmpMagic) ? TRUE : FALSE;
	bool		firstextendblock = TRUE;

#ifdef ALLDEBUG
	IOdebug("AdjustBuffers: start: %d, end:%d", start, end);
#endif
	/* adjust to block boundaries */
	start -= start % BlockMax;	/* pos of 1st byte in block */

	end += BlockMax - 1;	/* desired value for last */
	end -= end % BlockMax;	/* points to start of next buf */

#ifdef ALLDEBUG
	IOdebug("AdjustBuffers: altered start: %d end: %d", start, end);
#endif
	while (nextb != NULL) {
		thisb = nextb;
		buf = MILock(mypool, thisb);
		nextb = buf->c.Next;

		/* If buffer is before desired start, then delete it */
		if (buf->Pos < start) {
#ifdef ALLDEBUG
			IOdebug("AdjustBuffers: trim start");
#endif
			buf->c.Magic = 0;	/* make sure block stays
						 * deleted! */
			MIFree(mypool, thisb);

			/* note new start block */
			fileblock->c.Next = nextb;
			f->BufPosHint = NULL;

			continue;
		}
		/* If blocks exist past desired end, delete them and return */
		if (buf->Pos >= end) {
#ifdef ALLDEBUG
			IOdebug("AdjustBuffers:truncate");
#endif
			f->TailBuffer = (Node *)prevb;
			f->BufPosHint = NULL;

			/* Delete the first block past end */
			buf->c.Magic = 0;	/* make sure block stays
						 * deleted! */
			MIFree(mypool, thisb);

			/* Adjust last pointer to deleted blocks */
			if (prevb != NULL) {
				buf = MILock(mypool, prevb);
				buf->c.Next = NULL;
				MIUnLock(mypool, prevb);
			}
			else
				fileblock->c.Next = NULL;

			while (nextb != NULL) {
#ifdef ALLDEBUG
				IOdebug("AdjustBuffers:truncate");
#endif
				thisb = nextb;
				buf = MILock(mypool, thisb);
				nextb = buf->c.Next;

				buf->c.Magic = 0;	/* make sure block stays
							 * deleted! */
				MIFree(mypool, thisb);
			}

			MIUnLock(mypool, startb);	/* unlock the file block */

			return (true);
		}
#ifdef ALLDEBUG
		IOdebug("AdjBuf:not t/t bp:%d s:%d e:%d", buf->Pos, start, end);
#endif

		MIUnLock(mypool, thisb);
		prevb = thisb;
	}

	/* At end of blocks */
	/* See if we need to append new blocks to satisfy request */

	if (fileblock->c.Next == NULL) { /* empty file */
		currentend = start;
		buf = (RRDBuffBlock *) fileblock;
		thisb = NULL;
	}
	else { /* extend existing file */
		buf = MILock(mypool, thisb);
		currentend = buf->Pos + BlockMax;
	}

	while (currentend < end) {
		/* add new blocks */

		nextb = buf->c.Next = MIAlloc(mypool, sizeof(RRDBuffBlock) + BlockMax);

		if (thisb != NULL)
			MIUnLock(mypool, thisb);

		if (nextb == NULL) {
			/* if error in alloc */
#ifdef SYSDEB
			IOdebug("AdjustBuffers: extend: couldn't alloc block! (%d)", sizeof(RRDBuffBlock) + BlockMax);
#endif
			MIUnLock(mypool, startb);	/* unlock the file block */
			return (false);
		}

		if (firstextendblock) {
			f->BufPosHint = (Node *)nextb;
			firstextendblock = FALSE;
		}
		thisb = nextb;
		buf = MILock(mypool, thisb);

		/* initialise a new buffer block */
		buf->c.Magic = tmpfile ? RRDTmpMagic : RRDBuffMagic;
		buf->c.TotalSize = sizeof(RRDBuffBlock) + BlockMax;
		buf->Pos = currentend;
		buf->FileId = fileblock->FileId;
		buf->DataSize = 0;
		buf->DataStart = 4;	/* buf->DataStart = &((char *)buf->DataStart) + 4;ATOR(buf->DataStart); */

#ifdef WRITECHECK
		buf->c.Checksum = Checksum(buf);
#endif
		currentend += BlockMax;
		f->TailBuffer = (Node *)thisb;
	}

	buf->c.Next = NULL;

	if (thisb != NULL)
		MIUnLock(mypool, thisb);
	MIUnLock(mypool, startb);	/* unlock the file block */

	return (true);
}


/*--------------------------------------------------------
-- CreateFileId						--
-- 							--
-- Allocate a unique file ID. Each buffer block 	--
-- associated with a given file(block) will be marked	--
-- With this Id.					--
--							--
--------------------------------------------------------*/
static uword    CreateFileId(void)
{
	uword           FID;

	Wait(&FIDSem);
	FID = ++LastFID;
	Signal(&FIDSem);

	return (FID);
}

/*--------------------------------------------------------
-- shortname						--
-- 							--
-- Returns a pointer into the argument string that 	--
-- points past any '/proc/ram/sys/' or /proc/ram/sys'	--
-- prefix. If no prefix is found it returns the		--
-- original string.					--
--							--
--------------------------------------------------------*/

#define DIRPREFIX "/ram/sys"

static char    *shortname(char *src)
{
	char           *original = src;

	while (*src != '\0') {
		if (strncmp(src, DIRPREFIX, (sizeof(DIRPREFIX) - 1)) == 0) {
			src += sizeof(DIRPREFIX) - 1;
			while (*src == '/')
				src++;
			return (src);
		}
		src++;
	}

	return (original);
}

/*--------------------------------------------------------
-- Checksum						--
-- 							--
-- Calculates checksum for given block.			--
--							--
-- Area checksumed starts from the headers TotalSize	--
-- element, to the end of the buffer. The end of the	--
-- buffer being specified by TotalSize.			--
--							--
--------------------------------------------------------*/

static word     Checksum(RRDBuffBlock * buf)
{
	word            sum = 0;
	word            size = (buf->c.TotalSize - offsetof(RRDCommon, TotalSize)) / (word) sizeof(word);
	word           *start = &buf->c.TotalSize;

	while (size--)
		sum += *start++;

	return (sum);
}

/*--------------------------------------------------------
-- InitFileBlock, InitDirBlock & InitSymbBlock		--
-- 							--
-- Allocate and initialise relocateable file blocks	--
--							--
--------------------------------------------------------*/
static Handle   InitFileBlock(ObjNode * o, char *pathname, bool tempfile, MIInfo * mypool)
{
	RRDFileBlock   *fh;
	Handle          handle;
	word            size = (word) sizeof(RRDFileBlock) + strlen(pathname);

	/* IOdebug("InitFileBlock: size %d, strlen(pathname) %d", size, strlen(pathname));*/
	
	if ((handle = MIAlloc(mypool, size)) == NULL) {
		DEBUG("initfileblock: alloc fail");

		return (NULL);
	}
#if 1
Output("InitFileBlock:lock 1\n");
#endif
	fh = MILock(mypool, handle);

	/* Set common block info */
	if (tempfile)
		fh->c.Magic = RRDTmpMagic;
	else
		fh->c.Magic = RRDFileMagic;
	fh->c.TotalSize = size;

	/* Set file block info */
	strcpy(fh->FullName, pathname);
	fh->FileId = CreateFileId();	/* create unique FID */
	fh->c.Next = NULL;

	/* copy other info from objnode that we must keep over reset */
	fh->Matrix = o->Matrix;
	fh->Key = o->Key;
	fh->Date = o->Dates.Creation;

#ifdef WRITECHECK
	fh->c.Checksum = Checksum((RRDBuffBlock *) fh);
#endif

	MIUnLock(mypool, handle);

	return (handle);
}

/*---------------------------------------------------------------------------*/

static Handle   InitDirBlock(ObjNode * o, char *pathname, MIInfo * mypool)
{
	RRDDirBlock    *db;
	Handle          handle;
	word            size = (word) sizeof(RRDDirBlock) + strlen(pathname);

	if ((handle = MIAlloc(mypool, size)) == NULL)
		return (NULL);

	db = MILock(mypool, handle);

#ifdef ALLDEBUG
	IOdebug("initdirblock: memsize %x, blocksize %x", (((Memory *) db - 1)->Size) & ~Memory_Size_BitMask, size);
#endif

	/* Set common block info */
	db->c.Magic = RRDDirMagic;
	db->c.TotalSize = size;

	/* Set file block info */
	strcpy(db->FullName, pathname);

	/* copy other info from objnode that we must keep over reset */
	db->Matrix = o->Matrix;
	db->Key = o->Key;
	db->Date = o->Dates.Creation;

#ifdef WRITECHECK
	db->c.Checksum = Checksum((RRDBuffBlock *) db);
#endif
	MIUnLock(mypool, handle);

	return (handle);
}

/*---------------------------------------------------------------------------*/

static Handle   InitSymbBlock(LinkNode * l, char *pathname, MIInfo * mypool)
{
	RRDSymbBlock   *slb;
	Handle          handle;
	word            pathlen = strlen(pathname);
	word            size = sizeof(RRDSymbBlock) + pathlen + strlen(l->Link);

	if ((handle = MIAlloc(mypool, size)) == NULL)
		return (NULL);

	slb = MILock(mypool, handle);

	/* Set common block info */
	slb->c.Magic = RRDSymbMagic;
	slb->c.TotalSize = size;

	/* Set link block info */
	strcpy(slb->FullName, pathname);
	strcpy(slb->FullName + pathlen + 1, l->Link);

	/* copy other info from objnode that we must keep over reset */
	slb->Matrix = l->ObjNode.Matrix;
	slb->Key = l->ObjNode.Key;
	slb->Date = l->ObjNode.Dates.Creation;
	slb->Cap = l->Cap;

#ifdef WRITECHECK
	slb->c.Checksum = Checksum((RRDBuffBlock *) slb);
#endif
	MIUnLock(mypool, handle);

	return (handle);
}


#define MI(x,y) (void *)(x->MITable[y])

/*--------------------------------------------------------
-- ResurrectRRD()					--
--							--
-- Attempt to recreate any filesystem that may have     --
-- been present in the memory pool.			--
-- This function is used for the system ramdisk (sys)	--
-- and any NON write protected memory cards that are	--
-- inserted.						--
-- 							--
--------------------------------------------------------*/

static void     ResurrectRRD(MIInfo * mypool, word savedblocks)
{
	word            blocks2, blocks3, fsize;
	uword           biggestFID = 0;
	File           *fileobj;

	/* All blocks are locked in first two phases. */
	/* The final phase unlocks them again. */

	DEBUG("****** ResurrectRRD: First Phase: make hierarchy");

	/* First pass: create directory structure */
	blocks2 = savedblocks;
	while (blocks2) {
		RRDCommon      *b = MILock(mypool, blocks2);

		switch (b->Magic) {
		case RRDBuffMagic:
			/*
			 * mark blocks so we can check later which are orphans 
			 */
			if (b->Next == NULL)
				b->Next = MinInt + 1;	/* note possible EOF */
			else
				b->Next = MinInt;
			break;

		case RRDFileMagic:
			/* keep track of previously allocated FID's */
			if (((RRDBuffBlock *) b)->FileId > biggestFID)
				biggestFID = ((RRDBuffBlock *) b)->FileId;

			/*
			 * make sure supporting directory structure for this
			 * file exists 
			 */
			MakeHierarchy1(((RRDDirBlock *) b)->FullName, TRUE, mypool);
			break;

		case RRDSymbMagic:
			/*
			 * make sure supporting directory structure for this
			 * symbln exists 
			 */
			MakeHierarchy1(((RRDSymbBlock *) b)->FullName, TRUE, mypool);
			break;

		case RRDDirMagic:
			MakeHierarchy2((RRDDirBlock *) b, blocks2, TRUE, mypool);
			break;

		default:
			DEBUG("*** Unknown block type ignored");
		}
		blocks2--;
	}

	if (biggestFID > LastFID )
		LastFID = biggestFID;

	/* Second phase: Add files and symbolic links into the dir structure */
	DEBUG("****** ResurrectRRD: Second Phase: add files and links");

	blocks2 = savedblocks;
	while (blocks2) {
		RRDCommon      *b = MI(mypool, blocks2);

		if (b->Magic == RRDFileMagic) {
			/* create file block header */
			RestoreFile((RRDFileBlock *) b, blocks2);

			blocks3 = savedblocks;
			while (blocks3) {
				RRDBuffBlock   *bb = MI(mypool, blocks3);

				if ((bb->c.Magic == RRDBuffMagic) && (bb->FileId == ((RRDFileBlock *) b)->FileId))
					RestoreBlock((RRDFileBlock *) b, bb, blocks3, mypool);

				blocks3--;
			}
		}
		elif(b->Magic == RRDSymbMagic) {
			/* restore symb link */
			RestoreSymbLn((RRDSymbBlock *) b, blocks2);
		}
		blocks2--;
	}

	/* Third phase: Validate files and unlock the relocatable blocks */
	DEBUG("****** ResurrectRRD: Third Phase: Validation");

	blocks2 = savedblocks;
	while (blocks2) {
		RRDCommon      *b = MI(mypool, blocks2);

		switch (b->Magic) {
		case RRDBuffMagic:
			if ((b->Next == MinInt) || (b->Next == MinInt + 1)) {
#ifdef SYSDEB
				IOdebug("Deleting orphan block %x", blocks2);
#endif
				/* orphan block */
				/*
				 * @@@ could find partners and create file
				 * from them in lost+found 
				 */
				MIFree(mypool, blocks2);
			}
			else
				MIUnLock(mypool, blocks2);
			break;

		case RRDFileMagic:
			if (!ValidateFile((RRDFileBlock *) b, blocks2, &fsize, mypool))
				break;
#ifdef ALLDEBUG
			IOdebug("FileValidated: %s, size %x", ((RRDFileBlock *) b)->FullName, fsize);
#endif
			/* file is ok, so give it a size */
			fileobj = (File *) FindObj(((RRDFileBlock *) b)->FullName);
			fileobj->Upb = fsize;

			/* fall thru */

		case RRDDirMagic:
		case RRDSymbMagic:
			MIUnLock(mypool, blocks2);
			break;

		default:
			/* unknown block type */
#ifdef SYSDEB
			IOdebug("Unknown magic block: magic: %c%c%c%c, blockid: 0x%x, blockaddr: 0x%x", (char *) (&b->Magic)[0], (char *) (&b->Magic)[1], (char *) (&b->Magic)[2], (char *) (&b->Magic)[3], (word) blocks2, (word)(mypool->MITable[blocks2]));
#endif
#if 0			/* if its unknown, then we have corruption */
			/* so don't do this as its dangerous! */
			MIFree(mypool, blocks2);
#endif
		}
		blocks2--;
	}

	DEBUG("****** ResurrectRRD: Completed");
	return;
}


/*--------------------------------------------------------
-- ReCreateRRD()					--
--							--
-- Attempt to recreate any filesystem that may have     --
-- been present in the memory pool of a WRITE PROTECTED	--
-- card. This differs from ResurrectRRD() in that it	--
-- uses the structure it finds on the card, doesn't	--
-- automatically compact the memory or any other 	--
-- operation that would cause it to write to the card.	--
-- 							--
--------------------------------------------------------*/

static void     ReCreateRRD(MIInfo * mypool)
{
	word            savedblocks, blocks2, fsize;
	uword           biggestFID = 0;
	File           *fileobj;

	savedblocks = mypool->MITableSize;

	DEBUG("****** ReCreateRRD: First Phase: make hierarchy");

	/* First pass: create directory structure */
	blocks2 = savedblocks;
	while (blocks2) {
		RRDCommon      *b = MI(mypool, blocks2);

		if (b==NULL)
			continue; /* unused block handle */

		if (b->Magic == RRDDirMagic)
			MakeHierarchy2((RRDDirBlock *) b, blocks2, FALSE, mypool);

		blocks2--;
	}

	/* Second phase: Add files and symbolic links into the dir structure */
	DEBUG("****** ReCreateRRD: Second Phase: add files and links");

	blocks2 = savedblocks;
	while (blocks2) {
		RRDCommon      *b = MI(mypool, blocks2);

		if (b == NULL)
			continue;

		if (b->Magic == RRDFileMagic) {
			/* keep track of previously allocated FID's */
			if (((RRDBuffBlock *) b)->FileId > biggestFID)
				biggestFID = ((RRDBuffBlock *) b)->FileId;

			/* create file block header */
			RestoreFile((RRDFileBlock *) b, blocks2);
			if (ValidateROFile((RRDFileBlock *) b, blocks2, &fsize, mypool)) {
				/* file is ok, so give it a size */
				fileobj = (File *) FindObj(((RRDFileBlock *) b)->FullName);
				fileobj->Upb = fsize;
			}
		}
		elif (b->Magic == RRDSymbMagic) {
			/* restore symb link */
			RestoreSymbLn((RRDSymbBlock *) b, blocks2);
		}
		blocks2--;
	}

	if (biggestFID > LastFID )
		LastFID = biggestFID;

	DEBUG("****** ReCreateRRD: Completed");
	return;
}

/*--------------------------------------------------------
-- basename()						--
--							--
-- This is used for splitting up file names.		--
-- Copy chars out of str into pfix until either a null 	--
-- is found or '/' is encountered. Returns pos in str 	--
-- of char after '/', or 0 if null is encountered	--
-- 							--
--------------------------------------------------------*/

static int      basename(char *base, char *str)
{
	int             ptr = 0;

	while (str[ptr] != '/') {
		if (str[ptr] == '\0') {
			base[ptr] = '\0';
			return (0);
		}
		else if (ptr < (NameMax - 1))
			base[ptr] = str[ptr];
		ptr++;
	}
	base[ptr] = '\0';
	return (ptr + 1);
}

/*--------------------------------------------------------
-- 							--
-- MakeHierarchy1					--
-- 							--
-- Create a directory hierarchy, upto but not including	--
-- the penultimate name in the path.			--
-- 							--
-- If 'save' is true then create reset resilient 	--
-- structure.						--
-- 							--
-- Returns pointer to last directory.			--
--------------------------------------------------------*/

static DirNode *MakeHierarchy1(char *path, bool save, MIInfo * mypool)
{
	int             idx = 0;
	DirNode        *Dir = SysRoot;	/* "/ram/sys" */
	DirNode        *NDir = SysRoot;
	char            base[NameMax];

	while ((idx = basename(base, path)) != 0) {
		/* make sure supporting hierarchy exists */
		path += idx;
		if ((NDir = (DirNode *) Lookup(Dir, base, TRUE)) == NULL)
			NDir = NewDir(Dir, base, 0, DefDirMatrix, save, mypool);
		Dir = NDir;
	}

	return Dir;
}


/*--------------------------------------------------------
-- 							--
-- MakeHierarchy2					--
-- 							--
-- Create a directory hierarchy, if the hierarchy 	--
-- already exists then replace the contents of the tail	--
-- directory with the info from the dirblock.		--
--							--
-- 'save' will create dummy reset resilient blocks for	--
-- directories that haven't been restored yet.		--
-- 							--
--------------------------------------------------------*/

static void MakeHierarchy2(RRDDirBlock * db, Handle dirhandle, bool save, MIInfo * mypool)
{
	DirNode        *NDir, *Dir = MakeHierarchy1(db->FullName, save, mypool);
	char            *base;

	base = objname(db->FullName);

	if ((NDir = (DirNode *) Lookup(Dir, base, TRUE)) == NULL) {
		/* doesn't already exist, so create DirNode */
		NDir = NewDir(Dir, base, 0, DefDirMatrix, FALSE, mypool);
	}
	else {
		if (NDir->DirBlockH && save)
			/* if already created, replace default RRDDirBlock */
			MIFree(mypool, NDir->DirBlockH);
	}

	NDir->DirBlockH = dirhandle;

	/* Set common block info */
	/* copy into objnode info that has been saved over reset */
	NDir->Matrix = db->Matrix;
	NDir->Key = db->Key;
	NDir->Dates.Creation = NDir->Dates.Modified = NDir->Dates.Access = db->Date;
}

/*---------------------------------------------------------------------------*/

/* Restore a file from the contents of a resurrected fileblock */
static void     RestoreFile(RRDFileBlock * fb, Handle handle)
{
	int             idx = 0;
	DirNode        *Dir = SysRoot;	/* "/ram/sys" */
	char            base[NameMax];
	File           *f = New(File);
	char           *path = fb->FullName;

	fb->c.Next = NULL;

	/* find files parent directory */
	while ((idx = basename(base, path)) != 0) {
		path += idx;
		if ((Dir = (DirNode *) Lookup(Dir, base, TRUE)) == NULL) {
			DEBUG("RestoreFile: directory not found");

			return;
		}
	}

	InitNode(&f->ObjNode, objname(fb->FullName), Type_File, 0, fb->Matrix);

	f->Buffers = (Node *) handle;
	f->ObjNode.Dates.Modified = f->ObjNode.Dates.Access = f->ObjNode.Dates.Creation = fb->Date;
	f->ObjNode.Key = fb->Key;
	f->Upb = 0;		/* will be updated after file validation */
	f->Users = 0;

	Insert(Dir, &f->ObjNode, TRUE);
	return;
}

/*---------------------------------------------------------------------------*/
/*
 * Insert a recovered Buffer block into the correct position in a files buffer
 * chain 
 */
static void     RestoreBlock(RRDFileBlock * f, RRDBuffBlock * nb, Handle bhandle, MIInfo * mypool)
{
	RRDBuffBlock   *l;
	RRDBuffBlock   *b;
	Handle          nextb;

	nb->c.Next = NULL;

	nextb = f->c.Next;
	l = (RRDBuffBlock *) f;

	/* add depending on Pos, updating Upb total buffer position */
	while (nextb != NULL) {
		b = MI(mypool, nextb);

		if (nb->Pos < b->Pos) {
			l->c.Next = bhandle;
			nb->c.Next = nextb;
			return;
		}
		nextb = b->c.Next;
		l = b;
	}
	l->c.Next = bhandle;
	return;
}

/*---------------------------------------------------------------------------*/

static void     RestoreSymbLn(RRDSymbBlock * sb, Handle handle)
{
	int             idx = 0;
	DirNode        *Dir = SysRoot;	/* "/ram/sys" */
	char            base[NameMax];
	LinkNode       *l;
	char           *name = sb->FullName;

	/* find symblns directory */
	while ((idx = basename(base, name)) != 0) {
		name += idx;
		Dir = (DirNode *) Lookup(Dir, base, TRUE);
	}

	l = (LinkNode *) ServMalloc((word) sizeof(LinkNode) + (word) strlen(sb->FullName + strlen(sb->FullName) + 1));
	InitNode(&l->ObjNode, objname(sb->FullName), Type_Link, 0, sb->Matrix);
	l->Cap = sb->Cap;
	l->ObjNode.Dates.Modified = l->ObjNode.Dates.Access = l->ObjNode.Dates.Creation = sb->Date;
	l->ObjNode.Key = sb->Key;
	strcpy(l->Link, sb->FullName + strlen(sb->FullName) + 1);

	l->SymbBlockH = (Node *) handle;
	Insert(Dir, &l->ObjNode, TRUE);
	return;
}

/*---------------------------------------------------------------------------*/

/* Validate entire file. If file is OK return TRUE */
/* n.b. checksums have already been tested by the kernel */
static bool     ValidateFile(RRDFileBlock * fb, Handle fhandle, word * size, MIInfo * mypool)
{
	RRDBuffBlock   *b;
	Handle          lastb;
	Handle          nextb;
	word            expectedpos = 0;

	nextb = fb->c.Next;

	while (nextb != NULL) {
#ifdef ALLDEBUG
		IOdebug("ValidateFile block %x, expectedpos %x", nextb, expectedpos);
#endif

		lastb = nextb;
		b = MI(mypool, nextb);
		nextb = b->c.Next;

		/* Check blocks have no gaps between them */
		if (b->Pos != expectedpos) {
			File           *fo = (File *) FindObj(fb->FullName);

			/*
			 * blocks have gaps -- @@@ could place file into
			 * lost+found @@@ 
			 */
#ifdef SYSDEB
			IOdebug("Gap in %s, DELETEING FILE", fb->FullName);
#endif
			DeleteBuffBlocks(fb, mypool);
			fb->c.Magic = 0; /* dont resurrect file header */
			MIFree(mypool,fhandle);

			Unlink((ObjNode *) fo, TRUE);
			Free(fo);

			return (FALSE);
		}
		expectedpos += b->DataSize;

		MIUnLock(mypool, lastb);
	}

	*size = expectedpos;

	return (TRUE);
}

/* validate a file from a WRITE PROTECTED (ReadOnly) File system */
static bool ValidateROFile(RRDFileBlock * fb, Handle fhandle, word * size, MIInfo * mypool)
{
	RRDBuffBlock   *b;
	Handle          lastb;
	Handle          nextb;
	word            expectedpos = 0;

#ifdef ALLDEBUG
			IOdebug("ValidateROFile: entered");
#endif
	/* find file size */
	nextb = fb->c.Next;
	while (nextb != NULL) {
		lastb = nextb;
		b = MI(mypool, nextb);
		nextb = b->c.Next;

		/* if checksum bad or gaps in files contents, delete files directory entry */
		if (b->Pos != expectedpos || b->c.Checksum != Checksum(b)) {
			File *fo = (File *) FindObj(fb->FullName);

#ifdef SYSDEB
			IOdebug("ValidateROFile: checksum failure or gap, ignoring file %s", fb->FullName);
#endif
			Unlink((ObjNode *) fo, TRUE);
			return FALSE;
		}
		expectedpos += b->DataSize;
	}
	*size = expectedpos;

	return TRUE;
}
/*---------------------------------------------------------------------------*/
/* Delete all bufferblocks chained to a file header block */

static void     DeleteBuffBlocks(RRDFileBlock * fb, MIInfo * mi)
{
	Handle          lastb;
	Handle          nextb;
	RRDBuffBlock   *b;

	nextb = fb->c.Next;

	while (nextb != NULL) {
#ifdef SYSDEB
		int tabsize = mi->MITableSize;
	
		if (nextb < 1 || nextb > tabsize) {
			DEBUG("DeleteBuffBlocks: invalid handle error\n");
			break;
		}

		if ((nextb == MinInt) || (nextb == (MinInt + 1))) {
			DEBUG("Orphan block found in deletebuffblocks!");
			break;
		}
#endif

		lastb = nextb;
		b = MI(mi, nextb);
		nextb = b->c.Next;

		b->c.Magic = 0; /* dont bother me again */
		MIFree(mi, lastb);
	}

	return;
}

/*---------------------------------------------------------------------------*/
/*
 * Get pointer to ObjNode with a given filename. return NULL if nonexistant.
 * name is expected to be relative to "/ram/sys" 
 */
static ObjNode *FindObj(char *name)
{
	int             idx = 0;
	DirNode        *Dir = SysRoot;	/* "/ram/sys" */
	char            base[NameMax];

	while ((idx = basename(base, name)) != 0) {
		name += idx;

		if ((Dir = (DirNode *) Lookup(Dir, base, TRUE)) == NULL)
			return NULL;
	}

	/* 'base' now holds final name of file/symbln */

	return (Lookup(Dir, base, TRUE));
}

/*----------------------------------------------------------------
-- GetRootPool							--
--								--
-- Returns the memory information block for the given directory	--
--								--
-- Each inserted memory card has its own root directory in /ram --
-- that contains a pointer this block.	GetRootPool() returns	--
-- MISysMem if the target is just the /ram root directory.	--
--								--
----------------------------------------------------------------*/

static MIInfo  *GetRootPool(DirNode *d)
{
	DirNode        *prevdir = NULL;

	while (d != &Root) {
		prevdir = d;
		d = d->Parent;
	}

	if (prevdir) {
		/* pull out the memory indirection info block address */
		/* this is always stored in each "cards" root directory */
		return((MIInfo *) prevdir->MemBlockH);
	}

	/* default to /ram/sys (fixed internal memory) */
	return ((MIInfo *)SysRoot->MemBlockH);
}

#ifdef __CARD
/*---------------------------------------------------------------------------*/
/*-- CARD support -----------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

static ObjNode *CreateRootDir(char *dirname, word location)
{
	DirNode        *d = New(DirNode);

	if (d == NULL)
		return ((ObjNode *) NULL);

	InitNode((ObjNode *) d, dirname, Type_Directory, 0, DefDirMatrix);
	InitList(&(d->Entries));
	d->Nentries = 0;
	d->Parent = &Root;
	Insert(&Root, (ObjNode *) d, TRUE);

	return ((ObjNode *) d);
}

/*--------------------------------------------------------
-- CardHandlerProcess					--
--							--
-- This process waits on the "hard" semaphore from	--
-- the RamCardHandler function.				--
--------------------------------------------------------*/
static void     CardHandlerProcess(int slot)
{
	                forever
	{
                HardenedWait(&(CardVec[slot - 1].event));	/* wait for event */

		if (CardVec[slot - 1].insert)
			CardInsertHandler(slot);
		else
			CardRemovalHandler(slot);
	}
	/* this point should never be reached */
	return;
}

/*--------------------------------------------------------
-- CardInsertHandler					--
--							--
-- This process waits on the "insert" semaphore from	--
-- the RamCardHandler function.				--
--------------------------------------------------------*/
static void     CardInsertHandler(int slot)
{
	char            cname[8];	/* hardwired "card%d\0" */
	ObjNode        *dir;	/* built directory */
	LinkNode       *link;	/* link structure */
	char           *cardname;
	word            types;
	word            numareas;
	word            battery;
	word		writeprotect = FALSE;
	word           *base;
	word            size;
	word            type;

	/* get info on card that has just been inserted - return if error */
	if ((size = CardStatus(slot, &types, &numareas, &battery, &writeprotect, &cardname)) != CARDerr_none) {
#ifdef SYSDEB
		IOdebug("ram: CardInsertHandler: CardStatus returned %d", size);
#endif
		return;
	}
#ifdef SYSDEB
	IOdebug("ram: CardInsertHandler: type bitmask = &%x", types);
	IOdebug("ram: CardInsertHandler: number AREAs = %d", numareas);
	IOdebug("ram: CardInsertHandler: battery level = &%x", battery);
#endif
	if ((battery & BATTERY_PRESENT) && !(battery & BATTERY_OK)) {
		DEBUG("ram: CardInsertHandler: battery present, but failed");

		return;		/* battery has failing - @@@ possibly warn
				 * user in some way? */
	}
	/* remember name to unlink symblink */
	strcpy(CardVec[slot - 1].cardname, cardname);

#ifdef SYSDEB
	IOdebug("ram: CardInsertHandler: slot %d \"%s\"", slot, cardname);
#endif

	while (numareas) {
		int             status = CardInfo(slot, (int) numareas, &type, &size, (word *) & base);

#ifdef SYSDEB
		if (status != CARDerr_none)
			IOdebug("ram: CardInsertHandler: CardInfo status = %d", status);
#endif

#ifdef SYSDEB
		IOdebug("ram: CardInsertHandler: type = &%x", type);
		IOdebug("ram: CardInsertHandler: size = &%x", size);
		IOdebug("ram: CardInsertHandler: base = &%x", base);
#endif

		if (!(type & ((1 << DT_DRAM) | (1 << DT_SRAM)))) {
			/* if not a RAM area, continue */
#ifdef SYSDEB
			IOdebug("ram: CardInsertHandler: AREA %d not RAM", numareas);
#endif
			numareas--;
			continue;
		}
		if (battery & BATTERY_PRESENT) {
			/* construct name for card in /ram */
			if (numareas == 1) {
				strncpy(cname, "cardN\0", 6);
				cname[4] = (slot | '0');
			}
			else {
				strncpy(cname, "cardN.N\0", 8);
				cname[4] = (slot | '0');
				cname[6] = (char) (numareas | '0');
			}

			/* Generate a directory ("card%d",slot) */
			if ((dir = CreateRootDir(cname, slot)) != NULL) {
				Memory         *magicbase;
				Memory         *magictop;
				Pool           *CardFreePool;
				Pool           *CardRRDPool;
				MIInfo         *mi;

				if (writeprotect)
				{
					/* rebuild card using existing structure */
					/* *dangerous* as we can only assume memory hasn't been corrupted */

					/* align pool to 16 byte boundary */
					CardFreePool = (Pool *)(((word)base + 15) & ~15);

					/* find Pool areas on card */
					CardRRDPool = CardFreePool + 1;
					CardVec[slot - 1].mi = mi = (MIInfo *)(CardRRDPool + 1);

					/* note that the memory area is write protected */
					mi->MIWriteProtect = TRUE;

					/*
					 * remember cards mi pointer in the root
					 * subdirs Account field 
					 */
					dir->MemBlockH = (word) mi;

					/* enumerate the contents of the card */
					/* into the dynamic directory structure */
					ReCreateRRD(mi);
#ifdef SYSDEB
					IOdebug("ram: CardInsertHandler: (WriteProtected) MIInfo *pool = %x", (word) mi);
#endif
				}
				else	/* NON write protected card */
				{
					/* rebuild card pool saving ram magic blocks */
					CardFreePool = BuildPool((byte *) base, size, TRUE);

					/*
					 * get pointer to start of magic block save
					 * area 
					 */
					magicbase = (Memory *) ((word) (CardFreePool->Memory.Tail) + (((Memory *) (CardFreePool->Memory.Tail))->Size & ~Memory_Size_BitMask));
					magictop = (Memory *) CardFreePool->Memory.Head;
	
					/* initialise Pool areas for card */
					/* CardRRDPool and Card mi are automatically allocated on the card by build pool */
					CardRRDPool = CardFreePool + 1;
					CardVec[slot - 1].mi = mi = (MIInfo *)(CardRRDPool + 1);
					InitPool(CardRRDPool);
					MIInit(mi, CardFreePool, CardRRDPool);
					/*
					 * remember cards mi pointer in the root
					 * subdirs Account field 
					 */
					dir->MemBlockH = (word) mi;
					
					/*
					 * enumerate the saved area into the cards
					 * mi table and then expand the table
					 * into a dynamic server directory structure
					 */
					ResurrectRRD(mi, RRDPoolInit(mi, magicbase, magictop));

#ifdef SYSDEB
					IOdebug("ram: CardInsertHandler: MIInfo *pool = %x", (word) mi);
#endif
				}
			}
		}
		else {		/* heap card */
			Pool           *CardFreePool;

			DEBUG("ram: CardInsertHandler: using CARD for HEAP");

			/* @@@ must add dummy blocks to connect with system free pool */
			/* effectively creating contiguous linear pool memory */
			/* do this by adjusting tail dummy blocks size */

			/* construct pool in card memory */
			CardFreePool = BuildPool((byte *) base, size, FALSE);

			/* pass entire pool to the freepool */
			/* don't expect to get it back! */
			/* - system will have to be reset if card removed */
			FreePool(CardFreePool);
			dir->MemBlockH = (word) NULL;	/* not a RAMFS pool */
		}
		numareas--;
	}

	/*
	 * Create a soft-link to the newly created CARD directory (checking
	 * that the name of the CARD is NOT of the form ("card%d",slot)). 
	 */
	if (*cardname != '\0') {
		char fullname[NameMax];	/* nasty largish stack
					 * allocation */

		/* build full link name */
		strcpy(fullname, "/ram/");
		strcpy(&fullname[strlen(fullname)], cname);

		/* ignore any errors, since we have nobody to report them to */
		link = (LinkNode *) Malloc(sizeof(LinkNode) + strlen(fullname));
		if (link != Null(LinkNode)) {
			Object         *dirobj;

			/* We should only take the "leafname" here *//**** TODO ****/
			InitNode(&link->ObjNode, cardname, Type_Link, 0, DefLinkMatrix);
			if ((dirobj = Locate(NULL, fullname)) != NULL) {
				link->Cap = dirobj->Access;
				strcpy(link->Link, fullname);
				Insert(&Root, &link->ObjNode, TRUE);
			}
		}
	}
	CardVec[slot - 1].slot = slot;	/* mark this slot as filled */

	/*
	 * Notify applications that want to know about newly inserted CARDs
	 */
	CauseUserEvent(UVec_Card, Card_Insert);

	return;
}

/*--------------------------------------------------------
-- CardRemovalHandler					--
--							--
-- This process waits on the "remove" semaphore from	--
-- the RamCardHandler function.				--
--------------------------------------------------------*/

static void     CardRemovalHandler(int slot)
{
	DirNode        *dir = NULL;
	char            cname[6];	/* hardwired "card%d\0" */

	strncpy(cname, "cardN\0", 6);
	cname[4] = (slot | '0');

	/*
	 * Check for battery via CardStatus() to see if we need to reset the
	 * machine. Note: a better check is using the "MemBlockH" field. If
	 * NULL then the CARD must have been used as a heap... in which case
	 * the system should be RESET. NOTE: We have problems here... since if
	 * the CARD is heap, we may never be called... since the lo-level CARD
	 * handler signals the higher level processes... which go through the
	 * scheduler... which may start a higher priority process that will
	 * crash because it is using the heap on the CARD just removed... this
	 * whole system may need re-designing. 
	 */


	/*
	 * NOTE: We may be called to remove a CARD from a slot where we do NOT
	 * actually have one loaded. This is required, since we may have to
	 * deal with the case of the user removing a CARD without permission.
	 * The lo-level handler does NOT keep any state about state and type
	 * of loaded CARDs. 
	 */
	if (CardVec[slot - 1].slot == slot) {	/* Ignore calls on
						 * unrecognised slots */
		/*
		 * Perform a "Lookup" call to ascertain wether the directory
		 * exists 
		 */
		if ((dir = (DirNode *) Lookup(&Root, cname, TRUE)) != NULL) {
#ifdef SYSDEB
			IOdebug("RamRemoveHandler - should close %s", cname);
#endif

			/*
			 * We must deal with files open onto this CARD: 1)
			 * Ignore open files and allow the CARD to be removed
			 * (user will later get errors on open files). 2) Do
			 * not allow the CARD to be removed when files are
			 * open onto it. 
			 */

			/*
			 * We need to remove the directory "/ram/card%d" and
			 * the sub-directories beneath it. NOTE: We should
			 * remember to remove any soft-links to this directory
			 * that we created (we cannot do anything about user
			 * created links (I believe)). Should also remove the
			 * cards link. 
			 */

			/*
			 * "Unlink((ObjNode *)object,TRUE) ;" to remove object
			 * from directory 
			 */
			/* "Free(object) ;" to release the attached memory */
		}

		/*
		 * Notify applications that want to know when a card has been extracted
		 */
		CauseUserEvent(UVec_Card, Card_Extract);
	}

	return;
}

/*--------------------------------------------------------
-- RamCardHandler					--
--							--
-- This function processes RAM CARD events.		--
-- It should be called when a CARD of "typeRAM" is	--
-- inserted or removed.					--
-- 							--
-- NOTE: This function is called from an interrupt	--
--	 handler. It should execute as quickly as	--
--	 possible, using a minimum amount of stack.	--
--	 It should only communicate with higher level	--
--	 threads via HardSemaphores.			--
--							--
-- "Insert"   = TRUE for CARD insertion event		--
--	      = FALSE for CARD removal event		--
-- "slot"     = the physical CARD slot occupied		--
-- "type"     = the actual type of the CARD present	--
--------------------------------------------------------*/

static word     RamCardHandler(bool Insert, int slot, word type)
{
#ifdef SYSDEB
        IOdebug("RamCardHandler: Insert %x, slot %x, type %x",Insert, slot, type);	/* insert/remove */
#endif

	HardenedSignal(&(CardVec[slot - 1].event));	/* Card In/Out event */

	return (0);
}

/*---------------------------------------------------------------------------*/
/* Initialise CARD filing system support */

static void     InitCard(void)
{
	int             loop;
	int             cardlimit = CardSlots();

	CardVec = Malloc(cardlimit * sizeof(CardData));

	for (loop = loc_CARD1; (loop <= cardlimit); loop++) {
		CardVec[loop - 1].slot = -1;	/* unused slot */
		CardVec[loop - 1].mi = NULL;
		InitSemaphore(&(CardVec[loop - 1].event), 0);
		/* and fork off the foreground handler processes */
		Fork(defStack, CardHandlerProcess, sizeof(loop), loop);
	}

	/* @@@ Check "CardStatus()" to see if we already have CARDs loaded */

	/*
	 * "cardinfo" should be static and defined until
	 * "CardReleaseEventHandler" is called. In our system the RAM server
	 * should always be active, so we provide no method of removing the
	 * handler internally. The Kernel "Kill" code should be updated to
	 * remove handlers resident in tasks being killed. 
	 */
	cardinfo.Type = ((1 << DT_SRAM) | (1 << DT_DRAM));	/* CARD types bitmask */
	cardinfo.Handler = RamCardHandler;	/* handler function */
	CardDefineEventHandler(&cardinfo);	/* and register the function */

	 /* DEBUG */ (void) RamCardHandler(TRUE, 1, (1 << DT_SRAM) | (1 << DT_DRAM));
}

#endif				/* __CARD */

/*---------------------------------------------------------------------------*/
/*-- exception handling -----------------------------------------------------*/
/*---------------------------------------------------------------------------*/

#ifdef __XPUTER
void            _stack_error(void)
{
	                DEBUG("Ram Disk stack overflow!!");
}

#elif defined (__HELIOSARM)
static void     __stack_overflow(void)
{
	                DEBUG("RAM server stack overflow");
}
static void     __stack_overflow_1(void)
{
	                DEBUG("RAM server stack overflow 1");
}

#endif

/*---------------------------------------------------------------------------*/
/* > EOF ram.c < */
