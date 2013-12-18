/*------------------------------------------------------------------------
--                                                                      --
--                     H E L I O S   S E R V E R  T A S K               --
--                     ----------------------------------               --
--                                                                      --
--             Copyright (C) 1988, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- servtask.c								--
--                                                                      --
--	The initialisation program for use with the                	--
--	Mini Server.							--
--                                                                      --
--	Author:  BLV 15/8/88						--
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Id: servtask.c,v 1.3 1990/10/18 13:07:05 bart Exp $ (C) Copyright 1988, Perihelion Software Ltd. */ 
/* SccsId: 1.4 14/3/90 Copyright (C) 1989, Perihelion Software Ltd. */

#include <syslib.h>
#include <gsp.h>
#include <root.h>
#include <servlib.h>
#include <sem.h>
#include <nonansi.h>
#include <string.h>
#include <config.h>
#include <link.h>
#include <codes.h>
#include <attrib.h>
#include <ioevents.h>
#include <asm.h>
#include <signal.h>

#define GLOBAL
#include "servtask.h"

/**
*** BLV - here are a couple of early odds and ends for the MiniServer bits of
*** the world.
**/
PRIVATE void		StartServers(void);
PRIVATE WORD		Server_Flags;

int main(void)
{ int running = FALSE;

	/* only start up this system if we have been booted from an IOproc */
  { LinkInfo **linkv = GetRoot()->Links;
    while (*linkv != Null(LinkInfo))
     if ((*linkv)->Flags & Link_Flags_parent)
      if ((*linkv)->Flags & Link_Flags_ioproc)
	/* Start up all the device servers. */
       { StartServers(); running = TRUE; break; }
  }

  { MCB m;   /* synchronise with processor manager to keep it happy */
    word e;
    InitMCB(&m, 0, MyTask->Parent, NullPort, 0x456);
    e = PutMsg(&m);
  }
 
 if (running)
  { extern void StopProcess(void);   /* undocumented kernel call */
    StopProcess();
  }
 else
  Exit(0);
}

		/* This routine creates a new name in the name table. */
PRIVATE void NewName(string name, Port port, word matrix)
{ NameInfo info;
  Object   *processor = Locate(Null(Object), Machine_Name);
  
  if (processor == Null(Object)) return;
  
  info.Port	= port;
  info.Flags	= Flags_StripName;
  info.Matrix	= matrix;
  info.LoadData = NULL;		/* Not used at present */

  (void) Create(processor, name, Type_Name, sizeof(NameInfo), (byte *)&info);
  Close(processor);
}


/*-------------------------------------------------------------------------
---									---
--- Here is the code to interact with the Mini Server			---
---									---
-------------------------------------------------------------------------*/

/**
*** Error code : Server Task Timeout
**/
#define ST_Timeout (EC_Recover + SS_IOProc + EG_Timeout + EO_Link)

/**
*** #define's missing from the header files
**/
#define Link_Mode_dumb	1
#define Link_State_dumb	2

/**
*** Mini server protocol definition
**/
#define Pro_Synch	0
#define Pro_IOServ	1
#define Pro_Command	2
#define Pro_Message	3
#define Pro_Poll	4
#define Pro_Die		5
#define Pro_Sleep	6
#define Pro_Wakeup	7

#define Fun_OpenFile	  1
#define Fun_CloseFile	  2
#define Fun_ReadFile	  3
#define Fun_WriteFile	  4
#define Fun_SeekInFile	  5
#define Seek_start	  1
#define Seek_end	  2
#define Fun_CreateFile	  6
#define Fun_DeleteFile	  7
#define Fun_Rename	  8
#define Fun_Locate	  9
#define Fun_ReadDir	 10
#define Fun_CreateDir	 11
#define Fun_RemoveDir	 12
#define Fun_FileInfo	 13
#define Fun_DiskUsage	 14
#define Fun_ScreenWrite	 15
#define Fun_ChangeDate   16

#define Rep_Success	128
#define Rep_NotFound	129

	/* Device numbers for the various polling devices */
	/* At present only console, but others e.g. RS232 may be added later. */
#define Poll_Console	1

/**
*** There are problems with sizeof, sizeof(FullHead) gives 8 because Norcroft C
*** allocates two words for it. Hence I use sizeofFullHead rather than sizeof().
**/
typedef struct FullHead {
	BYTE	protocol;
	BYTE	fncode;
	BYTE	extra;
	UBYTE	highsize;
	UBYTE	lowsize;
} FullHead;
#define sizeofFullHead		5

typedef struct Head	{
	BYTE	fncode;
	BYTE	extra;
	UBYTE	highsize;
	UBYTE	lowsize;
} Head;
#define sizeofHead		4

#define Nothing_t	0
#define File_t		1
#define Dir_t		2

/**
*** Miniserver structure declarations
**/
typedef struct Device {
	Port		Port;
	char		Name[32];		/* E.g. helios or console */
	VoidFnPtr	Handlers[12];
} Device;

typedef struct Message {
	MCB	mcb;
	WORD	control[IOCMsgMax];
	BYTE	data[IOCDataMax];
} Message;

typedef struct DirStream {
	WORD	 number;
	WORD	 offset;
	BYTE	 entries[1];
} DirStream;

/**
*** Utility function declarations.
**/
PRIVATE void InitFileIO(void);
PRIVATE void TakeOverLink0(void);
PRIVATE	void InterceptRemote(void);
PRIVATE void InitialiseServerTask(void);
PRIVATE void InitialiseServers(void);
PRIVATE void SynchProcess(void);
PRIVATE void CommandProcess(void);
PRIVATE void ExecuteCommand(BYTE *command, WORD id);
PRIVATE void MessageProcess(void);
PRIVATE void PollingProcess(void);
PRIVATE void SetStaticsProcess(void);
PRIVATE void GSPServer(Device *, WORD);
PRIVATE void GSPWorker(Device *, MCB *);

PRIVATE string GetFullName(STRING, MCB *);
PRIVATE WORD   flatten(string);
PRIVATE string GetLocalName(string);
PRIVATE string lastbit(string);

PRIVATE bool ByteDownLink(WORD);
PRIVATE WORD ByteFromLink(void);
PRIVATE bool DataDownLink(BYTE *buffer, WORD amount);
PRIVATE bool DataFromLink(BYTE *buffer, WORD amount);

PRIVATE bool LinkWait(void);
PRIVATE void LinkSignal(void);

/**
*** These are the "local" routines.
**/
PRIVATE WORD exists_obj(STRING localname, int *timedout);
PRIVATE int  open_file(STRING localname, WORD openmode, int *timedout);
PRIVATE WORD seek_in_file(int stream, WORD mode, WORD NewPos, int *timedout);
PRIVATE WORD read_from_file(int stream, BYTE *buffer, WORD amount, int *timedout);
PRIVATE WORD write_to_file(int stream, BYTE *buffer, WORD amount, int *timedout);
PRIVATE void close_file(int stream, int *timedout);
PRIVATE DirStream *read_dir(STRING localname, int *timedout);
PRIVATE bool create_object(STRING localname, WORD type, int *timedout);
PRIVATE bool get_file_info(STRING localname, WORD *sizeptr, Date *dateptr, int *timedout);
PRIVATE bool delete_object(STRING localname, WORD exists, int *timedout);
PRIVATE bool rename_file(STRING fromname, STRING toname, int *timedout);
PRIVATE bool drive_statistics(STRING localname, WORD *sizeptr, WORD *availptr, int *timedout);
PRIVATE bool change_date(STRING localname, int *timedout);

/**
*** Variable declarations.
**/
PRIVATE Semaphore	ProcessesReady;
PRIVATE Semaphore	LinkAvailable;
PRIVATE Semaphore	LinkWaitLock;
PRIVATE Semaphore	ReplyAvailable;
PRIVATE Semaphore	CommandAvailable;
PRIVATE Semaphore	Done;
PRIVATE Semaphore	MessageAvailable;
PRIVATE Semaphore	PollingMessage;
PRIVATE Semaphore	Machine_Name_Sem;
PRIVATE Semaphore	Wakeup;

PRIVATE BYTE		Helios_Directory[128];
PRIVATE Date		Server_StartTime;
PRIVATE WORD		Processes_count = 0;
PRIVATE BYTE            *misc_buffer;
PRIVATE WORD		no_of_drives;

/**
*** Server dispatch info declarations.
**/

extern DispatchInfo Console_Info, Window_Info, Logger_Info;
extern void InitialiseWindowing(void);
extern void write_to_log(string);

PRIVATE void InvalidFun(MCB *, string);	

PRIVATE DirNode Message_Root;
PRIVATE void Message_Open(ServInfo *);
PRIVATE void Message_Forward(MCB *);

PRIVATE DispatchInfo Message_Info = {
	&Message_Root,
	NullPort,
	SS_IOProc,
	Machine_Name,
	{ NULL, 0 },
	{
		{ Message_Open,		4000 },
		{ InvalidFn,		2000 },
		{ DoLocate,		2000 },
		{ DoObjInfo,		2000 },
		{ NullFn,		2000 },
		{ InvalidFn,		2000 },
		{ DoRename,		2000 },
		{ DoLink,		2000 },
		{ DoProtect,		2000 },
		{ DoSetDate,		2000 },
		{ DoRefine,		2000 },
		{ NullFn,		2000 }
	}
};



PRIVATE void Drive_Open(MCB *, string);
PRIVATE void File_Open(MCB *, string, string, WORD openmode);
PRIVATE void Dir_Open(MCB *, string, string);
PRIVATE void Drive_Locate(MCB *, string);
PRIVATE void Drive_Create(MCB *, string);
PRIVATE void Drive_ObjInfo(MCB *, string);
PRIVATE void Drive_ServerInfo(MCB *, string);
PRIVATE void Drive_Delete(MCB *, string);
PRIVATE void Drive_Rename(MCB *, string);
#define Drive_Link	InvalidFun
#define Drive_Protect	InvalidFun
PRIVATE void Drive_SetDate(MCB *, string);
#define Drive_Refine	InvalidFun
PRIVATE void File_Read(MCB *, int);
PRIVATE void File_Write(MCB *, int);
PRIVATE void File_Seek(MCB *, int);
PRIVATE void File_GetSize(MCB *, int);
PRIVATE void Dir_Read(MCB *, DirStream *);
PRIVATE void Dir_Seek(MCB *, DirStream *);

PRIVATE Device Helios = {
	NullPort,
	"helios",
	{   	Drive_Open,
		Drive_Create,
		Drive_Locate,
		Drive_ObjInfo,
		Drive_ServerInfo,
		Drive_Delete,
		Drive_Rename,
		Drive_Link,
		Drive_Protect,
		Drive_SetDate,
		Drive_Refine,
		NullFn
	}
   
};


/**
*** The Server start-up routine. It begins by initialising all the semaphores
*** used. Then it takes over link 0, initialises the servers for the console
*** device and the various disk drives, installs a link guardian process
*** which analyses data coming down the link, sets up the ANSI terminal
*** emulation routines, and waits for a bit to allow the system to settle down.
*** Then it returns to main(), where the system continues to load the window
*** manager and run the shell.
**/

PRIVATE void MyLinkGuardian(void);

PRIVATE void StartServers(void)
{ int i;

  InitSemaphore(&ProcessesReady, 0);
  InitSemaphore(&LinkAvailable, 0);
  InitSemaphore(&LinkWaitLock, 1);
  InitSemaphore(&ReplyAvailable, 0);
  InitSemaphore(&Done, 0);
  InitSemaphore(&CommandAvailable, 0);
  InitSemaphore(&MessageAvailable, 0);
  InitSemaphore(&PollingMessage, 0);
  InitSemaphore(&Wakeup, 0);

  TakeOverLink0();

  Server_StartTime = GetDate();

  InitFileIO();
  
  InitialiseServerTask();

	  /* Install a process which waits on bytes arriving down the link */
  Fork(Stacksize, MyLinkGuardian, 0); Processes_count++;

	/* Allow all the various processes to use the link */
  Signal(&LinkAvailable);

  InitialiseWindowing();
  
  InitialiseServers();
  
		/* Wait for all the processes to initialise */
  for (i = 0; i < Processes_count; i++) Wait(&ProcessesReady);
}

/**
*** This process waits for data to come down the link. This data should always
*** be a header byte for one of the protocols, and the appropriate process is
*** enabled. The link guardian must not look at any more data arriving down the
*** link until the protocol transaction is completed.
**/

PRIVATE void MyLinkGuardian(void)
 { 
  WORD HeaderByte;

  Signal(&ProcessesReady);
  
  forever
   { 
     HeaderByte = ByteFromLink();

     switch(HeaderByte)
      { case Pro_IOServ	 :
      			  Signal(&ReplyAvailable);
			  Wait(&Done);
			  break;

	case Pro_Command : Signal(&CommandAvailable);
			  Wait(&Done);
			  break;

	case Pro_Message : Signal(&MessageAvailable);
			  Wait(&Done);
			  break;

	case Pro_Poll	 : Signal(&PollingMessage);
			  Wait(&Done);
			  break;

	case Pro_Wakeup	 : Signal(&Wakeup);
			   break;
			   
	case Pro_Synch	: break;	/* Synchronisation byte */
	default		: 
	                  break;
     }     
   }
}

/**
*** This routine claims link 0 of the root transputer which is configured as
*** a dumb link by the miniserver. 
**/

PRIVATE void TakeOverLink0(void)
{
  (void) AllocLink(BootLink);

  Fork(Stacksize, InterceptRemote, 0); Processes_count++;
}

/**
*** This process installs a new port as the Remote IOCPort for link 0 and
*** waits on that port for debugging messages from IOdebug and IOputc.
*** This data is buffered and written to the screen once a line has been
*** received. The start of the buffer is always "*** ". Writing to a screen
*** always involves a protocol interaction - if any messages arrive during this
*** time, tough. The process also deals with the special case of bytes -1 and
*** bytes -2, -1 being used to exit and -2 being used to go to sleep.
**/

PRIVATE void do_IOdebug(STRING);
PRIVATE void do_die(void);
void do_sleep(void);

PRIVATE void InterceptRemote(void)
{ Port myport = NewPort();
  MCB  mcb;
  BYTE *buffer;
  WORD count = 0, result;

  if (myport eq NullPort) return;
  (GetRoot())->Links[BootLink]->RemoteIOCPort = myport;

  for (buffer = (BYTE *) Malloc(200); buffer eq Null(BYTE); Delay(OneSec))
     buffer = (BYTE *) Malloc(200);

  Signal(&ProcessesReady);
  
  forever
   { mcb.MsgHdr.Dest 	= myport;
     mcb.Timeout	= -1;
     mcb.Data		= &(buffer[count]);
     if ((result = GetMsg(&mcb)) < 0)
      {
        if (result eq -1)
          do_die();
        elif (result eq -2)
          do_sleep();
        else
          continue;
      }

     if (result eq 0x22222222)		/* Is it an IOputc message ?	*/
      { if (buffer[count] eq '\n')	/* Yes, end of line ?		*/
         { buffer[++count] = '\r';	/* Yes, so send to IOProc	*/
           buffer[++count] = '\0';
           (void) Fork(Stacksize, do_IOdebug, 4, buffer);

           for (buffer = (BYTE *) Malloc(200); buffer eq Null(BYTE); )
             buffer = (BYTE *) Malloc(200);
	   count = 0;
         }
        else
         count++;			/* No, just buffer		*/
      }					/* No, so ignore the message	*/
   }
}

PRIVATE void do_IOdebug(string str)
{ BYTE buffer[200];
  strcpy(buffer, "*** ");
  strcat(buffer, str);
  write_to_log(buffer);
  Free(str);
}

PRIVATE void do_die(void)
{ while (!LinkWait());
  while (!ByteDownLink(Pro_Die));
  Terminate();
}

/**
**/

extern void switch_window(int);
void do_sleep(void)
{ Mode = Mode_Background;
  while (!LinkWait());
  while (!ByteDownLink(Pro_Sleep));
  LinkSignal();
  Wait(&Wakeup);
  Mode = Mode_Foreground;
  switch_window(0);
}

/**
*** This routine is responsible for starting communications with the mini
*** server, and for dispatching all the servers.
**/

PRIVATE void CreateServer(WORD type, string name, DispatchInfo *info)
{ DirNode *node = info->Root;
  Port    reqport = NewPort();
  WORD    matrix = (type eq Type_File) ? DefFileMatrix : DefDirMatrix;

  InitNode( (ObjNode *) node, name, type, 0, matrix);

  info->ReqPort = reqport;
  NewName(name, reqport, matrix);
  
  Fork(Stacksize, Dispatch, 4, info);  
}

PRIVATE BYTE init_buffer[256];
PRIVATE void InitialiseServerTask(void)
{ 
  WORD size;

  while (!ByteDownLink(0x00F1));

  while (ByteFromLink() ne 0x00F2);

  size = ByteFromLink();

  (void) DataFromLink(&(init_buffer[0]), size);

  Host = (init_buffer[0] eq DP2) ? DP2 : PC;

  Server_Flags = init_buffer[1];

  no_of_drives = init_buffer[2];
  Message_Limit = (init_buffer[3] << 24) + (init_buffer[4] << 16) +
  		(init_buffer[5] << 8) + init_buffer[6] + 2;
  strcpy(Helios_Directory, &(init_buffer[7]));
  misc_buffer = Malloc(Message_Limit + 128);   /* bit of leeway */
  MachineName(Machine_Name);

  if (Server_Flags & Config_flags_Nopop)
    windows_nopop = TRUE;
  else
    windows_nopop = FALSE;
  
  if (Server_Flags & Config_flags_Background)
    Mode = Mode_Background;
  else
    Mode = Mode_Foreground;
    
  (void) Fork(Stacksize, SetStaticsProcess, 0);	Processes_count++;
  if (Host eq DP2)
   { (void) Fork(Stacksize, SynchProcess, 0);	Processes_count++;
     (void) Fork(Stacksize, CommandProcess, 0);	Processes_count++;
     (void) Fork(Stacksize, MessageProcess, 0);	Processes_count++;
   }
  (void) Fork(Stacksize, PollingProcess, 0);	Processes_count++;
}

PRIVATE void InitialiseServers(void)
{ WORD i, index;

#ifdef NEVER
  CreateServer(Type_File, "console", &Console_Info);
#else
  CreateServer(Type_Directory, "window", &Window_Info);
#endif
    
  CreateServer(Type_File, "logger",  &Logger_Info);

  if (Host eq DP2)
   CreateServer(Type_File, (Host eq DP2) ? "DP2Mess" : "Message", &Message_Info);

  (void) Fork(Stacksize, GSPServer, 8, &Helios,  DefDirMatrix);  Processes_count++;

  index = 8 + strlen(Helios_Directory);

  for (i = 0; i < no_of_drives; i++)
   { Device *drive = Malloc(sizeof(Device));
     if (drive ne NULL)
       { memcpy((BYTE *) drive, (BYTE *) &Helios, sizeof(Device));
	 strcpy(&(drive->Name[0]), &(init_buffer[index]));
	 (void) Fork(Stacksize, GSPServer, 8, drive, DefDirMatrix);
	        Processes_count++;
	 index += strlen(&(init_buffer[index])) + 1;
       }
   }
}

/**
*** This process deals with static variables that need to be reset at regular
*** intervals. At the moment this affects only the processor name, which I
*** need so that I can give it in replies to Open, Locate and Create. I refresh
*** this name every 60 seconds.
**/

PRIVATE void SetStaticsProcess(void)
{ BYTE buffer[128];

  InitSemaphore(&Machine_Name_Sem, 1);

  Signal(&ProcessesReady);
  
  forever
   { 
     Delay(30 * OneSec);
     MachineName(&(buffer[0]));
     Wait(&Machine_Name_Sem);
     strcpy(&(Machine_Name[0]), &(buffer[0]) );     
     Signal(&Machine_Name_Sem);
   }
}

/**
*** This process deals with sending synchronisation bytes. The link guardian
*** process, which is never suspended indefinitely, keeps track of the time
*** since the last synchronisation, and every second it signals that a
*** synchronisation byte is required. This process waits for this signal, then
*** for the link to be ready, and sends a byte 0 down the link. Obviously it
*** needs to return the link to the system as soon as it is finished with it.
**/

PRIVATE void SynchProcess(void)
{ Signal(&ProcessesReady);
  forever
    { 
      Delay(5 * OneSec);
      if (!LinkWait()) continue;
      ByteDownLink(0);
      LinkSignal();
    }
}

/**
*** This deals with commands coming down the link. The entire command is read
*** in, and then a process is forked off to interpret and execute it.
**/


PRIVATE void CommandProcess(void)
{ BYTE *ptr = Null(BYTE);

  while ((ptr = Malloc(Command_Max)) eq Null(BYTE))
    Delay(OneSec);
  
  Signal(&ProcessesReady);
  forever
   { Head head;
     WORD size;

     Wait(&CommandAvailable);

     (void) DataFromLink( (BYTE *) &head, sizeofHead);
     size = (head.highsize << 8) + head.lowsize;
     if (size ne 0)
       (void) DataFromLink( ptr, size);
     ptr[size] = '\0';
     
     Signal(&Done);

     Fork(2 * Stacksize, ExecuteCommand, 8, ptr, head.extra);
     while ((ptr = Malloc(Command_Max)) eq Null(BYTE))
       Delay(OneSec);     
   }
}

/**
*** This code is responsible for executing commands sent by the host.
**/

PRIVATE int process_args(BYTE *string, char *command, char *currentdir,
                         char **streams, char **args);
PRIVATE void send_head(FullHead *, int);

PRIVATE void ExecuteCommand(BYTE *string, WORD id)
{ FullHead head;
  Object   *tfm = Null(Object);
  Object   *prog = Null(Object), *code = Null(Object);
  Stream   *s;
  Object   *objv[2];
  Stream   *strv[4];
  Environ  env;
  char     *dummy = Null(char);
  char     *argv[16];
  char     *streams[4];
  char     command[128];
  char     currentdir[128];
  word     temp;
    
  objv[0] = Null(Object); strv[0] = Null(Stream);
  strv[1] = Null(Stream); strv[2] = Null(Stream);
  objv[1] = Null(Object); strv[3] = Null(Stream);
    
  head.protocol = Pro_Command;
  head.extra    = id;
  head.highsize = 0;
  head.lowsize  = 0;
  
  env.Argv = argv;
  env.Envv = &dummy;
  env.Objv = objv;
  env.Strv = strv;

  if (!process_args(string, command, currentdir, streams, argv))
   { send_head(&head, 0x81); return; }

  objv[0] = Locate(NULL, currentdir);
  if (objv[0] eq Null(Object))
   { send_head(&head, 0x82); return; }
  objv[1] = Null(Object);

  strv[0] = Open(objv[0], streams[0], O_ReadOnly);
  if (strv[0] eq Null(Stream))
   { send_head(&head, 0x83); goto end; }
  strv[1] = Open(objv[0], streams[1], O_WriteOnly + O_Create);
  if (strv[1] eq Null(Stream))
   { send_head(&head, 0x84); goto end; }
  strv[2] = Open(objv[0], streams[2], O_WriteOnly + O_Create);
  if (strv[2] eq Null(Stream))
   { send_head(&head, 0x85); goto end; }
   
  code = Locate(objv[0], command);
  if (code eq Null(Object))
   { send_head(&head, 0x86); goto end; }
   
  tfm = Locate(NULL, "/tfm");     /* may fail, if so run command locally */

  prog = Execute(tfm, code);
  if (prog eq Null(Object))
   { send_head(&head, 0x87); goto end; }
   
  s = Open(prog, Null(char), O_ReadWrite);
  if (s eq Null(Stream))
   { send_head(&head, 0x88); goto end; }
   
  if (SendEnv(s->Server, &env) < 0)
   { send_head(&head, 0x89); goto end; }

  send_head(&head, 0x01);
  
  if (InitProgramInfo(s, PS_Terminate) < 0)
   { (void) SendSignal(s, SIGTERM);
     send_head(&head, 0x8a);
     goto end;
   }
   
  (void) GetProgramInfo(s, &temp, -1);
  send_head(&head, 0x02);
  
end:  
  if (objv[0] ne Null(Object)) Close(objv[0]);
  if (strv[0] ne Null(Stream)) Close(strv[0]);
  if (strv[1] ne Null(Stream)) Close(strv[1]);
  if (strv[2] ne Null(Stream)) Close(strv[2]);
  if (tfm  ne Null(Object)) Close(tfm);
  if (code ne Null(Object)) Close(code);
  if (prog ne Null(Object)) Close(prog);
  if (s    ne Null(Stream)) Close(s);
}

PRIVATE void send_head(FullHead *head, int code)
{ head->fncode = code;
  while (!LinkWait());
  (void) DataDownLink((BYTE *) head, sizeofFullHead);
  LinkSignal();
}

PRIVATE BYTE *skip_blanks(BYTE *string)
{ while ((*string eq ' ') || (*string eq '\t') || (*string eq '\r') ||
         (*string eq '\n') || (*string eq '\v') || (*string eq '\f'))
   string++;
  return(string);
}

PRIVATE BYTE *find_blank(BYTE *string)
{ while ((*string ne ' ') && (*string ne '\t') && (*string ne '\r') &&
         (*string ne '\n') && (*string ne '\v') && (*string ne '\f') &&
         (*string ne '\0'))
   string++;
  return(string);
}

PRIVATE int process_args(BYTE *string, BYTE *command, BYTE *currentdir,
                         BYTE **streams, BYTE **argv)
{ BYTE *end;
  BYTE temp;
  int  argv_off = 1;

  streams[0] = "/null";
  streams[1] = "/logger";
  streams[2] = "/logger";
  streams[3] = Null(BYTE);
  strcpy(currentdir, "/helios");
  
  string = skip_blanks(string);
  if (*string eq '\0') return(0);
  end    = find_blank(string);
  temp   = *end;
  *end   = '\0';
  if (*string eq '/')
   { BYTE *str = end;
     strcpy(command, string);
     while (*(--str) ne '/');
     str++;
     argv[0] = str;
   }
  else
   { strcpy(command, "/helios/bin/");
     strcat(command, string);
     argv[0] = string;
   }

  if (temp ne '\0')
    string = skip_blanks(++end);
  else
    string = end;
  
  while (*string ne '\0')
   { if (*string eq '{')         /* start of stream list */
      { int stream_count = 0;
        string = skip_blanks(++string);

        forever
         { if (*string eq '}') { string = skip_blanks(++string); break; }
         
           if (stream_count >= 3) return(0);
           if (*string eq ',')
            { stream_count++; string = skip_blanks(++string); continue; }
            
           for (end = string;
                ((*end ne ' ') && (*end ne '\t') && (*end ne '\r') &&
                  (*end ne '\n') && (*end ne '\v') && (*end ne '\f') &&
                  (*end ne ',') && (*end ne '}') && (*end ne '\0') );
                end++);
           
           temp = *end;
           if (temp eq '\0') return(0);
           *end = '\0';

           streams[stream_count++] = string;
           string = skip_blanks(++end);
           if (temp eq '}') break;
           if ((*string eq ',') && (temp ne ',')) string = skip_blanks(++string);
         }
      }
     else
      { end  = find_blank(string);
        temp = *end;
        *end = '\0';
        if (*string eq '@')   /* current directory */
         { if (*(++string) eq '\0') return(0);
           if (*string eq '/')
            strcpy(currentdir, string);
           else
            { strcat(currentdir, "/"); strcat(currentdir, string); }
         }
        else
         argv[argv_off++] = string;

        string = (temp eq '\0') ? end : skip_blanks(++end);
      }
   }     
  argv[argv_off] = Null(BYTE);
  return(1);
}
/**
*** This process deals with Helios messages coming down the link. After
*** the protocol byte there is an entire Helios message, i.e. the
*** message header flags byte, the control vector size byte, the data vector
*** short int with leading byte first, the Destination and Reply Ports,
*** the function code, possibly a control vector, and possibly a data vector.
*** All the memory is allocated as and when required. When the entire message
*** has been received I fork off a process to send it.
**/

PRIVATE void MySendMessage(MCB *);

PRIVATE void MessageProcess(void)
{ MCB *mcb; int temp;

  Signal(&ProcessesReady);

  forever
   { for ( mcb = (MCB *) Malloc(sizeof(MCB)); mcb eq Null(MCB);
   	   mcb = (MCB *) Malloc(sizeof(MCB)) )
       Delay(OneSec);
       
     Wait(&MessageAvailable);

     mcb->MsgHdr.Flags = ByteFromLink();
     mcb->MsgHdr.ContSize = ByteFromLink();
     temp = ByteFromLink();
     temp = (temp << 8) + ByteFromLink();
     mcb->MsgHdr.DataSize = temp;
     (void) DataFromLink( ((BYTE *) mcb) + 4, 12);
     if (mcb->MsgHdr.ContSize ne 0)
      { for ( mcb->Control = (WORD *) Malloc(mcb->MsgHdr.ContSize * sizeof(WORD));
              mcb->Control eq Null(WORD);
              mcb->Control = (WORD *) Malloc(mcb->MsgHdr.ContSize * sizeof(WORD))
            ) Delay(OneSec / 2);
        (void) DataFromLink( (BYTE *) mcb->Control,
               mcb->MsgHdr.ContSize * sizeof(WORD));
      }
     if (mcb->MsgHdr.DataSize ne 0)
      { for ( mcb->Data = Malloc(mcb->MsgHdr.DataSize);
              mcb->Data eq Null(BYTE);
              mcb->Data = Malloc(mcb->MsgHdr.DataSize)
            ) Delay(OneSec / 2);
        (void) DataFromLink( mcb->Data, mcb->MsgHdr.DataSize);
      }

     Fork(Stacksize, (VoidFnPtr) &MySendMessage, 4, mcb);

     Signal(&Done);
   }
}

PRIVATE void MySendMessage(MCB *mcb)
{ mcb->Timeout = -1;
  (void) PutMsg(mcb);
  if (mcb->MsgHdr.ContSize ne 0) Free(mcb->Control);
  if (mcb->MsgHdr.DataSize ne 0) Free(mcb->Data);
  Free(mcb);
}

/**
*** This is the polling process, which is woken up whenever the miniserver sends
*** polling data to the transputer. For the time being, this means keyboard
*** input only. The process starts by initialising the console attributes, and
*** I can safely assume that the process has enough time to finish this before
*** anything else can want to use them. Then it waits for a wake-up message
*** from the link guardian. The data coming from the link consists of a single
*** type byte, then a size byte, and finally the data. This data is processed,
*** depending on some of the current console attributes and the host machine,
*** by the code in window.c
**/

extern void ConsoleNewchar(BYTE);
PRIVATE Semaphore Polling_lock;

PRIVATE void ConsoleNewdata(int first, int size, BYTE *buffer)
{ Wait(&Polling_lock);

  ConsoleNewchar(first);
  if (size > 0)
   { int i;
     for (i = 0; i < size; i++)
      ConsoleNewchar(buffer[i]);
     Free(buffer);
   }
  Signal(&Polling_lock);
}

PRIVATE void PollingProcess(void)
{ BYTE *buffer;
  WORD size;

  InitSemaphore(&Polling_lock, 1);
  Signal(&ProcessesReady);
  
  forever
   { Head head;
     Wait(&PollingMessage);
     (void) DataFromLink((BYTE *) &head, sizeofHead);
     size = (head.highsize << 8) + head.lowsize;
     if (size ne 0)
      { buffer = Malloc(size);
        (void) DataFromLink(&(buffer[0]), size);
      }
     Signal(&Done);
     if (head.fncode eq Poll_Console)
      Fork(2000, ConsoleNewdata, 12, head.extra, size, buffer);
   }
}

/**
***
**/
PRIVATE void GSPServer(Device *Device, WORD matrix)
{ Message	*msg;

  if ((Device->Port= NewPort()) eq NullPort)
    return;

  NewName(&(Device->Name[0]), Device->Port, matrix);

  Signal(&ProcessesReady);
  
  forever
   { if ((msg = (Message *) Malloc(sizeof(Message))) eq Null(Message))
       { Delay(OneSec * 5); continue; }

     msg->mcb.MsgHdr.Dest	= Device->Port;
     msg->mcb.Timeout		= OneSec * 30 * 60;
     msg->mcb.Control		= &(msg->control[0]);
     msg->mcb.Data		= &(msg->data[0]);

     lab1:
     while ( GetMsg(&(msg->mcb)) eq EK_Timeout);
	
     unless( Fork(Stacksize, GSPWorker, 8, Device, &(msg->mcb)) )
	{ SendError(&(msg->mcb), EC_Error + SS_IOProc + EG_NoMemory + EO_Memory,
		    preserve);
	  goto lab1;
	}
   }
}

PRIVATE void GSPWorker(Device *Device, MCB *mcb)
{ WORD	    fn = mcb->MsgHdr.FnRc;
  VoidFnPtr fun;
  string    fullname;
  WORD      receipt = GetDate();
  
  if ((fn & FC_Mask) ne FC_GSP)
   { SendError(mcb, EC_Error + SS_IOProc + EG_FnCode + EO_Message, release);
     return;
   }

  if (fn eq 0)		/* ReplyOK message ? */
    return;

  fn &= FG_Mask;
  if ( (fn < FG_Open) || (fn > FG_CloseObj))
   { SendError(mcb, EC_Error + SS_IOProc + EG_FnCode + EO_Message, release);
     return;
   }

  if ((fullname = GetFullName(&(Device->Name[0]), mcb)) eq NULL)
     return;

  fun = Device->Handlers[(fn - FG_Open) >> FG_Shift];
  (*fun)(mcb, fullname);

  if (((GetDate() - receipt) > 5) && (fn ne FG_Open))
   { _Trace(0x123, 0, fn);
     _Trace(0x123, GetDate(), receipt);
   }
   
  Free(fullname);
}


/**
*** The Message passing server. There is a message process above which
*** deals with Helios messages coming down the link from the host. This
*** server deals with the other direction, allowing Helios programs to
*** send messages to the host. This involves opening a stream to a device
*** /Message, and sending messages to this stream. All such messages are sent
*** down the link. I do not bother to wait for a reply, nor do I send a
*** reply. To open a stream, it must be possible to locate and create it,
*** and object info is potentially useful. Note that the stream does not
*** accept any new messages while it is sending the current one.
**/

PRIVATE void Message_Open(ServInfo *servinfo)
{ MCB    *mcb   = servinfo->m;
  MsgBuf *r;
  DirNode *d;
  ObjNode *f;
  IOCMsg2 *req = (IOCMsg2 *) (mcb->Control);
  Port    StreamPort;
  BYTE    *data;
  char    *pathname = servinfo->Pathname;
  
  d = (DirNode *) GetTargetDir(servinfo);
  if (d == Null(DirNode))
   { ErrorMsg(mcb, EO_Directory);
     return;
   }
   
  f = GetTargetObj(servinfo);
  if (f == Null(ObjNode))
  { ErrorMsg(mcb, EO_File);
    return;
  }
  
  unless(CheckMask(req->Common.Access.Access, req->Arg.Mode & Flags_Mode) )
  { ErrorMsg(mcb, EC_Error+EG_Protected + EO_File);
    return;
  }  

  r = New(MsgBuf);
  if (r == Null(MsgBuf))
   { ErrorMsg(mcb, EC_Error + EG_NoMemory);
     return;
   }

  data = (BYTE *) Malloc(Message_Limit);   /* large message buffer */
  if (data == Null(BYTE))
   { Free(r);
     ErrorMsg(mcb, EC_Error + EG_NoMemory);
     return;
   } 
    
  if ((StreamPort = NewPort())  eq NullPort)
   { ErrorMsg(mcb, EC_Error + EG_Congested + EO_Port);
     Free(r); Free(data);
     return;
   }
  FormOpenReply(r, mcb, f, Flags_Closeable | Flags_Interactive, pathname);
  r->mcb.MsgHdr.Reply = StreamPort;
  
  PutMsg(&r->mcb);
  Free(r);

  { BYTE *temp = mcb->Data;  /* switch to large buffer */
    mcb->Data  = data;
    data       = temp;
  }
  
  f->Account++;
    
  UnLockTarget(servinfo);

  forever
   { WORD errcode;
     mcb->MsgHdr.Dest	= StreamPort;
     mcb->Timeout	= StreamTimeout;

     errcode = GetMsg(mcb);

     if (errcode eq EK_Timeout)
	 continue;

     if (errcode < Err_Null)    continue;

		/* I do not want to forward close requests. */
     if ( ((errcode & FG_Mask) eq FG_Close) &&
          ((errcode & FC_Mask) eq FC_GSP) )
       break;
       
     Message_Forward(mcb);
   }

  Free(mcb->Data);    /* restore data vector */
  mcb->Data = data;
  
  if (mcb->MsgHdr.Reply ne NullPort)
    Return(mcb, ReplyOK, 0, 0, preserve);

  FreePort(StreamPort);
}

/**
*** This sends a Helios message down the link, as soon as it becomes available.
*** The message is preceeded by a protocol byte, and then I send the flags,
*** control vector size, data vector size, and the rest of the message header.
*** If there is a control vector or a data vector these are sent as well.
*** Then the link is released.
**/

PRIVATE void Message_Forward(MCB *mcb)
{ BYTE table[5];

  while (!LinkWait());
  table[0] = Pro_Message;
  table[1] = mcb->MsgHdr.Flags;
  table[2] = mcb->MsgHdr.ContSize;
  table[3] = (mcb->MsgHdr.DataSize >> 8) & 0x00FF;
  table[4] = (mcb->MsgHdr.DataSize) & 0x00FF;
  (void) DataDownLink(table, 5);
  (void) DataDownLink( ((BYTE *) mcb) + 4, 12);
  if (mcb->MsgHdr.ContSize ne 0)
   (void) DataDownLink( (BYTE *) mcb->Control, mcb->MsgHdr.ContSize * sizeof(WORD));
  if (mcb->MsgHdr.DataSize ne 0)
   (void) DataDownLink(mcb->Data, mcb->MsgHdr.DataSize);

  LinkSignal();
}


/**
*** The Disk Drive routines
**/

PRIVATE void Drive_Open(MCB *mcb, string fullname)
{ string	localname = GetLocalName(fullname);
  WORD		exists;
  WORD		openmode;
  IOCMsg2	*msg = (IOCMsg2 *) mcb->Control;
  int		timedout = 0;
  
  if (localname eq Null(char))
   { SendError(mcb, EC_Error + SS_IOProc + EG_NoMemory + EO_Server, release);
     return;
   }

  openmode = msg->Arg.Mode;

  exists = exists_obj(localname, &timedout);

  if ((exists eq Nothing_t) && timedout)
   { SendError(mcb, ST_Timeout, release);
     return;
   }

  if ((exists ne Nothing_t) && (openmode & O_Create) &&
      (openmode & O_Exclusive))
   { SendError(mcb, EC_Error + SS_IOProc + EG_InUse + EO_Name, release);
     return;
   }
   
  if (exists eq File_t)
    File_Open(mcb, fullname, localname, openmode);
  elif (exists eq Dir_t)
    { if ( ((openmode & O_Mask) eq O_ReadOnly) ||
	   ((openmode & O_Mask) eq O_ReadWrite) )
       Dir_Open(mcb, fullname, localname);
      else
       SendError(mcb, EC_Error + SS_IOProc + EG_WrongFn + EO_Directory,
		 release);
   }
  else
   { if ((openmode & O_Create) eq 0)
      { SendError(mcb, EC_Error + SS_IOProc + EG_Unknown + EO_File, release);
        return;
      }
     elif (!create_object(localname, Type_File, &timedout))
      { SendError(mcb, 
         (timedout) ? ST_Timeout : EC_Error + SS_IOProc + EG_Create + EO_File,
                  release);
        return;
      }
 
     File_Open(mcb, fullname, localname, openmode);
   }

  Free(localname);
}

/**
*** Handling open file streams. It is desirable to limit the number
*** of open streams at any one time to a small number, currently 10,
*** closing other streams as and when necessary. It is also desirable
*** to avoid have the same file open twice by separate clients.
*** Unfortunately, to achieve this it is necessary to single-thread some
*** of the file I/O, using a semaphore. Error conditions are handled
*** by using a cycle counter.
**/

#define MaxOpen 10
typedef struct FileStream {
	int	in_use;
	Port	port;
	int	pos;
	int	handle;
	int	cycle;
	char	name[Name_Max];
} FileStream;

PRIVATE FileStream file_table[MaxOpen];
PRIVATE Semaphore single_threaded;
PRIVATE int last_abort = 0;

PRIVATE void InitFileIO(void)
{ int i;
  for (i = 0; i < MaxOpen; i++)
   { file_table[i].in_use = 0;
     file_table[i].cycle  = 1;
   }
  InitSemaphore(&single_threaded, 1);
}

  /* abort the stream using the specified slot */
PRIVATE void free_slot(int slot)
{ int timedout = 0;

  close_file(file_table[slot].handle, &timedout);
  file_table[slot].cycle++;
  FreePort(file_table[slot].port);
  file_table[slot].in_use = 0;
}

PRIVATE void File_Open(MCB *mcb, string fullname, string localname,
			 WORD openmode)
{ int		index;
  BYTE		*data = mcb->Data;
  int		timedout = 0;
  int           mycycle;
  
  Wait(&single_threaded);

     /* find a free slot in the table */  
  for (index = 0; index < MaxOpen; index++)
   if (!file_table[index].in_use)
    goto found_slot;
  
     /* No free slot, so abort an existing stream to make a free slot */
     /* Cycle around when picking the slot to abort */
  free_slot(last_abort);
  index = last_abort;
  last_abort = (last_abort + 1) % MaxOpen;
  
found_slot:

  file_table[index].in_use = 1;
  mycycle = ++(file_table[index].cycle);
  strcpy(file_table[index].name, localname);

  { int i;
    for (i = 0; i < MaxOpen; i++)
     { if (i eq index) continue;
       unless (file_table[i].in_use) continue;
       if (!strcmp(localname, file_table[i].name))
        { free_slot(i);
        }
     }
  }
        
  if ((file_table[index].port = NewPort()) eq NullPort)
   { SendError(mcb, EC_Warn + SS_IOProc + EG_Congested + EO_Port, release);
     file_table[index].in_use = 0;
     Signal(&single_threaded);
     return;
   }

   /* adjust the open mode */
  { WORD tempmode;
    if (openmode & O_Truncate)
     tempmode = O_WriteOnly;
    elif ((openmode & O_Mask) eq O_WriteOnly)
     tempmode = O_ReadWrite;
    else
     tempmode = openmode;
     
    openmode = tempmode;  
  } 

  file_table[index].handle = open_file(localname, openmode, &timedout);

  if ((file_table[index].handle eq -1) && timedout)
   { SendError(mcb, ST_Timeout, release);
     FreePort(file_table[index].port);
     file_table[index].in_use = 0;
     Signal(&single_threaded);
     return;   
   }

    /* The file is known to exist, so it is strange that it cannot be opened */
  if (file_table[index].handle eq -1)
   { SendError(mcb, EC_Error + SS_IOProc + EG_InUse + EO_File, release);
     FreePort(file_table[index].port);
     file_table[index].in_use = 0;
     Signal(&single_threaded);
     return;
   }

  SendOpenReply(mcb, fullname, Type_File, Flags_Closeable + Flags_MSdos,
		file_table[index].port);

  file_table[index].pos = 0;
  
			/* The stream is now open */
  forever
   { WORD errcode;
     WORD receipt = GetDate();
     mcb->MsgHdr.Dest	= file_table[index].port;
     mcb->Timeout	= StreamTimeout;
     mcb->Data		= data;

     if ((GetDate() - receipt) > 5)
      { _Trace(0x200, 0, errcode & FG_Mask);
        _Trace(0x200, receipt, GetDate());
      }
      
     Signal(&single_threaded);
     
     if ((errcode = GetMsg(mcb)) eq EK_Timeout) break;

     receipt = GetDate();

     Wait(&single_threaded);
          
      /* The file table slot used for this stream may have been reused, */
      /* because some other stream needed it. In that case, free_slot() */
      /* above will have closed the stream and freed the port, so I can */
      /* just exit. N.B. This will only be executed iff the slot has    */
      /* been freed between receipt of the message and return of the    */
      /* single_threaded wait. */
     if (mycycle ne file_table[index].cycle) 
      { 
        if ((errcode >= Err_Null) && (mcb->MsgHdr.Reply ne NullPort))
         SendError(mcb, EC_Recover + SS_IOProc + EG_Unknown + EO_Stream, release);
        Signal(&single_threaded);
        return;
      }

     if (errcode < Err_Null)
      { if ((errcode & EC_Mask) >= EC_Error)
         break;
        else
         continue;
       }
          
     if ((errcode & FC_Mask) ne FC_GSP)
      { SendError(mcb, EC_Error + SS_IOProc + EG_WrongFn + EO_Stream, preserve);
        continue;
      }

     switch ( errcode & FG_Mask )
      { case FG_Read	: File_Read(mcb, index); break;

	case FG_Write	: File_Write(mcb, index); break;

	case FG_Close	: if (mcb->MsgHdr.Reply ne NullPort)
                           Return(mcb, ReplyOK, 0, 0, preserve);
                          goto finished;
                          
	case FG_Seek	: File_Seek(mcb, index); break;

	case FG_GetSize	: File_GetSize(mcb, index);
	                  break;

	case FG_SetSize	:
	case FG_GetInfo	:
	case FG_SetInfo	:
	case FG_EnableEvents	:
	case FG_Acknowledge	:
	case FG_NegAcknowledge	:
	default		: SendError(mcb, EC_Error + SS_IOProc + EG_WrongFn +
				    EO_Stream, preserve);
			  break;
      }
   }

finished:
  close_file(file_table[index].handle, &timedout);
  FreePort(file_table[index].port);
  file_table[index].in_use = 0;
  Signal(&single_threaded);
}

PRIVATE void File_Read(MCB *mcb, int index)
{ ReadWrite *readwrite = (ReadWrite *) mcb->Control;
  WORD	read_so_far, to_read, read_this_time, seq = 0, temp;
  bool	eof = FALSE;
  BYTE  *buffer;
  Port	itsport = mcb->MsgHdr.Reply;
  int	timedout = 0;
#define handle file_table[index].handle
#define pos    file_table[index].pos
    
  if (readwrite->Pos ne pos)
   { if (seek_in_file(handle, Seek_start, readwrite->Pos, &timedout) eq -1)
      { SendError(mcb,
          (timedout) ? ST_Timeout : EC_Error + SS_IOProc + EG_Broken + EO_File,
           preserve);
        return;
      }
     else
      pos = readwrite->Pos;
   }
    
  if (readwrite->Size eq 0)
    { Return(mcb, ReadRc_EOD, 0, 0, preserve);
      return;
    }

  if ((buffer = (BYTE *) Malloc(Message_Limit - 2)) eq Null(BYTE))
    {
      SendError(mcb, EC_Warn + SS_IOProc + EG_NoMemory + EO_Server, preserve);
      return;
    }

  for ( read_so_far = 0; (read_so_far < readwrite->Size) && !eof; )
    { to_read = ((readwrite->Size - read_so_far) > (Message_Limit - 2) ) ?
		 (Message_Limit - 2) : (readwrite->Size - read_so_far);

      read_this_time = read_from_file(handle, buffer, to_read, &timedout);
      if (read_this_time < 0)
       pos = -1;
      else
       pos += read_this_time;

      if (timedout)
       { SendError(mcb, ST_Timeout, preserve);
         Free(buffer);
         return;
       }
      read_so_far += read_this_time;

      if (read_this_time < to_read) eof = TRUE;
      mcb->MsgHdr.Dest	= itsport;
      mcb->MsgHdr.Reply	= NullPort;
      mcb->MsgHdr.Flags	= (eof) ? 0 : MsgHdr_Flags_preserve;
      mcb->MsgHdr.FnRc  = seq + (eof ? ReadRc_EOF :
	 (read_so_far >= readwrite->Size) ? ReadRc_EOD : ReadRc_More);
      seq += ReadRc_SeqInc;
      mcb->MsgHdr.ContSize = 0;
      mcb->MsgHdr.DataSize = read_this_time;
      mcb->Data	= buffer;
      temp = PutMsg(mcb);
    }

  Free(buffer);			
#undef pos
#undef handle
}

PRIVATE void File_Write(MCB *mcb, int index)
{ ReadWrite *readwrite = (ReadWrite *) mcb->Control;
  BYTE *buffer, *ptr;
  bool ownbuf	= FALSE;
  Port itsport	= mcb->MsgHdr.Reply, myport = mcb->MsgHdr.Dest;
  WORD timeout, count, written;
  WORD MaxData   = Message_Limit - 2;
  int  timedout = 0;
#define handle file_table[index].handle
#define pos    file_table[index].pos
  
  if (readwrite->Pos ne pos)
   { if (seek_in_file(handle, Seek_start, readwrite->Pos, &timedout) eq -1)
      { SendError(mcb,
         (timedout) ? ST_Timeout : EC_Error + SS_IOProc + EG_Broken + EO_File,
         preserve); 
        return;
      }
     else
      pos = readwrite->Pos;
   }
    
  if (readwrite->Size eq 0)
    { mcb->Control[0] = 0;
      Return(mcb, ReplyOK, 1, 0, preserve);
      return;
    }
  else
   count = readwrite->Size;

  timeout = readwrite->Timeout;

  if (mcb->MsgHdr.DataSize ne 0)
   buffer = mcb->Data;
  elif ((buffer = (BYTE *) Malloc(count)) eq Null(BYTE))
    { SendError(mcb, EC_Warn + SS_IOProc + EG_NoMemory + EO_Server, preserve);
      return;
    }
  else
    { WORD fetched   = 0;
      ownbuf = TRUE;      

		/* The data is read in in lumps of upto Message_Limit, */
		/* and put into the same buffer. Once the buffer */
		/* has been filled I can start writing it.       */

		/* Start by sending the initial message, giving sizes */
      mcb->Control[0] = (count > Message_Limit) ? Message_Limit : count;
      mcb->Control[1] = Message_Limit;
      mcb->MsgHdr.Flags = MsgHdr_Flags_preserve;
      mcb->MsgHdr.Dest  = itsport;
      mcb->MsgHdr.Reply = NullPort;
      mcb->MsgHdr.FnRc  = WriteRc_Sizes;
      mcb->MsgHdr.ContSize = 2;
      mcb->MsgHdr.DataSize = 0;
      mcb->Timeout	   = timeout;
      (void) PutMsg(mcb);

      ptr = buffer;

		/* Now wait for all the data */
      while (fetched < count)
       { mcb->MsgHdr.Dest = myport;
	 mcb->Data	  = ptr;
	 mcb->Timeout	  = timeout;

	 if (GetMsg(mcb) < 0)	/* Help !!!!!!!! */
	  { if (ownbuf)		/* Just go back to waiting for GSP request */
	     Free(buffer);
	    return;
	  }
	 
         fetched   += (word) mcb->MsgHdr.DataSize;
         ptr       += mcb->MsgHdr.DataSize;           /* next bit of buffer */
       }
    }

	/* When I get here all the data has arrived, and I can start sending */
	/* it down the link. I need to write count bytes starting at buff.   */

  for (written = 0, ptr = buffer; written < count; )
   { WORD to_write = ((count - written) > MaxData) ? MaxData :
		     (count - written);

     if (!write_to_file(handle, ptr, to_write, &timedout))
      { pos = -1;
        if (ownbuf)
         Free(buffer);
	mcb->MsgHdr.Reply = itsport;
	SendError(mcb,
	   (timedout) ? ST_Timeout : EC_Error + SS_IOProc + EG_Broken + EO_File,
           preserve);
	return;
      }

     pos += to_write; written += to_write; ptr += to_write;
   }

  mcb->Control[0] = written;
  mcb->MsgHdr.Reply = itsport;
  Return(mcb, ReplyOK, 1, 0, preserve);
  if (ownbuf) Free(buffer);
#undef pos
#undef handle
}

PRIVATE void File_Seek(MCB *mcb, int index)
{ SeekRequest	*req = (SeekRequest *) mcb->Control;
  int		timedout = 0;
#define pos (file_table[index].pos)
#define handle (file_table[index].handle)
  
  if (req->Mode eq S_Beginning)
    { if ((pos = seek_in_file(handle,
              Seek_start, req->NewPos, &timedout)) ne -1)
	{ mcb->Control[0] = pos;
	  Return(mcb, ReplyOK, 1, 0, preserve);
        }
      else
       { SendError(mcb,
          (timedout) ? ST_Timeout : EC_Error + SS_IOProc + EG_Broken + EO_File,
          preserve);
       }
    }
  elif (req->Mode eq S_Relative)
    { if ((pos = seek_in_file(handle, Seek_start, req->CurPos + req->NewPos,
                              &timedout)) ne -1)
	{ mcb->Control[0] = pos;
	  Return(mcb, ReplyOK, 1, 0, preserve);
        }
      else
       { SendError(mcb,
          (timedout) ? ST_Timeout : EC_Error + SS_IOProc + EG_Broken + EO_File,
          preserve);
       }
    }
  elif (req->Mode eq S_End)
   { if ((pos = seek_in_file(handle, Seek_end, req->NewPos, &timedout)) eq -1)
        { SendError(mcb,
           (timedout) ? ST_Timeout :
                        EC_Error + SS_IOProc + EG_Broken + EO_File,
           preserve); 
        }
       else
        { mcb->Control[0] = pos;
          Return(mcb, ReplyOK, 1, 0, preserve);    
        }
   }
  else
   SendError(mcb, EC_Error + SS_IOProc + EG_WrongFn + EO_Stream, preserve);
#undef pos
#undef handle
}

PRIVATE void File_GetSize(MCB *mcb, int index)
{ int timedout = 0;

#define pos (file_table[index].pos)
#define handle (file_table[index].handle)

  if ((pos = seek_in_file(handle, Seek_end, 0, &timedout)) eq -1)
   SendError(mcb,
    (timedout) ? ST_Timeout : EC_Error + SS_IOProc + EG_Broken + EO_File,
    preserve);
  else
   { mcb->Control[0] = pos;
     Return(mcb, ReplyOK, 1, 0, preserve);    
   }

#undef pos
#undef handle
}

PRIVATE void Dir_Open(MCB *mcb, string fullname, string localname)
{ Port		StreamPort;
  DirStream	*stream;
  BYTE		*data = mcb->Data;
  int		timedout = 0;
  
  if ((StreamPort = NewPort()) eq NullPort)
   { SendError(mcb, EC_Warn + SS_IOProc + EG_Congested + EO_Port, release);
     return;
   }

  if ((stream = read_dir(localname, &timedout)) eq Null(DirStream))
   { SendError(mcb,
      (timedout) ? ST_Timeout : EC_Error + SS_IOProc + EG_NoMemory + EO_Server,
      release); 
     return;
   }
   
  SendOpenReply(mcb, fullname, Type_Directory, Flags_Closeable,
		StreamPort);

			/* The stream is now open */
  forever
   { WORD	errcode;
     mcb->MsgHdr.Dest	= StreamPort;
     mcb->Timeout	= StreamTimeout;
     mcb->Data		= data;

     if ((errcode = GetMsg(mcb)) eq EK_Timeout) break;

     if (errcode < Err_Null) continue;
     
     if ((errcode & FC_Mask) ne FC_GSP)
      { SendError(mcb, EC_Error + SS_IOProc + EG_WrongFn + EO_Stream, preserve);
        continue;
      }

     switch ( errcode & FG_Mask )
      { case FG_Read	: Dir_Read(mcb, stream); break;

	case FG_Close	: 
	                  Free(stream);
			  if (mcb->MsgHdr.Reply ne NullPort)
			    Return(mcb, ReplyOK, 0, 0, preserve);
			  Free(mcb);
			  FreePort(StreamPort);
			  return;

	case FG_Seek	: Dir_Seek(mcb, stream); break;

	case FG_GetSize	: mcb->Control[0] = stream->number * sizeof(DirEntry);
			  Return(mcb, ReplyOK, 1, 0, preserve);
			  break; 

	case FG_Write	:
	case FG_SetSize	:
	case FG_GetInfo	:
	case FG_SetInfo	:
	case FG_EnableEvents	:
	case FG_Acknowledge	:
	case FG_NegAcknowledge	:
	default		: SendError(mcb, EC_Error + SS_IOProc + EG_WrongFn +
				    EO_Stream, preserve);
			  break;
      }
   }
}

PRIVATE void Dir_Read(MCB *mcb, DirStream *stream)
{ ReadWrite *readwrite = (ReadWrite *) mcb->Control;
  bool	eof = FALSE;
  WORD  amount;

  stream->offset = readwrite->Pos;

  if (stream->offset >= (stream->number * sizeof(DirEntry)))
   { Return(mcb, ReadRc_EOF, 0, 0, preserve);
     return;
   }

  amount = readwrite->Size;
  if ((amount + stream->offset) > (stream->number * sizeof(DirEntry)))
   { amount = (stream->number * sizeof(DirEntry)) - stream->offset;
     eof    = TRUE;
   }

  mcb->Data = &( ((BYTE *) &(stream->entries[0])) [stream->offset]);
  Return(mcb, (eof ? ReadRc_EOF : ReadRc_EOF), 0, amount, preserve);
  stream->offset += amount;
}

PRIVATE void Dir_Seek(MCB *mcb, DirStream *stream)
{ SeekRequest *req = (SeekRequest *) mcb->Control;
  WORD	newoff;

  if (req->Mode eq S_Beginning)
    newoff = req->NewPos;
  elif(req->Mode eq S_Relative)
    newoff = req->CurPos + req->NewPos;
  elif(req->Mode eq S_End)
    newoff = (stream->number * sizeof(DirEntry)) - req->NewPos;
  else
   { SendError(mcb, EC_Error + SS_IOProc + EG_WrongFn + EO_Stream, preserve);
     return;
   }

  if ((newoff < 0) || (newoff > (stream->number * sizeof(DirEntry))))
   { SendError(mcb, EC_Error + SS_IOProc + EG_WrongSize + EO_File, preserve);
     return;
   }
  else
   { stream->offset = newoff;
     mcb->Control[0] = newoff;
     Return(mcb, ReplyOK, 1, 0, preserve);
   }
}


PRIVATE void Drive_Locate(MCB *mcb, string fullname)
{ string localname = GetLocalName(fullname);
  WORD	 exists;
  int	 timedout = 0;
  
  if (localname eq Null(char))
   { 
     SendError(mcb, EC_Error + SS_IOProc + EG_NoMemory + EO_Server, release);
     return;
   }

  exists = exists_obj(localname, &timedout);

  if (exists eq File_t)
   SendOpenReply(mcb, fullname, Type_File, 0, NullPort);
  elif (exists eq Dir_t)
   SendOpenReply(mcb, fullname, Type_Directory, 0, NullPort);
  else
   SendError(mcb,
     (timedout) ? ST_Timeout : EC_Error + SS_IOProc + EG_Unknown + EO_File,
     release); 

  Free(localname);
}

PRIVATE void Drive_Create(MCB *mcb, string fullname)
{ string localname = GetLocalName(fullname);
  IOCCreate *info  = (IOCCreate *) mcb->Control;
  WORD	 type;
  int    timedout = 0;
  
  type = info->Type;

  if (localname eq Null(char))
   { SendError(mcb, EC_Warn + SS_IOProc + EG_NoMemory + EO_Server, release);
     return;
   }

  if ((type ne Type_File) && (type ne Type_Directory))
   { SendError(mcb, EC_Error + SS_IOProc + EG_Create + EO_Object, release);
     Free(localname);
     return;
   }

  if (!create_object(localname, type, &timedout))
   { if (timedout)
      SendError(mcb, EC_Recover + SS_IOProc + EG_Timeout + EO_Link, release);
     else
      SendError(mcb, EC_Error + SS_IOProc + EG_Create +
		((type eq Type_File) ? EO_File : EO_Directory), release);
   }
  else
   SendOpenReply(mcb, fullname, type, 0, NullPort);

  Free(localname);
}

PRIVATE void Drive_ObjInfo(MCB *mcb, string fullname)
{ string localname = GetLocalName(fullname);
  WORD	 exists;
  int    timedout = 0;
  
  if (localname eq Null(char))
   { SendError(mcb, EC_Error + SS_IOProc + EG_NoMemory + EO_Server, release);
     return;
   }

  if ((exists = exists_obj(localname, &timedout)) eq Nothing_t)
   { SendError(mcb,
      (timedout) ? ST_Timeout : EC_Error + SS_IOProc + EG_Unknown + EO_File,
      release);
     Free(localname);
     return;
   }
  elif (exists eq Dir_t)    /* I do not need any other info about directories */
   { ObjInfo *info = (ObjInfo *) mcb->Data;
     info->DirEntry.Type	= Type_Directory;
     info->DirEntry.Flags	= 0;
     info->DirEntry.Matrix	= DefDirMatrix;
     strcpy(&(info->DirEntry.Name[0]), lastbit(fullname));
     info->Account		= 0;
     info->Size			= 0;	/* should be 44 * no. of entries */
     info->Dates.Creation	= 0;	/* 1 Jan 1970... */
     info->Dates.Access		= Server_StartTime;
     info->Dates.Modified	= 0;
     Return(mcb, ReplyOK, 0, sizeof(ObjInfo), release);
     Free(localname);
     return;
   }

   { ObjInfo *info	= (ObjInfo *) mcb->Data;
     if (!get_file_info(localname, &(info->Size), &(info->Dates.Creation), &timedout))
      { SendError(mcb,
         (timedout) ? ST_Timeout : EC_Error + SS_IOProc + EG_Unknown + EO_File,
         release);
        Free(localname);
        return;
      }
     info->DirEntry.Type	= Type_File;
     info->DirEntry.Flags	= 0;
     info->DirEntry.Matrix	= DefFileMatrix;
     strcpy(&(info->DirEntry.Name[0]), lastbit(fullname));
     info->Account		= 0;
     info->Dates.Access	= info->Dates.Creation;
     info->Dates.Modified	= info->Dates.Creation;
     Return(mcb, ReplyOK, 0, sizeof(ObjInfo), release);
   }

  Free(localname);
}

typedef struct servinfo { WORD	type;
			  WORD	size;
			  WORD	available;
			  WORD	alloc;
} servinfo;

PRIVATE void Drive_ServerInfo(MCB *mcb, string fullname)
{ string localname = GetLocalName(fullname);
  servinfo *info = (servinfo *) mcb->Data;
  int timedout = 0;
  
  if (localname eq Null(char))
   { SendError(mcb, EC_Error + SS_IOProc + EG_NoMemory + EO_Server, release);
     return;
   }

  info->type = Type_Directory;
  info->alloc = 1024;
  if (!drive_statistics(localname, &(info->size), &(info->available), &timedout))
   SendError(mcb,
      (timedout) ? ST_Timeout : EC_Error + SS_IOProc + EG_FnCode + EO_Message,
      release); 
  else
    Return(mcb, ReplyOK, 0, sizeof(servinfo), release);

  Free(localname);
}

PRIVATE void Drive_Delete(MCB *mcb, string fullname)
{ string localname = GetLocalName(fullname);
  WORD	 exists;
  int    timedout = 0;
  
  if (localname eq Null(char))
   { SendError(mcb, EC_Error + SS_IOProc + EG_NoMemory + EO_Server, release);
     return;
   }

  if ((exists = exists_obj(localname, &timedout)) eq Nothing_t)
   { SendError(mcb,
      (timedout) ? ST_Timeout : EC_Error + SS_IOProc + EG_Unknown + EO_File,
      release); 
     Free(localname);
     return;
   }

  if (!delete_object(localname, exists, &timedout))
   { if (timedout)
      SendError(mcb, EC_Recover + SS_IOProc + EG_Timeout + EO_Link, release);
     else
      SendError(mcb, EC_Error + SS_IOProc + EG_Delete + 
		(exists eq Dir_t) ? EO_Directory : EO_File, release);
   }
  else
   Return(mcb, ReplyOK, 0, 0, release);

  Free(localname);
}

PRIVATE void Drive_Rename(MCB *mcb, string fullname)
{ string localname = GetLocalName(fullname);
  string newname, newlocal, tempname;
  WORD   fromexists, toexists;
  IOCMsg2 *args = (IOCMsg2 *) mcb->Control;
  int    timedout = 0;
  
  if (localname eq Null(char))
   { SendError(mcb, EC_Error + SS_IOProc + EG_NoMemory + EO_Server, release);
     return;
   }

  for (tempname = fullname; (*tempname ne '/') && (*tempname ne '\0');
        tempname++);
  *tempname = '\0';
  
  args->Common.Name = args->Arg.ToName;
  if ((newname = GetFullName(fullname, mcb)) eq (string) NULL)
    { SendError(mcb, EC_Error + SS_IOProc + EG_NoMemory + EO_Server, release);
      Free(localname);
      return;
    }

  if ((newlocal = GetLocalName(newname)) eq (string) NULL)
    { SendError(mcb, EC_Error + SS_IOProc + EG_NoMemory + EO_Server, release);
      Free(localname); Free(newname);
      return;
    }

	/* Renaming something on top of itself ? */
  if (!strcmp(localname, newlocal))
   { Return(mcb, ReplyOK, 0, 0, release);
     goto rename_end;
   }

  fromexists = exists_obj(localname, &timedout);
  if (fromexists eq Nothing_t)
   { SendError(mcb,
      (timedout) ? ST_Timeout : EC_Error + SS_IOProc + EG_Unknown + EO_File,
      release); 
     goto rename_end;
   }
  elif(fromexists eq Dir_t)
   { SendError(mcb, EC_Error + SS_IOProc + EG_WrongFn + EO_Directory, release);
     goto rename_end;
   }

  toexists = exists_obj(newlocal, &timedout);
  if ((toexists eq Nothing_t) && timedout)
   { SendError(mcb, EC_Recover + SS_IOProc + EG_Timeout + EO_Link, release);
     goto rename_end;
   }
  if (toexists eq Dir_t)
   { SendError(mcb, EC_Error + SS_IOProc + EG_Protected + EO_Directory, release);
     goto rename_end;
   }
  elif (toexists eq File_t)
   { if (!delete_object(newlocal, File_t, &timedout))
      { SendError(mcb,
         (timedout) ? ST_Timeout : EC_Error + SS_IOProc + EG_Protected + EO_File,
         release); 
        goto rename_end;
      }
   }

  if (!rename_file(localname, newlocal, &timedout))
   { SendError(mcb,
      (timedout) ? ST_Timeout : EC_Error + SS_IOProc + EG_Broken + EO_File,
      release);
   }
  else
    Return(mcb, ReplyOK, 0, 0, release); 
   
rename_end:
  Free(localname);
  Free(newname);
  Free(newlocal);
}

PRIVATE void Drive_SetDate(MCB *mcb, string fullname)
{ string localname = GetLocalName(fullname);
  WORD	 exists;
  int    timedout;
  
  if (localname eq Null(char))
   { SendError(mcb, EC_Error + SS_IOProc + EG_NoMemory + EO_Server, release);
     return;
   }

  if ((exists = exists_obj(localname, &timedout)) eq Nothing_t)
   { SendError(mcb,
      (timedout) ? ST_Timeout : EC_Error + SS_IOProc + EG_Unknown + EO_File,
      release); 
     Free(localname);
     return;
   }

  if (exists eq Dir_t)
   { SendError(mcb, EC_Error + SS_IOProc + EG_WrongFn + EO_Directory, release);
     Free(localname);
     return;
   }

  if (!change_date(localname, &timedout))
   SendError(mcb,
    (timedout) ? ST_Timeout : EC_Error + SS_IOProc + EG_WrongFn + EO_Server,
    release);
  else
   Return(mcb, ReplyOK, 0, 0, release);

  Free(localname);
}

/**
*** GSP error message routines
**/

PRIVATE void InvalidFun(MCB *mcb, string fullname)
{ SendError(mcb, EC_Error + SS_IOProc + EG_WrongFn + EO_Server, release);
  fullname = fullname;
}

void SendError(MCB *mcb, WORD FnRc, WORD Preserve)
{ 
if (FnRc eq ST_Timeout)
 _Trace(0x3, 0, mcb->MsgHdr.FnRc);
 
  if (mcb->MsgHdr.Reply eq NullPort) return;
  *((int *) mcb) = 0;
  mcb->MsgHdr.Dest  = mcb->MsgHdr.Reply;
  mcb->MsgHdr.Reply = NullPort;
  mcb->MsgHdr.FnRc  = FnRc;
  mcb->Timeout      = 5 * OneSec;
  (void) PutMsg(mcb);
  if (Preserve eq release)
    Free(mcb);  
}

void Return(MCB *mcb, WORD FnRc, WORD ContSize, WORD DataSize,
		    WORD Preserve)
{ if (mcb->MsgHdr.Reply eq NullPort) return;
  mcb->MsgHdr.Flags	= 0;
  mcb->MsgHdr.ContSize	= ContSize;
  mcb->MsgHdr.DataSize	= DataSize;
  mcb->MsgHdr.Dest	= mcb->MsgHdr.Reply;
  mcb->MsgHdr.Reply	= NullPort;
  mcb->MsgHdr.FnRc	= FnRc;
  mcb->Timeout		= 5 * OneSec;
  (void) PutMsg(mcb);
  if (Preserve eq release)
    Free(mcb);  
}

void SendOpenReply(MCB *mcb, string name, WORD type, WORD flags,
			   Port Reply)
{ IOCReply1 *reply = (IOCReply1 *) mcb->Control;
  if (mcb->MsgHdr.Reply eq NullPort) return;
  reply->Type = type;
  reply->Flags	= flags;
  mcb->Control[2]	= -1;
  mcb->Control[3]	= -1;
  reply->Pathname	= 0;
  reply->Object		= 0;
  Wait(&Machine_Name_Sem);
  strcpy(mcb->Data, &(Machine_Name[0]));
  Signal(&Machine_Name_Sem);
  strcat(mcb->Data, "/");
  strcat(mcb->Data, name);

  mcb->MsgHdr.Flags	= 0;
  mcb->MsgHdr.ContSize	= sizeof(IOCReply1) / sizeof(WORD);
  mcb->MsgHdr.DataSize	= strlen(mcb->Data) + 1;
  mcb->MsgHdr.Dest	= mcb->MsgHdr.Reply;
  mcb->MsgHdr.Reply	= Reply;
  mcb->MsgHdr.FnRc	= ReplyOK;
  mcb->Timeout		= 5 * OneSec;
  (void) PutMsg(mcb);

  if (Reply eq NullPort)
    Free(mcb);  
}

/**
*** A name conversion routine, to turn the GSP bits of the name into a
*** simple string.
***
*** The first bit of code goes through the names, starting at pointer rest which
*** may be in either the context or the name fields, and puts all the data into
*** IOname. I try to update the next field in the message in case the
*** message has to be passed from one server to another, i.e. if another name
*** conversion may be required. This bit of code is rather dubious. It may be
*** necessary to extract bits of the name from the name field as well, in case
*** the rest pointer was still somewhere inside context. Having got hold of
*** the whole name I put in a terminator.
***
*** Sadly IOname may still contain elements . and .. which have to be filtered
*** out. Hence I must go through my entire local name again looking for these
*** special cases, and this is done by routine flatten() which is also called
*** by Rename handlers. Only now am I finished, and I can put in another
*** terminator just in case and produce another debugging option. It is
*** possible for the name conversion to fail if there is an attempt to
*** backtrack to a server outside the Server, as I refuse to forward messages
*** to other bits of the network.
**/

PRIVATE string GetFullName(string DeviceName, MCB *mcb)
{ BYTE *data     = mcb->Data;
  IOCCommon *common = (IOCCommon *) mcb->Control;
  int  context   = common->Context;
  int  name      = common->Name;
  int  next      = common->Next;
  string NewName, tmp;
  string dest = (string) Malloc(Name_Max);

  if (dest eq Null(char))
   { SendError(mcb, EC_Error + SS_IOProc + EG_NoMemory + EO_Server, release);
     return(Null(char));
   }
  else
   NewName = dest;

  for (tmp = &(DeviceName[0]); *tmp ne '\0'; )
    *dest++ = *tmp++;
  *dest++ = '/';

  for ( ; data[next] ne '/' && data[next] ne '\0'; next++)
    *dest++ = data[next];

  if (data[next] eq '/')
      for ( ; data[next] ne '\0'; next++)
        *dest++ = data[next];

  if (name eq -1) goto finished;

  if ( ((next < name) && (context < name)) ||
       ((next > name) && (context > name)) )
   { *dest++ = '/';
     for ( ; data[name] ne '\0'; name++)
       *dest++ = data[name];
   }

finished:

  if (*(--dest) ne '/') dest++;		/* Get rid of any trailing '/' */
  *dest = '\0';

  if (!flatten(NewName))
   { Free(NewName); return(Null(char)); }

  return(NewName);
}

PRIVATE WORD flatten(string name)
{ char *source = name, *dest = name;
  int  entries = 0;

  while(*source ne '\0')
   { if (*source eq '.')
       { source++;
         if   (*source eq '/') { source++; continue; }
         elif (*source eq '\0')
           { if (entries < 1) return(FALSE);
             dest--; break;
           }
         elif (*source eq '.')
           { source++;
             if (*source eq '/' || *source eq '\0')
               { 
                 if (entries <= 1) return(FALSE);
                 dest--; dest--; while (*dest ne '/')  dest--;
                 if (*source ne '\0') 
                  { dest++; source++; }
                 entries--; continue;
               }
             else
               { *dest++ = '.'; *dest++ = '.'; }
           }
         else *dest++ = '.';
       }

     while (*source ne '/' && *source ne '\0') *dest++ = *source++;
     if (*source ne '\0')
       { *dest++ = '/'; source++;
         while (*source eq '/') source++;	/* This gets around a bug */
         entries++;				/* in convert_name	  */
       }
   }

  *dest = '\0';

  return(TRUE);
}

/**
*** Another name conversion routine, to turn a Helios name into a local name.
**/

PRIVATE char *GetLocalName(string HeliosName)
{ string tempptr, destptr;
  string local_name = (char *) Malloc(Name_Max);

  if (local_name eq Null(char))
    return(Null(char));

  for ( destptr = local_name, tempptr = HeliosName;
	(*tempptr ne '/') && (*tempptr ne '\0'); tempptr++);

  if (*tempptr eq '\0')
    { if (!strcmp(HeliosName, "helios"))
        strcpy(local_name, Helios_Directory);
      else
        { strcpy(local_name, HeliosName);
          strcat(local_name, ":");
        }
      goto strip;
    }
  else
    { *tempptr = '\0';
      if (!strcmp(HeliosName, "helios"))
        strcpy(local_name, Helios_Directory);
      else
        { strcpy(local_name, HeliosName);
          if (Host eq PC) strcat(local_name, ":");
        }
      *tempptr++ = '/';
    }

  for (destptr = &(local_name[strlen(local_name)]);
       (*tempptr ne '\0'); )
   { for (*destptr++ = (Host eq PC) ? '\\' : ':'; 
	 (*tempptr ne '\0') && (*tempptr ne '/'); )
       *destptr++ = *tempptr++;
     if (*tempptr eq '/') tempptr++;
   }

  *destptr = '\0';

strip:

  if (Host eq DP2)	/* Upper case all of the name */
   { for (tempptr = local_name; *tempptr ne '\0'; tempptr++)
      { if ((*tempptr >= 'a') && (*tempptr <= 'z') )
          *tempptr = *tempptr - 'a' + 'A';
        if (*tempptr eq '_') *tempptr = '$';
      }
		/* Get rid of trailing ':' if any */
     if (*(--tempptr) eq ':')
       *tempptr = '\0';
   }
   
  return(local_name);
}

PRIVATE string lastbit(string fullname)
{ int i = strlen(fullname) - 1;
  for ( ; (fullname[i] ne '/') && (i >= 0); i--);
  return(&(fullname[i+1]));
}
    
/**
*** Here are the link IO routines. The first routine attempts to
*** resynchronise if things go wrong, by sending 10 synch bytes
*** at one-second intervals. Hopefully at the end of these 10
*** seconds the MiniServer will have recovered. The next
*** two routines deal with locking, but using timeouts. It is
*** undesirable to wait more than 5 seconds for the link to become
*** available, because the client side expects a reply. In the
*** absence of a timed semaphore mechanism in the kernel, I have
*** used a hack. Note that LinkWait() also protects misc_buffer.
**/

PRIVATE void LinkRecover(void)
{ int i;
_Trace(0x001, 0, 0);

  while (!LinkWait());
  for (i = 0; i < 10; i++)
   { ByteDownLink(Pro_Synch);
     Delay(OneSec);
   }
  LinkSignal();
}

PRIVATE bool LinkWait()
{ int i, j;
  extern int TestWait(Semaphore *);
  
  for (i = 0; i < 20; i++)
   { Wait(&LinkWaitLock);
     j = TestWait(&LinkAvailable);
     Signal(&LinkWaitLock);
     if (j)
      return(TRUE);
     if (i < 19) Delay(OneSec / 4);
   }
_Trace(0x2,0,0);
  return(FALSE);
}

PRIVATE void LinkSignal()
{
  Signal(&LinkAvailable);
}

PRIVATE bool ByteDownLink(WORD ch)
{ BYTE tab[1];
  tab[0] = ch;
  return((LinkOut(1, BootLink, tab, 5 * OneSec) eq 0) ? TRUE : FALSE);
}

PRIVATE WORD ByteFromLink()
{ WORD ch = 0;
  if (LinkIn(1, BootLink, &ch, 20 * 60 * OneSec) < 0)
    return(-1);
  else
    return(ch);
}

PRIVATE bool DataDownLink(BYTE *buffer, WORD amount)
{ 
  if (LinkOut(amount, BootLink, buffer, 10 * OneSec) < 0)
   { Fork(1000, &LinkRecover, 0);
     return(FALSE);
   }
  else
   return(TRUE);
}

PRIVATE bool DataFromLink(BYTE *buffer, WORD amount)
{ WORD result;
  result = LinkIn(amount, BootLink, buffer, 20 * 60 * OneSec);
  return((result >= 0) ? TRUE : FALSE);
}

/**
*** Here are the routines that actually do the work.
**/

PRIVATE WORD exists_obj(string localname, int *timedout)
{ WORD reply = Nothing_t;

  if (!LinkWait())
   { *timedout = 1; return(Nothing_t); }
   
  { FullHead head;
    WORD len = strlen(localname) + 1;
    head.protocol	= Pro_IOServ;
    head.fncode		= Fun_Locate;
    head.highsize	= len / 256;
    head.lowsize	= len % 256;
    if (!DataDownLink((BYTE *) &head, sizeofFullHead))
     { *timedout = 1; goto done; }
    if (!DataDownLink(localname, len))
     { *timedout = 1; goto done; }
  }
  Wait(&ReplyAvailable);
  { Head head;
    WORD amount;

    (void) DataFromLink((BYTE *) &head, sizeofHead);
    amount = (256 * head.highsize) + head.lowsize;
    if (amount ne 0)
      (void) DataFromLink(&(misc_buffer[0]), amount);
    if (head.fncode eq Rep_Success)	/* Does it exist ? */
     reply = head.extra;
  }
done:
  LinkSignal();
  Signal(&Done);
  return(reply);
}

PRIVATE int open_file(STRING localname, WORD openmode, int *timedout)
{ int stream = -1;
  if (!LinkWait())
   { *timedout = 1; return(-1); }
   
  { FullHead head;
    WORD len = strlen(localname) + 1;
    head.protocol	= Pro_IOServ;
    head.fncode		= Fun_OpenFile;
    head.extra		= openmode;
    head.highsize	= len / 256;
    head.lowsize	= len % 256;
    if (!DataDownLink((BYTE *) &head, sizeofFullHead))
     { *timedout = 1; goto done; }
    if (!DataDownLink(localname, len))
     { *timedout = 1; goto done; }
  }
  Wait(&ReplyAvailable);	/* Get the reply to open request */
  { WORD amount;
    Head head;

    (void) DataFromLink((BYTE *) &head, sizeofHead);
    amount = (256 * head.highsize) + head.lowsize;
    if (amount ne 0)
      (void) DataFromLink(&(misc_buffer[0]), amount);

    if (head.fncode eq Rep_Success)
      stream = (misc_buffer[0] << 8) + misc_buffer[1];
  }
done:
  LinkSignal();
  Signal(&Done);
  return(stream);
}

PRIVATE WORD seek_in_file(int fildes, WORD mode, WORD newpos, int *timedout)
{ WORD result = -1;
			/* send the seek request to the mini server */
  if (!LinkWait())
   { *timedout =1; return(-1); }
   
  { FullHead head;
    head.protocol	= Pro_IOServ;
    head.fncode		= Fun_SeekInFile;
    head.extra		= mode;
    head.highsize	= 0;
    head.lowsize	= 6;
    if (!DataDownLink((BYTE *) &head, sizeofFullHead))
     { *timedout = 1; goto done; }
     
    misc_buffer[0] = (newpos >> 24) & 0x00FF;
    misc_buffer[1] = (newpos >> 16) & 0x00FF;
    misc_buffer[2] = (newpos >> 8)  & 0x00FF;
    misc_buffer[3] = newpos & 0x00FF;
    misc_buffer[4] = (fildes >> 8) & 0x00FF;
    misc_buffer[5] = fildes & 0x00ff;
    if (!DataDownLink(&(misc_buffer[0]), 6))
     { *timedout = 1; goto done; }
  }
  Wait(&ReplyAvailable);	/* Get the reply */
  { Head head;
    WORD amount;

    (void) DataFromLink((BYTE *) &head, sizeofHead);
    amount = (256 * head.highsize) + head.lowsize;
    if (amount ne 0)
      (void) DataFromLink(&(misc_buffer[0]), amount);

    if (head.fncode eq Rep_Success)
     result = (misc_buffer[0] << 24) + (misc_buffer[1] << 16) +
		(misc_buffer[2] << 8) + misc_buffer[3];
  }
done:
  LinkSignal();
  Signal(&Done);
  return(result);
}

PRIVATE WORD read_from_file(int fildes, BYTE *buffer, WORD to_read, int *timedout)
{ WORD read_this_time = -1;

  if (!LinkWait())
   { *timedout = 1; return(-1); }

  { FullHead head;
    head.protocol	= Pro_IOServ;
    head.fncode	= Fun_ReadFile;
    head.highsize	= 0;
    head.lowsize	= 4;
    misc_buffer[0]	= (fildes >> 8) & 0x00FF;
    misc_buffer[1]	= fildes & 0x00FF;
    misc_buffer[2]	= (to_read >> 8) & 0x00FF;
    misc_buffer[3]	= to_read & 0x00FF;
    if (!DataDownLink( (BYTE *) &head, sizeofFullHead))
     { *timedout = 1; goto done; }
    if (!DataDownLink( (BYTE *) &(misc_buffer[0]), 4))
     { *timedout = 1; goto done; }
  }
  { Head	head;
    int         size;
    Wait(&ReplyAvailable);
    (void) DataFromLink( (BYTE *) &head, sizeofHead);
    size = (256 * head.highsize) + head.lowsize;
    if (size ne 0)
     (void) DataFromLink( buffer, size);
    if (head.fncode eq Rep_Success)
     read_this_time = size;
  }
done:
  LinkSignal();
  Signal(&Done);
  return(read_this_time);
}

PRIVATE WORD write_to_file(int fildes, BYTE *ptr, WORD to_write, int *timedout)
{ WORD result = FALSE;
  if (!LinkWait())
   { *timedout = 1; return(FALSE); }
  
 { FullHead	head;
   head.protocol	= Pro_IOServ;
   head.fncode	= Fun_WriteFile;
   head.highsize	= (to_write + 2) / 256;
   head.lowsize	= (to_write + 2) % 256;
   misc_buffer[0]	= (fildes >> 8) & 0x00FF;
   misc_buffer[1]	= fildes & 0x00FF;
 
   if (!DataDownLink((BYTE *) &head, sizeofFullHead))
    { *timedout = 1; goto done; }
   if (!DataDownLink(&(misc_buffer[0]), 2))
    { *timedout = 1; goto done; }
   if (!DataDownLink(ptr, to_write))
    { *timedout = 1; goto done; }
 }
 { Head head;
   WORD amount;
   Wait(&ReplyAvailable);
   (void) DataFromLink((BYTE *) &head, sizeofHead);
   amount = (head.highsize * 256) + head.lowsize;
   if (amount ne 0)
     (void) DataFromLink(&(misc_buffer[0]), amount);

   if (head.fncode eq Rep_Success)
    result = TRUE;
 }
done:
 LinkSignal();
 Signal(&Done);
 return(result);
}

PRIVATE void close_file(int fildes, int *timedout)
{
 if (!LinkWait())
  { *timedout = 1; return; }
  
 { FullHead	head;
   head.protocol	= Pro_IOServ;
   head.fncode		= Fun_CloseFile;
   head.highsize	= 0;
   head.lowsize		= 2;
   misc_buffer[0]	= (fildes >> 8) & 0x00FF;
   misc_buffer[1]	= fildes & 0x00FF;
   if (!DataDownLink( (BYTE *) &head, sizeofFullHead))
    { *timedout = 1; goto done; }
   if (!DataDownLink(&(misc_buffer[0]), 2))
    { *timedout = 1; goto done; }
 }
 { Head	head;
   WORD amount;
   Wait(&ReplyAvailable);
   (void) DataFromLink( (BYTE *) &head, sizeofHead);
   amount = (head.highsize << 8) + head.lowsize;
   if (amount ne 0)
    (void) DataFromLink(&(misc_buffer[0]), amount);
 }
done:
  LinkSignal();
  Signal(&Done);
}

PRIVATE DirStream *read_dir(STRING localname, int *timedout)
{ DirStream *stream = Null(DirStream);
  WORD	number = -1;
  if (!LinkWait())
   { *timedout = 1; return(Null(DirStream)); }
   
  { FullHead head;
    WORD len = strlen(localname) + 1;
    head.protocol	= Pro_IOServ;
    head.fncode		= Fun_ReadDir;
    head.highsize	= len / 256;
    head.lowsize	= len % 256;
    if (!DataDownLink((BYTE *) &head, sizeofFullHead))
     { *timedout = 1; goto done; }
    if (!DataDownLink(localname, len))
     { *timedout = 1; goto done; }
  }
  Wait(&ReplyAvailable);	/* Get the reply to open request */
  { WORD amount;
    Head head;
    
    (void) DataFromLink((BYTE *) &head, sizeofHead);
    amount = (256 * head.highsize) + head.lowsize;
    if (amount ne 0)
      (void) DataFromLink(&(misc_buffer[0]), amount);

    if (head.fncode eq Rep_Success)
     number = ((misc_buffer[0] << 8) & 0x00FF) + misc_buffer[1];
  }

done:
  if (number eq -1)
   { LinkSignal();
     Signal(&Done);
     return(Null(DirStream));
   }
  elif((stream = (DirStream *)
         Malloc(sizeof(DirStream) + (number * sizeof(DirEntry)))) eq
         Null(DirStream))
   { LinkSignal();
     Signal(&Done);
     return(Null(DirStream));
   }

  stream->number = number;
  stream->offset = 0;
  { BYTE *ptr = &(misc_buffer[2]), *name;
    WORD i;
    DirEntry *entry = (DirEntry *) &(stream->entries[0]);
    for (i = 0; i < number; i++, entry++)
     { entry->Type = (*ptr++ eq Dir_t) ? Type_Directory : Type_File;
       entry->Flags = 0;
       entry->Matrix = (entry->Type eq Type_Directory) ? DefDirMatrix :
			DefFileMatrix;
       for ( name = &(entry->Name[0]); *ptr ne '\0'; )
        *name++ = *ptr++;
       *name++ = *ptr++;	/* Another one for '\0' */

		/* Lower case the file name on a DP2 */
       if (Host eq DP2)
         for (name = &(entry->Name[0]); *name ne '\0'; name++)
          if ( ('A' <= *name) && (*name <= 'Z') )
            *name = *name - 'A' + 'a'; 
     }
  }
  LinkSignal();
  Signal(&Done);

  return(stream);
}

PRIVATE WORD create_object(STRING localname, WORD type, int *timedout)
{ WORD result = FALSE;

 if (!LinkWait())
   { *timedout = 1; return(FALSE); }
   
  { FullHead head;
    WORD len = strlen(localname) + 1;
    head.protocol	= Pro_IOServ;
    head.fncode		= (type eq Type_File) ? Fun_CreateFile :
			 		        Fun_CreateDir;
    head.highsize	= len / 256;
    head.lowsize	= len % 256;
    if (!DataDownLink((BYTE *) &head, sizeofFullHead))
     { *timedout = 1; goto done; }
    if (!DataDownLink(localname, len))
     { *timedout = 1; goto done; }
  }
  Wait(&ReplyAvailable);
  { Head head;
    WORD amount;

    (void) DataFromLink((BYTE *) &head, sizeofHead);
    amount = (256 * head.highsize) + head.lowsize;
    if (amount ne 0)
      (void) DataFromLink(&(misc_buffer[0]), amount);
    if (head.fncode eq Rep_Success) result = TRUE;
  }
done:
  LinkSignal();
  Signal(&Done);
  return(result);
}

PRIVATE WORD get_file_info(STRING localname, WORD *sizeptr, Date *dateptr, int *timedout)
{ WORD result = FALSE;
  if (!LinkWait())
   { *timedout = 1; return(FALSE); }
   
  { FullHead head;
    WORD len = strlen(localname) + 1;
    head.protocol	= Pro_IOServ;
    head.fncode		= Fun_FileInfo;
    head.highsize	= len / 256;
    head.lowsize	= len % 256;
    if (!DataDownLink((BYTE *) &head, sizeofFullHead))
     { *timedout = 1; goto done; }
    if (!DataDownLink(localname, len))
     { *timedout = 1; goto done; }
  }
  Wait(&ReplyAvailable);	/* And get the reply */
  { Head head;
    WORD amount;

    (void) DataFromLink((BYTE *) &head, sizeofHead);
    amount = (256 * head.highsize) + head.lowsize;
    if (amount ne 0)
      (void) DataFromLink(&(misc_buffer[0]), amount);

    *sizeptr = (misc_buffer[0] << 24) + (misc_buffer[1] << 16) +
               (misc_buffer[2] << 8) + misc_buffer[3];
    *dateptr = (Date) ( (misc_buffer[4] << 24) + (misc_buffer[5] << 16) +
               (misc_buffer[6] << 8) + misc_buffer[7] );
    if (head.fncode eq Rep_Success) result = TRUE;
  }
done:
    LinkSignal();
    Signal(&Done);
    return(result);
}

PRIVATE WORD delete_object(STRING localname, WORD exists, int *timedout)
{ WORD result = FALSE;
  if (!LinkWait())
   { *timedout = 1; return(FALSE); }
   
  { FullHead head;
    WORD len = strlen(localname) + 1;
    head.protocol	= Pro_IOServ;
    head.fncode		= (exists eq Dir_t) ? Fun_RemoveDir : Fun_DeleteFile;
    head.highsize	= len / 256;
    head.lowsize	= len % 256;
    if (!DataDownLink((BYTE *) &head, sizeofFullHead))
     { *timedout = 1; goto done; }
    if (!DataDownLink(localname, len))
     { *timedout = 1; goto done; }
  }
  Wait(&ReplyAvailable);
  { Head head;
    WORD amount;

    (void) DataFromLink((BYTE *) &head, sizeofHead);
    amount = (256 * head.highsize) + head.lowsize;
    if (amount ne 0)
      (void) DataFromLink(&(misc_buffer[0]), amount);
    if (head.fncode eq Rep_Success) result = TRUE;
  }
done:
  LinkSignal();
  Signal(&Done);
  return(result);
}

PRIVATE WORD rename_file(STRING sourcename, STRING destname, int *timedout)
{ WORD result = FALSE;

  if (!LinkWait())
   { *timedout = 1; return(FALSE); }
   	
  { WORD size = strlen(sourcename) + strlen(destname) + 2;
    FullHead head;
    head.protocol = Pro_IOServ;
    head.fncode   = Fun_Rename;
    head.highsize = size / 256;
    head.lowsize  = size % 256;

    if (!DataDownLink((BYTE *) &head, sizeofFullHead))
     { *timedout = 1; goto done; }
    if (!DataDownLink(sourcename, strlen(sourcename) + 1))
     { *timedout = 1; goto done; }
    if (!DataDownLink(destname, strlen(destname) + 1))
     { *timedout = 1; goto done; }
  }
  Wait(&ReplyAvailable);
  { Head head;
    WORD amount;
    (void) DataFromLink((BYTE *) &head, sizeofHead);
    amount = (256 * head.highsize) + head.lowsize;
    if (amount ne 0)
      (void) DataFromLink(&(misc_buffer[0]), amount);
    if (head.fncode eq Rep_Success) result = TRUE;
  }
done:
  LinkSignal();
  Signal(&Done);
  return(result);
}

PRIVATE WORD drive_statistics(STRING localname, WORD *sizeptr, WORD *availptr, int *timedout)
{ WORD result = FALSE;
  int i;
  for (i = 0; localname[i] ne ':'; i++);
  localname[i++] = '\0';

  if (!LinkWait())
   { *timedout = 1; return(FALSE); }
   
  { FullHead head;
    head.protocol	= Pro_IOServ;
    head.fncode		= Fun_DiskUsage;
    head.highsize	= i / 256;
    head.lowsize	= i % 256;
    if (!DataDownLink( (BYTE *) &head, sizeofFullHead))
     { *timedout = 1; goto done; }
    if (!DataDownLink(localname, i))
     { *timedout = 1; goto done; }
  }
  Wait(&ReplyAvailable);
  { Head head;
    WORD amount;
    (void) DataFromLink((BYTE *) &head, sizeofHead);
    amount = (256 * head.highsize) + head.lowsize;
    if (amount ne 0)
      (void) DataFromLink(&(misc_buffer[0]), amount);

    *sizeptr = (misc_buffer[0] << 24) + (misc_buffer[1] << 16) +
               (misc_buffer[2] << 8) + misc_buffer[3];
    *availptr = (misc_buffer[4] << 24) + (misc_buffer[5] << 16) +
               (misc_buffer[6] << 8) + misc_buffer[7];

    *sizeptr *= 1024; *availptr *= 1024;
    if (head.fncode eq Rep_Success) result = TRUE;
  }
done:
    LinkSignal();
    Signal(&Done);
    return(result);
}

PRIVATE WORD change_date(STRING localname, int *timedout)
{ WORD result = FALSE;
  if (!LinkWait())
   { *timedout = 1; return(FALSE); }
   
  { FullHead head;
    WORD len = strlen(localname) + 1;
    head.protocol	= Pro_IOServ;
    head.fncode		= Fun_ChangeDate;
    head.highsize	= len / 256;
    head.lowsize	= len % 256;
    if (!DataDownLink((BYTE *) &head, sizeofFullHead))
     { *timedout = 1; goto done; }
    if (!DataDownLink(localname, len))
     { *timedout = 1; goto done; }
  }
  Wait(&ReplyAvailable);
  { Head head;
    WORD amount;

    (void) DataFromLink((BYTE *) &head, sizeofHead);
    amount = (256 * head.highsize) + head.lowsize;
    if (amount ne 0)
      (void) DataFromLink(&(misc_buffer[0]), amount);
    if (head.fncode eq Rep_Success) result = TRUE;
  }
done:
    LinkSignal();
    Signal(&Done);
    return(result);
}

void write_to_screen(STRING str, int *timedout)
{
  if (!LinkWait())
   { *timedout = 1; return; }
   
  { WORD size = strlen(str) + 1;
    FullHead header;
    header.protocol	= Pro_IOServ;
    header.fncode	= Fun_ScreenWrite;
    header.highsize	= size / 256;
    header.lowsize	= size % 256;

    if (!DataDownLink((BYTE *) &header, sizeofFullHead))
     { *timedout = 1; goto done; }
    if (!DataDownLink(str, size))
     { *timedout = 1; goto done; }
  }
  Wait(&ReplyAvailable);	/* Then wait for the reply */
  { Head head;
    WORD amount;
    (void) DataFromLink((BYTE *) &head, sizeofHead);
    amount = (256 * head.highsize) + head.lowsize;
    if (amount ne 0)
      (void) DataFromLink(&(misc_buffer[0]), amount);
  }
done:
  LinkSignal();			/* Let another process/server write down link */
  Signal(&Done);		/* And reactivate the link guardian */
} 



/**
*** Utility routines
**/
void memmove(UBYTE *a, UBYTE *b, WORD n)
{ UBYTE *ca,*cb;
  if (a < b)
   for (ca = (UBYTE *)a, cb = (UBYTE *)b; n-- > 0;) *ca++ = *cb++;
  else for (ca = n+(UBYTE *)a, cb = n+(UBYTE *)b; n-- > 0;) *--ca = *--cb;
}


