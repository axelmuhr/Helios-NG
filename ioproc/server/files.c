/*------------------------------------------------------------------------
--                                                                      --
--         H E L I O S   I N P U T / O U T P U T   S E R V E R          --
--         ---------------------------------------------------          --
--                                                                      --
--              Copyright (C) 1987, Perihelion Software Ltd.            --
--                         All Rights Reserved.                         --
--                                                                      --
--  Files.c                                                             --
--                                                                      --
--  Author:  BLV 30/11/87                                               --
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Id: files.c,v 1.13 1994/06/29 13:42:25 tony Exp $ */
/* Copyright (C) 1987, Perihelion Software Ltd.       			*/

/**
*** This module contains the code which allows the transputer to access
*** the IO processor's files and directories. Most of the routines in this
*** module are called indirectly via the arrays of request handlers
*** described in module server.c, i.e. from routines General_Server() and
*** General_Stream().
**/

#define Files_Module
#include "helios.h"         /* Usual include file, see module server.c */
#if MSWINDOWS
#include "windows\menus.h"
#endif

#include <stdlib.h>

/**
*** To improve portability, a number of macros/functions are used to perform
*** certain operations on files. These vary from machine to machine. The
*** macros/functions are as follows:
***
*** stream - a data type corresponding to an open file, e.g. int or FILE *
***
*** get_local_name() - this converts the Helios name IOname,
*** e.g. c/helios/bin/ls, into a local name such as c:\helios\bin\ls,
*** storing the result in the static local_name.
***
*** itsadrive(name) - this checks whether the requests affects the entire
*** drive, which is usually illegal. It is only required if drives_are_special
*** is defined in your version of the header file defines.h
***
*** object_exists(name) - this checks that the object actually exists. It may
*** have useful side effects.
***
*** object_isadirectory(name) - this is only called immediately after a call
*** to object_exists(), so usually it does not involve another disk access
***
*** create_directory(name) - create the named directory
***
*** create_file(name) - create the file or truncate an existing file to zero
*** length
***
*** get_file_info(name, *ObjInfo) - fill in a Helios object info structure for
*** the named file or directory. This is only called immediately after a call
*** to object_exists() so the information may be stored somewhere already.
***
*** delete_directory(name) - obvious
***
*** delete_file(name) - obvious
***
*** rename_object(from, to) - rename an object. Both from and to are absolute
*** local file names. Exactly what this routine can do various from system to
*** system. At the very least it must allow renaming of a file, overwriting
*** the destination file if it exists already.
***
*** set_file_date(name, stamp) - this is used to set the date of last access
*** of the named file. The timestamp is a unix time stamp, seconds since 1970.
***
*** get_drive_info(name, *servinfo) - this is used to obtain information about
*** the current usage of a drive. The drive is identified by the name, and the
*** information must be put in the servinfo buffer. For more details see
*** Drive_ServerInfo() below.
***
*** All the above return true or false.
***
*** The following are for streams rather than drives:
***
*** stream open_file(name, Heliosmode) - returns an object of type stream,
*** or 0 to indicate failure. The file is always opened in binary mode.
***
*** close_file(stream) - returns true or false, almost certainly true
***
*** word get_file_size(stream, currentpos) - returns the size of the open file,
*** or -1 to indicate failure, and leaves the current file position at
*** currentpos.
***
*** write_to_file(stream, amount, buffer) - write the specified amount of data
*** to the file, returns true if completely successful, false otherwise
***
*** word read_from_file(stream, amount, buffer) - read the specified amount
*** of data from the file. It returns the amount of data actually read, which
*** should be less than the amount asked for if and only if end-of-file is
*** reached, or a negative number to indicate an error.
***
*** word seek_in_file(stream, offset, mode) - seek to the specified location in
*** the file. mode is 0 if seeking from the beginning of the file, 1 if seeking
*** from the end (in which case the offset is -ve or zero). This should return
*** the new position in the file, NB not the old one, or a negative number to
*** indicate failure.
***
*** word search_directory(name, List *) - search through the named directory,
*** putting a directory entry for each file and subdirectory in the linked
*** list provided. It returns the number of entries in the directory or a
*** negative number to indicate failure.
**/

#if ST
typedef int stream;

#define isadrive(name) ((strlen(name) eq drive_len))

#define object_exists(name) \
  ((Fsfirst(name, search_FileOrDir) eq 0L) ? true : false )

#define object_isadirectory(name) \
  (((searchbuffer.attr & FileAttr_Dir) eq FileAttr_Dir) ? true : false)

#define create_directory(name) \
  ((Dcreate(name) eq 0L) ? true : false)

#define delete_directory(name) \
  ((Ddelete(name) eq 0L) ? true : false)

#define delete_file(name)      \
  ((Fdelete(name) eq 0L) ? true : false)

extern word   create_file(), get_file_info(), rename_object(), set_file_date();
extern word   get_drive_info();
extern stream open_file();
extern word   get_file_size();

#define close_file(stream) ((Fclose(stream) eq 0L) ? true : false)

#define read_from_file(stream, amount, buffer) Fread(stream, amount, buffer) 

#define write_to_file(stream, amount, buffer) \
     ( (Fwrite(stream, amount, buffer) < amount) ? false : true )

#define seek_in_file(stream, offset, mode) \
 (  (mode eq 0) ? Fseek(offset, stream, 0) : \
                  Fseek(offset, stream, 2)  )

extern word search_directory();

#endif  /* ST */

#if PC

/**
*** Macros are defined here. Functions have been put into pclocal.h,
*** because they must be prototyped.
**/
typedef int stream;

#define isadrive(name) ((strlen(name) eq drive_len))

#define object_exists(name) \
  ((_dos_findfirst(name, search_FileOrDir, \
                    (struct find_t *) &searchbuffer) eq 0L) ? true : false )

#define object_isadirectory(name) \
  (((searchbuffer.attr & FileAttr_Dir) eq FileAttr_Dir) ? true : false)

# define create_directory(name) \
  ((_mkdir(name) eq 0L) ? true : false)

#define delete_directory(name) \
  ((_rmdir(name) eq 0L) ? true : false)

#define delete_file(name)      \
  ((remove(name) eq 0L) ? true : false)

#define close_file(stream) ((_dos_close(stream) eq 0) ? true : false)

#define seek_in_file(handle, offset, mode) \
 (  (mode eq 0) ? _lseek(handle, offset, SEEK_SET) : \
                  _lseek(handle, offset, SEEK_END)   )

PRIVATE void fn(get_local_name, (void));

#endif  /* PC */

#if     AMIGA
#define isadrive(name) \
        IsADrive(name)

#define object_exists(name) \
        (FindFile(name) ? true : false )

#define object_isadirectory(name) \
        (IsDir(name) ? true : false )

#define create_directory(name) \
        (DirCreate(name) ? true : false)

#define delete_directory(name) \
        (DirDelete(name) ? true : false)

#define delete_file(name)      \
        (FileDelete(name) ? true : false)

extern word   create_file(), get_file_info(), rename_object(), set_file_date();
extern word   get_drive_info();
extern stream open_file();
extern word   get_file_size();

#define close_file(stream) \
        (FileClose(stream) ? true : false)

#define read_from_file(stream, amount, buffer) \
        FileRead(stream, amount, buffer)

#define write_to_file(stream, amount, buffer) \
        FileWrite(stream, amount, buffer)
        
#define seek_in_file(stream, offset, mode) \
        FileSeek(offset,stream,mode)

extern word search_directory();

#endif /* AMIGA */

#if (UNIX || HELIOS)

/**
*** On a unix box I want all the file I/O to go via local routines so that
*** they can convert the error code in errno to a Helios error code in
*** Server_errno. The overhead should be fairly minimal.
**/

typedef int stream;

/* no def for isadrive, not needed */

struct stat searchbuffer;

extern word   fn( object_exists,    (char *));
extern word   fn( create_directory, (char *));
extern word   fn( delete_directory, (char *));
extern word   fn( delete_file,      (char *));
extern word   fn( rename_object,    (char *, char *));
extern word   fn( create_file,      (char *));
extern word   fn( get_file_info,    (char *, ObjInfo *));
extern word   fn( set_file_date,    (char *, word));
extern word   fn( get_drive_info,   (char *, servinfo *));
extern stream fn( open_file,        (char *, word));
extern word   fn( get_file_size,    (stream, word));
extern word   fn( close_file,       (stream));
extern word   fn( read_from_file,   (stream, word, byte *));
extern word   fn( write_to_file,    (stream, word, byte *));
extern word   fn( seek_in_file,     (stream, word, word));
extern word   fn( search_directory, (char *, List *));

#define object_isadirectory(name) \
  ((( (int) searchbuffer.st_mode & S_IFDIR) !=0) ? true : false)

#endif
#if MAC

/**
*** On a mac box I want all the file I/O to go via local routines so that
*** they can convert the error code in errno to a Helios error code in
*** Server_errno. The overhead should be fairly minimal.
**/

typedef int stream;


struct stat searchbuffer;
extern word   fn( isadrive,         (char *));
extern word   fn( object_exists,    (char *));
extern word   fn( create_directory, (char *));
extern word   fn( delete_directory, (char *));
extern word   fn( delete_file,      (char *));
extern word   fn( rename_object,    (char *, char *));
extern word   fn( create_file,      (char *));
extern word   fn( get_file_info,    (char *, ObjInfo *));
extern word   fn( set_file_date,    (char *, word));
extern word   fn( get_drive_info,   (char *, servinfo *));
extern stream fn( open_file,        (char *, word));
extern word   fn( get_file_size,    (stream, word));
extern word   fn( close_file,       (stream));
extern word   fn( read_from_file,   (stream, word, byte *));
extern word   fn( write_to_file,    (stream, word, byte *));
extern word   fn( seek_in_file,     (stream, word, word));
extern word   fn( search_directory, (char *, List *));
extern word   fn(object_isadirectory,(char *));

#endif

/**
*** The following bits deal with name conversion. On entry to any server routine
*** the global array IOname contains a Helios name such as c/helios/bin/ls,
*** which must be converted into whatever the local filing system expects.
*** This name conversion routine is done by get_local_name, which varies from
*** machine to machine.
***
*** Important : this name conversion is responsible for mapping the Helios
*** pseudo drive onto a real directory.
**/

PRIVATE char local_name[IOCDataMax];
PRIVATE word	msdos_flag = 0L;

#if !(drives_are_special)

#if (UNIX || HELIOS)
PRIVATE char *root = "/";
PRIVATE char *rubbish_file = "/nothing/garbage";
#endif

#if TRIPOS
PRIVATE char *root = "SYS:"         /* alloc access to sys: only initially */
PRIVATE char *rubbish_file = "BLV:kjdfhgsk";
#endif

#if AMIGA
PRIVATE char *root = "HELIOS:";
PRIVATE char *rubbish_file = "SER:kjdfhgsk";
#endif

PRIVATE void get_local_name()
{ register char *tempptr = IOname;
  register char *destptr;

  for ( ; *tempptr ne '/' && *tempptr ne '\0'; tempptr++);

  if (*tempptr eq '\0')
    { if (!strcmp(IOname, "helios"))
        strcpy(local_name, Heliosdir);
      elif (!strcmp(IOname, "files"))
        strcpy(local_name, root);
      else
        strcpy(local_name, rubbish_file);
      return;
    }
  else
    { *tempptr = '\0';
      if (!strcmp(IOname, "helios"))
        { 
#if (UNIX && !MEIKORTE && !UNIX386 && !IUNIX386)
          check_helios_name(&(tempptr[1]));  /* this checks for multi-user net_serv etc */
#endif
          strcpy(local_name, Heliosdir); 
          *tempptr = '/';
        }
      elif (!strcmp(IOname, "files"))
        { strcpy(local_name, root); *tempptr++ = '/'; }
      else
        { strcpy(local_name, rubbish_file); return; }
    }

  destptr = local_name + strlen(local_name);
  for (; *tempptr ne '\0'; )
    *destptr++ = *tempptr++;

  *destptr = '\0';
#if (ETC_DIR || HELIOS)
  check_helios_name(local_name);
#endif
}

#endif  /* drives not special */

#if (PC || ST)
PRIVATE void get_local_name()
{ register unsigned char *tempptr = IOname;
  register unsigned char *destptr;

  for ( ; *tempptr ne '/' && *tempptr ne '\0'; tempptr++);

  if (*tempptr eq '\0')
    { if (!strcmp(IOname, "helios"))
        strcpy(local_name, Heliosdir);
      else
        { strcpy(local_name, IOname);
          strcat(local_name, ":");
        }
    }
  else
    { *tempptr = '\0';
      if (!strcmp(IOname, "helios"))
        strcpy(local_name, Heliosdir);
      else
        { strcpy(local_name, IOname);
          strcat(local_name, ":");
        }
      *tempptr++ = '/';
    }

  destptr = local_name + strlen(local_name);
  for (; *tempptr ne '\0'; )
   { *destptr++ = '\\';
     for (; *tempptr ne '\0' && *tempptr ne '/'; )
       *destptr++ = *tempptr++;
     if (*tempptr eq '/') tempptr++;
   }

  *destptr = '\0';

  for (tempptr = local_name, destptr = local_name; *tempptr ne '\0';
       tempptr++ )
   if (isalnum(*tempptr))
    *destptr++ = *tempptr;
   else if((*tempptr eq '.') && (*(destptr-1) eq '\\'))
    continue;
   else if((*tempptr eq ':') || (*tempptr eq '\\') || (*tempptr eq '&') || 
           (*tempptr eq '.') || (*tempptr eq '$') || (*tempptr eq '%') ||
           (*tempptr eq '\'') || (*tempptr eq '_') || (*tempptr eq '@') ||
           (*tempptr eq '{') || (*tempptr eq '}') || (*tempptr eq '~') ||
           (*tempptr eq 0x60) || (*tempptr eq '!') || (*tempptr eq '#') ||
           (*tempptr eq '-') || (*tempptr >= 0x0080) )
        *destptr++ = *tempptr;

  *destptr = '\0';
 
  tempptr--;
  if (*tempptr eq '\\') *tempptr = '\0';  /* Strip trailing / */

  for ( ; (tempptr > local_name) && (*tempptr ne '\\'); tempptr--)
   if (*tempptr eq '.')  /* and allow only three characters after . */
    { tempptr++; tempptr++; tempptr++; tempptr++;
      *tempptr = '\0';
      break;
    }
#if (PC)
  check_helios_name(local_name);
#endif
}

#endif

#if MAC
PRIVATE void get_local_name()
{ register unsigned char *tempptr = IOname;

  register unsigned char *destptr;

  for ( ; *tempptr ne '/' && *tempptr ne '\0'; tempptr++);
  /* '/' at end of IOname, if helios map to heliosdir */

  if (*tempptr eq '\0')
    { if (!strcmp(IOname, "helios"))
        strcpy(local_name, Heliosdir);
      else
        { strcpy(local_name, IOname);
          strcat(local_name, ":");
        }
    }
  else
    { *tempptr = '\0';
      if (!strcmp(IOname, "helios"))
        strcpy(local_name, Heliosdir);
      else
        { strcpy(local_name, IOname);
        }
      *tempptr++ = '/';
    }
	/* replace all "/"'s by ":" */
  destptr = local_name + strlen(local_name);
  for (; *tempptr ne '\0'; )
   { *destptr++ = ':';
     for (; *tempptr ne '\0' && *tempptr ne '/'; )
       *destptr++ = *tempptr++;
     if (*tempptr eq '/') tempptr++;
   }

  *destptr = '\0';
}

#endif

#if AMIGA
PRIVATE char *root = "HELIOS:";

PRIVATE void get_local_name()
{ register char *tempptr = IOname;
  register char *destptr;

  for ( ; *tempptr ne '/' && *tempptr ne '\0'; tempptr++);
  /* '/' at end of IOname, if helios map to heliosdir */
  if (*tempptr eq '\0') {
    if (!strcmp(IOname, "helios"))
      strcpy(local_name, Heliosdir);
    else {
        strcpy(local_name, IOname);
        strcat(local_name, ":");
    }
  }
  else {
    *tempptr = '\0';
    if (!strcmp(IOname, "helios"))
      strcpy(local_name, Heliosdir);
    else {
        strcpy(local_name, IOname);
        strcat(local_name, ":");
    }
    *tempptr++ = '/';
  }
  /* add rest of name to my local name */
  destptr = local_name + strlen(local_name);
  for (; *tempptr ne '\0'; )
    *destptr++ = *tempptr++;

  *destptr = '\0';
  return ;
}
#endif


/**
*** This is temporary code to handle floppy disk errors
**/
#if floppies_available
void fn(floppy_handler, (void));

void floppy_handler()
{ if (floppy_errno eq 0)
    return;
  elif (floppy_errno eq floppy_invalid)
    output("***SERVER - Error accessing floppy disk drive.\r\n");
  elif (floppy_errno eq floppy_protected)
    output("***SERVER - The floppy disk is write-protected.\r\n");
  elif (floppy_errno eq floppy_removed)
    output("***SERVER - Please put the floppy disk back into the drive.\r\n");
  elif (floppy_errno eq floppy_full)
    output("***SERVER - The floppy disk is full.\r\n");

  floppy_errno = 0;
}
#endif

/**
*** This is used to remember the parent i.e. drive coroutine when opening
*** a file or directory.
**/
PRIVATE Conode *parentco;

/*----------------------------------------------------------------------------
--
-- Initialise the Helios drive server
--
----------------------------------------------------------------------------*/

/**
*** The "Helios" drive is the only drive server that needs initialisation.
*** This routine is responsible for initialising Heliosnode, the coroutine list
*** node used to keep the WaitingCo linked list in a sensible order. In
*** addition it inialises the list Open_files_list, used to keep track of all
*** open files.
**/

PRIVATE List Open_files_list;

/*
** initialise_files - called from main()
** sets up Open_files_list even if we have no_helios option
*/

void initialise_files()
{
  InitList(&Open_files_list);
#if (PC || ST)
  if (get_config("Unix_fileio") eq (char *) NULL)
   msdos_flag = Flags_MSdos;
#endif
}

void Helios_InitServer(myco)
Conode *myco;
{ 
  Heliosnode = (Node *) myco;
#if (PC || ST)
  if (get_config("Unix_fileio") eq (char *) NULL)
   msdos_flag = Flags_MSdos;
#endif
}

/*----------------------------------------------------------------------------
--
-- Locate a file or directory
--
----------------------------------------------------------------------------*/

/**
*** This bit of code handles Locate requests sent to a drive server. There
*** are separate servers for all the drives, i.e. one for each floppy and one
*** for each hard disk partition.
***
*** If the object I am supposed to locate is just the drive then I can
*** succeed straight away. The drive must exist or this server would not
*** exist. Otherwise I must check that the object exists, by a search call,
*** and what type it is.
***
*** Once I know that the object exists and what type it is I can fill in
*** the message area with all the necessary information, by a call to
*** form_open_reply(), and return the message. At this stage I can return
*** safely, and the server coroutine will be suspended again inside
*** General_Server() ready for the next request.
***
*** In Helios, the same reply format is used for Locate, Open and Create
*** requests, so having a single routine, form_open_reply(), to fill in
*** all the bits is a rather good idea.
**/

void Drive_Locate(myco)
Conode *myco;
{ word temp;

  get_local_name();

#if drives_are_special
  if (isadrive(local_name))                /* allow for drive identifier only */
    { temp = FormOpenReply(Type_Directory, 0L, -1L, -1L);
      Request_Return(ReplyOK, open_reply, temp);
      return;
    }
#endif

  Server_errno = EC_Error + SS_IOProc + EG_Unknown + EO_File;

                                        /* check whether or not file exists */
  unless(object_exists(local_name))
    {
#if floppies_available
      if (floppy_errno) floppy_handler();
#endif
      Request_Return(Server_errno, 0L, 0L);
      return;
    }

                                        /* it exists, dir or file ? */
  if (object_isadirectory(local_name))
    temp = FormOpenReply(Type_Directory, 0l, -1l, -1l);
  else
    temp = FormOpenReply(Type_File, 0l, -1l, -1l);

  Request_Return(ReplyOK, open_reply, temp);

use(myco)
}

/*----------------------------------------------------------------------------
--
--  Open a file or directory
--
----------------------------------------------------------------------------*/

/**
*** Under Helios, the same mechanism is used to open files and directories,
*** and directories can be read in the same way as files. First of all,
*** Open'ing files is a rather important event so I provide a debugging
*** option to monitor it.
***
*** Next, once again I have to treat drives as a special case. If the
*** object is a drive I want to Open it as a directory, so I have to
*** create a new coroutine for the corresponding Helios Stream. This
*** Stream is a General_Stream() with its own identifier/message port,
*** and set up so that it handles directory Streams. The new Stream is
*** started up, will initialise itself, and will return a suitable reply
*** to the Open request. Eventually the Stream will suspend itself to wait
*** for a message from the transputer, and control returns here. Drive_Open()
*** returns to General_Server(), and the server coroutine suspends itself
*** to wait for its next message from the transputer.
***
*** If it is not a drive, I do a search to check whether or not the object
*** exists. There are three possibilities. It may be a directory, in
*** which case I create a directory Stream as for the drive above. It may
*** be a file, in which case I create a file Stream similar to the above
*** but with a different set of handler routines. Or it may not exist at
*** all. There is a facility in the Open protocol for creating a file if
*** it does not yet exist, by setting a bit in the OpenMode.
***
*** To create the new stream coroutine there is a general-purpose NewStream()
*** utility in module server.c. The Helios open mode is stored in the
*** extra field of the coroutine node, because the mcb gets zapped by
*** NewStream.
**/

void Drive_Open(myco)
Conode *myco;
{ word itsadirectory;
  word openmode = (mcb->Control)[OpenMode_off];

  get_local_name();

  Debug(Open_Flag, ("Supposed to open %s", local_name));

#if drives_are_special
  if (isadrive(local_name))                          /* drive identifier only */
    itsadirectory = true;
  else
#endif
   {
     itsadirectory = false;     /* default is file */

     Server_errno = EC_Error + SS_IOProc + EG_Unknown + EO_File;

     if (object_exists(local_name))
      { if ((openmode & O_Create) && (openmode & O_Exclusive))
         { Request_Return(EC_Error + SS_IOProc + EG_InUse + EO_Name, 0L, 0L);
           return;
         }
        if (object_isadirectory(local_name))
          itsadirectory = true;   /* opening existing directory */
      }
     elif ((openmode & O_Create) eq 0L)
       { Debug(Open_Flag, ("File not found"));
         Request_Return(Server_errno, 0L, 0L);
         return;
       }
     }

  parentco = myco;

  if (itsadirectory)
    NewStream(Type_Directory, Flags_Closeable, openmode,
              Dir_Handlers);
  else
    NewStream(Type_File, Flags_Closeable | msdos_flag,
              openmode, File_Handlers);

use(myco)
}

/*----------------------------------------------------------------------------
--
-- Create a file or directory.
--
----------------------------------------------------------------------------*/

/**
*** This code deals with creating files and directories. Helios Create does
*** not leave an open stream, so I do not have to worry about creating
*** new Stream coroutines or anything like that. Create can operate on
*** existing files, truncating them to zero length, but it does not
*** operate on existing directories for rather obvious reasons.
***
*** Again I have to treat drives as a special case, but they are an easy
*** special case since there is no way of creating new drives.
*** Then I have to check whether I am supposed to create a file or a
*** directory, by checking one of the arguments in the message, and I
*** call a suitable routine to handle whichever case it is. Formatting
*** floppies is also done via Create, but it is only supported on the ST
*** at the moment.
**/

#if floppies_available
#define format_mask     0xFFFFFF00L
#define format_magic    0x659A2B00L
#define format_single   0x00000080L
#define format_double   0x00000000L
#define format_40       0x00000040L
#define format_80       0x00000000L
PRIVATE void fn( Drive_Format, (word));
#endif

PRIVATE void fn( Drive_Createdir, (void));
PRIVATE void fn( Drive_Createfile, (void));

void Drive_Create(myco)
Conode *myco;
{ word type = (mcb->Control)[CreateType_off];

  get_local_name();

#if floppies_available
  if ((type & format_mask) eq format_magic)
    { Drive_Format(type);
      return;
    }
#endif

#if drives_are_special
  if (isadrive(local_name))    /* creation of drives is only valid for Format */
    { Request_Return(EC_Error + SS_IOProc + EG_WrongFn + EO_Server, 0L, 0L);
      return;
    }
#endif

  if ((type & 0x0F) eq Type_Stream)
    Drive_Createfile();
  elif ((type & 0x0F) eq Type_Directory)
    Drive_Createdir();
  else
    Request_Return(EC_Error + SS_IOProc + EG_Parameter + EO_Message, 0L, 0L);
use(myco)
}

PRIVATE void Drive_Createfile()
{ word temp;

  Server_errno = EC_Error + SS_IOProc + EG_Congested + EO_Server;

  Debug( FileIO_Flag, ("Creating/truncating file %s", local_name));

#ifdef DEMONSTRATION
  ServerDebug("*** Helios demonstration version - the disk is write-protected");
  Request_Return(EC_Error + SS_IOProc + EG_Protected + EO_Server, 0L, 0L);
  return;

#else
  unless(create_file(local_name))
    {
#if floppies_available
      if (floppy_errno) floppy_handler();
      Request_Return(Server_errno, 0L, 0L);
      return;
#endif
    }

  temp = FormOpenReply(Type_File, 0l, -1l, -1l);
  Request_Return(ReplyOK, open_reply, temp);
#endif
}


PRIVATE void Drive_Createdir()
{ word temp;

  Server_errno = EC_Error + SS_IOProc + EG_Congested + EO_Server;

  Debug(FileIO_Flag, ("Creating directory %s", local_name));

#ifdef DEMONSTRATION
  ServerDebug("*** Helios demonstration version - the disk is write-protected");
  Request_Return(EC_Error + SS_IOProc + EG_Protected + EO_Server, 0L, 0L);
  return;
#else
  if (!create_directory(local_name))           /* try to create the directory */
   {
#if floppies_available
     if (floppy_errno) floppy_handler();
#endif
     Request_Return(Server_errno, 0L, 0L);  /* failed */
     return;
   }

  temp = FormOpenReply(Type_Directory, 0l, -1l, -1l);
  Request_Return(ReplyOK, open_reply, temp);
#endif
}

#if floppies_available
/**
*** Formatting floppies is done by 16 successive Create messages, and the
*** actual work is done by a local routine format_floppy() which you have
*** to provide. All of this is likely to be generalised at some future
*** stage, to allow you to format hard disks etc from Helios.
**/

PRIVATE void Drive_Format(type)
word type;
{ word tracks = (type & format_40) ? 40L : 80L;
  word sides  = (type & format_single) ? 1L : 2L;
  word count  = type & 0x0FL, temp;
  word size   = mcb->Control[CreateSize_off];
  char *label = (char *) &(misc_buffer1[0]);

  Debug(FileIO_Flag, ("Floppy disk format requested : stage %ld", count));

  if (size > 0L)
    strncpy(label, &(mcb->Data[(int)mcb->Control[CreateInfo_off]]),
          (int) size);
  else
    label[0] = '\0';

  Server_errno = EC_Error + SS_IOProc + EG_Broken + EO_Server;

  temp = format_floppy(local_name, count, sides, tracks, label);
  if (floppy_errno) floppy_handler();

  if (!temp)
     Request_Return(Server_errno, 0L, 0L);
  else
   { temp = FormOpenReply(Type_Directory, 0l, -1l, -1l);
     Request_Return(ReplyOK, open_reply, temp);
   }
}
#endif

/*----------------------------------------------------------------------------
--
-- Delete a file or directory
--
----------------------------------------------------------------------------*/

/**
*** This code deals with deleting files and directories. There is the usual
*** for the special case of a drive, and drives are as impossible to delete
*** as they are to create. I have to check that the object to be deleted
*** exists at all, and if it does exist whether it is a file or a directory.
*** If an object exists but I cannot delete it I start to get worried.
**/

void Drive_Delete(myco)
Conode *myco;
{ 
  if (!strcmp(IOname, "helios"))
   { myco->name[0] = '\0';
     Request_Return(ReplyOK, 0L, 0L);
     return;
   }

  get_local_name();

#if drives_are_special
  if (isadrive(local_name))              /* getting rid of a drive ? */
    { Request_Return(EC_Error + SS_IOProc + EG_WrongFn + EO_Server, 0L, 0L);
      return;
    }
#endif

  Server_errno = EC_Error + SS_IOProc + EG_Unknown + EO_File;

  if (!object_exists(local_name))
    {
      Debug(Delete_Flag,("Delete request for unknown object %s", local_name));

#if floppies_available
      if (floppy_errno) floppy_handler();
#endif
      Request_Return(Server_errno, 0L, 0L);
      return;
    }

  if (object_isadirectory(local_name))
    { int result;

      Server_errno = EC_Error + SS_IOProc + EG_Delete + EO_Directory;

      Debug(Delete_Flag, ("Deleting directory %s", local_name));

      result = (int) delete_directory(local_name);

#if ST || PC
      unless(result) /* first time always seems to fail, this one works */ 
        result = (int) delete_directory(local_name);
#endif
      if (result)
       Request_Return(ReplyOK, 0L, 0L);  /* success */
      else      /* most likely reason for failure is directory not empty */
       {
#if floppies_available
        if (floppy_errno) floppy_handler();
#endif
        Request_Return(Server_errno, 0L, 0L);
       }
    }
  else
    { Server_errno = EC_Error + SS_IOProc + EG_Delete + EO_File;

      Debug(Delete_Flag, ("Deleting file %s", local_name));

      if (delete_file(local_name))        /* must be a file */
        Request_Return(ReplyOK, 0L, 0L);
      else
       {
#if floppies_available
        if (floppy_errno) floppy_handler();
#endif
        Request_Return(Server_errno, 0L, 0L);
       }
    }

use(myco)
}


/*----------------------------------------------------------------------------
--
-- ObjectInfo on a file or directory
--
----------------------------------------------------------------------------*/

/**
*** This routine handles ObjectInfo requests, which are used to obtain extra
*** information about particular files or directories. The ObjInfo structure
*** is declared in header file iosyslib.h. All the information that I am
*** going to send back can be put straight into the message area because I
*** have extracted all the information I need already, namely the name of the
*** the object. 
***
*** I check for the special case of a drive. If it is a drive I can fill in all
*** the information in the ObjInfo structure straight away, and send the
*** reply. Note that all the integer values are swapped(). These values are
*** passed in the data area so the message primitives are not going to swap
*** them around to the transputer's byte ordering automatically. On the
*** PC swap() is hash-defined to be a no-op.
**/

void Drive_ObjectInfo(myco)
Conode *myco;
{                      /* put info in data vector straightaway, nothing in */
                       /* there that we need any more */
  register ObjInfo *Heliosinfo = (ObjInfo *) mcb->Data;

  if (!strcmp(IOname, "helios"))    /* Check for an ObjInfo on /helios */
    { Heliosinfo->DirEntry.Type     = swap(Type_Directory);
      Heliosinfo->DirEntry.Flags    = swap(0L);
      Heliosinfo->DirEntry.Matrix   = swap(DefDirMatrix);
      Heliosinfo->Size              = swap(0L);
      Heliosinfo->Account           = swap(0L);
      Heliosinfo->Creation          = swap(0L);
      Heliosinfo->Access            = swap(0L);
      Heliosinfo->Modified          = swap(0L);
      strcpy(&(Heliosinfo->DirEntry.Name[0]), "helios");
      Request_Return(ReplyOK, 0L, (word) sizeof(ObjInfo));
      return;
    }
    
  get_local_name();

  Debug(FileIO_Flag, ("ObjectInfo request for %s", local_name));

#if drives_are_special
                            /* treat drives specially */
  if (isadrive(local_name))
    { Heliosinfo->DirEntry.Type     = swap(Type_Directory);
      Heliosinfo->DirEntry.Flags    = swap(0L);
      Heliosinfo->DirEntry.Matrix   = swap(DefDirMatrix);
      Heliosinfo->Size              = swap(0L);
      Heliosinfo->Account           = swap(0L);
      Heliosinfo->Creation          = swap(0L);
      Heliosinfo->Access            = swap(0L);
      Heliosinfo->Modified          = swap(0L);
      Heliosinfo->DirEntry.Name[0]  = local_name[0];
      Heliosinfo->DirEntry.Name[1]  = '\0';
      Request_Return(ReplyOK, 0L, (word) sizeof(ObjInfo));
      return;
    }
#endif

  Server_errno = EC_Error + SS_IOProc + EG_Unknown + EO_File;

                            /* object must be a file or directory, get the */
                            /* required info checking that it exists */
  unless(object_exists(local_name))
    {
#if floppies_available
      if (floppy_errno) floppy_handler();
#endif
      Debug (FileIO_Flag, ("object %s does not exist locally", local_name));
      Request_Return(Server_errno, 0L, 0L);
      return;
    }

  Server_errno = EC_Error + SS_IOProc + EG_Broken + EO_Directory;

  unless(get_file_info(local_name, Heliosinfo))
    {
#if floppies_available
      if (floppy_errno) floppy_handler();
#endif 
      Debug (FileIO_Flag, ("failed to get file info for %s", local_name));
      Request_Return(Server_errno, 0L, 0L);
      return;
    }

  Request_Return(ReplyOK, 0L, (word) sizeof(ObjInfo));
use(myco)
}



/*----------------------------------------------------------------------------
--
-- Renaming a file or directory
--
----------------------------------------------------------------------------*/

/**
*** This routine is an attempt to implement the Rename request, which is well
*** known to be rather tricky. I do a check for renaming drives, because this
*** is not a very sensible thing to do.
***
*** Getting hold of the destination name is extremely tricky. The full network
*** name of the destination can be obtained by concatenating bits of the data
*** vector at the right offsets and flattening out . and .. bits. However,
*** finding out which bit of the network name belongs here is rather difficult,
*** particularly since there is no guarantee that the full name corresponds
*** to the current network node name. I get around it by doing the same
*** concatenation for the source name, and working out the offset in the
*** source name where the local name begins. Then I can use this offset for
*** the concatenation of the destination name, giving me the local destination
*** assuming not too much has gone wrong. This is converted into a machine
*** dependent name, and now I can do a machine-dependant rename.
**/

void Drive_Rename(myco)
Conode *myco;
{ char *fromname = (char *) &(misc_buffer2[0]);       /* to store source name */
  char *newname  = (char *) &(misc_buffer1[0]);
  char *data = mcb->Data;
  int  context = (int) mcb->Control[Context_off];
  int  name    = (int) mcb->Control[Pathname_off];
  int  dest    = (int) mcb->Control[RenameToname_off];
  int  offset;

  Debug(FileIO_Flag, ("Rename request received"));

  get_local_name();

#if drives_are_special
  if (isadrive(local_name))            /* renaming drives is illegal */
    { Request_Return(EC_Error + SS_IOProc + EG_WrongFn + EO_Server, 0L, 0L);
      return;
    }
#endif

  Server_errno = EC_Error + SS_IOProc + EG_Unknown + EO_File;

                 /* check that the source file exists */
  unless(object_exists(local_name))
   {
#if floppies_available
     if (floppy_errno) floppy_handler();
#endif
     Request_Return(Server_errno, 0L, 0L);
     return;
   }

  strcpy(fromname, local_name);               /* source name is in local_name */

  if (context ne -1 && data[name] ne '/')
   { 
     strcpy(newname, &(data[context]));
     pathcat(newname, &(data[name]));
   }
  else
   strcpy(newname, &(data[name]));

  unless(flatten(newname))
   { Request_Return(EC_Error + SS_IOProc + EG_Name + EO_Message, 0L, 0L);
     return;
   }

  offset = strlen(newname) - strlen(IOname);

  if (context ne -1 && data[dest] ne '/')
   { 
     strcpy(newname, &(data[context]));
     pathcat(newname, &(data[dest]));
   }
  else
   strcpy(newname, &(data[dest]));

  unless(flatten(newname))
   { Request_Return(EC_Error + SS_IOProc + EG_Name + EO_Message, 0L, 0L);
     return;
   }

  data = newname + offset;
  strcpy(IOname, data);
  get_local_name();

  Server_errno = EC_Error + SS_IOProc + EG_WrongFn + EO_Directory;

  Debug(FileIO_Flag, ("Renaming %s to %s", fromname, local_name));

  if (rename_object(fromname, local_name))
    Request_Return(ReplyOK, 0L, 0L);
  else
   {
#if floppies_available
     if (floppy_errno) floppy_handler();
#endif
     Request_Return(Server_errno, 0L, 0L);
   }

use(myco)
}

/*------------------------------------------------------------------------------
--
-- Setdate()
--
------------------------------------------------------------------------------*/

/**
*** The main purpose of Setdate() is to implement the TOUCH command. 
**/

void Drive_SetDate(myco)
Conode *myco;
{ word unixstamp = (mcb->Control)[SetDateDate_off];

  get_local_name();

  Debug(FileIO_Flag, ("SetDate request for %s", local_name));

#if drives_are_special
                            /* now check that it exists and that it is a file */
  if (isadrive(local_name))                      /* cannot set date on drives */
    { Request_Return(ReplyOK, 0L, 0L); return; }
#endif

  Server_errno = EC_Error + SS_IOProc + EG_Unknown + EO_File;

  unless(object_exists(local_name))
    {
#if floppies_available
      if (floppy_errno) floppy_handler();
#endif
      Request_Return(Server_errno, 0L, 0L);
      return;
    }

  if (object_isadirectory(local_name))
    { Request_Return(ReplyOK, 0L, 0L); return; }

  Server_errno = EC_Error + SS_IOProc + EG_WrongFn + EO_File;

  if (set_file_date(local_name, unixstamp))
    Request_Return(ReplyOK, 0L, 0L);
  else
    { 
#if floppies_available
      if (floppy_errno) floppy_handler();
#endif
      Request_Return(Server_errno, 0L, 0L);
    }

  use(myco)
}

/*------------------------------------------------------------------------------
--
-- ServerInfo()
--
------------------------------------------------------------------------------*/

/**
*** ServerInfo requests are used at the moment solely to obtain statistics
*** about the drive usage.
**/

void Drive_ServerInfo(myco)
Conode *myco;
{ servinfo *reply = (servinfo *) mcb->Data;

  get_local_name();

  Debug(FileIO_Flag, ("ServerInfo request received for %s", local_name));

  Server_errno = EC_Error + SS_IOProc + EG_WrongFn + EO_Server;

  if (get_drive_info(local_name, reply))
    Request_Return(ReplyOK, 0L, (word) sizeof(servinfo));
  else
    {
#if floppies_available
      if (floppy_errno) floppy_handler();
#endif
      Request_Return(Server_errno, 0L, 0L);
    }

  use(myco)
}

/**
*** Symbolic links are supported on the Sun, but not on any other machine
*** as far as I am aware.
***
*** BLV - NB I have not been able to check this code properly since I do
*** not have a Sun, but it seems to work.
**/

#if (UNIX)

void Drive_Link(myco)
Conode *myco;
{ char *linkname = (char *) &(misc_buffer1[0]);      /* to store link name */
  char *newname  = (char *) &(misc_buffer2[0]);
  char *data = mcb->Data;
  int  context = (int) mcb->Control[Context_off];
  int  name    = (int) mcb->Control[Pathname_off];
  int  dest    = (int) mcb->Control[RenameToname_off];
  int  offset;

  get_local_name();

  strcpy(linkname, local_name);              /* source name is in local_name */

  if (context ne -1 && data[name] ne '/')
   { strcpy(newname, &(data[context]));
     pathcat(newname, &(data[name]));
   }
  else
   strcpy(newname, &(data[name]));

  unless(flatten(newname))
   { Request_Return(EC_Error + SS_IOProc + EG_Name + EO_Message, 0L, 0L);
     return;
   }

  offset = strlen(newname) - strlen(IOname);

  if (context ne -1 && data[dest] ne '/')
   { strcpy(newname, &(data[context]));
     pathcat(newname, &(data[dest]));
   }
  else
   strcpy(newname, &(data[dest]));

  unless(flatten(newname))
   { Request_Return(EC_Error + SS_IOProc + EG_Name + EO_Message, 0L, 0L);
     return;
   }

  data = newname + offset;
  strcpy(IOname, data);
  get_local_name();

  Server_errno = EC_Error + SS_IOProc + EG_Unknown + EO_File;

                 /* check that the source file exists */
  unless(object_exists(local_name))
   {
     Request_Return(Server_errno, 0L, 0L);
     return;
   }

#if (!SUN)
  ServerDebug("link from %s to %s",local_name,linkname);
  Request_Return(ReplyOK, 0L, 0L);

#else

  Server_errno = EC_Error + SS_IOProc + EG_WrongFn + EO_Directory;

  if (symlink(local_name,linkname)==0) 
    Request_Return(ReplyOK, 0L, 0L);
  else
   {
     Request_Return(Server_errno, 0L, 0L);
   }
#endif

  use(myco)
}
#else

void Drive_Link(myco)
Conode *myco;
{ Request_Return(EC_Error + SS_IOProc + EG_WrongFn + EO_Server, 0L, 0L);
  use(myco)
}

#endif
  
/*------------------------------------------------------------------------------
--
-- Dummy routines
--
----------------------------------------------------------------------------*/

/**
*** Lots of Helios requests cannot be implemented satisfactorily under
*** MSdos/TOS so they are defined as invalid functions in header file
*** fundefs.h . The requests affected are : Link, Protect, Refine, 
*** and CloseObj.
**/

/**
***
***
*** That was all the server handler routines. We still need routines for the
*** streams, both file and directory streams.
***
***
**/

/*----------------------------------------------------------------------------
--
--  File streams
--
----------------------------------------------------------------------------*/

/**
*** I begin with File streams. First of all, I declare a structure which
*** contains all the information I need about the open file, namely the
*** file handle and the current byte position. Each stream runs as a
*** separate coroutine, with its own coroutine list node, and the extra
*** field in this node will point to one of these structures.
***
*** When the stream is opened the server creates a new coroutine, and the
*** first thing done by this coroutine is to call the InitStream() routine,
*** which is one of the array of request handlers. It is InitStream that is
*** responsible for opening the file if possible, and for setting up the
*** reply to the Open request. Routine General_Stream() will send this
*** reply, based on the value returned by InitStream() which should be a
*** Helios error code or ReplyOK. If the machine-dependant open succeeds I
*** can get hold of a File_extra structure, initialise it, and store it in the
*** coroutine list node. The Stream is now ready to do some operations on the
*** file, so I can reply to the Open request.
**/


typedef struct { Node       node;
                 Conode     *conode;
                 word       curr_pos;
                 stream     handle;
} File_extra;

word File_InitStream(myco)
Conode *myco;
{ File_extra *extra;
  stream     handle;
  word       openmode = (word) myco->extra;
  int        firsttry = 1;

#ifdef DEMONSTRATION
                    /* Demonstration mode, no writing */
  openmode = (openmode & 0xFFFFFFF0L) + O_ReadOnly;
#endif

  myco->flags = parentco->flags | CoFlag_FileStream;

#if files_cannot_be_opened_twice
/**
*** Check through the list of open files, to see if we are trying to open a file
*** that is already open.
**/
  { PRIVATE void fn( check_file_name, (Conode *, Conode *));
    WalkList(&WaitingCo, func(check_file_name), myco);
    WalkList(&PollingCo, func(check_file_name), myco);
  }
#endif

  extra = (File_extra *) malloc(sizeof(File_extra));
  if (extra eq (File_extra *) NULL)
      return(EC_Warn + SS_IOProc + EG_NoMemory + EO_Server);
  extra->curr_pos = 0L;
  extra->conode   = myco;
  myco->extra     = (word *) extra;

retry_open:
  Server_errno = EC_Error + SS_IOProc + EG_Unknown + EO_File;
  handle = open_file(local_name, (word) openmode);

  if (handle eq 0)
   {      /* magic, too many files open */
          /* Retry once, closing some other file */
     if ((Server_errno eq EC_Error + SS_IOProc + EG_NoResource + EO_Server) &&
         firsttry)
	  { File_extra *newextra = (File_extra *) Open_files_list.tail;
        Conode     *conode   = newextra->conode;
        firsttry             = 0;
        conode->type         = CoSuicide;
        StartCo(conode);
        goto retry_open;
      }

     iofree(extra);
#if floppies_available
     if (floppy_errno) floppy_handler();
#endif
     Debug(Open_Flag, ("Failed to open"));

     return(Server_errno);
   }

  extra->handle   = handle;
  AddHead(&(extra->node), &Open_files_list);
  return(ReplyOK); 
}

#if files_cannot_be_opened_twice

/**
*** If the system does not support having files open twice at the same time in
*** a sensible way, then I have to prevent this. 
*** When I get an Open request I check whether or not there is another 
*** stream with the same name, and if so I tell the corresponding
*** coroutine to die off. This is perfectly safe, since the file will be
*** reopened automatically next time Helios tries to access it.
**/

                           /* This is called via a WalkList() in open above */
PRIVATE void check_file_name(cur_node, mynode)
Conode *cur_node, *mynode;
{
  if (cur_node eq mynode) return;
#if (ST || PC)
  if (!mystrcmp(&(cur_node->name[1]), IOname))     /* Is it the same file ? */
#else
  if (!strcmp(&(cur_node->name[1]), IOname))
#endif
   { 
     cur_node->type   = CoSuicide;        /* Die, you silly coroutine */
     StartCo(cur_node);
   }
}

#endif

/**
*** TidyStream() is called when the Stream coroutine is instructed to die
*** off, i.e. when the Server has received a ReqDie request from the
*** transputer or when the user wishes to exit or reboot the Server. It is
*** rather important that under those circumstances all open files are closed.
**/

word File_TidyStream(myco)
Conode *myco;
{ File_extra *extra = (File_extra *) myco->extra;
  word x;

  x = close_file(extra->handle);
#if floppies_available
  if (floppy_errno) floppy_handler();
#endif
  Remove(&(extra->node));
  iofree(extra);
  return(0L);
}

/**
*** The following code handles Helios Read's, which are rather tricky
*** requests because they can involve sending a number of different messages
*** to the transputer. Most requests involve a single message going each
*** way, which is rather simpler to support.
***
*** First of all I extract all the information I need to satisfy the
*** request, e.g I find out how many bytes to read, which open file to read
*** them from, and so on. File Reads are rather important, so there is a
*** debugging option specially for them. It is possible that the current
*** file position is not the same as the one specified in the message,
*** because there may have been a Seek on the transputer side or a message
*** may have got lost. Hence I have to check the file position and do a
*** seek if necessary and if possible.
***
*** I am now at the right position in the file and can start reading in the
*** data and sending it to the transputer. Sending to the transputer involves
*** the message passing routines so I need to worry about saving and restoring
*** all the ports. Then I read in enough data for the current message,
*** update my pointers and counters, test for the end-of-file condition
*** by comparing the amount actually read with the amount asked for, a test
*** that seems to work happily, and send off the data. There are various
*** nasties affecting the return code, which are described in header file
*** IOGSP.H .
***
*** It is necessary to allow for timeouts, in case somebody wants to load the
*** compiler in a fraction of a second or so. Similarly, it is necessary to
*** suspend the coroutine between reads to allow the rest of the system to use
*** the CPU, particularly important when e.g. moving the mouse during a big
*** read.
**/


void File_Read(myco)
Conode *myco;
{ word  asked, actual=0L, req_pos, curr_pos, curr, temp, end_of_file=false,
        seq=0L;
  word  reply_port, reply;
  File_extra *extra = (File_extra *) myco->extra;
  stream     handle;

  AddHead(Remove(&(extra->node)), &Open_files_list);

  handle     = extra->handle;
  asked      = (mcb->Control)[ReadSize_off];
  reply_port = mcb->MsgHdr.Reply;
  req_pos    = (mcb->Control)[ReadPos_off];
  curr_pos   = extra->curr_pos;

  Debug(Read_Flag, ("Supposed to read %ld bytes at %ld in %s.", asked, req_pos, &(myco->name[1])) );

  if (req_pos ne curr_pos)          /* Do I have to seek to the current pos ? */
    { Debug(Read_Flag,("Having to seek to %ld from %ld.", req_pos, curr_pos));

      Server_errno = EC_Error + SS_IOProc + EG_Broken + EO_File;

      if ((temp = seek_in_file( handle, req_pos, 0)) < 0L)
       { 
#if floppies_available
         if (floppy_errno) floppy_handler();
#endif
         Request_Return(Server_errno, 0L, 0L);
         extra->curr_pos = -1L;
         return;
       }
      else
        curr_pos = req_pos;
    }

  forever                          /* Read in data bit by bit and send it off */
    { mcb->MsgHdr.Reply = reply_port;   /* must be saved */
      mcb->MsgHdr.Dest  = 0L;           /* clear reply port */

                        /* how much to read this time around ? */
      curr = (asked - actual) > maxdata ? maxdata : asked-actual;

      Server_errno = EC_Error + SS_IOProc + EG_Broken + EO_File;

      temp = read_from_file(handle, curr, mcb->Data);
#if floppies_available
      if (floppy_errno) floppy_handler();
#endif

      if (temp < 0L)     /* error of some sort... */
        { Request_Return(Server_errno, 0L, 0L);
          extra->curr_pos = -1L;
          return;
        }
      actual   += temp;
      curr_pos += temp;

                               /* finished ? */
      if (temp < curr) end_of_file = true;
      reply = seq + (end_of_file ? ReadRc_EOF : 
                     ( (asked eq actual)
                       ? ReadRc_EOD : 0L) );
      seq += 16L;

                  /* what flag should be used ? */
      if (end_of_file || (asked eq actual) )
        mcb->MsgHdr.Flags = 0;
      else
        mcb->MsgHdr.Flags = MsgHdr_Flags_preserve;

      Debug(Read_Flag, ("returning %ld bytes.", temp));

                                 /* send off current chunk of data */
      Request_Return(reply, 0L, temp);

                                  /* finished for one reason or another ? */
      if ((actual >= asked) || end_of_file )
         { extra->curr_pos = curr_pos; return; }

                                  /* suspend myself for a bit */
      AddTail(Remove(&(myco->node)), PollingCo);

      myco->type = CoReady;
      myco->timelimit = MAXINT;
      Multi_nowait++;
      Suspend();
      Multi_nowait--;
                                 /* make sure I get back into waiting list */
      PostInsert(Remove(&(myco->node)), Heliosnode);
      if ((myco->type eq CoSuicide) || (myco->type eq CoTimeout))
       { word x;
         x = close_file(extra->handle);
         iofree(extra);
         Seppuku();
       }
    }
}

/**
*** Having dealt with Helios Reads, I must now worry about Helios Writes.
*** These are also quite nasty because they may involve multiple messages
*** from the transputer.
***
*** As for the Read I extract the information I need and perform a seek if
*** the file positions do not match. Then I test for the case where the amount
*** to be written is less than IOCDataMax and the data came with the message.
*** If it did I perform the Write and send back the reply. I also test for
*** the special case of writing 0 bytes, which can happen!
***
*** If the amount to be written is more than IOCDataMax, I must first send
*** a message to the transputer indicating how it is send all the data.
*** This is described in header file IOGSP.H . I do not perform any caching
*** so it does not matter how the data is sent, as long as it fits into the
*** data vector of the message buffer. Once this message is sent
*** I must wait for all the data to arrive from the transputer, i.e. the
*** coroutine must repeatedly suspend itself to wait for its next message.
*** I have to worry about the user wanting to exit during this time, and
*** about timeouts. A timeout is likely to happen if one of the data messages
*** got lost for one reason or another. The coroutine waits in WaitingCo,
*** so I can use the coroutine timeout mechanism that is already available.
***
*** Various errors are possible. The transputer side may time out in the
*** middle of sending the multiple writes, e.g. if the filing system is
*** very slow. The probable recovery action is for Helios to send a new
*** Write request, which I must detect. Also, it is possible for the 
*** write itself to fail, e.g. when the disk is full. In that case I must
*** still accept the data, ignore it, and at the end send an error message.
**/ 

void File_Write(myco)
Conode *myco;
{ word asked, actual, req_pos, reply_port, time_interval, temp;
  File_extra *extra = (File_extra *) myco->extra;
  stream handle;
  int    write_error;
try_again:

  AddHead(Remove(&(extra->node)), &Open_files_list);

  write_error = 0;
  handle = extra->handle;
  asked = (mcb->Control)[WriteSize_off];
  req_pos = (mcb->Control)[WritePos_off];

  Debug(Write_Flag, ("Supposed to write %ld bytes at %ld in %s", asked, req_pos, &(myco->name[1])) );

#ifdef DEMONSTRATION
  ServerDebug("*** Helios demonstration version - the disk is write-protected");
  Request_Return(EC_Error + SS_IOProc + EG_Protected + EO_Server, 0L, 0L);
  return;
#else 
                                            /* Start by moving to the right */
  if (req_pos ne extra->curr_pos)           /* location in the file.        */
    { 
      Debug( Write_Flag, ("Having to seek to %ld from %ld.", req_pos, extra->curr_pos) );

      Server_errno = EC_Error + SS_IOProc + EG_Broken + EO_File;

      if (seek_in_file(handle, req_pos, 0) < 0L)
       {
#if floppies_available
         if (floppy_errno) floppy_handler();
#endif
         Request_Return(Server_errno, 0L, 0L);
         extra->curr_pos = -1;
         return;
       }
      else
         extra->curr_pos = req_pos;
    }
           /* default error for writes is disk full */
  Server_errno = EC_Error + SS_IOProc + EG_NoMemory + EO_File;

                                /* In the simple case, the data comes with  */
                                /* the message.                             */
  if (mcb->MsgHdr.DataSize > 0)
    { actual = (word) mcb->MsgHdr.DataSize;
      unless(write_to_file(handle, actual, mcb->Data) )
        {
#if floppies_available
          if (floppy_errno) floppy_handler();
#endif
          Request_Return(Server_errno , 0L, 0L);
          return;
        }
      mcb->Control[Reply1_off] = actual;
      extra->curr_pos         += actual;
      Request_Return(WriteRc_Done, 1L, 0L);
      return;
    }

  if (asked eq 0L)                  /* special case */
    { mcb->Control[Reply1_off] = 0L;
      Request_Return(WriteRc_Done, 1L, 0L);
      return;
    }
  actual           = 0L;
  reply_port       = mcb->MsgHdr.Reply;
  time_interval    = divlong (mcb->Control[WriteTimeout_off], time_unit);

                    /* send the first reply indicating how to send the data */
#if floppies_available
   if (myco->flags & CoFlag_Floppy)
    { (mcb->Control)[Reply1_off] = (maxdata > 1024L) ? 1024L : maxdata;
      (mcb->Control)[Reply2_off] = (mcb->Control)[Reply1_off];
    }
   else
#endif
    { (mcb->Control)[Reply1_off] = maxdata; 
      (mcb->Control)[Reply2_off] = maxdata;
    }
  mcb->MsgHdr.Flags = MsgHdr_Flags_preserve; /* must preserve the route here */
  Request_Return(WriteRc_Sizes, 2L, 0L);  

  while (actual < asked)
    { myco->timelimit  = Now + time_interval;  /* reset the time limit */
                                              /* for next message to arrive */
      Suspend();                             /* wait for the next lot of data */

      if (myco->type eq CoSuicide)
          goto write_fail;
      elif (myco->type eq CoTimeout)
          break;

      if (getfnrc(mcb) eq FG_Write)
       goto try_again;

      unless(write_error)
       { temp = write_to_file(handle, (word) mcb->MsgHdr.DataSize, mcb->Data);
#if floppies_available
         if (floppy_errno) floppy_handler();
#endif
            /* On failure, send an error message and get rid of the stream */
         unless(temp)
          write_error = 1;
       }

      actual += (word) mcb->MsgHdr.DataSize;
      extra->curr_pos += mcb->MsgHdr.DataSize;
    }

  mcb->MsgHdr.Reply        = reply_port;
  if (write_error)
   Request_Return(Server_errno, 0L, 0L);
  else
   { mcb->Control[Reply1_off] = actual;
     Request_Return(WriteRc_Done, 1L, 0L);
   }
  return;

write_fail:
  { word x;
    x = close_file(extra->handle);
  }
  iofree(extra);
  Seppuku();
#endif   /* DEMONSTRATION VERSION */
}

/**
*** Closing a file is fairly painless.
**/

void File_Close(myco)
Conode *myco;
{ File_extra *extra = (File_extra *) myco->extra;

  Debug(Close_Flag, ("Supposed to close %s", &(myco->name[1])) );

  Server_errno = EC_Error + SS_IOProc + EG_Broken + EO_File;

  unless(close_file(extra->handle))
    {
#if floppies_available
      if (floppy_errno) floppy_handler();
#endif
      if (mcb->MsgHdr.Reply ne 0L)      
        Request_Return(Server_errno, 0L, 0L);
      return;
    }
  else
    { if (mcb->MsgHdr.Reply ne 0L)
         Request_Return(ReplyOK, 0L, 0L);
      Remove(&(extra->node));
      iofree(extra);
      Seppuku();
    }
}

/**
*** GetSize is used to obtain the size of a file when the stream is already
*** open. Hence I cannot use any directory information because the size
*** may have changed by writing data at the end.
**/
  
void File_GetSize(myco)
Conode *myco;
{ File_extra *extra = (File_extra *) myco->extra;
  stream handle;
  word  size;
  handle = extra->handle;

  AddHead(Remove(&(extra->node)), &Open_files_list);

  Server_errno = EC_Error + SS_IOProc + EG_WrongSize + EO_File; 
  if ( (size = get_file_size(handle, extra->curr_pos)) < 0)
   {
#if floppies_available
     if (floppy_errno) floppy_handler();
#endif
     Request_Return(Server_errno, 0L, 0L);
   }
  else
    { mcb->Control[Reply1_off] = size;
      Request_Return(ReplyOK, 1L, 0L);
    }
}

/**
*** Seek is not a very useful call, because all seeks can be done internally
*** by adjusting the pointer to the new position - the next Read or Write
*** would cause an implicit Seek. There is a Seek request to allow clever
*** file servers, i.e. ones with buffering etc., to reorganise their caching.
*** Due to a problem with one version of the system library I was forced to
*** support the request as well, and the code below does that. It is fairly
*** easy, because all I need to do is work out where Helios thinks the new
*** file position should be and seek to that. Ofcourse there may have been
*** some implicit Seeks which did not generate requests, so I cannot rely on
*** my local position.
**/

void File_Seek(myco)
Conode *myco;
{ word mode         = mcb->Control[SeekMode_off];    /* beginning, end etc. */
  word newpos       = mcb->Control[SeekNewPos_off];  /* offset */
  word pos          = mcb->Control[SeekPos_off];     /* old position */
  File_extra *extra = (File_extra *) myco->extra;
  stream handle;
  word temp;

  AddHead(Remove(&(extra->node)), &Open_files_list);

  handle = extra->handle;

  if ((mode eq S_Relative) || (mode eq S_Beginning))
    { if (mode eq S_Relative) newpos += pos;

      Server_errno = EC_Error + SS_IOProc + EG_Broken + EO_File;
      if (seek_in_file(handle, newpos, 0) < 0L )
        {  
#if floppies_available
          if (floppy_errno) floppy_handler();
#endif
          Request_Return(Server_errno, 0L, 0L);
          return;
        }
      mcb->Control[Reply1_off] = newpos;
      Request_Return(ReplyOK, 1L, 0L);
      return;
    }

  if (mode eq S_End)
    { Server_errno = EC_Error + SS_IOProc + EG_Broken + EO_File;
      if ((temp = seek_in_file(handle, newpos, 1)) < 0L)
        {
#if floppies_available
          if (floppy_errno) floppy_handler();
#endif
          Request_Return(Server_errno, 0L, 0L);
          return;
        }
      mcb->Control[Reply1_off] = temp;
      Request_Return(ReplyOK, 1L, 0L);
      return;
    }

  Request_Return(EC_Error + SS_IOProc + EG_Parameter + EO_Message, 0L, 0L);
                  /* invalid mode */
}


/*----------------------------------------------------------------------------
--
-- Directory streams
--
----------------------------------------------------------------------------*/

/**
*** The Helios approach to directory searches is rather different from that of
*** most other systems. A directory can be opened and read just like a file,
*** and the data in the file represents the entries in the directory. This
*** data is organised in units of DirEntry, one DirEntry for each entry in
*** the directory, and DirEntry is described in the header file IOSYSLIB.H .
***
*** It is important when reading a directory that a consistent view of the
*** universe is obtained. After all, it is perfectly possible for some other
*** task to create more files inside the directory as you are reading it.
*** To achieve this consistent view of the world, the entire directory is
*** searched when the Stream is opened and the information is stored in,
*** surprise surprise, a linked list.
***
*** ObjNode is the list structure used to hold all the entries in the
*** directory, and is defined in header file structs.h. The extra field of the
*** directory stream coroutine points to a DirHeader structure, also defined
*** in structs.h, which contains the list header and a count of the number of
*** entries.
***
*** When the Stream is opened, the new coroutine will call InitStream which is
*** responsible for examining the whole directory and storing the information
*** in the linked list. Most of the work is done by routine do_search().
***
*** TidyStream() is called when the Stream has to be closed for one reason or
*** another, and frees all the space taken up by the linked list.
**/


#if ANSI_prototypes
word Dir_TidyStream(Conode * myco)
#else
word Dir_TidyStream (myco)
Conode *myco;
#endif
{ DirHeader *extra = (DirHeader *) myco->extra;

#if PC
  WalkList(&(extra->list), func(FreeObjNode));
#else
  FreeList(&(extra->list));
#endif
  iofree(extra);

  return(0L);
}


word Dir_InitStream(myco)
Conode *myco;
{ DirHeader *extra;

                                 /* Cannot open directory in WriteOnly mode */
  if ( ((word) myco->extra & 0x0F) eq O_WriteOnly)
   { Debug(Directory_Flag, ("Wrong mode for opening directory") );
     return(EC_Error + SS_IOProc + EG_Protected + EO_Directory);
   }

  extra = (DirHeader *) malloc(sizeof(DirHeader));
  if (extra eq NULL)
     return(EC_Warn + SS_IOProc + EG_NoMemory + EO_Server);
  InitList(&(extra->list));             /* set up the linked list structure */
  extra->entries = 0L;
   
  myco->extra = (ptr) extra;

  Server_errno = EC_Error + SS_IOProc + EG_Broken + EO_Directory;

  Debug(Directory_Flag, ("Reading directory %s", local_name));

                                        /* read in all the data */
  if ((extra->entries = search_directory(local_name, &(extra->list))) < 0L)
     {
       Dir_TidyStream(myco);       /* assumes list is in a consistent state */
#if floppies_available
       if (floppy_errno) floppy_handler();
#endif
       Debug(Directory_Flag, ("Failed to read directory") );
       return(Server_errno);
     }

  Debug(Directory_Flag, ("There are %ld entries in the directory", extra->entries) );

  return(ReplyOK);
}

/**
*** Closing a directory is easy, because all that is involved is freeing the
*** memory taken up by the linked list, which is done by TidyStream(), and
*** replying the message.
**/

void Dir_Close(myco)
Conode *myco;
{
  Debug(Directory_Flag, ("Closing directory %s", &(myco->name[1])) );

  Dir_TidyStream(myco);
  if (mcb->MsgHdr.Reply ne 0L)
    Request_Return(ReplyOK, 0L, 0L);
  Seppuku();
}


/**
*** Reading a directory is similar to reading a file, but there are some
*** simplifications. First of all, the size of direntry is 44 bytes so for
*** any reasonable size directory I will never overflow maxdata on a single
*** read. Hence I only ever return one message. If I do have more data
*** than can fit into the message I just "time-out" before all of it was read.
*** Now the Read just involves extracting data from the linked list, putting
*** it in the message data area, and sending it off. All of these are quite
*** simple. To avoid nasty problems with running out of list at inconvenient
*** moments I do all my checks for this sort of thing at the beginning,
*** adapting the read as appropriate.
**/

void Dir_Read(myco)
Conode  *myco;
{ byte  *dest, *temp;
  word  i;
  word  count, offset;
  DirHeader *extra = (DirHeader *) myco->extra;
  ObjNode   *curnode;
  word  first_block;
  word  amount;

  count  = (mcb->Control)[ReadSize_off];
  offset = (mcb->Control)[ReadPos_off]; 
  dest   = &((mcb->Data)[0]);

  Debug(Directory_Flag, ("Directory read for %s : %ld bytes at %ld", &(myco->name[1]), count, offset) );

  if (count > maxdata) 
    count = maxdata; 

#if PC  
                   /* Under Microsoft C long division is not reliable */
  for (first_block = 0L; offset > (word) sizeof(DirEntry); first_block++)
    offset -= (word) sizeof(DirEntry);

#else

  first_block  = offset / (word) sizeof(DirEntry);
  offset       = offset % (word) sizeof(DirEntry);

#endif

/**
*** When I get here I should read the last (sizeof(DirEntry) - offset) bytes
*** in first_block, then the next block, etc.
**/
  if (first_block >= extra->entries)
   { Request_Return(ReadRc_EOF, 0L, 0L); return; }

                /* move to first block, known to exist */
  amount = 0; curnode = (ObjNode *) (extra->list).head; 
  for (i = 0; i < first_block; i++)
   curnode = (ObjNode *) (curnode->node.next);

  dest = mcb->Data;
  temp = (char *) &(curnode->direntry);
  temp = &(temp[offset]);
  memcpy(dest, temp, sizeof(DirEntry) - (int) offset);
  amount = sizeof(DirEntry) - offset;
  dest  += sizeof(DirEntry) - offset;

  while (amount < count)
   { curnode = (ObjNode *) curnode->node.next;
     if (curnode->node.next eq (Node *) NULL)
      break;
     temp = (char *) &(curnode->direntry);
     memcpy(dest, temp, sizeof(DirEntry));
     amount += sizeof(DirEntry);
     dest   += sizeof(DirEntry);
   }

  Request_Return((amount < count) ? ReadRc_EOF : ReadRc_EOD, 0L,
                 (amount < count) ? amount : count);
}

/**
*** GetSize is an important request for directories because e.g. ls uses it
*** to find out how many entries there are in the directory. The data is stored
*** in the extra field of the coroutine node.
**/ 

void Dir_GetSize(myco)
Conode *myco;
{ DirHeader *extra = (DirHeader *) myco->extra;

  (mcb->Control)[Reply1_off] = (word) (extra->entries * sizeof(DirEntry));
  Debug(Directory_Flag, ("Directory getsize : size is %ld", mcb->Control[Reply1_off]) );

  Request_Return(ReplyOK, 1L, 0L);
}


/*----------------------------------------------------------------------------
--
-- Logger service
--
----------------------------------------------------------------------------*/

PRIVATE FILE *logfile;
PRIVATE char *logname;
PRIVATE char *default_logname;
PRIVATE int  log_atend;
PRIVATE word log_curpos;
PRIVATE void fn( log_toend, (void));
 
void init_logger()
{ char *dest = get_config("logging_destination");
  logname = get_config("logfile");
  if (logname eq (char *) NULL)
   logname = "logfile";
  default_logname = logname;

  if (dest eq NULL)
   log_dest = Log_to_screen;
  else
   { if (!mystrcmp(dest, "FILE"))
      log_dest = Log_to_file;
     elif (!mystrcmp(dest, "BOTH"))
      log_dest = Log_to_both;
     else
      log_dest = Log_to_screen;
   }
#if (UNIX || HELIOS || MSWINDOWS)   
  /* perror() is too useful a call, waste a file descriptor */
  /* stderr appears to be erratic under Windows             */
  logfile = fopen(logname, "w+b");
#else
  logfile = freopen(logname, "w+b",stderr);
#endif
  log_atend = 1; log_curpos = 0L;   
}

void tidy_logger()
{
  fflush(logfile);
  fclose(logfile);
}

void Logger_Open(myco)
Conode *myco;
{ NewStream(Type_File, Flags_Closeable + Flags_Interactive,
            NULL, Logger_Handlers);
  use(myco);
}

void Logger_Delete(myco)
Conode *myco;
{ 
#if (UNIX || HELIOS)
  logfile = fopen(logname, "w+b");
#else
  logfile = freopen(logname, "w+b", stderr);
#endif
  log_atend = 1; log_curpos = 0L;
  Request_Return(ReplyOK, 0L, 0L);
  use(myco)
}

void Logger_ObjectInfo(myco)
Conode *myco;
{ register ObjInfo *Heliosinfo = (ObjInfo *) mcb->Data;

  if (!log_atend) log_toend();

  Heliosinfo->DirEntry.Type   = swap(Type_File);
  Heliosinfo->DirEntry.Flags  = swap(0L);
  Heliosinfo->DirEntry.Matrix = swap(DefFileMatrix);
  Heliosinfo->Size            = swap(log_curpos);
  Heliosinfo->Account         = swap(0L);
  Heliosinfo->Creation        = swap(Startup_Time);
  Heliosinfo->Access          = swap(Startup_Time);
  Heliosinfo->Modified        = swap(Startup_Time);
  strcpy(Heliosinfo->DirEntry.Name, IOname);

  Request_Return(ReplyOK, 0L, (word) sizeof(ObjInfo));
  use(myco)
}

void Logger_Close(myco)
Conode *myco;
{ if (mcb->MsgHdr.Reply ne 0L)
   Request_Return(ReplyOK, 0L, 0L);
  Seppuku();
  use(myco)
}

void Logger_Write(myco)
Conode *myco;
{ word asked, actual, reply_port, time_interval;

  asked = (mcb->Control)[WriteSize_off];
  if (!log_atend) log_toend();

                                /* In the simple case, the data comes with  */
                                /* the message.                             */
  if (mcb->MsgHdr.DataSize > 0)
    { mcb->Data[mcb->MsgHdr.DataSize] = '\0';
      if ((log_dest eq Log_to_file) || (log_dest eq Log_to_both))
        write_to_log(mcb->Data);
      if ((log_dest eq Log_to_screen) || (log_dest eq Log_to_both))
       { char	*temp;
         for (temp = mcb->Data; *temp ne '\0'; temp++)
          if (*temp eq '\n')
           *temp = 0x01;	/* hack, see ansi_out */
         output(mcb->Data);
       }
      mcb->Control[Reply1_off] = (word) mcb->MsgHdr.DataSize;
      Request_Return(WriteRc_Done, 1L, 0L);
      return;
    }

  if (asked eq 0L)                  /* special case */
    { mcb->Control[Reply1_off] = 0L;
      Request_Return(WriteRc_Done, 1L, 0L);
      return;
    }
  actual           = 0L;
  reply_port       = mcb->MsgHdr.Reply;
  time_interval    = divlong(mcb->Control[WriteTimeout_off], time_unit);

  (mcb->Control)[Reply1_off] = maxdata; 
  (mcb->Control)[Reply2_off] = maxdata;
  mcb->MsgHdr.Flags = MsgHdr_Flags_preserve; /* must preserve the route here */
  Request_Return(WriteRc_Sizes, 2L, 0L);  

  while (actual < asked)
    { myco->timelimit  = Now + time_interval;  /* reset the time limit */
                                              /* for next message to arrive */
      Suspend();                             /* wait for the next lot of data */

      if (myco->type eq CoSuicide)
          Seppuku();
      elif (myco->type eq CoTimeout)
          break;

      mcb->Data[mcb->MsgHdr.DataSize] = '\0';
      if ((log_dest eq Log_to_file) || (log_dest eq Log_to_both))
        write_to_log(mcb->Data);
      if ((log_dest eq Log_to_screen) || (log_dest eq Log_to_both))
       { char	*temp;
         for (temp = (char *) mcb->Data; *temp ne '\0'; temp++)
          if (*temp eq '\n')
           *temp = 0x01;	/* Hack - see ansi_out() */
         output(mcb->Data);
       }
      actual += (word) mcb->MsgHdr.DataSize;
    }

  mcb->Control[Reply1_off] = actual;
  mcb->MsgHdr.Reply        = reply_port;
  Request_Return(WriteRc_Done, 1L, 0L);
}

void Logger_Read(myco)
Conode *myco;
{ word  asked, actual=0L, req_pos, curr, temp, end_of_file=false, seq=0L;
  word  reply_port, reply;

  if ((logfile eq (FILE *) NULL) || (myco->extra eq (ptr) -1L))
   { Request_Return(ReadRc_EOF, 0L, 0L);
     return;
   }

  asked      = (mcb->Control)[ReadSize_off];
  reply_port = mcb->MsgHdr.Reply;
  req_pos    = (mcb->Control)[ReadPos_off];

  forever                          /* Read in data bit by bit and send it off */
    { mcb->MsgHdr.Reply = reply_port;   /* must be saved */
      mcb->MsgHdr.Dest  = 0L;           /* clear reply port */

                        /* how much to read this time around ? */
      curr = (asked - actual) > maxdata ? maxdata : asked-actual;

      if (req_pos ne log_curpos)          /* Do I have to seek to the current pos ? */
       { log_atend = 0;
         Server_errno = EC_Error + SS_IOProc + EG_Broken + EO_File;
         if (seek_in_file(fileno(logfile), req_pos, 0) < 0L)
          { Request_Return(Server_errno, 0L, 0L);
            log_curpos = -1L;
            return;
          }
         else
          log_curpos = req_pos;
       }

      Server_errno = EC_Error + SS_IOProc + EG_Broken + EO_File;
      temp = read_from_file(fileno(logfile), curr, mcb->Data);

      if (temp < 0L)     /* error of some sort... */
        { Request_Return(Server_errno, 0L, 0L);
          log_curpos = -1L; log_atend = 0;
          return;
        }
      actual     += temp;
      log_curpos += temp;
      req_pos    += temp;
                               /* finished ? */
      if (temp < curr)
       {  end_of_file = true;
          myco->extra = (ptr) -1L;
       }

      reply = seq + (end_of_file ? ReadRc_EOF : 
                     ( (asked eq actual)
                       ? ReadRc_EOD : 0L) );
      seq += 16L;

                  /* what flag should be used ? */
      if (end_of_file || (asked eq actual) )
        mcb->MsgHdr.Flags = 0;
      else
        mcb->MsgHdr.Flags = MsgHdr_Flags_preserve;

                                 /* send off current chunk of data */
      Request_Return(reply, 0L, temp);

                                  /* finished for one reason or another ? */
      if ((actual >= asked) || end_of_file )
       break;
    }
  use(myco)
}

void Logger_GetSize(myco)
Conode *myco;
{ if (!log_atend) log_toend();
  mcb->Control[Reply1_off] = swap(log_curpos + 1L);  /* lie to confuse cp */
  Request_Return(ReplyOK, 1L, 0L);
  use(myco)
}

void Logger_Seek(myco)
Conode *myco;
{ word mode         = mcb->Control[SeekMode_off];    /* beginning, end etc. */
  word newpos       = mcb->Control[SeekNewPos_off];  /* offset */
  word pos          = mcb->Control[SeekPos_off];     /* old position */

  if ((mode eq S_Relative) || (mode eq S_Beginning))
    { if (mode eq S_Relative) newpos += pos;

      Server_errno = EC_Error + SS_IOProc + EG_Broken + EO_File;
      if (logfile ne (FILE *) NULL)
        if ((log_curpos = seek_in_file(fileno(logfile), newpos, 0)) < 0L )
          { Request_Return(Server_errno, 0L, 0L);
            return;
          }
      mcb->Control[Reply1_off] = newpos;
      Request_Return(ReplyOK, 1L, 0L);
      return;
    }

  Request_Return(EC_Error + SS_IOProc + EG_Parameter + EO_Message, 0L, 0L);
  use(myco)
}

/**
*** A private protocol between clients and the error logger may be used
*** to measure the performance of the I/O link.
**/
#ifdef NEVER
#define LL_Bounce (1 << 24)
#define LL_Empty  (2 << 24)
#define LL_Fill   (3 << 24)
#define C_shift   16
#else
#define LL_Bounce 0x01000000L
#define LL_Empty  0x02000000L
#define LL_Fill   0x03000000L
#define LL_Mask   0x0F000000L
#endif

void Logger_PrivateStream(myco)
Conode *myco;
{ word fnrc = mcb->MsgHdr.FnRc & LL_Mask;
  word dsize = mcb->MsgHdr.FnRc & 0x0000FFFFL;
  word csize = mcb->MsgHdr.FnRc & 0x00FF0000L;

  if (fnrc eq LL_Bounce)
   Request_Return(ReplyOK, (word) mcb->MsgHdr.ContSize,
                  (word) mcb->MsgHdr.DataSize);
  elif(fnrc eq LL_Empty)
   Request_Return(ReplyOK, 0L, 0L);
  else
   { csize      = divlong(csize, 0x00010000L);
     Request_Return(ReplyOK, csize, dsize);
   }
  use(myco)
}

void write_to_log(data)
char *data;
{ word amount = (word) strlen(data);
  if (!log_atend) log_toend();

#if (UNIX || HELIOS)
  { char *tmp;
    for (tmp = data; *tmp != '\0'; tmp++)
     if (*tmp eq '\r')
      *tmp = ' ';
   }
#endif

  if (logfile ne (FILE *) NULL)
   { if (write_to_file(fileno(logfile), amount, data))
      log_curpos += amount;
     else
      { log_curpos = 0L; log_atend = 0; }
   }
}

PRIVATE void log_toend()
{ if (logfile ne (FILE *) NULL)
   { if ((log_curpos = seek_in_file(fileno(logfile), 0L, 1)) < 0L)
      log_atend = 0;
     else
      log_atend = 1;
   }
}

#if MSWINDOWS

void show_logger_name(hDlg)
   HWND hDlg;
{
   SetDlgItemText(hDlg, IDD_FNAME, logname);
}


void set_logger_name(hDlg)
   HWND hDlg;
{
   char buf[80];
   GetDlgItemText(hDlg, IDD_FNAME, buf, 80);
   if (!mystrcmp(logname, buf)) return;
   if (logname ne default_logname) iofree(logname);
   logname = malloc(strlen(buf) + 1);
   if (logname eq NULL)
    { logname = default_logname; return; }
   strcpy(logname, buf);

   fclose(logfile);
   logfile = fopen(logname, "w+b");
   if (logfile eq (FILE *) NULL) {
	 MessageBox(NULL, "Unable to open new logger file", "Logger Error",
		    MB_OK);
	 return;
     }
   log_atend = 1; log_curpos = 0L;
}
#endif	 /* MSWINDOWS */

/*------------------------------------------------------------------------
--                                                                      --
--  rawdisk support - this raw disk device allows you to run the Helios --
--  filing system on a transputer. It needs just three local routines   --
--  to determine the number of sectors, and to read and write a         --
--  number of sectors. Sector size is assumed to be 512 bytes for now.  --
--                                                                      --
------------------------------------------------------------------------*/

#if Rawdisk_supported

#define Sector_size 512L

/**
*** The device /rawdisk is a directory containing entries 0, 1, 2 etc
*** corresponding to the individual drives.
***
*** number_rawdisks should return the number of drives or 0 
***
*** size_rawdisk should return the size of the specified drive in sectors
***
*** read_rawdisk and write_rawdisk should do the appropriate disk I/O,
*** returning 1 for success or 0 for failure
***
*** To store the directory structures, I use ObjNode structures with the
*** account equal to the drive id.
**/

PUBLIC int  fn( number_rawdisks, (void));
PUBLIC word fn( size_rawdisk,    (int));
PUBLIC word 
  fn( read_rawdisk,    (int disk, word first_sec, word no_secs, byte *buff ));
PUBLIC word 
  fn( write_rawdisk,   (int disk, word first_sec, word no_secs, byte *buff ));

PRIVATE DirHeader rawdisk_list;

/**
*** This routine is called when the Server starts up or is rebooted.
**/

void RawDisk_Testfun(result)
word *result;
{
  if (number_rawdisks() > 0)
   *result = 1L;
  else
   *result = 0L;
}

void RawDisk_InitServer(myco)
Conode *myco;
{ int number_disks = number_rawdisks();
  int i;

  InitList(&(rawdisk_list.list));
  rawdisk_list.entries = (word) number_disks;
  myco->extra = (ptr) &rawdisk_list;

  for (i = 0; i < number_disks; i++)
   { ObjNode *node = (ObjNode *) malloc(sizeof(ObjNode));
     if (node eq (ObjNode *) NULL)
      { ServerDebug("Insufficient memory for rawdisk devices.");
/*        longjmp(exit_jmpbuf, 1); */
	longjmp_exit;
      }
     node->size    = size_rawdisk(i);
     node->account = (word) i;
     node->direntry.Name[0] = '0' + i;
     node->direntry.Name[1] = '\0';
     node->direntry.Type   = swap(Type_File);
     node->direntry.Matrix = swap(DefFileMatrix);
     node->direntry.Flags  = swap(0L);
     AddTail(&(node->node), &(rawdisk_list.list));
  }

}

void RawDisk_Open(myco)
Conode *myco;
{ ObjNode *node;

  if (!strcmp(IOname, myco->name))    /* is it for the server ? */
   { NewStream(Type_Directory, Flags_Closeable, (word) myco->extra,
             PortDir_Handlers);
     return;
   }
  elif ( (node = Dir_find_node(myco) ) eq (ObjNode *) NULL)
   Request_Return(EC_Error + SS_IOProc + EG_Unknown + EO_File, 0L, 0L);
  else
   { Debug(HardDisk_Flag, ("Rawdisk : open") );
     NewStream(Type_File, Flags_Closeable, (word) node, Rawdisk_Handlers );
   }
}

void RawDisk_Close(myco)
Conode *myco;
{
  Debug(HardDisk_Flag, ("Rawdisk : close") );

  if (mcb->MsgHdr.Reply ne 0L)
   Request_Return(ReplyOK, 0L, 0L);
  Seppuku();
  use(myco)
}

/**
*** Seek's are fairly redundant, but I must support them anyway. Unfortunately
*** this means at least one redundant message transaction every time I try
*** to do something with the board.
**/
void RawDisk_Seek(myco)
Conode *myco;
{ word mode     = mcb->Control[SeekMode_off];    /* beginning, end etc. */
  word newpos   = mcb->Control[SeekNewPos_off];  /* offset */
  word pos      = mcb->Control[SeekPos_off];     /* old position */
  ObjNode *node = (ObjNode *) myco->extra;

  Debug(HardDisk_Flag, ("Rawdisk : seek to %lx", newpos) );

  if ((mode eq S_Relative) || (mode eq S_Beginning))
    { if (mode eq S_Relative) newpos += pos;
      mcb->Control[Reply1_off] = newpos;
      if ((newpos & (Sector_size - 1L)) ne 0L)
       Request_Return(EC_Error + SS_IOProc +EG_Parameter + EO_Message, 0L, 0L);
      else
       Request_Return(ReplyOK, 1L, 0L);
      return;
    }

  Request_Return(EC_Error + SS_IOProc + EG_Parameter + EO_Message, 0L, 0L);
                  /* invalid mode */
  use(myco)
}

void RawDisk_GetSize(myco)
Conode *myco;
{ ObjNode *node = (ObjNode *) myco->extra;

  mcb->Control[Reply1_off] = (word) (node->size * Sector_size);
  Debug(HardDisk_Flag, ("Rawdisk : GetSize request, size is %lx", \
        mcb->Control[Reply1_off]) ); 
  Request_Return(ReplyOK, 1L, 0L);
  use(myco)
}

void RawDisk_Read(myco)
Conode *myco;
{ word    asked, req_pos;
  word    first_sec, no_secs;
  word    result;
  ObjNode *node = (ObjNode *) myco->extra;

  asked      = (mcb->Control)[ReadSize_off];
  req_pos    = (mcb->Control)[ReadPos_off];

  if ((req_pos & (Sector_size - 1L)) ne 0L)
   { Debug(HardDisk_Flag, ("R : not on sector boundary") );
     Request_Return(EC_Error + SS_IOProc + EG_WrongSize + EO_File, 0L, 0L);
     return;
   }

  if ((asked & (Sector_size - 1L)) ne 0L)
   { Debug(HardDisk_Flag, ("R : not whole number of sectors") );
     Request_Return(EC_Error + SS_IOProc + EG_WrongSize + EO_File, 0L, 0L);
     return;
   }
  if (asked > 0x08000L) asked = 0x08000L;  /* limit read to 32K */

  no_secs   = divlong(asked, Sector_size);
  first_sec = divlong(req_pos, Sector_size);

  if ((first_sec + no_secs) > node->size)
   { Debug(HardDisk_Flag, ("R : past end of disk") );
     Request_Return(EC_Error + SS_IOProc + EG_WrongSize + EO_File, 0L, 0L);
     return;
   }

  Debug(HardDisk_Flag, ("R : @ %lx, %ld sectors", first_sec, no_secs) );

  Server_errno = EC_Error + SS_IOProc + EG_Broken + EO_File;
  result = read_rawdisk( (int) node->account, first_sec, no_secs, mcb->Data);
  if (result eq 0L)
   { Debug(HardDisk_Flag, ("R : hardware error") );
     Request_Return(Server_errno, 0L, 0L);
     return;
   }

  Request_Return(ReadRc_EOD, 0L, asked);
  use(myco)
}


void RawDisk_Write(myco)
Conode *myco;
{ word    asked, req_pos;
  word    first_sec, no_secs;
  word    result;
  ObjNode *node = (ObjNode *) myco->extra;

  asked      = (mcb->Control)[ReadSize_off];
  req_pos    = (mcb->Control)[ReadPos_off];

  if ((req_pos & (Sector_size - 1L)) ne 0L)
   { Debug(HardDisk_Flag, ("W : not on sector boundary") );
     Request_Return(EC_Error + SS_IOProc + EG_WrongSize + EO_File, 0L, 0L);
     return;
   }


  if ((asked & (Sector_size - 1L)) ne 0L)
   { Debug(HardDisk_Flag, ("W : not whole number of sectors") );
     Request_Return(EC_Error + SS_IOProc + EG_WrongSize + EO_File, 0L, 0L);
     return;
   }
  if (asked > 0x08000L) asked = 0x08000L;  /* limit read to 32K */

  no_secs   = divlong(asked, Sector_size);
  first_sec = divlong(req_pos, Sector_size);

  if ((first_sec + no_secs) > node->size)
   { Debug(HardDisk_Flag, ("W : past end of disk") );
     Request_Return(EC_Error + SS_IOProc + EG_WrongSize + EO_File, 0L, 0L);
     return;
   }

  if (mcb->MsgHdr.DataSize eq 0)
   { word reply = mcb->MsgHdr.Reply;
     mcb->Control[Reply1_off] = asked;
     mcb->Control[Reply2_off] = 0L;
     mcb->MsgHdr.Flags = MsgHdr_Flags_preserve;
     Request_Return(WriteRc_Sizes, 2L, 0L);

     myco->timelimit = Now + (10L * OneSec);
     Suspend();
     if ((myco->type eq CoSuicide) || (myco->type eq CoTimeout))
      Seppuku();
     mcb->MsgHdr.Reply = reply;
   }

  Debug(HardDisk_Flag, ("W : @ %ld, %ld sectors", first_sec, no_secs) );

  Server_errno = EC_Error + SS_IOProc + EG_Broken + EO_File;
  result = write_rawdisk((int) node->account, first_sec, no_secs, mcb->Data);
  if (result eq 0)
   { Debug(HardDisk_Flag, ("W : hardware error %x", result) );
     Request_Return(Server_errno, 0L, 0L);
     return;
   }
  mcb->Control[Reply1_off] = asked;
  Request_Return(WriteRc_Done, 1L, 0L);
}

#endif /* Rawdisk_supported */



/*------------------------------------------------------------------------
--                                                                      --
--  romdisk support - this romdisk device allows access from helios     --
--  to roms on the pc board						--
--                                                                      --
------------------------------------------------------------------------*/

#if Romdisk_supported


/**
*** The device /rom is a directory containing entries 0
*** corresponding to three of the four roms
*** the first is used to store DR DOS and the server
***
*** size_rawdisk should return the size of the specified drive in sectors
***
*** read_rawdisk and write_rawdisk should do the appropriate disk I/O,
*** returning 1 for success or 0 for failure
***
*** To store the directory structures, I use ObjNode structures with the
*** account equal to the drive id.
**/

PUBLIC int  fn( number_romdisks, (void));
PUBLIC word fn( size_romdisk,    (int));
PUBLIC word 
  fn( read_romdisk,    (int disk, word first_sec, word no_secs, byte *buff ));
PUBLIC word 
  fn( write_romdisk,   (int disk, word first_sec, word no_secs, byte *buff ));

PRIVATE DirHeader romdisk_list;

/**
*** This routine is called when the Server starts up or is rebooted.
**/

void RomDisk_Testfun(result)
word *result;
{
  if (number_romdisks() > 0)
   *result = 1L;
  else
   *result = 0L;
}

void RomDisk_InitServer(myco)
Conode *myco;
{ int number_disks = number_romdisks();
  int i;

  InitList(&(romdisk_list.list));
  romdisk_list.entries = (word) number_disks;
  myco->extra = (ptr) &romdisk_list;

  for (i = 0; i < number_disks; i++)
   { ObjNode *node = (ObjNode *) malloc(sizeof(ObjNode));
     if (node eq (ObjNode *) NULL)
      { ServerDebug("Insufficient memory for romdisk devices.");
/*        longjmp(exit_jmpbuf, 1); */
	longjmp_exit;
      }
     node->size    = size_romdisk(i);
     node->account = (word) i;
     node->direntry.Name[0] = '0' + i;
     node->direntry.Name[1] = '\0';
     node->direntry.Type   = swap(Type_File);
     node->direntry.Matrix = swap(DefFileMatrix);
     node->direntry.Flags  = swap(0L);
     AddTail(&(node->node), &(romdisk_list.list));
  }

}

void RomDisk_Open(myco)
Conode *myco;
{ ObjNode *node;

  if (!strcmp(IOname, myco->name))    /* is it for the server ? */
   { NewStream(Type_Directory, Flags_Closeable, (word) myco->extra,
             PortDir_Handlers);
     return;
   }
  elif ( (node = Dir_find_node(myco) ) eq (ObjNode *) NULL)
   Request_Return(EC_Error + SS_IOProc + EG_Unknown + EO_File, 0L, 0L);
  else
   { Debug(HardDisk_Flag, ("romdisk : open") );
     NewStream(Type_File, Flags_Closeable, (word) node, Romdisk_Handlers );
   }
}

void RomDisk_Close(myco)
Conode *myco;
{
  Debug(HardDisk_Flag, ("romdisk : close") );

  if (mcb->MsgHdr.Reply ne 0L)
   Request_Return(ReplyOK, 0L, 0L);
  Seppuku();
  use(myco)
}

void RomDisk_GetSize(myco)
Conode *myco;
{ ObjNode *node = (ObjNode *) myco->extra;

  mcb->Control[Reply1_off] = (word) (node->size);
  Debug(HardDisk_Flag, ("romdisk : GetSize request, size is %lx", \
        mcb->Control[Reply1_off]) ); 
  Request_Return(ReplyOK, 1L, 0L);
  use(myco)
}

void RomDisk_Read(myco)
Conode *myco;
{ word    asked, req_pos;
  word    result;
  ObjNode *node = (ObjNode *) myco->extra;

  asked      = (mcb->Control)[ReadSize_off];
  req_pos    = (mcb->Control)[ReadPos_off];

  Debug(HardDisk_Flag, ("R : @ %ld, %ld bytes ", req_pos, asked) );

  Server_errno = EC_Error + SS_IOProc + EG_Broken + EO_File;
  
  result = read_romdisk( (int) node->account, req_pos, asked, mcb->Data);
  
  if (result < 0)
  {
    Request_Return(ReadRc_EOF, 0L, 0L);
  }
  else
  {
    Request_Return(ReadRc_EOD, 0L, result);
  }   
  use(myco)
}

#endif /* romdisk_supported */



