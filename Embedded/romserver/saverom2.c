/*------------------------------------------------------------------------
--                                                                      --
--			H E L I O S   S E R V E R S			--
--			---------------------------			--
--                                                                      --
--             Copyright (C) 1991, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- rom.c								--
--                                                                      --
--	This is a ROM file server derived from the Helios include       --
--      server example documented in the Helios Parallel Operating      --
--      system book. Reference to the example is recomended before      --
--      modification is made to this example.                           --
--                                                                      --
--	Author:  BLV 21.3.91						--
--      Author:  ACC 16.5.91                                            --
--	Author:  TC  10.12.93						--
--									--
--  Tony says ...							--
--	Rom server modified to pass the loader the address of any 	--
-- programs in the rom disk directly when they are in rom or ram (ie	--
-- when the rom disk file is either read into memory or already in	--
-- memory).								--
--									--
------------------------------------------------------------------------*/

/* $Id: rom.c,v 1.4 1993/12/14 15:39:15 tony Exp $ */

#include <helios.h>
#include <syslib.h>
#include <servlib.h>
#include <task.h>
#include <sys/types.h>
#include <fcntl.h>
#include <posix.h>
#include <string.h>

#include <root.h>

#pragma -s1

#define DEBUG 1

/*
 * Define FILEBASED means that the filing system is being provided by a file
 * on another server. For example a host development computer which holds an
 * image of the filing system on disk. 
 * In this case the definition FILEBASEDFILE is required.
 * All reads on this filing system will be modified and passed onto that server
 * so that that reply goes straight back to the client instead of through
 * here.
 * 
 * Define ROMBASED means that the filing system is in local memory. This can
 * be done in either of two ways :-
 * 1) By copying a remote file into memory - In which case a definition of 
 *     ROMBASEDFILE is required.
 * 2) By finding the nucleus vector table and checking all the entries
 *     for T_RomDisk - In which case ROMBASEDNUCLEUS is required.
 */

/*
 * We shall access the ROM using the following generic functions 
 * 
 * ROMINFO *OpenRom();
 * int ReadRom(ROMINFO *stream, BYTE *buffer, WORD size, WORD timeout);
 * void CloseRom(ROMINFO *stream);
 */

#ifndef FILEBASED
#  ifndef ROMBASED
#    if 0
#      define FILEBASED
#    else
#      define ROMBASED
#    endif
#  endif
#endif

#ifdef FILEBASED

#ifndef FILEBASEDFILE
# define FILEBASEDFILE "/d/helios/user/guest/rom/romdir.fil"
# if 0
#  define FILEBASEDFILE "/c/helios/lib/romdisk"
# endif
#endif

typedef Stream *	ROMHANDLE;

static struct ROMINFO *	OpenRom (void);

#define ReadRom(rominfo,buf,size,timeout) \
		Read (rominfo->instream, buf, size, timeout)

#define CloseRom(rominfo) \
		Close (rominfo->instream); \
		Free (rominfo)

#else	/* ROMBASED */

# ifndef ROMBASEDFILE
#  ifndef ROMBASEDNUCLEUS
#    if 0
#      define ROMBASEDFILE "/d/helios/user/guest/rom/romdir.fil" */
#    endif
#  endif
#endif

typedef struct RomFile
{
  	UBYTE *	rombuffer;
  	WORD 	pos;
} RomFile;

#define ROMHANDLE	RomFile

static struct ROMINFO *	OpenRom(void);

static int	ReadRom (struct ROMINFO * stream,
		 	 BYTE * buffer,
			 WORD size,
			 WORD timeout);

static int	CloseRom (struct ROMINFO * stream);

#endif

typedef struct ROMINFO
{
#ifndef ROMBASEDNUCLEUS
  	struct ImageHdr	ihdr;
#endif
	struct Module	modhdr;
  	long		romsize;
  	ROMHANDLE	instream;
} ROMINFO;

ROMINFO *	RomInfo;

static void 	getrombuffer (ROMINFO * f);
static void 	extract_files (DirNode * root, int number);

static DirNode	Root;

static char	processor_name[IOCDataMax];

static void	do_open (ServInfo *);
static void	do_delete(ServInfo *);
static void	do_serverinfo(ServInfo *);

static DispatchInfo RomDispatchInfo =
{
	&Root,
	NullPort,
	SS_RamDisk,
	NULL,
	{NULL, 0},
	{
    		{do_open, 2000},	/* FG_Open		*/
    		{InvalidFn, 2000},	/* FG_Create		*/
    		{DoLocate, 2000},	/* FG_Locate		*/
    		{DoObjInfo, 2000},	/* FG_ObjectInfo	*/
    		{do_serverinfo, 2000},	/* FG_ServerInfo	*/
    		{do_delete, 2000},	/* FG_Delete		*/
    		{DoRename, 2000},	/* FG_Rename		*/
    		{DoLink, 2000},		/* FG_Link		*/
    		{DoProtect, 2000},	/* FG_Protect		*/
    		{DoSetDate, 2000},	/* FG_SetDate		*/
    		{DoRefine, 2000},	/* FG_Refine		*/
    		{NullFn, 2000},		/* FG_CloseObj		*/
    		{DoRevoke, 2000},	/* FG_Revoke		*/
    		{InvalidFn, 2000},	/* Reserved 		*/
    		{InvalidFn, 2000}	/* Reserved		*/
  }
};

#ifdef FILEBASED

static ROMINFO * OpenRom (void)
{
	ROMINFO *	r;
	Object *	rom_disk;

	r = New (ROMINFO);
	if (r == NULL)
	{
		IOdebug ("rom: failed to allocate ROMINFO structure");

		return NULL;
	}

	rom_disk = Locate (Null(Object), FILEBASEDFILE);
	if (rom_disk == Null(Object))
	{
		IOdebug ("rom : failed to locate %s", FILEBASEDFILE);

		return NULL;
	}

	r -> instream = Open (rom_disk, Null(char), O_ReadOnly);
	if (r -> instream == Null(Stream))
	{
		IOdebug ("rom : failed to open %s, fault %x", rom_disk->Name,

		Result2 (rom_disk));

		return NULL;
	}

	Close (rom_disk);

	return r;
}

#else	/* ROMBASED */

static ROMINFO * OpenRom (void)
{
	ROMINFO *	r = New (ROMINFO);

#ifdef ROMBASEDFILE
	Object *	rom_disk;
	Stream *	f;
	WORD 		size;
	WORD		n;

	/* attempt to find rom disk globally */
	rom_disk = Locate (Null(Object), ROMBASEDFILE);
	if (rom_disk == Null(Object))
	{
		/* attempt to find it locally */
		rom_disk = Locate (CurrentDir, ROMBASEDFILE);

		if (rom_disk == Null (Object))
		{
			IOdebug ("rom : failed to locate %s", ROMBASEDFILE);

			return NULL;	
		}
	}

	f = Open (rom_disk, Null(char), O_ReadOnly);
	if (f == Null(Stream))
	{
		IOdebug ("rom : failed to open %s, fault %x", rom_disk->Name,

		Result2 (rom_disk));

		return NULL;
	}

	Close (rom_disk);

	if ((size = GetFileSize (f)) < Err_Null)
	{
		IOdebug ("rom : failed to get file size of %s, fault %x",
				f->Name, Result2 (f));

		return NULL;
	}

	r -> instream.rombuffer = Malloc (size);

	IOdebug ("rombuffer starts at 0x%x (size: %d bytes)", 
			(long)(r -> instream.rombuffer), size);

	n = Read (f, (BYTE *) r -> instream.rombuffer, size, -1);
	if (n != size)
	{
		IOdebug("rom : failed to get %d bytes from %s, fault %x",
				size, f->Name, Result2 (f));

		return NULL;
	}

	Close (f);

#else	/* !ROMBASEDFILE => ROMBASEDNUC */

	MPtr 	vector_ptr = MInc_(GetSysBase (), sizeof (MPtr));


#ifdef DEBUG
	IOdebug ("Nucleus Base  = %x", vector_ptr);
	IOdebug ("*Nucleus Base = %x", MWord_(vector_ptr, 0));
#endif

	while (MWord_(vector_ptr, 0))
	{
		MPtr	p = MRTOA_(vector_ptr);
#ifdef DEBUG
		IOdebug ("vector_ptr =  %x", vector_ptr);
		IOdebug ("*vector_ptr = %x", MWord_(vector_ptr, 0));
#endif
		IOdebug ("p  = %x", (long)p);
		IOdebug ("*p = %x", MWord_(p, 0));

		if (ModuleWord_(p, Type) == T_RomDisk)
		{
			break;
		}
		vector_ptr = MInc_(vector_ptr, sizeof (MPtr));
	}

/*	if (*vector_ptr == NULL) */
	if (MWord_(vector_ptr, 0) == NULL)
	{
		IOdebug ("Failed to find rom disk in nucleus");

		return NULL;
	}

	r -> instream.rombuffer = (UBYTE *)(MWord_(vector_ptr, 0));
#endif
	r->instream.pos = 0;

	return r;
}

static int ReadRom (ROMINFO * 	stream,
		    BYTE * 	buffer,
		    WORD 	size,
		    WORD 	timeout)
{
	timeout = timeout;
	memcpy (buffer,
		&stream -> instream.rombuffer[stream -> instream.pos],
		size);

	stream -> instream.pos += size;

	return size;
}

static int CloseRom (ROMINFO * stream)
{
#ifdef ROMBASEDFILE
	Free (stream->instream.rombuffer);
#endif
	Free(stream);

	return NULL;
}

#endif

int main (void)
{
	Environ env;
	WORD res;
	bool in_nucleus = ((long) main) < (long) GetRoot ();

	GetRoot () -> ATWFlags = 0x87654321;

	if (!in_nucleus)
	{
		(void) GetEnv (MyTask->Port, &env);
	}

	MachineName (processor_name);

	if ((RomInfo = OpenRom ()) == NULL)
	{
		IOdebug ("rom: failed to find rom disk data");

		Exit (128);
	}

#ifndef ROMBASEDNUCLEUS
	/*
	 * If the rom disk file is part of the nucleus, there is no 
	 * Image Header (sysbuild removes it).
	 */

	if ((res = ReadRom (RomInfo,
			    (BYTE *) &RomInfo -> ihdr,
			    sizeof (struct ImageHdr), -1))
	    != sizeof (struct ImageHdr))
	{
		IOdebug ("rom: error reading image header %x", res);

		return 1;
	}
#ifdef DEBUG
	else
	{
		IOdebug ("rom: read Image Header (%d bytes)", sizeof (ImageHdr));
	}
#endif

#ifdef DEBUG
	IOdebug ("rom: Image Header = [%x, %x, %x]", 
  			RomInfo -> ihdr.Magic,
			RomInfo -> ihdr.Flags,
			RomInfo -> ihdr.Size);
#endif

#endif

	if ((res = ReadRom (RomInfo,
			    (BYTE *) &RomInfo->modhdr,
			    sizeof (struct Module), -1))
	    != sizeof (struct Module))
	{
		IOdebug ("rom: error reading module header %x", res);

		return 1;
	}
#ifdef DEBUG
	else
	{
		IOdebug ("rom: read Module Header (%d bytes)", sizeof (struct Module));
	}
#endif

	if ((res = ReadRom (RomInfo,
			    (BYTE *) &Root,
			    sizeof(DirNode), -1))
	    != sizeof(DirNode))
	{
		IOdebug ("rom: error reading root DirNode %x", res);

		return 1;
	}
#ifdef DEBUG
	else
	{
		IOdebug ("rom: read root DirNode (%d bytes)", sizeof (struct DirNode));
	}
#endif
  
	RomInfo->romsize = 8;

	{
		WORD 		number = Root.Nentries;
		LinkNode *	parent;
		Object *	me = Locate (Null(Object), processor_name);

		parent = (LinkNode *)Malloc (sizeof(LinkNode) + strlen(processor_name));

		InitNode(&parent -> ObjNode,
			 "..",
			 Type_Link,
			 0,
			 DefDirMatrix);

		parent -> Cap = me -> Access;
		strcpy (parent -> Link, processor_name);

		Close (me);

		InitNode ((ObjNode *)&Root,
			  Root.Name,
			  Root.Type,
			  Root.Flags,
			  Root.Matrix);
		InitList (&(Root.Entries));

		Root.Parent = (DirNode *)parent;

#ifdef DEBUG
		IOdebug ("rom: Calling extract files ... ");
#endif

		extract_files (&Root, number);
	}

	/* Read the buffer now */
#ifdef NEVER
	getrombuffer (RomInfo);
#endif

	RomDispatchInfo.ReqPort = NewPort ();

	{
		NameInfo 	name;
		Object *	ThisProcessor;
		Object *	NameEntry;

		ThisProcessor = Locate (Null(Object), processor_name);

		if (ThisProcessor == Null(Object))
		{
			IOdebug ("rom : failed to locate own processor %s",
					processor_name);

			return 1;
		}

		name.Port     = RomDispatchInfo.ReqPort;
		name.Flags    = Flags_StripName;
		name.Matrix   = DefDirMatrix;	/* BLV - for now */
		name.LoadData = Null(WORD);
    
#ifdef DEBUG
		IOdebug ("rom: creating %s hanging from processor %s",
					Root.Name, processor_name);
#endif
    
		NameEntry = Create (ThisProcessor,
				    Root.Name,
				    Type_Name,
				    sizeof(NameInfo),
				    (BYTE *)&name);

		if (NameEntry == Null(Object))
		{
			IOdebug ("rom : failed to enter name in name table, error code 0x%08x",
						Result2(ThisProcessor));

			return 1;
		}
		Close (ThisProcessor);
	}

	if (in_nucleus)
	{
		MCB m;

		InitMCB (&m, 0, MyTask->Parent, NullPort, 0x456);
		(void) PutMsg (&m);
	}

	Dispatch (&RomDispatchInfo);
	CloseRom (RomInfo);
}

static ObjNode *getobjnode()
{
	ObjNode *	objnode = New (ObjNode);

	if (ReadRom (RomInfo, (BYTE *)objnode, sizeof (ObjNode), -1)
		!= sizeof(ObjNode))
	{
		IOdebug ("rom: error when reading objnode");
		Exit (256);
	}

	RomInfo->romsize += sizeof (ObjNode);
#ifdef DEBUG
	IOdebug ("objnode %s %d", objnode->Name, objnode->Type);
#endif

	return objnode;
}

static void LoadRomImage (char *	name,
			  byte *	addr)
{
	Object *	loader;
	MCB		mcb;

#ifdef DEBUG
	IOdebug ("rom: LoadRomImage (%s, %x)", name, (int)addr);
#endif

	while ((loader = Locate (NULL, "/loader")) == NULL)
		;

	mcb.Data    = (byte *)(Malloc (IOCDataMax));
	mcb.Control = (word *)(Malloc (sizeof (IOCCommon) + (2 * sizeof (word))));

	InitMCB (&mcb, 0, MyTask -> IOCPort, NullPort, FC_GSP | FG_RomCreate);

	MarshalCommon (&mcb, loader, NULL);
	MarshalString (&mcb, name);
	MarshalWord (&mcb, (word)addr);

	PutMsg (&mcb);

	Free (mcb.Control);
	Free (mcb.Data);

	Close (loader);
}

static void extract_files (DirNode * 	parent,
			   int		number)
{
	ObjNode *objnode;

	IOdebug ("rom: extract_files (%s, %d)", parent -> Name, number);

	while (number-- > 0)
	{
		int number_entries;

		objnode = getobjnode ();

		if (   (objnode -> Type != Type_File)
		    && (objnode -> Type != Type_Directory))
		{
			IOdebug ("rom : ROM image appears to be corrupt");

			Exit (256);
		}

		number_entries = objnode->Size;

		InitNode (objnode,
			  objnode->Name,
			  objnode->Type,
			  objnode->Flags, objnode->Matrix);

		Insert(parent, objnode, FALSE);

		if (objnode->Type == Type_Directory)
		{
			IOdebug ("rom: extract_files () - found DirNode");

			InitList (&((DirNode *)objnode) -> Entries);

			extract_files ((DirNode *)objnode, number_entries);
		}
		else
		{
			byte *	addr;
			word	offset;

			IOdebug ("rom: extract_files () - found ObjNode");

			/*
			 * Address of the executable (if such it is) is
			 *	address of buffer + offset
			 * where offset can be found in the first word of the
			 * Contents field in objnode.
			 */

			offset = (word)((&objnode->Size)[1]);
#ifndef ROMBASEDNUCLEUS
			/*
			 * Real rom disk files have an Image Header placed
			 * on the front which the offset does not take into
			 * account.
			 */
			offset += sizeof (ImageHdr);
#endif

			addr = (byte *)((int)(RomInfo -> instream.rombuffer) + offset);

#ifdef DEBUG
			IOdebug ("rom: %s appears at %x in romdisk",
					objnode -> Name, (long)addr);
			IOdebug ("rom: *(word *)addr = %x", *(word *)addr);
#endif

			/* If the file is an executable, enter it into the loader */
			if (*(word *)(addr) == Image_Magic)
			{
#ifdef DEBUG
				IOdebug ("rom: %s is an executable",
						objnode -> Name);
#endif
				LoadRomImage (objnode -> Name, addr);
			}
#ifdef DEBUG
			else
			{
				IOdebug ("rom: %s is not an executable", objnode -> Name);
			}
#endif
			objnode->Size = number_entries;
		}
	}
}

static void getrombuffer (ROMINFO * f)
{
	WORD size;

	if (ReadRom(f, (BYTE *)&size, 4, -1) != 4)
	{
		IOdebug ("failed to read rom buffer size");
		Exit (256);
	}

	f -> romsize = (size + 4);
#ifdef DEBUG
	IOdebug("rom: getrombuffer () - size %d (0x%x)", size, size);
#endif

}

static void	handle_read (MCB * m, ObjNode * f);
static void	handle_seek (MCB * m, ObjNode * f);

static void do_open (ServInfo * servinfo)
{
	MCB *		m = servinfo -> m;
	MsgBuf *	r;
	DirNode *	d;
	ObjNode *	f;
	IOCMsg2 *	req = (IOCMsg2 *)(m -> Control);
	Port 		reqport;
	BYTE *		data = m -> Data;
	char *		pathname = servinfo -> Pathname;

	d = GetTargetDir (servinfo);
	if (d == Null(DirNode))
	{
		ErrorMsg (m, Err_Null);

		return;
	}

	f = GetTargetObj (servinfo);
	if (f == Null(ObjNode))
	{
		ErrorMsg (m, Err_Null);
		return;
	}

	if (!CheckMask (req -> Common.Access.Access,
		        req -> Arg.Mode & Flags_Mode))
	{
		ErrorMsg (m, EC_Error + EG_Protected + EO_File);

		return;
	}

	r = New (MsgBuf);
	if (r == Null (MsgBuf))
	{
		ErrorMsg (m, EC_Error + EG_NoMemory + EO_Message);

		return;
	}

	FormOpenReply (r,
		       m,
		       f,
		       Flags_Server|Flags_Closeable|Flags_MSdos,
		       pathname);

	reqport = NewPort ();
	r -> mcb.MsgHdr.Reply = reqport;

	PutMsg (&r -> mcb);

	Free (r);

	if (f -> Type == Type_Directory)
	{
		DirServer (servinfo, m, reqport);
		FreePort (reqport);
 
		return;
	}

	f -> Account++;

	UnLockTarget (servinfo);

	forever
	{
		WORD e;

		m -> MsgHdr.Dest = reqport;
		m -> Timeout	 = StreamTimeout;
		m -> Data	 = data;

		e = GetMsg (m);
		if (e == EK_Timeout)
			break;
		if (e < Err_Null)
			continue;

		Wait (&(f -> Lock));

		switch (m -> MsgHdr.FnRc & FG_Mask)
		{
			case FG_Read:
				handle_read (m, f);
				break;

			case FG_Close:
				if (m -> MsgHdr.Reply != NullPort)
				ErrorMsg (m, Err_Null);
				FreePort (reqport);
				f->Account--;
				Signal (&(f -> Lock));

				return;

			case FG_GetSize:
				InitMCB (m, 0, m -> MsgHdr.Reply, NullPort, Err_Null);
				MarshalWord (m, f -> Size);
				PutMsg (m);
				break;

			case FG_Seek:
				handle_seek (m, f);
				break;

			case FG_Select:
				ErrorMsg (m, e & Flags_Mode);
				break;

			default:
				ErrorMsg (m, EC_Error + EG_FnCode + EO_File);
				break;
		}
		Signal (&(f -> Lock));
	}

	f -> Account--;
	FreePort (reqport);
}

static void handle_read (MCB * m, ObjNode * f)
{
	ReadWrite	*rw	= (ReadWrite *)(m -> Control);
	WORD		 pos	= rw -> Pos;
	WORD		 size	= rw -> Size;
	WORD		 offset	= (&f -> Size)[1];
	WORD		 i;
	Port		 reply;
	WORD		 sequence;

	if (pos < 0)
	{
		ErrorMsg (m, EC_Error + EG_Parameter + 1);
		return;
	}

	if (size < 0)
	{
		ErrorMsg (m, EC_Error + EG_Parameter + 2);
		return;
	}

	if (pos >= f -> Size)
	{
		m -> MsgHdr.FnRc = ReadRc_EOF;
		ErrorMsg (m, 0);
		return;
	}

	if ((pos + size) > f->Size)
	{
		size = f -> Size - pos;
		rw -> Size = size;
	}

	if (size == 0)
	{
		m -> MsgHdr.FnRc = ReadRc_EOD;
		ErrorMsg (m, 0);
		return;
	}

	/* For file-based pseudo-romdisks, the read request should	*/
	/* be forwarded directly to the file server unmodified.		*/
#ifdef FILEBASED
	m -> MsgHdr.Dest	= RomInfo -> instream -> Server;

	rw -> Pos += offset;

	PutMsg(m);
#else
	/* For real ROM systems the data can be sent directly. It may	*/
	/* have to be split over more than one message.			*/
	reply	  = m -> MsgHdr.Reply;
	sequence = 0;

	for (i = 0; i < size; i += 65535)
	{
		m -> MsgHdr.Dest	= reply;
		m -> MsgHdr.Reply	= NullPort;
		m -> MsgHdr.ContSize	= 0;

		/* Is this the last message ? */
		if ((i + 65535) < size)
		{	/* Not yet */
			m -> MsgHdr.DataSize	= 65535;
			m -> MsgHdr.Flags	= MsgHdr_Flags_preserve;
			m -> MsgHdr.FnRc	= sequence + ReadRc_More;
			sequence += ReadRc_SeqInc;
		}
		else
		{
			m -> MsgHdr.DataSize = size - i;
			m -> MsgHdr.Flags    = 0;
			m -> MsgHdr.FnRc     = sequence + ReadRc_EOD;
		}

# define MOVEWORKS
# ifdef MOVEWORKS
		m -> Data = (byte *)&(RomInfo -> instream.rombuffer[pos + offset + i]);
# else
		{
			int   	j;
			byte *	buf = (byte *)&(RomInfo -> instream.rombuffer[pos + offset + i]);

			static char	buffer[65536];

#if 0
			for(j= 0; j < i; j++)
#else
			/* Alenia bug fix. */
			for(j= 0; j < m -> MsgHdr.DataSize ; j++)
#endif
				buffer[j] = *buf++;
			m -> Data = buffer;
		}
#endif	/* MOVEWORKS */
		PutMsg (m);
	}
#endif	/* FILEBASED */
}

static void handle_seek (MCB * m, ObjNode * f)
{
	SeekRequest *	req = (SeekRequest *)(m -> Control);

	WORD 	curpos = req -> CurPos;
	WORD 	mode   = req -> Mode;
	WORD 	newpos = req -> NewPos;

	switch (mode)
	{
		case S_Beginning:
			break;
		case S_Relative:
			newpos += curpos;
			break;
		case S_End:
			newpos += f -> Size;
			break;
	}

	if (newpos > f -> Size)
		newpos = f -> Size;

	if (newpos < 0)
		newpos = 0;

	InitMCB (m, 0, m->MsgHdr.Reply, NullPort, Err_Null);
	MarshalWord (m, newpos);
	PutMsg (m);
}

static void do_delete(ServInfo * servinfo)
{
	MCB *		m = servinfo -> m;
	ObjNode *	obj = GetTarget (servinfo);
	IOCCommon *	req = (IOCCommon *)(m -> Control);

	if (obj == NULL)
	{
		ErrorMsg (m, EO_File);
		return;
	}

	if (!CheckMask (req -> Access.Access, AccMask_D))
	{
		ErrorMsg (m, EC_Error + EG_Protected + EO_File);
		return;
	}

	if (obj -> Type == Type_Directory &&
	    ((DirNode *)obj) -> Nentries != 0)
	{
		ErrorMsg (m, EC_Error + EG_Delete + EO_Directory);
		return;
	}

	Unlink (obj, FALSE);

	/*
	 * To stop ServLib worker indirection through Free'd
	 * target on unlock
	 */
	servinfo -> TargetLocked = FALSE;

	Free (obj);

	ErrorMsg (m, Err_Null);
}


static void do_serverinfo (ServInfo * servinfo)
{
	MCB *	m = servinfo -> m;
	WORD 	info[4];

	InitMCB (m, 0, m -> MsgHdr.Reply, NullPort, Err_Null);

	info[0] = Type_Directory;
	info[1] = RomInfo->romsize;
	info[2] = 0;
	info[3] = 1;

	m -> Data = (byte*)info;
	m -> MsgHdr.DataSize = 16;
	PutMsg (m);
}
