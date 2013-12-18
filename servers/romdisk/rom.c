/*------------------------------------------------------------------------
--                                                                      --
--                     H E L I O S   N U C L E U S                      --
--                     ---------------------------                      --
--                                                                      --
--             Copyright (C) 1989, Perihelion Software Ltd.             --
--	       Copyright (c) 1990, Active Book Company Ltd.		--
--                        All Rights Reserved.                          --
--                                                                      --
-- rom.c								--
--                                                                      --
--	ROM disc handler						--
--                                                                      --
--	Author:  NHG / PAB 	1/3/90					--
--	Updated: JGS		8/11/90					--
--                                                                      --
------------------------------------------------------------------------*/
/* RCSId: $Id: rom.c,v 1.6 1991/05/06 17:07:03 paul Exp $ */

#include <helios.h>	/* standard header */

#define __in_rom 1	/* flag that we are in this module */

/*--------------------------------------------------------
-- 		     Include Files			--
--------------------------------------------------------*/

#include <string.h>
#include <codes.h>
#include <syslib.h>
#include <servlib.h>
#include <task.h>
#include <root.h>
#include <sem.h>
#include <process.h>
#include <event.h>

#include <abcARM/manifest.h>	/* useful Executive manifests */
#include <abcARM/PCcard.h>	/* external CARD information */
#include <abcARM/ROMitems.h>	/* ROM ITEM information and access functions */
#include <abcARM/ABClib.h>	/* CARD interface functions */

/*--------------------------------------------------------
--		Private Data Definitions 		--
--------------------------------------------------------*/

typedef struct FileData {
	word		Size;
	char		Name[NameMax];
} FileData;

typedef struct File {
	ObjNode		ObjNode;	/* node for directory struct	*/
	void		*Data;		/* pointer to file data		*/
	Buffer		Buf;		/* buffer for DoRead		*/
} File;

#ifdef __CARD
typedef struct CardData {
	int		slot;		/* slot we are attached to */
	int		insert;		/* TRUE if card is inserted */
	Semaphore	event;		/* hard semaphore to handler process */
	char		*cardname;	/* card link name */
} CardData ;
#endif /* __CARD */

/*--------------------------------------------------------
--	      Private Function Definitions 		--
--------------------------------------------------------*/

void FileRead(MCB *m,File *f);
void FileSeek(MCB *m,File *f);
static ObjNode *CreateRootDir(char *name,word location) ;
static void loadFiles(DirNode *lroot,word location) ;
static int basename(char *base, char *str);
bool LoadRomImage(char *name, char *addr);
static void do_open(ServInfo *);
static void do_serverinfo(ServInfo *);

#ifdef __CARD
static void CardInsertHandler(int slot);
static void CardRemovalHandler(int slot);
static word RomCardHandler(bool insert, int slot, word type);
static void InitCard(void);
#endif /* __CARD */

/*--------------------------------------------------------
--		Private Data Definitions 		--
--------------------------------------------------------*/

#define Upb		ObjNode.Size	/* use ObjNode size field	*/
#define Users		ObjNode.Account  /* number of opens		*/

DirNode		Root;			/* root of directory system	*/

#define DefStack	(0x0800)	/* 2K default stack		*/
#ifdef __CARD
#if 1 /* temporary */
#define HandStack	(0x2000)	/* 8K default handler stack	*/
#else
#define HandStack	(0x0800)	/* 2K default handler stack	*/
#endif

static CardEvent	cardinfo;	/* CARD handler information	*/
static CardData 	*CardVec;

#endif /* __CARD */

#ifndef DEMANDLOADED
NameInfo Info =
{	NullPort,
	Flags_StripName,
	DefNameMatrix,
	(word *)NULL
};
#endif

static DispatchInfo RomInfo = {
	&Root,
	NullPort,
	SS_RomDisk,
	NULL,
	{ NULL, 0 },
	{
		{ do_open,	DefStack },	/* FG_Open		*/
		{ InvalidFn,	DefStack },	/* FG_Create		*/
		{ DoLocate,	DefStack },	/* FG_Locate		*/
		{ DoObjInfo,	DefStack },	/* FG_ObjectInfo	*/
		{ do_serverinfo,DefStack },	/* FG_ServerInfo	*/
		{ InvalidFn,	DefStack },	/* FG_Delete		*/
		{ InvalidFn,	DefStack },	/* FG_Rename		*/
		{ InvalidFn,	DefStack },	/* FG_Link		*/
		{ InvalidFn,	DefStack },	/* FG_Protect		*/
		{ InvalidFn,	DefStack },	/* FG_SetDate		*/
		{ InvalidFn,	DefStack },	/* FG_Refine		*/
		{ NullFn,	DefStack }	/* FG_CloseObj		*/
	}
};

/*--------------------------------------------------------
-- main							--
--							--
-- Entry point of Rom handler.				--
--							--
--------------------------------------------------------*/

int main()
{
#ifndef DEMANDLOADED
	Object *nte;
#endif
	char    mcname[100];

#ifdef STANDALONE
	/* if we are run from the shell via posix execl, */
	/* we will have to read the Env that has been sent to us */
	Environ env;
	GetEnv(MyTask->Port,&env); /* posix exec send's env ! */
#endif

	MachineName(mcname);
	
	RomInfo.ParentName = mcname;
	
	InitNode( (ObjNode *)&Root, "rom", Type_Directory, 0, DefRootMatrix );
	InitList( &Root.Entries );
	Root.Nentries = 0;

#ifdef DEMANDLOADED
	/* if we are demand loaded our server port is passed in task struct */
	RomInfo.ReqPort = MyTask->Port;
#else
	Info.Port = RomInfo.ReqPort = NewPort();
#endif

	/* .. of root is a link to our machine root	*/
	{
		Object *o;
		LinkNode *Parent;

		o = Locate(NULL,mcname);

		Parent = (LinkNode *)Malloc(sizeof(LinkNode) + strlen(mcname) + 1);	
		InitNode( &Parent->ObjNode, "..", Type_Link, 0, DefLinkMatrix );
		Parent->Cap = o->Access;
		strcpy(Parent->Link,mcname);
		Root.Parent = (DirNode *)Parent;

#ifndef DEMANDLOADED
		/* demand loaded servers already have name entry */
		nte = Create(o,"rom",Type_Name,sizeof(NameInfo),
				(byte *)&Info);
#endif				
		Close(o);
	}

	/* build default directory structure */
	(void)CreateRootDir("sys",loc_internal) ;

#ifdef __CARD
	InitCard(); /* initialise card handlers */
#endif

#ifdef INSYSTEMIMAGE
	/* reply to procman that we have started */
	/* if we are part of system image 0x456 is expect to be returned */
	{
		MCB m;
		word e;
		InitMCB(&m,0,MyTask->Parent,NullPort,0x456);
		e = PutMsg(&m);
	}
#endif

	Dispatch(&RomInfo);

#ifndef DEMANDLOADED
	Delete(nte,NULL);
#endif
}

static ObjNode *CreateRootDir(char *dirname,word location)
{
	DirNode *d = New(DirNode) ;
	
	if (d == NULL)
		return((ObjNode *)NULL) ;
	InitNode((ObjNode *)d,dirname,Type_Directory,0,DefDirMatrix) ;
	InitList(&(d->Entries)) ;
	d->Nentries = 0 ;
	d->Parent = &Root ;
	Insert(&Root,(ObjNode *)d,TRUE) ;

	loadFiles(d,location) ;

	return((ObjNode *)d) ;
}

static void loadFiles(DirNode *lroot,word location)
{
 word           index = 0 ;	/* ITEM index, (updated by exec) */
 char          *name ;		/* filename */
 word           size ;		/* file size */
 Matrix         matrix ;	/* file access */
 char          *data ;		/* file contents */
 word           timestamp ;	/* file timestamp */
 ITEMstructure *iteminfo ;	/* ITEM header */

 /*IOdebug("ROM: loadFiles: lroot %x, location %d, index %x",(int)lroot,location,index) ;*/

 while (GetROMItem(location,&index,&iteminfo))
  {
   File *file ;

   name = (char *)&(iteminfo->ITEMName[0]) ;
   size = iteminfo->OBJECTLength ;
   matrix = iteminfo->ITEMAccess ;
   data = (char *)(iteminfo->OBJECTOffset + (int)iteminfo) ;
   timestamp = iteminfo->ITEMDate[1] ;	/* only the hi timestamp */

   /*IOdebug("ROM: %s, size %x, addr %x, time %x",name,size,data,timestamp) ;*/

   if (*name == '/')
    continue ;	/* ignore absolute pathnames */

   if ((file = New(File)) != NULL)
    {
     int      idx = 0 ;
     DirNode *Dir = lroot ;
     DirNode *NewDir = lroot ;
     char     base[NameMax] ;

     while ((idx = basename(base,name)) != 0)
      {
       name += idx ;
       if ((NewDir = (DirNode *)Lookup(Dir,base,TRUE)) == NULL)
	{
         DirNode *d = New(DirNode) ;
 	 if (d == NULL)
	  return ;

	 /*IOdebug("New dir (base) %s",base) ;*/

	 InitNode((ObjNode *)d,base,Type_Directory,0,DefDirMatrix) ;
	 InitList(&d->Entries) ;
	 d->Nentries = 0 ;
	 d->Parent = Dir ;
	 d->Dates.Creation = timestamp ;
	 d->Dates.Access   = timestamp ;
	 d->Dates.Modified = timestamp ;

	 Insert(Dir,(ObjNode *)d,TRUE) ;
	 NewDir = d ;
	}

       Dir = NewDir ;
      }

     InitNode(&file->ObjNode,base,Type_File,0,matrix) ;
     file->Upb = size ;
     file->Data = data ;
     file->ObjNode.Dates.Creation = timestamp ;
     file->ObjNode.Dates.Access   = timestamp ;
     file->ObjNode.Dates.Modified = timestamp ;
     Insert(NewDir,&file->ObjNode,FALSE) ;
		
     /* if its an executable image, enter it into loader */
     if (*(word *)data == Image_Magic)
      LoadRomImage(base,data) ;
    }
  }
 return ;
}

/*--------------------------------------------------------
-- Action Procedures					--
--							--
--------------------------------------------------------*/

static void do_open(ServInfo *servinfo)
{
	MCB *m = servinfo->m;
	MsgBuf *r;
	DirNode *d;
	File *f;
	IOCMsg2 *req = (IOCMsg2 *)(m->Control);
	Port reqport;
	byte *data = m->Data;
	char *pathname = servinfo->Pathname;

	d = (DirNode *)GetTargetDir(servinfo);

	if( d == NULL )
	{
		ErrorMsg(m,Err_Null);
		return;
	}

	r = New(MsgBuf);

	if( r == NULL )
	{
		ErrorMsg(m,EC_Error+EG_NoMemory);
		return;
	}

	f = (File *)GetTargetObj(servinfo);

	if( f == NULL )
	{
		ErrorMsg(m,Err_Null);
		Free(r);
		return;
	}

	unless( CheckMask(req->Common.Access.Access,req->Arg.Mode&Flags_Mode) ) 
	{
		ErrorMsg(m,EC_Error+EG_Protected+EO_File);
		Free(r);
		return;
	}

	FormOpenReply(r,m,&f->ObjNode,Flags_Server|Flags_Closeable, pathname);

	reqport = NewPort();
	r->mcb.MsgHdr.Reply = reqport;
	PutMsg(&r->mcb);
	Free(r);

	if( f->ObjNode.Type == Type_Directory )
	{
		DirServer(servinfo,m,reqport);
		FreePort(reqport);
		return;
	}

	f->Users++;
	
	UnLockTarget(servinfo);
	
	forever
	{
		word e;
		m->MsgHdr.Dest = reqport;
		m->Timeout = StreamTimeout;
		m->Data = data;

		e = GetMsg(m);
	
		if( e == EK_Timeout ) break;

		if( e < Err_Null ) continue;

		Wait(&f->ObjNode.Lock);

		switch( m->MsgHdr.FnRc & FG_Mask )
		{
		case FG_Read:
			FileRead(m,f);
			break;

		case FG_Close:
			if( m->MsgHdr.Reply != NullPort ) ErrorMsg(m,Err_Null);
			FreePort(reqport);
			f->Users--;
			Signal(&f->ObjNode.Lock);
			return;

		case FG_GetSize:
			InitMCB(m,0,m->MsgHdr.Reply,NullPort,Err_Null);
			MarshalWord(m,f->Upb);
			PutMsg(m);
			break;

		case FG_Seek:
			FileSeek(m,f);
			break;

		case FG_Select:
			/* a file is always ready for I/O so reply now */
			ErrorMsg(m,e&Flags_Mode);
			break;
			
		default:
			ErrorMsg(m,EC_Error+EG_FnCode+EO_File);
			break;
		}
		Signal(&f->ObjNode.Lock);
	}

	f->Users--;
	FreePort(reqport);
}

static void do_serverinfo(ServInfo *servinfo)
{
	NullFn(servinfo);
}

/*--------------------------------------------------------
-- FileRead						--
-- 							--
-- Read data from the File and return it to the client.	--
--							--
--------------------------------------------------------*/

static Buffer *GetRomBuffer(word pos, File *f)
{
	Buffer *b = &f->Buf;
	
	b->Pos = 0;
	b->Size = b->Max = f->Upb;
	b->Data = f->Data;
	
	return b;
	pos=pos;
}

void FileRead(MCB *m, File *f)
{
	ReadWrite *rw = (ReadWrite *)m->Control;

	word pos = rw->Pos;
	word size = rw->Size;

	if( pos < 0 )
	{
		ErrorMsg(m,EC_Error|EG_Parameter|1);
		return;
	}

	if( size < 0 )
	{
		ErrorMsg(m,EC_Error|EG_Parameter|2);
		return;
	}

	if( pos == f->Upb )
	{
		m->MsgHdr.FnRc = ReadRc_EOF;
		ErrorMsg(m,0);
		return;
	}


	if( pos + size > f->Upb ) size = rw->Size = f->Upb - pos;

	DoRead(m,GetRomBuffer,f);

	f->ObjNode.Dates.Access = GetDate();
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

void FileSeek(MCB *m, File *f)
{
	SeekRequest *req = (SeekRequest *)m->Control;
	word curpos = req->CurPos;
	word mode = req->Mode;
	word newpos = req->NewPos;

	switch( mode )
	{
	case S_Beginning:	break;
	case S_Relative:	newpos += curpos; break;
	case S_End:		newpos += f->Upb; break;
	}

	InitMCB(m,0,m->MsgHdr.Reply,NullPort,Err_Null);

	MarshalWord(m,newpos);

	PutMsg(m);
}

/*--------------------------------------------------------
-- basename()						--
--							--
-- This is used for splitting up file names.		--
-- Copy chars out of str into pfix until either a null 	--
-- is found or '/' is encountered. Returns pos in str 	--
-- of char after ch, or 0 if null is encountered	--
-- 							--
--------------------------------------------------------*/

static int basename(char *base, char *str)
{
	int ptr = 0;
	while( str[ptr] != '/' )
	{
		if( str[ptr] == '\0' )
		{ base[ptr] = '\0'; return 0; }
		else if( ptr < NameMax-1 ) base[ptr] = str[ptr];
		ptr++;
	}
	base[ptr] = '\0';
	return ptr+1;
}
#if 0
int basename(char *base, char *str)
{
	int ptr = 0;
	while( *str != '/' )
	{
		if( *str == '\0' )
		{ *base = '\0'; return 0; }
		else if( ptr < NameMax ) *base++ = *str++, ptr++;
	}
	*base = '\0';
	return ptr+1;
}
#endif

#ifdef TEST
/* test data */
struct testitem
{
	char *name;
	word size;
	Matrix matrix;
	char *data;
} thetestitem[] =
	{
		{ "Executive", 5, DefRomMatrix, "Executive" },
		{ "Nucleus", 5, DefRomMatrix, "Nucleus" },
		{ "FPEmulator", 5, DefRomMatrix, "FPEmulator" },
		{ "etc/config", 5, DefRomMatrix, "etc/config" },
		{ "lib/fifo", 5, DefRomMatrix, "lib/fifo" },
		{ "lib/pipe", 5, DefRomMatrix, "lib/pipe" },
		{ "etc/initrc", 5, DefRomMatrix, "etc/initrc" },
		{ "etc/motd", 5, DefRomMatrix, "etc/motd" },
		{ "etc/faults", 5, DefRomMatrix, "etc/faults" },
		{ "bin/login", 5, DefRomMatrix, "bin/login" },
		{ "bin/shell", 5, DefRomMatrix, "bin/shell" },
		{ "bin/ls", 5, DefRomMatrix, "bin/ls" },
		{ "bin/ln", 5, DefRomMatrix, "bin/ln" },
		{ "bin/cat", 5, DefRomMatrix, "bin/cat" },
		{ "bin/more", 5, DefRomMatrix, "bin/more" },
		{ "bin/rm", 5, DefRomMatrix, "bin/rm" },
		{ "bin/mkdir", 5, DefRomMatrix, "bin/mkdir" },
		{ "bin/cp", 5, DefRomMatrix, "bin/cp" },
		{ "bin/mv", 5, DefRomMatrix, "bin/mv" },
		{ "bin/wsh", 5, DefRomMatrix, "bin/wsh" },
		{ "bin/map", 5, DefRomMatrix, "bin/map" },
		{ "bin/free", 5, DefRomMatrix, "bin/free" },
		{ "bin/mem", 5, DefRomMatrix, "bin/mem" },
		{ "bin/make", 5, DefRomMatrix, "bin/make" },
		{ "bin/emacs", 5, DefRomMatrix, "bin/emacs" },
		{ "etc/emacs.hlp", 5, DefRomMatrix, "etc/emacs.hlp" },
		{ "end", 8, DefRomMatrix, "Beskeen" }
	};
		
/* test function */
word GetROMItem1(word *index, char **name, word *size, Matrix *matrix, char **data)
{
	struct testitem *item;

	if (*index == -1)
		return FALSE;

	item = &thetestitem[*index];
	
	*name = item->name;
	*size = item->size;
	*matrix = item->matrix;
	*data = item->data;
	
	(*index)++;

	item = &thetestitem[*index];
	if (strcmp(item->name, "end") == 0)
		*index = -1;

	return TRUE;
}
#endif /* test code */


/* functions for sending private protocol messages to the loader */
/* in our case we wish to create images that point directly to */
/* their rom position, rather than get loaded into RAM */

/*--------------------------------------------------------
-- LoadRomImage						--
--							--
-- Load a Rom Image into the Loader			--
-- 							--
--------------------------------------------------------*/

bool LoadRomImage(char *name, char *addr)
{
/*	Object *loader = Locate(NULL,"/loader");*/
	Object *loader;
	MCB mcb;

	/*IOdebug("LoadRomImage: \"%s\"",name) ;*/

	while ((loader = Locate(NULL,"/loader")) == NULL) ; /*null stat */

	mcb.Data = (byte *)Malloc(IOCDataMax);
	mcb.Control = (word *)Malloc(sizeof(IOCCommon)+(2 * sizeof(word))) ;
	InitMCB(&mcb,0,MyTask->IOCPort,NullPort,FC_GSP|FG_RomCreate);
	MarshalCommon(&mcb,loader,NULL);
	MarshalString(&mcb,name);
	MarshalWord(&mcb,(word)addr);

	PutMsg(&mcb);

	Free(mcb.Control);
	Free(mcb.Data);
	Close(loader);

	return TRUE;
}

#ifdef __CARD
/*--------------------------------------------------------
-- CardHandlerProcess					--
--							--
-- This process waits on the "hard" semaphore from	--
-- the RomCardHandler function.				--
--------------------------------------------------------*/
void CardHandlerProcess(int slot)
{
#if 0
 IOdebug("CardHandlerProcess: entered\n") ;
#endif

 forever
  {
   HardenedWait(&(CardVec[slot - 1].event)) ;	/* wait for event */

#if 0
   IOdebug("CardHandlerProcess: HardenedWait resumed\n") ;
#endif

   if (CardVec[slot - 1].insert)
    CardInsertHandler(slot) ;
   else
    CardRemovalHandler(slot) ;
  }
 /* should never get to this point */
 return ;
}


/*--------------------------------------------------------
-- CardInsertHandler					--
--							--
-- This process waits on the "insert" semaphore from	--
-- the RomCardHandler function.				--
--------------------------------------------------------*/
static void CardInsertHandler(int slot)
{
 char      cname[6] ;		/* hardwired "card%d\0" */
 ObjNode  *dir ;		/* built directory */
 LinkNode *link ;		/* link structure */

 char	  *cardname ;
 word	   types ;
 word	   numareas ;
 word	   battery ;
 word      writeprotect ;

 /* get info on card that has just been inserted - return if error */
 if (CardStatus(slot, &types, &numareas, &battery, &writeprotect, &cardname))
  return ;

 strncpy(cname,"cardN\0",6) ;
 cname[4] = (slot | '0') ;

 /*IOdebug("CardInsertHandler: slot %d \"%s\"",slot,cname) ;*/

 /* NOTE: This code assumes we are never called twice, without a
  *       suitable call to "CardRemovalHandler" being made between us.
  *       Otherwise we will create another directory entry with
  *       ("card%d",slot) as the name.
  */

 /* Generate a directory ("card%d",slot) and enumerate its contents */
 if ((dir = CreateRootDir(cname,slot)) != NULL)
  {
   /* Create a soft-link to the newly created CARD directory
    * (checking that the name of the CARD is NOT of the form
    * ("card%d",slot)).
    */
   /*IOdebug("ROM: CardInsertHandler: link name validity check TO BE DONE") ;*/

   if (*cardname != '\0')
    {
     char fullname[100] ; /* nasty largish stack alloc */

     /* build internal link reference */
     /* We do not use the machine name here */
     strcpy(fullname,"/rom/") ;
     strcpy(&fullname[strlen(fullname)],cname) ;

     /* ignore any errors, since we have nobody to report them to */
     link = (LinkNode *)Malloc(sizeof(LinkNode) + strlen(fullname)) ;
     if (link != Null(LinkNode))
      {
       Object *dirobj ;

       /* We should only take the "leafname" here */	/**** TODO ****/
       InitNode(&link->ObjNode,cardname,Type_Link,0,DefLinkMatrix) ;
       if ((dirobj = Locate(NULL,fullname)) != NULL)
        {
         link->Cap = dirobj->Access ; /* copied from "/helios" server */
         strcpy(link->Link,fullname) ;
         Insert(&Root,&link->ObjNode,TRUE) ;
        }
#if 1
       else
        IOdebug("rom CardInsertHandler: cannot Locate link destination\n") ;
#endif
      }
    }
    /*
     * Notify applications that want to know about newly inserted CARDs
     */
    CauseUserEvent(UVec_Card, Card_Insert);
  }

 CardVec[slot - 1].slot = slot ;	/* mark this slot as filled */

 return ;
}

/*--------------------------------------------------------
-- CardRemovalHandler					--
--							--
-- This process waits on the "remove" semaphore from	--
-- the RomCardHandler function.				--
--------------------------------------------------------*/

static void CardRemovalHandler(int slot)
{
 DirNode *dir = NULL ;
 char     cname[6] ;		/* hardwired "card%d\0" */

 char	 *cardname ;
 word	  types ;
 word	  numareas ;
 word	  battery ;

 strncpy(cname,"cardN\0",6) ;
 cname[4] = (slot | '0') ;

 /* NOTE: We may be called to remove a CARD from a slot where we do NOT
  *       actually have one loaded. This is required, since we may have
  *       to deal with the case of the user removing a CARD without
  *       permission. The lo-level handler does NOT keep any state about
  *       state and type of loaded CARDs.
  */

 /* We will need to kill tasks that are executing within the CARD */

 if (CardVec[slot - 1].slot == slot) /* Ignore calls on unrecognised slots */
  {
   /* Perform a "Lookup" call to ascertain wether the directory exists */
   if ((dir = (DirNode *)Lookup(&Root,cname,TRUE)) != NULL)
    {
     /*IOdebug("RomRemoveHandler - should close %s",cname) ;*/

     /* We must deal with files open onto this CARD:
      *  1) Ignore open files and allow the CARD to be removed (user
      *     will later get errors on open files).
      *  2) Do not allow the CARD to be removed when files are open
      *     onto it.
      */

     /* We need to remove the directory "/rom/card%d" and the
      * sub-directories beneath it. NOTE: We should remember to
      * remove any soft-links to this directory that we created (we
      * cannot do anything about user created links (I believe)).
      */

     /* "Unlink((ObjNode *)object,TRUE) ;" to remove object from
      * directory
      */
     /* "Free(object) ;" to release the attached memory */
    }
    /*
     * Notify applications that want to know when a card has been extracted
     */
    CauseUserEvent(UVec_Card, Card_Extract);
  }

 return ;
}

/*--------------------------------------------------------
-- RomCardHandler					--
--							--
-- This function processes ROM CARD events.		--
-- It should be called when a CARD of "typeROM" is	--
-- inserted or removed.					--
-- 							--
-- NOTE: This function is called from an interrupt	--
--	 handler. It should execute as quickly as	--
--	 possible, using a minimum amount of stack.	--
--	 It should only communicate with higher level	--
--	 threads via Semaphores.			--
--							--
-- "Insert"   = TRUE for CARD insertion event		--
--	      = FALSE for CARD removal event		--
-- "slot"     = the physical CARD slot occupied		--
-- "type"     = the actual type of the CARD present	--
--------------------------------------------------------*/

static word RomCardHandler(bool Insert, int slot, word type)
{
#if 0
# if 1
 IOdebug("RomCardHandler: Insert &%x, slot: 0x%x, type: 0x%x\n", Insert, slot, type);
# else
 Output("RomCardHandler: Insert &"), WriteHex8(Insert) ;	/* insert */
 Output(" (slot &"), WriteHex8(slot) ;				/* slot */
 Output(") type &"), WriteHex8(type) ;				/* type */
 Output("\n") ;
# endif
#endif
 HardenedSignal(&(CardVec[slot - 1].event)) ;	/* card event */

 return(0) ;
}

/*---------------------------------------------------------------------------*/
/* Initialise CARD filing system support */
static void InitCard(void)
{
 int loop ;
 int cardlimit = CardSlots() ;

#if 0
 IOdebug("InitCard: cardlimit = 0x%x\n",cardlimit);
#endif

 CardVec = Malloc(cardlimit * sizeof(CardData)) ;
#if 1 /* DEBUG */
 if (CardVec == NULL)
  IOdebug("rom: InitCard: CardVec Malloc returned NULL\n") ;
#endif

 for (loop = loc_CARD1; (loop <= cardlimit); loop++)
  {
   CardVec[loop - 1].slot = -1 ;	/* unused slot */
   InitSemaphore(&(CardVec[loop - 1].event),0) ;
   /* and fork off the foreground handler processes */
   Fork(HandStack,CardHandlerProcess,sizeof(loop),loop) ;
  }

 /* @@@ Check "CardStatus()" to see if we already have CARDs loaded */

 /* "cardinfo" should be static and defined until
  * "CardReleaseEventHandler" is called. In our system the rom server
  * should always be active, so we provide no method of removing the
  * handler internally. The Kernel "Kill" code should be updated to
  * remove handlers resident in tasks being killed.
  */
 cardinfo.Type = (1 << DT_MASK_ROM) ;	/* bitmask of CARD types */
 cardinfo.Handler = RomCardHandler ;	/* handler function */
 CardDefineEventHandler(&cardinfo) ;	/* and register the function */

 /*DEBUG*/ (void)RomCardHandler(TRUE,1,(1 << DT_MASK_ROM)) ;
}
#endif /* __CARD */


/*---------------------------------------------------------------------------*/

#ifdef __XPUTER
void _stack_error(void)
{
	IOdebug("Rom Disk stack overflow!!");
}
#elif defined (__HELIOSARM)
#pragma check_stack
static void __stack_overflow(void)
{
	IOdebug("Rom Disk stack overflow!");
}
static void __stack_overflow_1(void)
{
	IOdebug("Rom Disk stack overflow1!");
}
#endif

/*---------------------------------------------------------------------------*/
/* -- End of rom.c */
