/*------------------------------------------------------------------------
--                                                                      --
--           H E L I O S   N E T W O R K I N G   S O F T W A R E	--
--           ---------------------------------------------------	--
--                                                                      --
--             Copyright (C) 1990, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- readcdl.c								--
--                                                                      --
--	Purpose : read an old-style CDL binary and convert it to a	--
--	Taskforce structure.						--
--                                                                      --
--	Author:  BLV 26/7/90						--
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Header: /hsrc/network/RCS/readcdl.c,v 1.15 1993/12/20 13:33:39 nickc Exp $*/

/*{{{  headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <message.h>
#include <syslib.h>
#include <nonansi.h>
#include <posix.h>
#include "private.h"
#include "exports.h"
#include "netutils.h"
#include "rmlib.h"

#ifdef Malloc
#undef Malloc
#endif
/*}}}*/
/*{{{  old CDL binary format */
/**
*** The old format of a CDL binary. The same structure is used in the
*** binary file and in memory. All the strings occur at the end of the
*** file, and some arithmetic is needed to figure out where these
*** start. Then there are various indices into the table of strings.
***
*** First there is a fixed size header. This contains a magic number,
*** which should have been followed by a Flags field (zero for now) and
*** an image size. This would have given compatibility with binary programs.
*** Instead there are no Flags or size fields.
***
*** At the start of the header are various counts. These can be used to
*** determine the start of the strings.
**/

typedef struct CDL_Header {
	WORD		Type;		/* should be 0x12345677 */
	WORD		NoComponents;	/* Not FLAGS !!! @%*!&!;@! */
	WORD		NoCStreams;
	WORD		NoIStreams;
	WORD		NoAttribs;
	WORD		Ignore1;	/* actually char * */
	WORD		CurrentDirIndex;
	WORD		Ignore2;	/* another char * */
	WORD		TfNameIndex;
} CDL_Header;

/**
*** Following the commands are all the components. This contains large
*** quantities of junk.
**/

typedef enum { ANY_PROCESSOR, T212, T414, T800 } PTYPE;

typedef struct CDL_Component {
	WORD		Ignore3;	/* char * for name */
	WORD		NameIndex;
	WORD		Flags;
	WORD		Ignore4;	/* Object pointer to code */
	WORD		Ignore5;	/* char * for processor name */
	WORD		PuidIndex;
	PTYPE		Ptype;		/* enum */
	WORD		NumberAttribs;
	WORD		AttribIndex;	/* to first attribute */
	List		Ignore6;
	UWORD		Memory;
	WORD		LifeTime;	/* Mortal or immortal */
	UWORD		TimeLimit;
	WORD		Priority;
	WORD		NumberArguments;
	WORD		Ignore7;	/* char * for arguments */
	WORD		ArgumentsIndex;
	WORD		NoIStreams;
	WORD		IStreamIndex;
	WORD		Ignore8;	/* pointer to table of IStreams */
} CDL_Component;

/**
*** CDL_CStreams. These form a table of the pipes used for communication.
*** Every task maintains a number of CDL_IStreams, some of which index
*** into this table.
**/
typedef struct	CDL_CStream {
	WORD	Ignore8;
	WORD	NameIndex;
	WORD	Flags;
	WORD	Count;
} CDL_CStream;

/**
*** CDL_IStream. Every component has a number of these. The Index may be
*** an index into the table of CDL_CStreams, indicating that this stream
*** is used for communicating. Alternatively it may be -1 indicating that
*** the stream is inherited from the parent. The mode can be 1, 2, or 3,
*** read-only, write-only, read-write. The index is the standard stream
*** number within this task.
**/
typedef struct CDL_IStream {
	WORD	Index;
	WORD	Mode;
	WORD	Standard;
} CDL_IStream;

/**
*** And attributes
**/
typedef struct CDL_Attribute {
	Node	Node;
	WORD	Count;
	WORD	Ignore;
	WORD	Index;
} CDL_Attribute;

/**
*** This structure is used when setting up the communications.
**/
typedef struct	Comms {
	RmTask		Writer;
	int		WriterChannel;
	RmTask		Reader;
	int		ReaderChannel;
	char		*FileName;
	int		OpenMode;
} Comms;

/**
*** This structure is used internally to contain various pointers as
*** the CDL binary is being analysed.
**/
typedef struct Pointers {
	CDL_IStream	*IStreams;
	CDL_Attribute	*Attribs;
	Comms		*Comms;
	char		*Strings;
	Object		*CurrentDirectory;
} Pointers;
/*}}}*/
/*{{{  forward declarations */
/**
*** Forward declarations.
**/
static RmTaskforce	extract_taskforce(BYTE *buffer);
static RmTaskforce	extract_header(CDL_Header *, Pointers *);
static RmTask		extract_component(CDL_Component *, Pointers *);
static bool		handle_redirection(Comms *Comms, Pointers *);
/*}}}*/
/*{{{  RmReadCDL() - entry point for this module */

RmTaskforce RmReadCDL(Stream *CDLFile, ImageHdr *hdr, int Size)
{ BYTE		*Buffer = Null(BYTE);  
  RmTaskforce	Taskforce = (RmTaskforce) NULL;

  Buffer = (BYTE *) Malloc(Size);
  if (Buffer eq Null(BYTE)) goto done;
  memcpy(Buffer, hdr, sizeof(ImageHdr));
  if (Read(CDLFile, &(Buffer[sizeof(ImageHdr)]), (word)Size - sizeof(ImageHdr), -1)
  	  ne ((word)Size - sizeof(ImageHdr)))
   goto done;

  (void) Close(CDLFile); CDLFile = Null(Stream);

  Taskforce = extract_taskforce(Buffer);
done:  
  if (CDLFile ne Null(Stream)) Close(CDLFile);
  if (Buffer  ne Null(BYTE)) Free(Buffer);
  return(Taskforce);
}

/*}}}*/
/*{{{  extract_taskforce(buffer) */
static RmTaskforce	extract_taskforce(BYTE *Buffer)
{ CDL_Header	*Header		= (CDL_Header *) Buffer;
  CDL_Component	*Component	= (CDL_Component *)
  				  &(Buffer[sizeof(CDL_Header)]);
  CDL_CStream	*CStreams;
  RmTaskforce	Taskforce	= (RmTaskforce) NULL;
  Pointers	pointers;
  int		i;
  bool		success		= FALSE;
  int		rc;

  CStreams	    = (CDL_CStream *) (Buffer + sizeof(CDL_Header) +
  		(Header->NoComponents * sizeof(CDL_Component)));
  pointers.IStreams = (CDL_IStream *) (((BYTE *) CStreams) +
  		(Header->NoCStreams * sizeof(CDL_CStream)));
  pointers.Attribs  = (CDL_Attribute *) (((BYTE *) pointers.IStreams) +
  		(Header->NoIStreams * sizeof(CDL_IStream)));
  pointers.Strings  = ((char *) pointers.Attribs) +
  		(Header->NoAttribs * sizeof(CDL_Attribute)) +
		sizeof(int);
  pointers.CurrentDirectory = Null(Object);

  pointers.Comms = (Comms *) Malloc(Header->NoCStreams * sizeof(Comms));
  if (pointers.Comms eq Null(Comms)) goto done;
  for (i = 0; i < Header->NoCStreams; i++)
   { pointers.Comms[i].Writer	= (RmTask) NULL;
     pointers.Comms[i].Reader	= (RmTask) NULL;
     if (CStreams[i].Flags & 0x80000000)	/* Indicates file */
      { pointers.Comms[i].FileName = &(pointers.Strings[CStreams[i].NameIndex]);
        if (CStreams[i].Flags & O_Create)
         pointers.Comms[i].OpenMode	= O_Create | O_Truncate;
        else
         pointers.Comms[i].OpenMode	= O_Append | O_Create;
      }      
     else
      { pointers.Comms[i].FileName	= Null(char);
        pointers.Comms[i].OpenMode	= 0;
      }
   }

  Taskforce = extract_header(Header, &pointers);
  if (Taskforce eq (RmTaskforce) NULL) goto done;

  for (i = 0; i < Header->NoComponents; i++, Component++)
   { RmTask	task = extract_component(Component, &pointers);
     if (task eq (RmTask) NULL) goto done;
     if (RmAddheadTask(Taskforce, task) ne task)
      { RmFreeTask(task); goto done; }
   }

  for (i = 0; i < Header->NoCStreams; i++)
   { Comms	*Comms = &(pointers.Comms[i]);
     if (Comms->FileName ne Null(char))
      { unless(handle_redirection(Comms, &pointers)) goto done; }
     else
      { if ((rc = RmMakeChannel(Comms->Writer, Comms->WriterChannel,
			 Comms->Reader, Comms->ReaderChannel))
	   ne RmE_Success)
	 goto done;
      }
   }

  success = TRUE;
  
done:
  if (pointers.CurrentDirectory ne Null(Object))
   Close(pointers.CurrentDirectory);
  if (pointers.Comms ne Null(Comms)) Free(pointers.Comms);
  if (!success)
   { if (Taskforce ne (RmTaskforce) NULL) RmFreeTaskforce(Taskforce);
     return((RmTaskforce) NULL);
   }
  return(Taskforce);
}
/*}}}*/
/*{{{  extract_header() */
static RmTaskforce	extract_header(CDL_Header *Header, Pointers *pointers)
{ RmTaskforce	Taskforce	= RmNewTaskforce();
  char		*Strings	= pointers->Strings;
  char		*DirectoryName;

  if (Taskforce eq (RmTaskforce) NULL) return((RmTaskforce) NULL);
  if (RmSetTaskforceId(Taskforce, objname(&(Strings[Header->TfNameIndex])))
      ne RmE_Success)
   { RmFreeTaskforce(Taskforce); return((RmTaskforce) NULL); }
  
  DirectoryName = &(Strings[Header->CurrentDirIndex]);
  pointers->CurrentDirectory = Locate(Null(Object), DirectoryName); 
  if (pointers->CurrentDirectory eq Null(Object))
   { RmFreeTaskforce(Taskforce); return((RmTaskforce) NULL); }
   
  return(Taskforce);
}
/*}}}*/
/*{{{  extract_component() */

static RmTask	extract_component(CDL_Component *Component, Pointers *pointers)
{ RmTask	task		= RmNewTask();
  bool		success		= FALSE;
  char		*Strings	= pointers->Strings;
  CDL_IStream	*IStreams	= pointers->IStreams;
      
  if (task eq (RmTask) NULL) goto done;

  if (Strings[Component->NameIndex] eq '/')
   { if (RmSetTaskCode(task, &(Strings[Component->NameIndex])) ne RmE_Success)
      goto done;
   }
  else
   { char *	 buf = (char *) Malloc(IOCDataMax);
     int	 rc;
     if (buf eq NULL) goto done;
     strcpy(buf, pointers->CurrentDirectory->Name);
     pathcat(buf, &(Strings[Component->NameIndex]));
     rc = RmSetTaskCode(task, buf);
     Free(buf);
     if (rc ne RmE_Success) goto done;
   }

	/* Skip the flags, set to non-zero iff binaries included, but	*/
	/* that option is no longer supported.				*/

	/* The CDL binary appears to always contain puid strings, but	*/
	/* they tend to be empty.					*/
  if (Component->PuidIndex >= 0)
   if (strlen(&(Strings[Component->PuidIndex])) > 0)
    { char	*puid = &(Strings[Component->PuidIndex]);
      char	*buf  = (char *) Malloc(IOCDataMax);
      if (buf eq Null(char)) goto done;

      strcpy(buf, "puid=");
      strcat(buf, puid);
      if (RmAddTaskAttribute(task, buf) ne RmE_Success)
       { Free(buf); goto done; }
      Free(buf);
   }
 
  { int		ptype	= RmT_Default;
    switch(Component->Ptype)
     { case	T212	: ptype = RmT_T212; break;
       case	T414	: ptype = RmT_T414; break;
       case	T800	: ptype = RmT_T800; break;
     }
    if (RmSetTaskType(task, ptype) ne RmE_Success) goto done;
  }

  if (Component->Memory ne 0)
   if (RmSetTaskMemory(task, Component->Memory) ne RmE_Success) goto done;

  if (Component->NumberAttribs > 0)
   { CDL_Attribute *attr	= pointers->Attribs;
     int	    i;
     char	*attrib;
     
     attr = &(attr[Component->AttribIndex]);
     for (i = 0; i < Component->NumberAttribs; i++, attr++)
      { attrib = &(Strings[attr->Index]);
        if (RmAddTaskAttribute(task, attrib) ne RmE_Success) goto done;
      }
   }

  { char	*arg	= objname(&(Strings[Component->ArgumentsIndex]));
    int		i;
    if (RmSetTaskId(task, arg) ne RmE_Success) goto done;
    if (Component->NumberArguments > 1)
     { for (i = 1; i < Component->NumberArguments; i++)
        { arg += (strlen(arg) + 1);
          if (RmAddTaskArgument(task, i, arg) ne RmE_Success) goto done;
        }
     }
  }

	/* Interpretation of the IStream structure...		*/
	/* The index field can be -1 to indicate a standard	*/
	/* stream that is inherited from the environment.	*/
	/* These can be ignored. Alternatively it is an index	*/
	/* into the CStream table. This may mean that the	*/
	/* channel should be connected to a named file, in	*/
	/* case the file name has been resolved already.	*/
	/* Otherwise it refers to a pipe between two components.*/
  { CDL_IStream	*stream = &(IStreams[Component->IStreamIndex]);
    int		i;
    int		index;
    Comms	*Comms = pointers->Comms;
    
    for (i = 0; i < Component->NoIStreams; i++, stream++)
     { if (stream->Index eq -1)
        continue;		/* This stream is inherited */
       index = (int) stream->Index;
       if (Comms[index].FileName ne Null(char))
        { 	/* mode = 80000001 for read-only */
        	/* 1 for write-only or append, depending on create bit */
          if (stream->Mode eq 0x80000001)
           Comms[index].OpenMode = O_ReadOnly;
          else
           Comms[index].OpenMode |= O_WriteOnly;
          Comms[index].Writer		= task;
          Comms[index].WriterChannel	= (int) stream->Standard;
        }
       else
        { if (stream->Mode eq O_ReadOnly)
           { Comms[index].Reader	= task;
             Comms[index].ReaderChannel	= (int) stream->Standard;
           }
          else
           { Comms[index].Writer	= task;
             Comms[index].WriterChannel	= (int) stream->Standard;
           }
        }
     }
  }

  success = TRUE;

done:
  if (!success)
   if (task ne (RmTask) NULL) RmFreeTask(task);
  return(task);
}

/*}}}*/
/*{{{  handle_redirection() */
static bool handle_redirection(Comms *Comms, Pointers *pointers)
{ char	*name = Comms->FileName;
  char	*buffer;
  int	length, rc;
        
	/* The name to be put into the Taskforce. This causes problems. */
	/* The name may be relative to the current directory, in which	*/
	/* case I turn it into an absolute name here. Otherwise the	*/
	/* name may be absolute. Either way the current CDL compiler	*/
	/* does not store any capabilities so in a protected mode 	*/
	/* it does not work anyway.					*/
  if (name[0] eq '/')
   length = strlen(name) + 1;
  else
   length = strlen(pointers->CurrentDirectory->Name) + strlen(name) + 2;

  buffer = (char *) Malloc(length);
  if (buffer eq Null(char)) return(FALSE);
  if (name[0] eq '/')
   strcpy(buffer, name);
  else
   { strcpy(buffer, pointers->CurrentDirectory->Name);
     pathcat(buffer, name);
   }

  rc = RmConnectChannelToFile(Comms->Writer, Comms->WriterChannel,
        			buffer, Comms->OpenMode);
  Free(buffer);
  return((rc eq RmE_Success) ? TRUE : FALSE);
}
/*}}}*/

