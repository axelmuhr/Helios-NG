/*----------------------------------------------------------------
--                                                              --
--        H E L I O S   M S D O S - F S   S O F T W A R E	--
--        -----------------------------------------------	--
--                                                              --
--          Copyright (C) 1991, Perihelion Software Ltd.        --
--          Copyright (C) 1990, Telmat Informatique.            --
--                     All Rights Reserved.                     --
--                                                              --
-- msdosfs.c							--
--                                                              --
--	The main module of the MSDOSFS Server			--
--                                                              --
--	Author:  Olivier Reins, Martyn Tovey                    --
--                                                              --
----------------------------------------------------------------*/

static char *rcsid = "$Header: /hsrc/servers/msdosfs/RCS/msdosfs.c,v 1.3 1991/03/28 18:04:09 martyn Exp $";

#include "msdosfs.h"
#include "prototype.h"

/*----------------------------------------------------------------------*/
/*				main()					*/
/*----------------------------------------------------------------------*/

int main(int argc, char **argv)
{
  char infoname[10];

  /* check for different server name */

  if(argc > 2)
	{
	fprintf(stderr, "%s : usage -\n\t%s [servername]\n", argv[0], argv[0]);
	fflush(stderr);
	exit(-1);
	}

  if(argc == 2)
	{
	char *p=argv[1];
	while(*p && *p == '/')
		p++;
	strncpy(floppy.Name, p, 32);
	}

  IOdebug("");
  IOdebug("msdosfs %s : starting server /%s", VERSION, floppy.Name);
  IOdebug("");
  
  Server_StartTime = GetDate();

	/* trap kill signal to exit tidily */
  act.sa_handler = handle_interrupt;
  act.sa_mask = 0;
  act.sa_flags = SA_ASYNC;
  (void) sigaction(SIGINT, &act, NULL);
  (void) sigaction(SIGHUP, &act, NULL);

  MachineName(Machine_Name);

#ifdef debug
  IOdebug("msdosfs : Machine_Name = %s",Machine_Name);
#endif

  InitSemaphore(&Time_access, 1);
  InitSemaphore(&Var_access, 1);

/* @@@ set up for 720k disc for testing purposes */

  driveinfo.Next            =         -1; /* driver uses -1 for list end */
  driveinfo.DriveId         =          0;
  driveinfo.DriveType       =         (DT_MFM | DT_IBM);
  driveinfo.SectorSize      =        512;
  driveinfo.SectorsPerTrack =          9;
  driveinfo.TracksPerCyl    =          2;
  driveinfo.Cylinders       =          NCYLS;

  Sector_size = driveinfo.SectorSize;

  partinfo.Next             =         -1; /* driver uses -1 for end of list */
  partinfo.Drive            =          0;
  partinfo.StartCyl         =          0;
  partinfo.EndCyl           =         79;
  partinfo.StartSector      =          0;

  strcpy(infoname, "floppy");
  discinfo.Name             =          (RPTR)&infoname;
  discinfo.Name             =          ATOR(discinfo.Name);
  discinfo.Controller       =          FLOPPY_BASE;
  discinfo.Addressing       =          Sector_size; /*for blkSize field of DCB*/
  discinfo.Mode             =          0;
  discinfo.Drives           =          (RPTR)&driveinfo;
  discinfo.Drives           =          ATOR(discinfo.Drives);
  discinfo.Partitions       =          (RPTR)&partinfo;
  discinfo.Partitions       =          ATOR(discinfo.Partitions);

  dcb = OpenDevice(DEVICE_DIRECTORY, &discinfo);

  if( dcb == NULL )
  { 
#ifdef debug
	IOdebug("msdosfs ERROR : main : can't boot driver");
#endif
     return -1;
  }

  GSPServer(&floppy, DefDirMatrix);
}


/*----------------------------------------------------------------------*/
/*			handle_interrupt()				*/
/*----------------------------------------------------------------------*/

void handle_interrupt(void)
{
  IOdebug("msdosfs : terminated by signal");

  CloseDevice((DCB*)dcb);

  if(name_table_entry != Null(Object) )
      Delete(name_table_entry, Null(char));
  exit(12);
}

/*----------------------------------------------------------------------*/
/*				NewName()				*/
/*----------------------------------------------------------------------*/

/* This routine creates a new name in the name table. */

void NewName(string name, Port port, word matrix)
{ NameInfo info;
  Object   *processor = Locate(Null(Object), Machine_Name);
  
  if (processor == Null(Object)) return;
  
  info.Port	= port;
  info.Flags	= Flags_StripName;
  info.Matrix	= matrix;
  info.LoadData = NULL;		/* Not used at present */

  name_table_entry = Create(processor, name, Type_Name, sizeof(NameInfo),
			    (byte *)&info);
  Close(processor);
  if(name_table_entry == Null(Object))
	{
  	char *msgbuf = Malloc(MESSAGE_SIZE);
	if(msgbuf == NULL)
		IOdebug("msdosfs : Can't create name table entry");
	else
		{
		Fault(Result2(processor), msgbuf, MESSAGE_SIZE);
		IOdebug("msdosfs : %s", msgbuf);
		}
	CloseDevice((DCB *)dcb);
	Free(msgbuf);
	exit(-1);
	}
	
#ifdef debug
 IOdebug("msdosfs : NewName(%s)",name);
#endif
}

/*----------------------------------------------------------------------*/
/*			GSPServer() 					*/ 
/*----------------------------------------------------------------------*/

void GSPServer(MyDevice *Device, WORD matrix)
{
  Message	*msg;

#ifdef debug
	IOdebug("msdosfs : GSPServer");
#endif

	/* allocate a port for server request */
  if ((Device->Port= NewPort()) == NullPort)
    return;

	/* put the server's name in the name table */
  NewName(&(Device->Name[0]), Device->Port, matrix);

	/* Initialize access time */
  Last_time = 0;

  forever
   {		/* allocate a buffer */
     if ((msg = (Message *) Malloc(sizeof(Message))) == Null(Message))
       {
	 Delay((word) (OneSec * 5));
	 continue;
       }


     msg->mcb.MsgHdr.Dest	= Device->Port;
     msg->mcb.Timeout		= (word) (OneSec * 60 * 30); 
     msg->mcb.Control		= &(msg->control[0]);
     msg->mcb.Data		= &(msg->data[0]);
     
     forever
	{
     	while ( GetMsg(&(msg->mcb)) == EK_Timeout);

#ifdef debug
	IOdebug("msdosfs :GSPServer get a %x msg, call a %d worker",
           msg->mcb.MsgHdr.FnRc & FG_Mask, ++num_worker);
#endif

     	unless( Fork(Stacksize, GSPWorker, 8, Device, &(msg->mcb)) )
     		{
#ifdef debugE
	 	IOdebug("msdosfs ERROR : GSPServer : failed to fork");
#endif
		SendError(&(msg->mcb),
		  EC_Error + SS_FloppyDisk + EG_NoMemory + EO_Server);
		continue;
      		}
	break;
	}
  }
}

/*----------------------------------------------------------------------*/
/*			GSPWorker() 					*/ 
/*----------------------------------------------------------------------*/

/** GSPWorker calls an appropriate action routine for message
**/

void GSPWorker(MyDevice *Device, MCB *mcb)
{ WORD	    fn = mcb->MsgHdr.FnRc;
  VoidFnPtr fun;
  string    fullname;
  Date      receipt;
  
  if ((fn & FC_Mask) != FC_GSP)
   {
#ifdef debugE
     IOdebug("msdosfs ERROR: GSPWorker : fn not GSP");
     num_worker--;
#endif
     SendError(mcb, EC_Error + SS_FloppyDisk + EG_FnCode + EO_Message);
     Free(mcb);
     return;
   }

  if (fn == 0)		/* ReplyOK message ? */
  { 
#ifdef debugE
	IOdebug("msdosfs ERROR: GSPWorker : fn = NULL");
	num_worker--;
#endif
    Free(mcb);
    return;
  }

  fn &= FG_Mask;
  if ( (fn < FG_Open) || (fn > FG_CloseObj))
   {
    SendError(mcb, EC_Error + SS_FloppyDisk + EG_WrongFn + EO_Message);
#ifdef debugE
	IOdebug("msdosfs ERROR : GSPWorker : wrong FN");
	num_worker--;
#endif
     Free(mcb);
     return;
   }

	/* if request on parent of root dos -> error */

  if ((fullname = GetFullName(&(Device->Name[0]), mcb)) == NULL)
  {
#ifdef debugE
	IOdebug("msdosfs ERROR : GSPWorker : name = null");
	num_worker--;
#endif
     SendError(mcb, EC_Error + SS_FloppyDisk + EG_Unknown + EO_File);
     Free(mcb);
     return;
  }

  /** if the last access time is too old, re-initialise and
  *** reload the fat. 
  **/

  Wait( &Time_access );
  receipt = GetDate();

  if( (receipt-Last_time) > (Date) MAX_TIME_ACCESS || (Last_time == (Date) 0) )
  {
    int reret;

    Wait( &Var_access );
    reret = reload();
    Signal( &Var_access );

    if( reret )		/* re-load failed for some reason */
    {
      SendError(mcb, EC_Error + SS_FloppyDisk + reret);
      Signal(&Time_access);
#ifdef debug
	num_worker--;
#endif
      Free(fullname);
      Free(mcb);
      return;
    }
  }

  Last_time = receipt;
  Signal(&Time_access);
      
#ifdef debug
    IOdebug("msdosfs : GSPWorker : dispatch to %d",(fn-FG_Open)>> FG_Shift);
#endif
  fun = Device->Handlers[(fn - FG_Open) >>  FG_Shift];
  (*fun)(mcb, fullname);
  Free(fullname);
  Free(mcb);
#ifdef debug
	IOdebug("msdosfs :$ GSPWorker");
	num_worker--;
#endif
}
  
		/********   The Disk Drive routines   ********/
	
/*----------------------------------------------------------------------*/
/*			Drive_Open()					*/
/*----------------------------------------------------------------------*/

void Drive_Open(MCB *mcb, string fullname)
{ string	localname = GetLocalName(fullname);
  int		exists;
  WORD		openmode;
  IOCMsg2	*msg = (IOCMsg2 *) mcb->Control;

#ifdef debug
	IOdebug("msdosfs : Drive_Open(%s)",fullname);
#endif
  
  if (localname == Null(char))
  { 
#ifdef debugE
	IOdebug("msdosfs ERROR : Drive_Open : name = null");
#endif
   SendError(mcb, EC_Error + SS_FloppyDisk + EG_NoMemory + EO_Server);
   return;
  }

  if (localname == (char *) -1)
  { 
#ifdef debugE
	IOdebug("msdosfs ERROR : Drive_Open : name invalid");
#endif
   SendError(mcb, EC_Error + SS_FloppyDisk + EG_Name + EO_File);
   return;
  }

  openmode = msg->Arg.Mode;

	/* does object exist ? If it does,what type (file, dir)? */
  exists = exists_obj(localname);

   if( exists != Type_Directory && exists != Type_File && 
	exists != Type_Nothing ) 	/* Some sort of disk error */
   {
     SendError(mcb, EC_Error + SS_FloppyDisk + exists);
     Free(localname);
     return;
   }

	/* if already exists and to be created -> error */
  if ((exists != Type_Nothing) && (openmode & O_Create) &&
	(openmode & O_Exclusive))
  { 
#ifdef debugE
  	IOdebug("msdosfs ERROR : Drive_Open : %s exist->can't be created",
		localname);
#endif

   SendError(mcb, EC_Error + SS_FloppyDisk + EG_InUse + EO_Name);
   Free(localname);
   return;
  }
   
	/* if object==file, call File_Open() */
  if (exists == Type_File)
    File_Open(mcb, fullname, localname, openmode);

	/* elif object==dir, call Dir_Open() */
  elif (exists == Type_Directory)
  {  if ( ((openmode & O_Mask) == O_ReadOnly) ||
	   ((openmode & O_Mask) == O_ReadWrite) )
         Dir_Open(mcb, fullname, localname);
      else
      {
#ifdef debugE
	  IOdebug("msdosfs ERROR : Drive_Open : incorrect access for dir (%s)",
	 		localname);
#endif
	SendError(mcb, EC_Error + SS_FloppyDisk + EG_WrongFn + EO_Directory);
      }
  }

  else
	/* else if file doesn't exist : try to create it ... */
  { if ((openmode & O_Create) == 0)
    { 
#ifdef debugE
	  IOdebug("msdosfs ERROR : Drive_Open : no right to create %s",
			localname);
#endif
	SendError(mcb, EC_Error + SS_FloppyDisk + EG_Unknown + EO_File);
	Free(localname);
        return;
     }
     elif( create_obj(localname, Type_File)<0 )
     { SendError(mcb, EC_Error + SS_FloppyDisk + EG_Create + EO_File);
	Free(localname);
        return;
     }
 
	/* ... and call File_Open */
     File_Open(mcb, fullname, localname, openmode &= ~O_Create);
   }

  Free(localname);

#ifdef debug
     IOdebug("msdosfs :$ Drive_Open ");
#endif
}

/** Olivier :
*** I don't limit the number of open streams at any one time.
*** The system doesn't impose it, and the floppy is not to be 
*** used as intensively as for a PC-host based architecture.
**/


	/** open a file, send a msg port and wait for request **/

/*----------------------------------------------------------------------*/
/*			File_Open() 					*/ 
/*----------------------------------------------------------------------*/

void File_Open(MCB *mcb, string fullname, string localname,
			 WORD openmode)
{ 
  BYTE		*data = mcb->Data;
  FileInfo	file_info;
  int		exists;


#ifdef debug
  IOdebug("msdosfs : File_Open(%s) mode=%d",fullname,openmode);
#endif
        
	/* get a new port for requests that follow */
  if ((file_info.Port = NewPort()) == NullPort)
  {
#ifdef debugE
     IOdebug("msdosfs ERROR : File_Open : can't get a NewPort()");
#endif
     SendError(mcb, EC_Error + SS_FloppyDisk + EG_Congested + EO_Server);
     return;
   }

  if(openmode & O_Truncate)	/* file is to be truncated */
	{
#ifdef debug
	  IOdebug("msdosfs : Truncate file %s", fullname);
#endif
  	  if( (exists = delete_object(localname))  < 0 ) /* delete */
   	  {
     	    SendError(mcb, EC_Error + SS_FloppyDisk + EG_Open + EO_Server);
     	    return;
   	  }

     	  if( create_obj(localname, Type_File)<0 )      /* re-create */
	  {
     	    SendError(mcb, EC_Error + SS_FloppyDisk + EG_Open + EO_File);
            return;
     	  }
	}

	/* try to open the file */

  if( open_file(localname, &file_info) )
    				/* The file is known to exist, so it is */
				/* strange that it cannot be opened */
   {
     SendError(mcb, EC_Error + SS_FloppyDisk + EG_Open + EO_File);
     FreePort(file_info.Port);
     return;
   }

	/* all's ok, we send reply with port ... */

#ifdef debug
	IOdebug("msdosfs : File_Open : SendOpenReply, wait msg ...");
#endif		

  SendOpenReply(mcb, fullname, Type_File, Flags_Closeable + Flags_MSdos,
		file_info.Port);
  file_info.Pos = 0;
  
		/* ... we wait for request message */
  forever
   { WORD errcode;
     Date receipt = GetDate();
     mcb->MsgHdr.Dest	= file_info.Port;
     mcb->Timeout	= (word) (StreamTimeout);
     mcb->Data		= data;

    		/* get a message */ 
     if ((errcode = GetMsg(mcb)) == EK_Timeout)
	{
#ifdef debug
	IOdebug("msdosfs WARN : File_Open : Timedout -> close file");
#endif
	 break;
	}
#ifdef debug
	IOdebug("msdosfs : File_Open : get a message %x", errcode & FG_Mask);
#endif
     receipt = GetDate();

	/* look for return code */
	/* that gives error code (<0) or object type */
     if (errcode < Err_Null)
      { if ((errcode & EC_Mask) >= EC_Error)
         break;
        else
         continue;
       }
          
	/* if we don't get a GSP request -> error */
     if ((errcode & FC_Mask) != FC_GSP)
      { SendError(mcb, EC_Error + SS_FloppyDisk + EG_WrongFn +EO_Server);
        continue;
      }

	/* we dispatch the request */
     switch ( errcode & FG_Mask )
      { case FG_Read : 
		    if (openmode & O_ReadOnly)
			File_Read(mcb, &file_info);
		    else
      			SendError(mcb,
			 EC_Error + SS_FloppyDisk + EG_WrongFn + EO_File);
 
		    break;

	case FG_Write :
		    if (openmode & O_WriteOnly)
			File_Write(mcb, &file_info);
		    else
      			SendError(mcb,
			 EC_Error + SS_FloppyDisk + EG_WrongFn + EO_File);
 
		    break;

	case FG_Close	: if (mcb->MsgHdr.Reply != NullPort)
                              Return(mcb, ReplyOK, 0, 0);
#ifdef debug
			  IOdebug("msdosfs : File_Open : close Request");
#endif
                          goto finished;
                          
	case FG_Seek	: File_Seek(mcb, &file_info);
			  break;

	case FG_GetSize	: File_GetSize(mcb, &file_info);
	                  break;

	case FG_SetSize	        :
	case FG_GetInfo	        :
	case FG_SetInfo	        :
	case FG_EnableEvents	:
	case FG_Acknowledge	:
	case FG_NegAcknowledge	:
	default		        :
				SendError(mcb,
				 EC_Error + SS_FloppyDisk
				 + EG_WrongFn +EO_Server);
			  break;
      }
   }

	/* end of request session : we leave */
  finished:
    close_file(&file_info);
    FreePort(file_info.Port);
}


/*----------------------------------------------------------------------*/
/*			File_Read() 					*/ 
/*----------------------------------------------------------------------*/

void File_Read(MCB *mcb, FileInfo *file_info)
{
  ReadWrite *readwrite = (ReadWrite *) mcb->Control;
  WORD	read_so_far,			/* data read till now */
        to_read, 			/* data to read this time */
        real_read, 	
	read_this_time,			/* data actually read this time */
	seq = 0, 			/* count of messages send till now */
	temp;
  bool	eof = (bool) FALSE;
  BYTE  *buffer;
  Port	itsport = mcb->MsgHdr.Reply;
  word  pos;
  
  pos = readwrite->Pos;
    
#ifdef debug
  IOdebug("msdosfs : File_Read, %d to read",readwrite->Size);
#endif
    
  if (readwrite->Size == 0)
    { Return(mcb, ReadRc_EOD, 0, 0);
      return;
    }

  if ((buffer = (BYTE *) Malloc(2*Cluster_byte)) == Null(BYTE))
    {
#ifdef debugE
      IOdebug("msdosfs ERROR : File_Read : can't Malloc Message buffer");
#endif
      SendError(mcb, EC_Error + SS_FloppyDisk + EG_NoMemory + EO_Server);
      return;
    }

	/** send message(s) that contains data of the file to read **/

  for ( read_so_far = 0; (read_so_far < readwrite->Size) && !eof; )
    {
	 to_read = ((readwrite->Size - read_so_far) > (Cluster_byte) ) ?
		 (Cluster_byte) : (readwrite->Size - read_so_far);

		/* Helios can request more than file contains */

      if( pos + to_read > file_info->Size )
	real_read = file_info->Size - pos;
      else real_read = to_read;
      read_this_time = read_from_file(pos, real_read, buffer, file_info);
      if( read_this_time < real_read )
       pos = -1;
      else
       pos += read_this_time;

      read_so_far += read_this_time;

      if (read_this_time < to_read)
	     eof = (bool) TRUE;

      mcb->MsgHdr.Dest	= itsport;
      mcb->MsgHdr.Reply	= NullPort;
      mcb->MsgHdr.Flags	= (eof) ? 0 : MsgHdr_Flags_preserve;

	/* ReadRc_More : if more data to send	*/
	/* ReadRc_EOD  : end of data		*/

      mcb->MsgHdr.FnRc  = seq + (eof ? ReadRc_EOF :
	 (read_so_far >= readwrite->Size) ? ReadRc_EOD : ReadRc_More);
      seq += ReadRc_SeqInc;
      mcb->MsgHdr.ContSize = 0;
      mcb->MsgHdr.DataSize = read_this_time;
      mcb->Data	= buffer;
#ifdef debug
      IOdebug("msdosfs : File_Read : send %d msg",seq);
#endif
      temp = PutMsg(mcb);
    }

#ifdef debug
  IOdebug("msdosfs :$ File_Read");
#endif
  Free(buffer);			
}

/*----------------------------------------------------------------------*/
/*			File_Write() 					*/ 
/*----------------------------------------------------------------------*/

void File_Write(MCB *mcb, FileInfo *file_info)
{
  ReadWrite *readwrite = (ReadWrite *) mcb->Control;
  BYTE *buffer;
  bool ownbuf	= (bool) FALSE;
  Port itsport	= mcb->MsgHdr.Reply, myport = mcb->MsgHdr.Dest;
  WORD timeout, count;
  WORD pos;
  WORD fetched   = 0;

  pos = readwrite->Pos;
  
#ifdef debug
  IOdebug("msdosfs : File_Write : %d to write",readwrite->Size);
#endif
    
	/* no data to write -> Return */
  if (readwrite->Size == 0)
    {
      mcb->Control[0] = 0;
      Return(mcb, ReplyOK, 1, 0);
      return;
    }
  else
   count = readwrite->Size;

  timeout = readwrite->Timeout;

  if (mcb->MsgHdr.DataSize != 0)
  {
#ifdef debug
    IOdebug("msdosfs : File_Write : data in mcb->Data = %d",count);
#endif

    if( write_to_file(pos, mcb->MsgHdr.DataSize, mcb->Data, file_info) 
	       < mcb->MsgHdr.DataSize )
    {
      mcb->MsgHdr.Reply = itsport;
      SendError(mcb, EC_Error + SS_FloppyDisk + EG_NoMemory + EO_File);
      return;
    }

    fetched = mcb -> MsgHdr.DataSize;
  }

  elif ((buffer = (BYTE *) Malloc(Message_Limit)) == Null(BYTE))

  { 
#ifdef debugE
    IOdebug("msdosfs ERROR : File_Write : can't Malloc");
#endif

    mcb->MsgHdr.Reply = itsport;
    SendError(mcb, EC_Error + SS_FloppyDisk + EG_NoMemory + EO_Server);
    return;
  }
	/** ... and wait for msg containing data. **/
  else
    {
      bool skip = (bool) FALSE;
      ownbuf = (bool) TRUE;      

#ifdef debug
    IOdebug("msdosfs : File_Write : count : %d",count);
#endif

		/* The data is read in lumps of upto Message_Limit, */

		/* Start by sending the initial message, giving sizes */

      mcb->Control[0]      = (count > Message_Limit) ? Message_Limit : count;
      mcb->Control[1]      = Message_Limit;
      mcb->MsgHdr.Flags    = MsgHdr_Flags_preserve;
      mcb->MsgHdr.Dest     = itsport;
      mcb->MsgHdr.Reply    = NullPort;
      mcb->MsgHdr.FnRc     = WriteRc_Sizes;
      mcb->MsgHdr.ContSize = 2;
      mcb->MsgHdr.DataSize = 0;
      mcb->Timeout	   = timeout;
#ifdef debug
      IOdebug("msdosfs : File_Write : send initial message");
#endif
      (void) PutMsg(mcb);

		/* Now wait for all the data */
      while (fetched < count)
      {
         mcb->MsgHdr.Dest = myport;
	 mcb->Data	  = buffer;
	 mcb->Timeout	  = timeout;

#ifdef debug
         IOdebug("msdosfs : File_Write : get a bloc ");
#endif
	 if (GetMsg(mcb) < 0)
	  {            		/* Just go back to waiting for GSP request */
	    Free(buffer);
#ifdef debugE
	    IOdebug("msdosfs WARN: File_Write : GetMsg failed");
#endif
	    return;
	  }
	 
         fetched += (word) mcb->MsgHdr.DataSize;

	 if(!skip)
         if( write_to_file(pos, mcb->MsgHdr.DataSize, buffer, file_info) 
	       < mcb->MsgHdr.DataSize )
	   skip = (bool)TRUE;

         pos += mcb->MsgHdr.DataSize;

	 if((mcb -> MsgHdr.FnRc & ReadRc_EOD) == ReadRc_EOD)
		break;

      }

      if(skip)
         {
           Free(buffer);
	   mcb->MsgHdr.Reply = itsport;
	   SendError(mcb, EC_Error + SS_FloppyDisk + EG_NoMemory + EO_File);
	   return;
         }

   }

#ifdef debug
  IOdebug("msdosfs :$ File_Write");
#endif
  mcb->Control[0] = fetched;
  mcb->MsgHdr.Reply = itsport;
  Return(mcb, ReplyOK, 1, 0);
  if (ownbuf)
      Free(buffer);
}

/*----------------------------------------------------------------------*/
/*			File_Seek() 					*/ 
/*----------------------------------------------------------------------*/
/** file seek: three choices, seek beginning, relative or end.
*** Only the latter requires any work
**/

void File_Seek(MCB *mcb, FileInfo *file_info)
{ SeekRequest *req = (SeekRequest *) mcb->Control; 
  WORD newoff = -1;
  
#ifdef debug
  IOdebug("msdosfs : File_Seek %d %d -> %d",req->CurPos,req->Mode,req->NewPos);
#endif

  if (req->Mode == S_Beginning)
    newoff = req->NewPos;
  elif(req->Mode == S_Relative)
    newoff = req->CurPos + req->NewPos;
  elif(req->Mode == S_End)
    newoff = file_info->Size - req->NewPos;

  if ((newoff < 0) || (newoff > file_info->Size))
   {
#ifdef debugE
     IOdebug("msdosfs ERROR : File_Seek : wrong position");
#endif
     SendError(mcb, EC_Error + SS_FloppyDisk + EG_WrongSize + EO_File);
     return;
   }

  mcb->Control[0] = newoff;
  Return(mcb, ReplyOK, 1, 0);
}

/*----------------------------------------------------------------------*/
/*			File_GetSize() 					*/ 
/*----------------------------------------------------------------------*/
/** return size field from FileInfo structure.
**/

void File_GetSize(MCB *mcb, FileInfo *info)
{ 

#ifdef debug
  IOdebug("msdosfs : File_GetSize : %d ",info->Size);
#endif
  mcb->Control[0] = info->Size;
  Return(mcb, ReplyOK, 1, 0);
}


/*----------------------------------------------------------------------*/
/*			Dir_Open() 					*/ 
/*----------------------------------------------------------------------*/

/** open a directory, send a req port and wait for request
**/

void Dir_Open(MCB *mcb, string fullname, string localname)
{ Port		StreamPort;
  DirStream	*stream;
  BYTE		*data = mcb->Data;

#ifdef debug
  IOdebug("msdosfs : Dir_Open %s",fullname);
#endif
  
  if ((StreamPort = NewPort()) == NullPort)
  {
#ifdef debugE
     IOdebug("msdosfs ERROR : Dir_Open : can't get a NewPort");
#endif
     SendError(mcb, EC_Error + SS_FloppyDisk + EG_Congested + EO_Server);
     return;
  }

  Wait( &Var_access );	/* read_dir needs exclusive access */

  if ((stream = read_dir(localname)) == Null(DirStream))
  { SendError(mcb, EC_Error + SS_FloppyDisk + EG_NoMemory + EO_Server);
#ifdef debugE
    IOdebug("msdosfs ErROR : Dir_Open : read_dir failed");
#endif
    FreePort(StreamPort);
    Signal( &Var_access );
    return;
  }

  Signal( &Var_access );

#ifdef debug
  IOdebug("msdosfs : Dir_Open : SendOpenReply and wait request ...");
#endif
  SendOpenReply(mcb, fullname, Type_Directory, Flags_Closeable,
		StreamPort);

			/* The stream is now open */
  forever
   { WORD	errcode;
     mcb->MsgHdr.Dest	= StreamPort;
     mcb->Timeout	= (word) (StreamTimeout);
     mcb->Data		= data;

     if ((errcode = GetMsg(mcb)) == EK_Timeout) break;
#ifdef debug
     IOdebug("msdosfs : Dir_Open get a request");
#endif
     if (errcode < Err_Null) 
     {
#ifdef debug
       IOdebug("msdosfs WARN: Dir_Open : msg_code < Err_Null");
#endif
       continue;
      }

     if ((errcode & FC_Mask) != FC_GSP)
      { 
#ifdef debugE
	IOdebug("msdosfs ERROR : Dir_Open : not a GSP request");
#endif
	SendError(mcb, EC_Error + SS_FloppyDisk + EG_WrongFn + EO_Server);
        continue;
      }

     switch ( errcode & FG_Mask )
      { case FG_Read	: Dir_Read(mcb, stream);
			  break;

	case FG_Close	: 
#ifdef debug 
			  IOdebug("msdosfs :$ Dir_Open(%s)",fullname);
#endif
	                  Free(stream);
			  if (mcb->MsgHdr.Reply != NullPort)
			    Return(mcb, ReplyOK, 0, 0);
			  FreePort(StreamPort);
			  return;

	case FG_Seek	: Dir_Seek(mcb, stream);
			  break;

	case FG_GetSize	: mcb->Control[0] = stream->number * sizeof(DirEntry);
#ifdef debug 
			  IOdebug("msdosfs :$ Get_Size");
#endif
			  Return(mcb, ReplyOK, 1, 0);
			  break; 

	case FG_Format	:
			  Format_or_Check(mcb);
			  break;

	case FG_Write	        :
	case FG_SetSize	        :
	case FG_GetInfo	        :
	case FG_SetInfo	        :
	case FG_EnableEvents	:
	case FG_Acknowledge	:
	case FG_NegAcknowledge	:
	default		        :
#ifdef debugE
	  		  IOdebug("msdosfs ERROR : Dir_Open : Wrong Fn");
#endif
			  SendError(mcb, EC_Error + SS_FloppyDisk + EG_WrongFn +
				    EO_Stream);
			  break;
      }
   }
#ifdef debug
   IOdebug("msdosfs WARN: Dir_Open : Timedout -> BREAK");
#endif
   Free(stream);
   FreePort(StreamPort);
}

/*----------------------------------------------------------------------*/
/*			Dir_Read() 					*/ 
/*----------------------------------------------------------------------*/

void Dir_Read(MCB *mcb, DirStream *stream)
{ ReadWrite *readwrite = (ReadWrite *) mcb->Control;
  bool	eof = (bool) FALSE;
  WORD  amount;

#ifdef debug
  IOdebug("msdosfs : Dir_Read : Pos=%d, size=%d",
		readwrite->Pos,readwrite->Size);
#endif

  stream->offset = readwrite->Pos;

  if (stream->offset >= (stream->number * sizeof(DirEntry)))
   { 
#ifdef debugE
     IOdebug("msdosfs ERROR : Dir_Read : end of dir !");
#endif

     Return(mcb, ReadRc_EOF, 0, 0);
     return;
   }

  amount = readwrite->Size;
  if ((amount + stream->offset) > (stream->number * sizeof(DirEntry)))
   { amount = (stream->number * sizeof(DirEntry)) - stream->offset;
     eof    = (bool) TRUE;
   }

  mcb->Data = &( ((BYTE *) &(stream->entries[0])) [stream->offset]);
  Return(mcb, (eof ? ReadRc_EOF : ReadRc_EOF), 0, amount);
  stream->offset += amount;
}

/*----------------------------------------------------------------------*/
/*			Dir_Seek() 					*/ 
/*----------------------------------------------------------------------*/

void Dir_Seek(MCB *mcb, DirStream *stream)
{
  SeekRequest *req = (SeekRequest *) mcb->Control;
  WORD	newoff = -1;
#ifdef debug
  IOdebug("msdosfs : Dir_Seek : mode=%d pos=%d",req->Mode,req->NewPos);
#endif

  if (req->Mode == S_Beginning)
    newoff = req->NewPos;
  elif(req->Mode == S_Relative)
    newoff = req->CurPos + req->NewPos;
  elif(req->Mode == S_End)
    newoff = (stream->number * sizeof(DirEntry)) - req->NewPos;

  if ((newoff < 0) || (newoff > (stream->number * sizeof(DirEntry))))
   {
#ifdef debugE
     IOdebug("msdosfs ERROR : Dir_Seek : wrong position");
#endif
     SendError(mcb, EC_Error + SS_FloppyDisk + EG_WrongSize + EO_Directory);
     return;
   }
  else
   {
     stream->offset = newoff;
     mcb->Control[0] = newoff;
     Return(mcb, ReplyOK, 1, 0);
   }
}


/*----------------------------------------------------------------------*/
/*			Drive_Locate() 					*/ 
/*----------------------------------------------------------------------*/

void Drive_Locate(MCB *mcb, string fullname)
{
  string localname = GetLocalName(fullname);
  int	 exists;


#ifdef debug
  IOdebug("msdosfs : Drive_Locate :%s",fullname);
#endif
  
  if( localname == Null(char) )
  { 
#ifdef debugE
     IOdebug("msdosfs ERROR : Drive_Locate : try to locate NULL string");
#endif
     SendError(mcb, EC_Error + SS_FloppyDisk + EG_NoMemory + EO_Server);
     return;
  }

  if (localname == (char *) -1)
  { 
#ifdef debugE
   IOdebug("msdosfs ERROR : Drive_Locate : name invalid");
#endif
   SendError(mcb, EC_Error + SS_FloppyDisk + EG_Name + EO_File);
   return;
  }

  exists = exists_obj(localname);

  if( exists == Type_File || exists == Type_Directory )
     SendOpenReply(mcb, fullname, exists, 0, NullPort);

  else
  {
#ifdef debugE
     IOdebug("msdosfs ERROR : Drive_Locate(%s) : Unknown object or timeout",
 	  	localname);
#endif
     SendError(mcb, EC_Error + SS_FloppyDisk + EG_Unknown + EO_Object);
  }

  Free(localname);
}

/*----------------------------------------------------------------------*/
/*			Drive_Create() 					*/ 
/*----------------------------------------------------------------------*/

void Drive_Create(MCB *mcb, string fullname)
{
  string localname = GetLocalName(fullname);
  IOCCreate *info  = (IOCCreate *) mcb->Control;
  word	 type;
  
  type = info->Type;

  
#ifdef debug
  IOdebug("msdosfs : Drive_Create, %s",fullname);
#endif

  if (localname == Null(char))
  { 
#ifdef debugE
     IOdebug("msdosfs ERROR : Drive_Create : try to locate NULL string");
#endif
     SendError(mcb, EC_Error + SS_FloppyDisk + EG_NoMemory + EO_Server);
     
     return;
   }

  if (localname == (char *) -1)
  { 
#ifdef debugE
   IOdebug("msdosfs ERROR : Drive_Create : name invalid");
#endif
   SendError(mcb, EC_Error + SS_FloppyDisk + EG_Name + EO_File);
   return;
  }

  if ((type != Type_File) && (type != Type_Directory))
  { 
#ifdef debugE
     IOdebug("msdosfs ERROR : Drive_Create : wrong type");
#endif
     SendError(mcb, EC_Error + SS_FloppyDisk + EG_Create + EO_Object);
     Free(localname);
     
     return;
   }

  if( create_obj(localname, type) < 0 )
  { 
#ifdef debugE
      IOdebug("msdosfs : create object failure\n");
#endif
      SendError(mcb, EC_Error + SS_FloppyDisk + EG_Create +
		((type == Type_File) ? EO_File : EO_Directory));
  }
  else
      SendOpenReply(mcb, fullname, type, 0, NullPort);

  
  Free(localname);
}

/*----------------------------------------------------------------------*/
/*			Drive_ObjInfo()					*/ 
/*----------------------------------------------------------------------*/

void Drive_ObjInfo(MCB *mcb, string fullname)
{
  string localname = GetLocalName(fullname);
  FileInfo finfo;
  int	 exists;
  
#ifdef debug
  IOdebug("msdosfs : Drive_ObjInf,%s",fullname);
#endif

  if (localname == Null(char))
  { 
#ifdef debugE
   IOdebug("msdosfs ERROR : Drive_ObjInfo : local name = null");
#endif
   SendError(mcb, EC_Error + SS_FloppyDisk + EG_NoMemory + EO_Server);
   return;
  }

  if (localname == (char *) -1)
  { 
#ifdef debugE
   IOdebug("msdosfs ERROR : Drive_ObjInfo : name invalid");
#endif
   SendError(mcb, EC_Error + SS_FloppyDisk + EG_Name + EO_File);
   return;
  }

  exists = object_info(localname, &finfo);
  if( exists < 0 )
  {		
     SendError(mcb, EC_Error + SS_FloppyDisk + EG_Unknown + EO_Object);
     Free(localname);
     return;
  }
  
  if( exists == Type_Nothing )  /* if object doesn't exist send error msg */
   { 
#ifdef debugE
     IOdebug("msdosfs ERROR : Drive_ObjInfo : unknown object");
#endif
     SendError(mcb, EC_Error + SS_FloppyDisk + EG_Unknown + EO_Object);
     Free(localname);
     return;
   }  

   {
     ObjInfo *info = (ObjInfo *) mcb->Data;
     info->Dates.Creation	= finfo.Time; /* 1 Jan 1970... */
     info->Dates.Access		= finfo.Time;
     info->Dates.Modified	= finfo.Time;
     info->DirEntry.Flags	= 0;
     info->Account		= 0;
     strcpy(&(info->DirEntry.Name[0]), 
	(!strcmp(finfo.Name, "/")) ? floppy.Name : finfo.Name);

     if( exists == Type_Directory)    
      {
        info->DirEntry.Type	= Type_Directory;
        info->DirEntry.Matrix	= DefDirMatrix;
        info->Size		= 0;	/* should be 44 * no. of entries */
      }
     else		   /* object is a file */
      {
        info->DirEntry.Type	= Type_File;
        info->DirEntry.Matrix	= DefFileMatrix;
        info->Size 		= finfo.Size;
      }
    }

  Return(mcb, ReplyOK, 0, sizeof(ObjInfo));
  Free(localname);
}

/*----------------------------------------------------------------------*/
/*			Drive_ServerInfo()				*/ 
/*----------------------------------------------------------------------*/

void Drive_ServerInfo(MCB *mcb, string fullname)
{
  string localname = GetLocalName(fullname);
  servinfo *info   = (servinfo *) mcb->Data;

#ifdef debug
  IOdebug("msdosfs : Drive_ServerInfo, %s",fullname);
#endif
  if (localname == Null(char))
   { 
#ifdef debugE
     IOdebug("msdosfs ERROR : Drive_ServerInfo : localname = null");
#endif
     SendError(mcb, EC_Error + SS_FloppyDisk + EG_NoMemory + EO_Server);
     return;
   }

  if (localname == (char *) -1)
  { 
#ifdef debugE
   IOdebug("msdosfs ERROR : Drive_ServerInfo : name invalid");
#endif
   SendError(mcb, EC_Error + SS_FloppyDisk + EG_Name + EO_File);
   return;
  }

  info->type = Type_Directory;
  info->alloc = 1024;
  drive_statistics(&(info->size), &(info->available));
  Return(mcb, ReplyOK, 0, sizeof(servinfo));

  Free(localname);
}

/*----------------------------------------------------------------------*/
/*			Drive_Delete()					*/ 
/*----------------------------------------------------------------------*/

void Drive_Delete(MCB *mcb, string fullname)
{
  string localname = GetLocalName(fullname);
  int	 exists;
  
#ifdef debug
  IOdebug("msdosfs : Drive_Delete, %s",fullname);
#endif
  if (localname == Null(char))
  { 
#ifdef debugE
     IOdebug("msdosfs ERROR : Drive_Delete : localname = null");
#endif
     SendError(mcb, EC_Error + SS_FloppyDisk + EG_NoMemory + EO_Server);
     return;
   }

  if (localname == (char *) -1)
  { 
#ifdef debugE
   IOdebug("msdosfs ERROR : Drive_Delete : name invalid");
#endif
   SendError(mcb, EC_Error + SS_FloppyDisk + EG_Name + EO_File);
   return;
  }

  if( (exists = delete_object(localname)) == Type_Nothing )
  {
#ifdef debugE
     IOdebug("msdosfs ERROR : Drive_Delete : unknown object");
#endif
     SendError(mcb, EC_Error + SS_FloppyDisk + EG_Unknown + EO_Object);
   }
  else
  if( exists < 0 )
   {
      SendError(mcb,EC_Error + SS_FloppyDisk + EG_NoMemory + EO_Server);
   }
  else
  if( exists > 0 )
   {
      SendError(mcb, EC_Error + SS_FloppyDisk + EG_Delete + 
		((exists == Type_Directory) ? EO_Directory : EO_File));
   }
  else
   Return(mcb, ReplyOK, 0, 0);

  Free(localname);
}

/*----------------------------------------------------------------------*/
/*			Drive_Rename()					*/ 
/*----------------------------------------------------------------------*/

void Drive_Rename(MCB *mcb, string fullname)
{
  string localname = GetLocalName(fullname);
  string newname, newlocal, tempname;
  WORD   fromexists, toexists;
  IOCMsg2 *args = (IOCMsg2 *) mcb->Control;
  
#ifdef debug
  IOdebug("msdosfs : Drive_Rename %s",fullname);
#endif
  if (localname == Null(char))
  { 
#ifdef debugE
   IOdebug("msdosfs ERROR : Drive_Rename : name = null");
#endif
   SendError(mcb, EC_Error + SS_FloppyDisk + EG_NoMemory + EO_Server);
   return;
  }

  if (localname == (char *) -1)
  { 
#ifdef debugE
   IOdebug("msdosfs ERROR : Drive_Rename : name invalid");
#endif
   SendError(mcb, EC_Error + SS_FloppyDisk + EG_Name + EO_File);
   return;
  }

  for(tempname = fullname; (*tempname != '/') && (*tempname != '\0');
        tempname++);
  *tempname = '\0';
  
  args->Common.Name = args->Arg.ToName;
  if ((newname = GetFullName(fullname, mcb)) == (string) NULL)
  { 
#ifdef debugE
    IOdebug("msdosfs ERROR : Drive_Rename : can't GetFullName");
#endif
    SendError(mcb, EC_Error + SS_FloppyDisk + EG_NoMemory + EO_Server);
    Free(localname);
    return;
  }
   
  if ((newlocal = GetLocalName(newname)) == (string) NULL)
  { 
#ifdef debugE
    IOdebug("msdosfs ERROR : Drive_Rename : can't GetLocalName");
#endif
    SendError(mcb, EC_Error + SS_FloppyDisk + EG_NoMemory + EO_Server);
    Free(localname);
    Free(newname);
    return;
  }

  if (newlocal == (char *) -1)
  { 
#ifdef debugE
   IOdebug("msdosfs ERROR : Drive_Rename : name invalid");
#endif
   SendError(mcb, EC_Error + SS_FloppyDisk + EG_Name + EO_File);
   return;
  }

	/* Renaming something on top of itself ? */
  if (!strcmp(localname, newlocal))
  {
     Return(mcb, ReplyOK, 0, 0);
     goto rename_end;
  }

  toexists = exists_obj(newlocal);

  if( toexists == Type_Directory )
  { 
#ifdef debugE
    IOdebug("msdosfs ERROR : Drive_Rename : dest (%s) is a dir",newlocal);
#endif
    SendError(mcb, EC_Error+SS_FloppyDisk+EG_Protected+EO_Directory);
    goto rename_end;
  }
  elif( toexists == Type_File )
   {
      if( delete_object(newlocal) < 0 )
      {
         SendError(mcb, EC_Error+SS_FloppyDisk+EG_Protected+EO_File);
         goto rename_end;
      }
   }

#ifdef debug
  IOdebug("msdosfs : Drive_Rename : %s -> %s",localname,newlocal);
#endif

  fromexists = rename_object(localname, newlocal);
  if( fromexists == Type_Nothing )
  { 
#ifdef debugE
    IOdebug("msdosfs ERROR : Drive_Rename : source (%s) doesn't exist",
             localname);
#endif
    SendError(mcb, EC_Error + SS_FloppyDisk + EG_Unknown + EO_Object);
    goto rename_end;
  }

  if( fromexists < 0 )  
    SendError(mcb, EC_Error + SS_FloppyDisk + EG_Broken + EO_Object);

  else
    Return(mcb, ReplyOK, 0, 0);
   
rename_end:
  Free(localname);
  Free(newname);
  Free(newlocal);
}

/*----------------------------------------------------------------------*/
/*			Drive_SetDate()					*/ 
/*----------------------------------------------------------------------*/

void Drive_SetDate(MCB *mcb, string fullname)
{
  string localname = GetLocalName(fullname);
  int	 exists;
  
#ifdef debug
  IOdebug("msdosfs : Drive_SetDate, %s",fullname);
#endif

  if( localname == Null(char) )
  { 
#ifdef debugE
     IOdebug("msdosfs ERROR : Drive_SetDate : name = null");
#endif
     SendError(mcb, EC_Error + SS_FloppyDisk + EG_NoMemory + EO_Server);
     return;
  }

  if (localname == (char *) -1)
  { 
#ifdef debugE
   IOdebug("msdosfs ERROR : Drive_SetDate : name invalid");
#endif
   SendError(mcb, EC_Error + SS_FloppyDisk + EG_Name + EO_File);
   return;
  }

  exists = change_date(localname);
  if( exists < 0 )
  {
   SendError(mcb, EC_Error + SS_FloppyDisk + EG_WrongFn + EO_Server);
   Free(localname);
   return;
  }

  if( exists == Type_Nothing )
  { 
#ifdef debugE
    IOdebug("msdosfs ERROR : Drive_SetDate : %s doesn't exist",localname);
#endif
    SendError(mcb, EC_Error + SS_FloppyDisk + EG_Unknown + EO_Object);
    Free(localname);
    return;
  }

  if( exists == Type_Directory )
  { 
#ifdef debugE
    IOdebug("msdosfs ERROR : Drive_SetDate : %s is a dir",localname);
#endif
    SendError(mcb, EC_Error+SS_FloppyDisk+EG_WrongFn+EO_Directory);
    Free(localname);
    return;
  }
  
  Return(mcb, ReplyOK, 0, 0);
  Free(localname);
}

/**
*** GSP error message routines
**/

/*----------------------------------------------------------------------*/
/*			InvalidFun()					*/ 
/*----------------------------------------------------------------------*/

void InvalidFun(MCB *mcb, string fullname)
{
  SendError(mcb, EC_Error + SS_FloppyDisk + EG_WrongFn + EO_Server);
  fullname = fullname;
}

/*----------------------------------------------------------------------*/
/*			SendError()					*/ 
/*----------------------------------------------------------------------*/

void SendError(MCB *mcb, WORD FnRc)
{ 
 
#ifdef debugE
  IOdebug("msdosfs : send error %x", FnRc);
#endif
  if (mcb->MsgHdr.Reply == NullPort) return;
  *((int *) mcb) = 0;
  mcb->MsgHdr.Dest  = mcb->MsgHdr.Reply;
  mcb->MsgHdr.Reply = NullPort;
  mcb->MsgHdr.FnRc  = FnRc;
  mcb->Timeout      = (word) (5 * OneSec);
  (void) PutMsg(mcb);
}

/*----------------------------------------------------------------------*/
/*			Return()					*/ 
/*----------------------------------------------------------------------*/

void Return(MCB *mcb, WORD FnRc, WORD ContSize, WORD DataSize)
{
  if (mcb->MsgHdr.Reply == NullPort)
      return;
  mcb->MsgHdr.Flags	= 0;
  mcb->MsgHdr.ContSize	= ContSize;
  mcb->MsgHdr.DataSize	= DataSize;
  mcb->MsgHdr.Dest	= mcb->MsgHdr.Reply;
  mcb->MsgHdr.Reply	= NullPort;
  mcb->MsgHdr.FnRc	= FnRc;
  mcb->Timeout		= (word) (5 * OneSec);
  (void) PutMsg(mcb);
}

/*----------------------------------------------------------------------*/
/*			SendOpenReply()					*/ 
/*----------------------------------------------------------------------*/

void SendOpenReply(MCB *mcb, string name, WORD type, WORD flags,
			   Port Reply)
{
  IOCReply1 *reply = (IOCReply1 *) mcb->Control;

  if (mcb->MsgHdr.Reply == NullPort)
       return;
  reply->Type = type;
  reply->Flags	= flags;
  mcb->Control[2]	= -1;
  mcb->Control[3]	= -1;
  reply->Pathname	= 0;
  reply->Object		= 0;

  strcpy(mcb->Data, &(Machine_Name[0]));
  strcat(mcb->Data, "/");
  strcat(mcb->Data, name);

  mcb->MsgHdr.Flags	= 0;
  mcb->MsgHdr.ContSize	= sizeof(IOCReply1) / sizeof(WORD);
  mcb->MsgHdr.DataSize	= strlen(mcb->Data) + 1;
  mcb->MsgHdr.Dest	= mcb->MsgHdr.Reply;
  mcb->MsgHdr.Reply	= Reply;
  mcb->MsgHdr.FnRc	= ReplyOK;
  mcb->Timeout		= (word) (5 * OneSec);
  (void) PutMsg(mcb);
}
/*----------------------------------------------------------------------*/
/*			Format_or_Check() 				*/ 
/*----------------------------------------------------------------------*/

void Format_or_Check(MCB *mcb)
{
     word mode = (int) *mcb->Data;		/* format or fsck */
     word error, ret_code;

#ifdef debug
  IOdebug("msdosfs : Format/Check : mode = %x", mode);
#endif

     error = (mode & FSCK) ? check_disc(mode, &ret_code) : format_disc(mode);

     mcb -> Control[0] = ret_code;

     if(error)
        Return(mcb, EC_Error + SS_FloppyDisk + error, 1, 0);

     else
	Return(mcb, ReplyOK, 1, 0);
}
