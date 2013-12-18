/*------------------------------------------------------------------------
--                                                                      --
--			H E L I O S   S E R V E R S			--
--			---------------------------			--
--                                                                      --
--             Copyright (C) 1991, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- include.c								--
--                                                                      --
--	The main include disk program.					--
--                                                                      --
--	Author:  BLV 21.3.91						--
--                                                                      --
------------------------------------------------------------------------*/

/* $Header: /hsrc/servers/include/RCS/include.c,v 1.3 1992/09/04 10:45:03 nickc Exp $ */

#include <helios.h>
#include <syslib.h>
#include <servlib.h>
#include <nonansi.h>
#include <message.h>
#include <gsp.h>
#include <task.h>
#include <stdlib.h>

	/* The header file defines the name of the include disk binary image */
	/* It also defines the FileEntry structure.			     */
#include "include.h"

	/* Function prototypes */
static	BYTE	*extract_files(DirNode *root, int number, BYTE *buffer);
static	void	do_open(ServInfo *);
static	void	do_private(ServInfo *);		/* for debugging */

	/* Debugging flags	*/
static	word	DebugOptions	= 0;

	/* Root directory and DispatchInfo structure	*/
static	DirNode	Root;
static	DispatchInfo	IncludeInfo = {
	&Root,
	NullPort,
	SS_RomDisk,
	NULL,
	{ do_private, 2000 },
	{
		{ do_open,	2000 },		/* FG_Open		*/
		{ InvalidFn,	2000 },		/* FG_Create		*/
		{ DoLocate,	2000 },		/* FG_Locate		*/
		{ DoObjInfo,	2000 },		/* FG_ObjectInfo	*/
		{ InvalidFn,	2000 },		/* FG_ServerInfo	*/
		{ InvalidFn,	2000 },		/* FG_Delete		*/
		{ InvalidFn,	2000 },		/* FG_Rename		*/
		{ InvalidFn,	2000 },		/* FG_Link		*/
		{ InvalidFn,	2000 },		/* FG_Protect		*/
		{ InvalidFn,	2000 },		/* FG_SetDate		*/
		{ InvalidFn,	2000 },		/* FG_Refine		*/
		{ InvalidFn,	2000 },		/* FG_CloseObj		*/
	}
};

int main(void)
{ Object	*inc_disk;
  Stream	*file;
  BYTE		*buffer;
  int		size;  
  int		number_entries;
  BYTE		processor_name[IOCDataMax];
  Object	*name_entry;

#ifdef DEBUG
	/* If the program is to be started from the shell	*/
	/* but still linked with s0.o then it must accept 	*/
	/* an environment.					*/
  { Environ	env;
    (void) GetEnv(MyTask->Port, &env);
  }
#endif
  
	/* Find and open the IncDisk binary image		*/
  inc_disk	= Locate(Null(Object), IncludeDisk);
  if (inc_disk == Null(Object))
   { IOdebug("include : failed to locate %s", IncludeDisk);
     Exit(EXIT_FAILURE << 8);
   }
  file		= Open(inc_disk, Null(char), O_ReadOnly);
  if (file == Null(Stream))
   { IOdebug("include : failed to open %s, fault %x", inc_disk->Name,
   		Result2(inc_disk));
     Exit(EXIT_FAILURE << 8);
   }
  Close(inc_disk);

	/* Read the file into a suitable buffer.		*/
  size = (int)GetFileSize(file);
  if (size < 0)
   { IOdebug("include : error getting file size of %s, fault 0x%x",
   		file->Name, size);
     Exit(EXIT_FAILURE << 8);
   }
  buffer = (BYTE *) Malloc(size);
  if (buffer == Null(BYTE))
   { IOdebug("include : not enough memory for include disk %s",
   		file->Name);
     Exit(EXIT_FAILURE << 8);
   }
  if (Read(file, buffer, size, -1) != size)
   { IOdebug("include : failed to read all of include disk %s",
   		file->Name);
     Exit(EXIT_FAILURE << 8);
   }
  Close(file);

  number_entries = *((int *) buffer);  
  InitNode((ObjNode *) &Root, "include", Type_Directory, 0, 0x21110905);
  InitList(&(Root.Entries));

	/* extract all the files and subdirectories	*/
  (void) extract_files(&Root, number_entries, &(buffer[sizeof(int)]));

	/* Register the server and call the dispatcher	*/
  MachineName(processor_name);
  IncludeInfo.ReqPort	= NewPort();
  { NameInfo	name;
    Object	*this_processor;
    
    this_processor = Locate(Null(Object), processor_name);
    if (this_processor == Null(Object))
     { IOdebug("include : failed to locate own processor %s", processor_name);
       Exit(EXIT_FAILURE << 8);
     }
     
    name.Port   = IncludeInfo.ReqPort;
    name.Flags  = Flags_StripName;
    name.Matrix = DefNameMatrix;
    name.LoadData = Null(WORD);

    name_entry = Create(this_processor, "include", Type_Name, sizeof(NameInfo),
      (BYTE *) &name);
    if (name_entry == Null(Object))
     { IOdebug("include : failed to enter name in name table, error code 0x%x",
           Result2(this_processor));
       Exit(EXIT_FAILURE << 8);
     }
    Close(this_processor);
  }

  Dispatch(&IncludeInfo);
  Delete(name_entry, Null(char));
  Exit(0);
}

/**-----------------------------------------------------------------------------
*** This routine extracts the various header files from the buffer and
*** uses the information to initialise the directory tree.
**/
static BYTE	*extract_files(DirNode *parent, int number, BYTE *buffer)
{ ObjNode	*objnode;
  int		size;

  while (number-- > 0)
   { 	/* Insert next entry into the current directory.	*/
     objnode = (ObjNode *) buffer;
     if ((objnode->Type != Type_File) && (objnode->Type != Type_Directory))
      { IOdebug("include : file %s appears to be corrupt", IncludeDisk);
        Exit(EXIT_FAILURE << 8);
      }
     Insert(parent, objnode, FALSE);
     objnode->Dates.Creation = objnode->Dates.Access =
     objnode->Dates.Modified = GetDate();

	/* If a file, move to the next entry	*/
     if (objnode->Type == Type_File)
      { size = sizeof(FileEntry) + (int)objnode->Size;
        size = (size + 3) & ~3;
        buffer  = &(buffer[size]);
      }
     else
	/* If a directory, extract its contents	*/
      { DirNode	*subdir		= (DirNode *) objnode;
        int	number_entries	= (int)subdir->Nentries;
	subdir->Nentries	= 0;
	InitList(&(subdir->Entries));
	buffer = &(buffer[sizeof(DirNode)]);
        buffer = extract_files((DirNode *) objnode, number_entries, buffer);
      }
   }
  return(buffer);	
}

/**-----------------------------------------------------------------------------
*** Incoming Open requests.
**/
static void handle_read(MCB *m, ObjNode *f);
static void handle_seek(MCB *m, ObjNode *f);

static void do_open(ServInfo *servinfo)
{ MCB		*m	= servinfo->m;
  MsgBuf	*r;
  ObjNode	*f;
  Port		reqport;
  BYTE		*data	= m->Data;
  char		*pathname = servinfo->Pathname;
  
  f = GetTarget(servinfo);
  if (f == Null(ObjNode))
   { ErrorMsg(m, EO_File); return; }

  r = New(MsgBuf);
  if (r == Null(MsgBuf))
   { ErrorMsg(m, EC_Error + EG_NoMemory + EO_Message); return; }

  FormOpenReply(r, m, f, Flags_Server | Flags_Closeable | Flags_Selectable,
		 pathname);
  reqport = NewPort();
  r->mcb.MsgHdr.Reply = reqport;
  PutMsg(&r->mcb);
  Free(r);
  
  if (f->Type == Type_Directory)
   { DirServer(servinfo, m, reqport); FreePort(reqport); return; }

  f->Account++;  
  f->Dates.Access	= GetDate();

  forever
   { WORD	e;
    
     m->MsgHdr.Dest	= reqport;
     m->Timeout		= StreamTimeout;
     m->Data		= data;

     UnLockTarget(servinfo);      
     e = GetMsg(m);
     m->MsgHdr.FnRc	= SS_RomDisk;
     LockTarget(servinfo);

     if (e < Err_Null) break;	/* abort on any error	*/

     f->Dates.Access = GetDate();
      
     switch(e & FG_Mask)
      { case	FG_Read		: handle_read(m, f); break;

     	case	FG_Close	: 
       	 	if (m->MsgHdr.Reply != NullPort) ErrorMsg(m, Err_Null);
		goto done;

       	case	FG_GetSize	:
       	 	InitMCB(m, 0, m->MsgHdr.Reply, NullPort, Err_Null);
       	 	MarshalWord(m, f->Size);
       	 	PutMsg(m);
       	 	break;
       	 	
	case	FG_Seek : handle_seek(m, f); break;
	 
	case	FG_Select :
		e &= O_ReadOnly;
		if (e == 0)
		 ErrorMsg(m, EC_Error + EG_Protected + EO_File);
		else
		 ErrorMsg(m, e);
		break;
	 
	default	: ErrorMsg(m, EC_Error + EG_FnCode + EO_File); break;
      }
   }

done:
  f->Account--;
  FreePort(reqport);
}

static void handle_read(MCB *m, ObjNode *f)
{ ReadWrite	*rw	= (ReadWrite *) m->Control;
  WORD		pos	= rw->Pos;
  WORD		size	= rw->Size;
  FileEntry	*file	= (FileEntry *) f;
  int		i;
  word		sequence= 0;
  Port		reply	= m->MsgHdr.Reply;

  if (pos < 0)
   { ErrorMsg(m, EC_Error + EG_Parameter + 1); return; }
  if (size < 0)
   { ErrorMsg(m, EC_Error + EG_Parameter + 2); return; }
   
  if (pos >= f->Size)
   { m->MsgHdr.FnRc = ReadRc_EOF; ErrorMsg(m, 0); return; }
   
  if ((pos + size) > f->Size) size = f->Size - pos;

	/* Send the data in chunks of up to 65535 bytes	*/
  for (i = 0; i < size; i+= 65535)
   { m->MsgHdr.Dest	= reply;
     m->MsgHdr.Reply	= NullPort;
     m->MsgHdr.ContSize	= 0;

	/* Is this the last message ?	*/
     if (((WORD)i + 65535) < size)
      { 	/* Not the last message yet */
	m->MsgHdr.DataSize	 = 65535;
	m->MsgHdr.Flags		 = MsgHdr_Flags_preserve;
	m->MsgHdr.FnRc		 = sequence + ReadRc_More;
	sequence		+= 16;
      }
     else
      { m->MsgHdr.DataSize	= (int)size - i;
	m->MsgHdr.Flags		= 0;
	m->MsgHdr.FnRc		= sequence + ReadRc_EOD;
      }

	/* point to next bit of buffer	*/
     m->Data	= &(file->Data[i + pos]);
     m->Timeout	= 5 * OneSec;
	/* And send the data to the client	*/
     PutMsg(m);
   }
}

static void handle_seek(MCB *m, ObjNode *f)
{ SeekRequest	*req	= (SeekRequest *) m->Control;
  WORD		curpos	= req->CurPos;
  WORD		mode	= req->Mode;
  WORD		newpos	= req->NewPos;
  
  switch(mode)
   { case S_Beginning	: break;
     case S_Relative	: newpos += curpos; break;
     case S_End		: newpos += f->Size; break;
   }
  if (newpos > f->Size) newpos = f->Size;
  if (newpos < 0) newpos = 0;
  InitMCB(m, 0, m->MsgHdr.Reply, NullPort, Err_Null);
  MarshalWord(m, newpos);
  PutMsg(m);
}

/**-----------------------------------------------------------------------------
*** Private protocol messages
**/


static void do_private(ServInfo *servinfo)
{ ObjNode	*f;
  MCB		*m	= servinfo->m;
  IOCMsg2	*req	= (IOCMsg2 *) m->Control;

	/* The message must refer to the root directory of /include	*/
  f = GetTarget(servinfo);
  if (f == Null(ObjNode))
   { ErrorMsg(m, EO_Directory); return; }
  if (f != (ObjNode *) &Root)
   { ErrorMsg(m, EC_Error + EG_WrongFn + EO_Object); return; }

	/* Cope with the three different debugging requests.		*/
  switch(servinfo->FnCode & FG_Mask)
   { case	FG_GetInfo :
	InitMCB(m, 0, m->MsgHdr.Reply, NullPort, Err_Null);
	MarshalWord(m, DebugOptions);
	PutMsg(m);
	break;

     case	FG_SetInfo :
	DebugOptions	= req->Arg.Mode;
	ErrorMsg(m, Err_Null);
	break;

     case	FG_Terminate :
	ErrorMsg(m, Err_Null);
	AbortPort(IncludeInfo.ReqPort, EC_Fatal + EG_Exception + EE_Abort);
	break;

     default : ErrorMsg(m, EC_Error + EG_WrongFn + EO_Object); break;
   }
}

		
