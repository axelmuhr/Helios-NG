/*------------------------------------------------------------------------
--                                                                      --
--			H E L I O S   S E R V E R S			--
--			---------------------------			--
--                                                                      --
--             Copyright (C) 1991, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- logger.c								--
--                                                                      --
--	Implementation of a simple error logger.			--
--                                                                      --
--	Author:  BLV 19.3.91						--
--                                                                      --
------------------------------------------------------------------------*/

/* $Header: /dsl/HeliosRoot/Helios/servers/logger/RCS/logger.c,v 1.3 1994/03/16 14:23:02 tony Exp $ */

/**
*** The error logger provides a /logger service which can be written to,
*** read from, and deleted as per the I/O Server's error logger. In addition
*** it, optionally, intercepts IOdebug() requests. Data written to the logger
*** can go to up to three different places:
*** 1) it is always held in a buffer in memory
*** 2) it may be passed on to a device driver
*** 3) it may be written to another stream, if defined.
**/

/**
*** Possible defines:
***	-DIn_Nucleus	logger is part of system image. This is normally the
***			case, since the logger is intended mainly for
***			standalone systems. 
**/

#pragma -s0	/* Suppress stack checking	*/
#pragma -f0	/* and vector stack		*/
#pragma -g0	/* Leave the names out of the code */

#include <helios.h>
#include <syslib.h>
#include <servlib.h>
#include <codes.h>
#include <gsp.h>
#include <nonansi.h>
#include <task.h>
#include <root.h>
#include <device.h>
#include <string.h>
#include "logger.h"

/**
*** A suitable Subsystem code should be added to the Master database sometime.
**/
#ifndef SS_Logger
#define SS_Logger 0x1B000000
#endif

#ifndef eq
#define eq ==
#define ne !=
#endif

/**
*** Static declarations. The error logger uses three routines of its own,
*** for Open, Delete, and ServerInfo, plus a routine for private protocol
*** message used to redirect output etc.
**/
static	void	intercept_IOdebugs(void);
static	void	write_to_log(char *str);
static	void	Logger_Private(ServInfo *info);
static	void	Logger_Open(ServInfo *info);
static	void	Logger_Delete(ServInfo *info);
static	void	Logger_ServerInfo(ServInfo *info);

#ifndef In_Nucleus
static void parse_args(void);
#endif

static	BYTE		*LogBook	= Null(BYTE);
static	int		LogSize		= 10 * 1024;
static	int		LogHead 	= 0;
static	char		*LogName	= "logger";
static	Stream		*DiagStream	= Null(Stream);
static	Stream		*DefaultStream	= Null(Stream);
static	DCB		*LogDevice	= Null(DCB);
#ifndef In_Nucleus
static	LoggerDCB	DeviceStruct;
#endif
static	bool		LeaveIOdebugs	= FALSE;
static	Port		OldIOdebugPort	= NullPort;
static	Port		MyIOdebugPort	= NullPort;
static	Semaphore	LogLock;
static	Semaphore	DeviceLock;

static ObjNode		Logger_Root;
static DispatchInfo	Logger_Info = {
	(DirNode *)	&Logger_Root,
	NullPort,
	SS_Logger,
	Null(char),
	{ Logger_Private, 2000 },
	{
		{ Logger_Open,		4000 },
		{ InvalidFn,		2000 },	/* Create	*/
		{ DoLocate,		2000 },
		{ DoObjInfo,		2000 },
		{ Logger_ServerInfo,	2000 },
		{ Logger_Delete,	2000 },
		{ InvalidFn,		2000 }, /* Rename	*/
		{ InvalidFn,		2000 }, /* Link		*/
		{ InvalidFn,		2000 }, /* Protect	*/
		{ InvalidFn,		2000 }, /* SetDate	*/
		{ InvalidFn,		2000 }, /* Refine	*/
		{ NullFn,		2000 }, /* CloseObj	*/
		{ InvalidFn,		2000 }, /* Revoke	*/
		{ InvalidFn,		2000 }, /* Reserved1	*/
		{ InvalidFn,		2000 }  /* Reserved2	*/
	}
};


int main(void)

{ Object	*name_entry = Null(Object);

#ifdef In_Nucleus
	/* It is necessary to synchronise with the processor manager */
  { MCB		m;
    InitMCB(&m, 0, MyTask->Parent, NullPort, 0x456);
    (void) PutMsg(&m);
  }
#endif

  InitSemaphore(&LogLock, 0);
  InitSemaphore(&DeviceLock, 1);

#ifndef In_Nucleus
  parse_args();
#endif
	/* The Logbook size should always be a multiple of 16 bytes.	*/
	/* This allows sensible shifting of the data.			*/
  LogSize = (LogSize + 15) & ~0x0F;
  
  LogBook = Malloc(LogSize);
  if (LogBook eq Null(BYTE))
#ifdef In_Nucleus
   Terminate();
#else
   { IOdebug("Logger: not enough memory to hold logging information.");
     Exit(0x100);
   }
#endif

  GetRoot()->ATWFlags = (WORD) LogBook;
  
  InitNode(&Logger_Root, LogName, Type_File, Flags_Interactive, DefFileMatrix);
  InitList(&Logger_Root.Contents);
  Logger_Root.Parent = Null(DirNode);
  
  { NameInfo	info;
    BYTE	buffer[IOCDataMax];
    Object	*this_processor;
    
    MachineName(buffer);
    this_processor = Locate(Null(Object), buffer);
    if (this_processor eq Null(Object))
#ifdef In_Nucleus
     Terminate();	/* Any better suggestions ? */
#else
     { IOdebug("Logger: failed to locate own processor.");
       Exit(0x100);
     }
#endif

    info.Port		= Logger_Info.ReqPort = NewPort();
    info.Flags		= Flags_StripName;
    info.Matrix		= DefNameMatrix;
    info.LoadData	= NULL;
    
    name_entry = Create(this_processor, LogName, Type_Name, 
    		sizeof(NameInfo), (BYTE *) &info);
    Close(this_processor);
  }

  unless(LeaveIOdebugs)
   intercept_IOdebugs();

  Signal(&LogLock);
  
  Dispatch(&Logger_Info);

  Delete(name_entry, Null(char));
  Close(name_entry);

  if (LogDevice ne Null(DCB))
   CloseDevice(LogDevice);

  if (MyIOdebugPort ne NullPort)
   FreePort(MyIOdebugPort);
   
  unless(LeaveIOdebugs)
   GetRoot()->IODebugPort = OldIOdebugPort;
   
  Exit(0);
}

/**
*** Parsing options. If the program is not part of the nucleus then it will
*** receive an environment complete with some options.
*** 1) -b blocksize, size of the error logger
*** 2) -n, leave IOdebug()'s alone
*** 3) device name, the name of a device driver which should be opened
***    and invoked to generate debugging.
*** In addition, the standard error stream (if any) is used as the
*** initial and default diagnostics stream.
**/
#ifndef In_Nucleus
static int  logger_atoi(char *);

static void parse_args(void)
{ Environ	env;
  int		i;
  char		*devname = Null(char);
    
  if (GetEnv(MyTask->Port, &env) < Err_Null)
   { IOputs("Logger: failed to receive environment");
     Exit(0x100);
   }
   
  for (i = 0; i < 2; i++)
   if (env.Strv[i] eq Null(Stream)) 
    goto skip_stream;
    
  if (env.Strv[2] ne (Stream *) MinInt)
   DiagStream = DefaultStream = env.Strv[2];

skip_stream:

  if (env.Argv[0] eq Null(char)) return;

     
  for (i = 1; env.Argv[i] ne Null(char); i++)
   { char *tmp = env.Argv[i];
     if (tmp[0] eq '-')
      { if (((tmp[1] eq 'n') || (tmp[1] eq 'N')) && (tmp[2] eq '\0'))
         { LeaveIOdebugs = TRUE;
           continue;
         }
        if ((tmp[1] eq 'b') || (tmp[1] eq 'B'))
         { if (tmp[2] eq '\0')
            { if (env.Argv[i+1] eq Null(char))
               { IOputs("Logger: missing logbook size");
                 Exit(0x100);
               }
              LogSize = logger_atoi(env.Argv[++i]);
            }
           else
            LogSize = logger_atoi(&(tmp[2]));
           
           if (LogSize <= 0)
            { IOdebug("Logger: invalid logbook size %d", LogSize);
              Exit(0x100);
            }
           if (LogSize <= 500)
            { IOdebug("Logger: specified size too small, defaulting to 500");
              LogSize = 500;	/* Sensible minimum */
            }
         }

        IOdebug("Logger: warning, unrecognised option %s", tmp);
      }
     else
      { if (devname ne Null(char))
         IOputs("Logger: warning, multiple device names specified");
        else
         devname = tmp;
      }
   }

  if (devname ne Null(char))
   { LogDevice = OpenDevice(devname, &DeviceStruct);
     if (LogDevice eq Null(DCB))
      { IOdebug("Logger: warning, failed to open device %s", devname);
        return;
      }
   }
}

static	int	logger_atoi(char *str)
{ int	count	= 0;
  
  if ((str[0] eq '0') && ((str[1] eq 'x') || (str[1] eq 'X')))
   for (str = &(str[2]); ; str++)
    { if ((*str >= '0') && (*str <= '9'))
       count = (16 * count) + (*str - '0');
      elif ((*str >= 'a') && (*str <= 'f'))
       count = (16 * count) + (*str - 'a' + 10);
      elif ((*str >= 'A') && (*str <= 'F'))
       count = (16 * count) + (*str - 'A' + 10);
      else
       break;
     }
  elif (str[0] eq '0')
   for (++str; ; str++)
    { if ((*str >= '0') && (*str <= '7'))
       count = (8 * count) + (*str - '0');
      else
       break;
    }
  else
   for ( ; ; str++)
    { if ((*str >= '0') && (*str <= '9'))
       count = (10 * count) + (*str - '0');
      else
       break;
    }
    
  return(count);
}

#endif

/**
*** Intercepting IOdebug()'s. This is rather easier in 1.2 than it was
*** before, as there is now a separate IOdebug port in the root data
*** structure. This port is zapped, and a process is Fork()'ed off
*** to wait for messages and handle them. At present the process assumes
*** that write_to_log() takes negligible time, i.e. that it finishes
*** before the next IOdebug message.
**/
static	void IOdebug_process(void);

static	void intercept_IOdebugs(void)
{ RootStruct	*my_root = GetRoot();

  MyIOdebugPort 	= NewPort();
  OldIOdebugPort	= my_root->IODebugPort;
  my_root->IODebugPort	= MyIOdebugPort;
  
  (void) Fork(2000, &IOdebug_process, 0);
}

	/* 512 bytes should be enough to cope with several incoming */
#define	IOdebug_Limit	512
static	BYTE	IObuffer[IOdebug_Limit + 5];

static	void	IOdebug_process(void)
{ MCB		mcb;
  int		index;
  int		result;
  int		i;

  strcpy(IObuffer, "*** ");
  index = 4;
  
  forever
   { mcb.MsgHdr.Dest	= MyIOdebugPort;
     mcb.Timeout	= -1;
     mcb.Data		= &(IObuffer[index]);

     result = GetMsg(&mcb);
     if (result < 0)
      { result &= EC_Mask;
        if ((result eq EC_Error) || (result eq EC_Fatal))
         return;
        else
         continue;
      }
     if (result ne 0x22222222)	/* Whats going on ? */
      continue;

     for (i = 0; i < mcb.MsgHdr.DataSize; i++)
      if (IObuffer[index + i] eq '\n')
       { BYTE junk = IObuffer[index + i +1];	/* Write the current line */
         IObuffer[index + i + 1] = '\0';
         write_to_log(IObuffer);
         IObuffer[index + i + 1] = junk;
         
         mcb.MsgHdr.DataSize -= (i + 1);      /* Discard bit already written */
         if (mcb.MsgHdr.DataSize > 0)
          memcpy(&(IObuffer[4]), &(IObuffer[index + i + 1]), mcb.MsgHdr.DataSize);
         index = 4;	/* after "*** " */
         i = -1;
       }
     index += mcb.MsgHdr.DataSize;
   }
}

/**-----------------------------------------------------------------------------
*** The Server library routines.
*** 1) Logger_Private() is used for controlling the diagnostics stream,
***    either redirecting it, cancelling it, or reverting it to default.
*** 2) Logger_ServerInfo() gives an indication of logger usage, as if it were
***    a file server
*** 3) Logger_Delete() is used to clear the buffer
*** 4) Logger_Open() does most of the real work.
**/

	/* This is based closely on the network software diag_ns etc */
static	void Logger_Private(ServInfo *servinfo)
{ MCB		*m		= servinfo->m;
  DirNode	*d;
  ObjNode	*f;
  IOCMsg2	*req = (IOCMsg2 *) (m->Control);
    
  d = GetTargetDir(servinfo);
  if (d eq Null(DirNode))
   { ErrorMsg(m, EO_Directory); return; }
   
  f = GetTargetObj(servinfo);
  if (f eq Null(ObjNode))
   { ErrorMsg(m, EO_File); return; }

  	/* BLV - there is an open question who should be able to redirect streams */
  if (f ne &Logger_Root)
   { ErrorMsg(m, EC_Error + EG_WrongFn + EO_Object); return; }
   
  if ((servinfo->FnCode & FG_Mask) ne FG_GetInfo)
   { ErrorMsg(m, EC_Error + EG_WrongFn + EO_Object); return; }

  if (req->Arg.Mode eq Logger_Revert)
   { DiagStream		= DefaultStream;   
     m->MsgHdr.FnRc	= 0;
     ErrorMsg(m, Err_Null);
     return;
   }

  if (req->Arg.Mode eq Logger_Clear)
   { DiagStream		= Null(Stream);
     m->MsgHdr.FnRc	= 0;
     ErrorMsg(m, Err_Null);
     return;
   }
      
  if (req->Arg.Mode eq Logger_Redirect)
   { char	*message = "Logger: output redirected\n";
     int	length	= strlen(message);
     Stream	*stream;
     word	index	= m->Control[6];
     StrDesc	*desc	= (StrDesc *) &(m->Data[index]);
     
     stream = NewStream(desc->Name, &(desc->Cap), desc->Mode);
     if (stream eq Null(Stream))
      { ErrorMsg(m, EC_Error + EG_Open + EO_Stream); return; }
      
     stream->Pos = desc->Pos;
     if (Write(stream, (BYTE *) message, length, -1) ne length)
      { ErrorMsg(m, EC_Error + EG_Open + EO_Stream);
        Close(stream);
        return;
      }
      
     if (DiagStream ne DefaultStream) Close(DiagStream);
     DiagStream		= stream;
     m->MsgHdr.FnRc	= 0;
     ErrorMsg(m, Err_Null);
     return;
   }

  if (req->Arg.Mode eq Logger_Abort)
   { m->MsgHdr.FnRc = 0;
     ErrorMsg(m, Err_Null);
     AbortPort(Logger_Info.ReqPort, EC_Fatal);
   }
      
  ErrorMsg(m, EC_Error + EG_WrongFn + EO_Object);
}

	/* ServerInfo simply returns disk statistics information.	*/
static	void Logger_ServerInfo(ServInfo *servinfo)
{ FSInfo	info;
  MCB		*m	= servinfo->m;
  DirNode	*d;
  ObjNode	*f;
  IOCMsg1	*req 	= (IOCMsg1 *) (m->Control);

  d = GetTargetDir(servinfo);
  if (d eq Null(DirNode))
   { ErrorMsg(m, EO_Directory); return; }
   
  f = GetTargetObj(servinfo);
  if (f eq Null(ObjNode))
   { ErrorMsg(m, EO_File); return; }

  if (!CheckMask(req->Common.Access.Access, AccMask_R))
   { ErrorMsg(m, EC_Error + EG_Protected + EO_File); return; }
   
  if (f ne &Logger_Root)
   { ErrorMsg(m, EC_Error + EG_WrongFn + EO_Object); return; }
   
  Wait(&LogLock);
  info.Flags		= Flags_Server;
  info.Size		= LogSize;
  info.Avail		= LogSize - LogHead;
  info.Used		= LogHead;
  Signal(&LogLock);
  
  InitMCB(m, 0, m->MsgHdr.Reply, NullPort, Err_Null);
  m->Data		= (BYTE *) &info;
  m->MsgHdr.DataSize	= sizeof(FSInfo);
  PutMsg(m);
}

static	void Logger_Delete(ServInfo *servinfo)
{ MCB		*m	= servinfo->m;
  DirNode	*d;
  ObjNode	*f;
  IOCMsg1	*req	= (IOCMsg1 *) (m->Control);

  d = GetTargetDir(servinfo);
  if (d eq Null(DirNode))
   { ErrorMsg(m, EO_Directory); return; }
   
  f = GetTargetObj(servinfo);
  if (f eq Null(ObjNode))
   { ErrorMsg(m, EO_File); return; }
   
  if (!CheckMask(req->Common.Access.Access, AccMask_D))
   { ErrorMsg(m, EC_Error + EG_Protected + EO_File); return; }
   
  if (f ne &Logger_Root)
   { ErrorMsg(m, EC_Error + EG_WrongFn + EO_Object); return; }
   
  Wait(&LogLock);
  LogHead = 0;	/* This suffices to clear the buffer */
  Signal(&LogLock);
  
  m->MsgHdr.FnRc	= 0;
  ErrorMsg(m, Err_Null);
}

static	void Logger_Read(MCB *);
static	void Logger_Write(MCB *);
static	void Logger_Seek(MCB *);
static	void Logger_GetSize(MCB *);

static	void Logger_Open(ServInfo *servinfo)
{ MCB		*m	= servinfo->m;
  MsgBuf	*r;
  DirNode	*d;
  ObjNode	*f;
  IOCMsg2	*req = (IOCMsg2 *) (m->Control);
  Port		stream_port;
  BYTE		*data = m->Data;
  char		*pathname = servinfo->Pathname;
  
  d = GetTargetDir(servinfo);
  if (d eq Null(DirNode))
   { ErrorMsg(m, EO_Directory); return; }
   
  f = GetTargetObj(servinfo);
  if (f eq Null(ObjNode))
   { ErrorMsg(m, EO_File); return; }
   
  unless(CheckMask(req->Common.Access.Access, req->Arg.Mode & Flags_Mode))
   { ErrorMsg(m, EC_Error + EG_Protected + EO_File); return; }
   
  unless (f eq &Logger_Root)
   { ErrorMsg(m, EC_Error + EG_WrongFn + EO_Object); return; }
   
  r = New(MsgBuf);
  if (r eq Null(MsgBuf))
   { ErrorMsg(m, EC_Error + EG_NoMemory + EO_Message); return; }

  FormOpenReply(r, m, f, Flags_Closeable | Flags_Interactive, pathname);
  r->mcb.MsgHdr.Reply = stream_port = NewPort();
  PutMsg(&r->mcb);
  Free(r);
  
  f->Account++;
  UnLockTarget(servinfo);
  forever
   { word	errcode;
   
     m->MsgHdr.Dest	= stream_port;
     m->Timeout		= StreamTimeout;
     m->Data		= data;
     
     errcode = GetMsg(m);
     m->MsgHdr.FnRc	= SS_Logger;
     
     if (errcode < Err_Null)
      { if (errcode eq EK_Timeout) break;
        errcode &= EC_Mask;
        if ((errcode eq EC_Error) || (errcode eq EC_Fatal)) 
         break;
        else
         continue;	/* Ignore warnings and recoverable errors */
      }
      
     if ((errcode & FC_Mask) ne FC_GSP)
      { ErrorMsg(m, EC_Error + EG_WrongFn + EO_Stream); continue; }

	/* This server does not need to lock ServInfo every time,	*/
	/* because a separate server maintains the locking.		*/      
     switch(errcode & FG_Mask)
      { case FG_Read	: Logger_Read(m); break;
      	case FG_Write	: Logger_Write(m); break;
      	case FG_Close	:
      			  if (m->MsgHdr.Reply ne NullPort)
      			   { m->MsgHdr.FnRc = 0;
      			     ErrorMsg(m, Err_Null);
      			   }
      			  FreePort(stream_port);
      			  f->Account--;
      			  return;

	case FG_Seek	: Logger_Seek(m); break;
	
	case FG_GetSize	: Logger_GetSize(m); break;
	
	default		: ErrorMsg(m, EC_Error + EG_WrongFn + EO_Stream);
			  continue;
      }
   }
   
  f->Account--;
  FreePort(stream_port);
}

static	void Logger_Seek(MCB *m)
{ SeekRequest	*req = (SeekRequest *) m->Control;
  word		newoff;
  
  Wait(&LogLock);
  
  if (req->Mode eq S_Beginning)
   newoff = req->NewPos;
  elif (req->Mode eq S_Relative)
   newoff = req->CurPos + req->NewPos;
  elif (req->Mode eq S_End)
   newoff = LogHead;
  else
   { ErrorMsg(m, EC_Error + EG_Parameter + 1); goto done; }

  if ((newoff < 0) || (newoff > LogHead))
   { ErrorMsg(m, EC_Error + EG_WrongSize + EO_File); goto done; }

  InitMCB(m, 0, m->MsgHdr.Reply, NullPort, Err_Null);   
  MarshalWord(m, newoff);
  PutMsg(m);
  
done:  
  Signal(&LogLock);
}

static	void Logger_GetSize(MCB *m)
{ Wait(&LogLock);
  InitMCB(m, 0, m->MsgHdr.Reply, NullPort, Err_Null);
  MarshalWord(m, LogHead);
  PutMsg(m);
  Signal(&LogLock);
}

static	void Logger_Read(MCB *m)
{ ReadWrite	*req	= (ReadWrite *) m->Control;
  Port		reply	= m->MsgHdr.Reply;
  int		i;
  
  Wait(&LogLock);
  if (req->Pos >= LogHead)
   { InitMCB(m, 0, reply, NullPort, ReadRc_EOF);
     PutMsg(m);
     goto done;
   }

  if (req->Pos < 0)
   { ErrorMsg(m, EC_Error + EG_Parameter + 1); goto done; }
   
  if (req->Size < 0)
   { ErrorMsg(m, EC_Error + EG_Parameter + 2); goto done; }
   
  if (req->Pos + req->Size > LogHead)
   req->Size = LogHead - req->Pos;
   
	/* Send the data in chunks of at most 16 K */
#define	ChunkSize	16384	  
  for (i = 0; i < req->Size; i += ChunkSize)
   { int	size = ChunkSize;
     bool	lastmsg = FALSE;
     
     if ((i + ChunkSize) > req->Size)
      { size = req->Size - i; lastmsg = TRUE; }
      
     InitMCB(m, lastmsg ? 0 : MsgHdr_Flags_preserve, reply, NullPort,
     		lastmsg ? ReadRc_EOD : ReadRc_More);
     m->MsgHdr.DataSize = size;
     m->Data = &(LogBook[req->Pos + i]);
     PutMsg(m);
   }
#undef ChunkSize

done:
  Signal(&LogLock); 
}

static void Logger_Write(MCB *m)
{ ReadWrite	*req	= (ReadWrite *) m->Control;
  BYTE		*buffer, *ptr;
  bool		ownbuf	= FALSE;
  Port		reply	= m->MsgHdr.Reply;
  Port		myport	= m->MsgHdr.Dest;
  word		amount, fetched;

  	/* Completely ignore the position, it is irrelevant to the logger */

 amount = req->Size; 

	/* Cope with immediate and non-immediate data */
 if (m->MsgHdr.DataSize ne 0)	/* immediate */
   buffer = m->Data;
 else
  { if ((buffer = Malloc(req->Size + 1)) eq Null(BYTE))
     { ErrorMsg(m, EC_Error + EG_NoMemory + EO_Message); return; }
    ownbuf = TRUE;

#define	ChunkSize	16384    
    InitMCB(m, MsgHdr_Flags_preserve, reply, NullPort, WriteRc_Sizes);
    MarshalWord(m, (amount > ChunkSize) ? ChunkSize : amount);
    MarshalWord(m, ChunkSize);
    PutMsg(m);

    ptr = buffer; fetched = 0;
    while (fetched < amount)
     { m->MsgHdr.Dest	= myport;
       m->Data		= ptr;
       if (GetMsg(m) < 0)
        goto done;
       fetched += m->MsgHdr.DataSize;
       ptr = &(ptr[m->MsgHdr.DataSize]);
     }
  }
 	/* Acknowledge the Write	*/
  InitMCB(m, 0, reply, NullPort, WriteRc_Done);
  MarshalWord(m, amount);
  PutMsg(m);

	/* send the data to the error logger, suitably null-terminated	*/   
	/* BLV - possible problems with exactly 512 bytes...		*/
  buffer[amount] = '\0';
  write_to_log(buffer);
  
done:
  if (ownbuf) Free(buffer);     
}

/**----------------------------------------------------------------------------
*** Writing to the log, the useful bit of code.
***
*** 1) this routine accepts a string. The length has to be determined.
*** 2) this string must be copied into the main buffer. This may require
***    suitable manipulation of the buffer. In addition, it may require
***    some manipulation of the string if that is too big to fit into the
***    buffer at all. 
*** 3) next, if there is a device driver then this is invoked. Typical
***    device drivers might write the data to non-volatile ram or to a
***    a link.
*** 4) finally, if there is currently a diagnostics stream then the data
***    is written to that stream.
**/

static	void write_to_log(char *str)
{ int	length		= strlen(str);
  char	*book_str	= str;

  Wait(&LogLock);

  	/* BLV - should a single Write be able to zap the whole buffer ? */
  if (length >= LogSize)
   { book_str	= &(str[length - LogSize]);
     length	= LogSize;
     LogHead	= 0;
   }

	/* Check whether or not there is enough space to hold the new data */   
	/* If not, the logbook is shifted in blocks of 1/16 of its size to */
	/* make space. This may leave slightly funny data at the start of  */
	/* the file, and may cause confusion if reading a file while it is */
	/* being shifted. However, it is rather easy to implement.	   */
  if ((length + LogHead) > LogSize)
   { int shift		= 0;
     int shiftsize	= LogSize / 16;
     int oldhead	= LogHead;
     
     until ((length + LogHead) <= LogSize)
      { shift	+= shiftsize;
        LogHead	-= shiftsize;
      }
     if (LogHead < 0) LogHead = 0;
     if (oldhead > shift)
      memcpy(LogBook, &(LogBook[shift]), oldhead - shift);
   }

	/* Copy the data into the buffer.	*/   
  memcpy(&(LogBook[LogHead]), book_str, length);
  LogHead += length;
  
 	/* The logbook is now in a sensible state.	*/
  Signal(&LogLock);

	/* If there is a device driver, activate it.	*/
  if (LogDevice ne Null(DCB))
   { Wait(&DeviceLock);
     Operate(LogDevice, str);
     Signal(&DeviceLock);
   }

	/* If there is a diagnostics stream, write the data to it. */
  if (DiagStream ne Null(Stream))
   { length = strlen(str);
     (void) Write(DiagStream, (BYTE *) str, length, OneSec);
   }  
}

