/*------------------------------------------------------------------------
--                                                                      --
--          H E L I O S   I N P U T / O U T P U T   S E R V E R         --
--          ---------------------------------------------------         --
--                                                                      --
--               Copyright (C) 1987, Perihelion Software Ltd.           --
--                             All Rights Reserved.                     --
--                                                                      --
--  Devices.c                                                           --
--                                                                      --
--           This module contains code to interact with the various     --
--                                                                      --
--           devices: clock, printer, rs232, centronics,                --
--                                                                      --
--           mouse, and raw keyboard. It also contains some polling     --
--                                                                      --
--           code                                                       --
--                                                                      --
--  Author:  BLV 30/11/87                                               --
--                                                                      --
------------------------------------------------------------------------*/

/* RcsId: $Id: devices.c,v 1.8 1994/06/29 13:42:25 tony Exp $ */
/* Copyright (C) 1987, Perihelion Software Ltd.        			*/

/**
*** The usual header file and some function declarations.
**/

#define Devices_Module
#include "helios.h"

/*------------------------------------------------------------------------
--------------------------------------------------------------------------
--  I start off with the various polling routines.                      --
--  My usual approach is to put data into circular buffers when         --
--  handling interrupts, and extract them here. Poll_the_devices is     --
--  called from within the main loop of the server.                     --
--------------------------------------------------------------------------
------------------------------------------------------------------------*/

/**
*** Routine poll_the_devices() is called from inside the Server's and
*** from the debugger's main loop. The purpose of the routine is to examine the
*** status of all the devices that need checking and take whatever action
*** is appropriate. This may involve sending mouse and keyboard events to
*** the transputer, reading the keyboard for each window, polling Gem,
*** and checking some internal flags.
**/

PUBLIC void fn( poll_the_windows,  (void));     /* in terminal.c */

void poll_the_devices()
{

#if MSWINDOWS
  { extern void poll_windows(void);
    poll_windows();
  }
#endif

#if keyboard_supported
  PRIVATE int  fn( poll_the_keyboard, (void));

  if (poll_the_keyboard())
#endif
     poll_the_windows();

#if mouse_supported
  { PRIVATE void fn( poll_the_mouse, (void));
    poll_the_mouse();
  }
#endif

#if gem_supported
    poll_gem();
#endif

#if RS232_supported
  { if (RS232_errno)
     RS232_check_events();
    RS232_errno = 0;
  }
#endif

}

void RS232_send_event(port, event_type)
word port;
word event_type;
{ 
  IOEvent *event = (IOEvent *) mcb->Data;

  event->Type    = event_type;
  event->Counter = 0;
  event->Stamp   = Now;

  copy_event(event, event);   /* This does all the swapping and conversions */

  mcb->MsgHdr.Reply = port;
  mcb->MsgHdr.Dest  = (word) NULL;
  mcb->MsgHdr.Flags = MsgHdr_Flags_preserve;

  Request_Return(EventRc_IgnoreLost, 0L, (word) sizeof(IOEvent));
}


/**
*** The following code is very useful for taking an event such as a mouse
*** or keyboard event and putting it into a message data area, because it
*** does all the swapping required and does the time stamp conversion.
*** NB it is possible for the copy/conversion to happen in place.
*** The time stamp is supposed to be in milliseconds since start-up, but
*** the stamp I store at the moment comes from Now.
**/

void copy_event(to, from)
IOEvent *to, *from;
{ to->Type     = swap(from->Type);
  to->Counter  = swap(from->Counter);
  to->Stamp    =
   swap(divlong( ((from->Stamp - initial_stamp) * 1000L), (word) CLK_TCK) );
  to->Device.Break.junk1 = swap(from->Device.Break.junk1);
  to->Device.Break.junk2 = swap(from->Device.Break.junk2);
}


/*-----------------------------------------------------------------------------
-------------------------------------------------------------------------------
--
--    The following routines deal with setting up the devices. Initialise is --
-- called during startup only, and restore devices is called when the server --
-- exits. Restart is called during a reboot and during start-up              --
--
------------------------------------------------------------------------------- 
-----------------------------------------------------------------------------*/

/**
*** The code for this tends to be rather specialised so I prefer to put it
*** into the hardware-specific files, ST/STLOCAL.C etc.
*** It is important that all the event handlers are cleared each time the
*** Server exits or reboots.
***
*** I am not certain what to do about data still in the circular buffers
*** when the user wishes to reboot, but scrapping it is probably sensible.
**/

void initialise_devices()
{ 
#if (UNIX || HELIOS)
  unix_initialise_devices();  /* on the Sun this must be done before creating */
                              /* any new windows */
#endif

#if MAC
 mac_initialise_devices();
#endif

#if MSWINDOWS
  { extern void initialise_Windows(void);
    initialise_Windows();
   }
#endif

  initialise_windowing();     /* in module terminal.c */

#if ST
  ST_initialise_devices();
#endif

#if PC
  { PUBLIC void fn( PC_initialise_devices, (void));
    PC_initialise_devices();
  }
#endif

#if AMIGA
  {
  extern int (*_ONBREAK)();
  extern int mybreak();
  _ONBREAK = mybreak;   /* convince Lattice not to use their requesters */
                        /* cause more crashes than anything else */
  Amiga_initialise_devices();
  }
#endif
}

void restore_devices()
{ 
#if MSWINDOWS
  { extern void restore_windows(void);
    if (debugflags & Quit_Flag) printf("Calling restore_windows");
    restore_windows();
  }
#endif

#if ST
  ST_restore_devices();
#endif
#if PC
  { PUBLIC void fn( PC_restore_devices, (void));
    PC_restore_devices();
  }
#endif
#if AMIGA
  { extern void ServerMenu();

#if debugger_incorporated
    extern void DebugMenu();
    if (DebugMode)
     DebugMenu();
    else
#endif
     ServerMenu();
    Amiga_restore_devices();
  }
#endif

  restore_windowing();

#if MAC
  mac_restore_devices();
#endif

#if (UNIX)
  unix_restore_devices();
#endif
}

void restart_devices()
{

#if MSWINDOWS
  { extern void restart_windows(void);
    restart_windows();
  }
#endif

#if RS232_supported
  RS232_errno = 0;
#endif

#if Centronics_supported
  Centronics_errno = 0;
#endif

#if Printer_supported
  Printer_errno = 0;
#endif

#if floppies_available
  floppy_errno = 0;
#endif

#if Midi_supported
  Midi_errno = 0;
#endif

  restart_windowing();

#if gem_supported
  restart_gem();
#endif
}

/*****************************************************************************
******************************************************************************
******************************************************************************
******************************************************************************
******************************************************************************
******* That is all the general purpose stuff. The code below deals with *****
******* the devices actually supported.                                  *****
******************************************************************************
******************************************************************************
******************************************************************************
******************************************************************************
*****************************************************************************/

/*------------------------------------------------------------------------
--------------------------------------------------------------------------
--
-- The Clock device
--
--------------------------------------------------------------------------
------------------------------------------------------------------------*/

/**
*** The clock is a very simple device to implement because the only valid
*** requests are Locate and Create, which can be done by default handlers,
*** ObjectInfo, which is used by the transputer to discover the current
*** time, and SetDate to allow the transputer to change the time.
***
*** ObjectInfo is straightforward because most of the fields in the ObjInfo
*** structure are useless, and I only need to worry about the date stamps
*** which are the system startup time for creation and the current time for
*** last altered and accessed. It is useful to have some way of finding out
*** when the system started up, and clock creation seems the best way of doing
*** it. To get hold of the current time I use the rawest system clock available
*** thus bypassing any conversions, e.g. timezone ones, done by the C library.
**/

void Clock_ObjectInfo(myco)
Conode *myco;
{                      /* put info in data vector straightaway, nothing in */
                       /* there that we need any more */
  register ObjInfo *Heliosinfo = (ObjInfo *) mcb->Data;
  word current_time;
  if (strcmp(IOname, "clock") )
    { Request_Return(EC_Error + SS_IOProc + EG_Name + EO_Message, 0L, 0L);
      return;
    }   

  current_time = get_unix_time();

  Heliosinfo->DirEntry.Type   = swap(Type_Private);
  Heliosinfo->DirEntry.Flags  = swap(0L);
  Heliosinfo->DirEntry.Matrix = swap(DefDirMatrix);
  strcpy(Heliosinfo->DirEntry.Name, "clock");
  Heliosinfo->Account   = swap(0L);
  Heliosinfo->Size      = swap(0L);
  Heliosinfo->Creation  = swap(Startup_Time);
  Heliosinfo->Access    = swap(current_time);
  Heliosinfo->Modified  = Heliosinfo->Access;

  Request_Return(ReplyOK, 0L, (word) sizeof(ObjInfo));
  use(myco)
}

/**
*** Setting the system clock is not possible on all machines, e.g. under Unix
*** only the super user is allowed to change the clock.
**/

#if (ST || PC || AMIGA)

void Clock_SetDate(myco)
Conode *myco;
{ word unixstamp = (mcb->Control)[SetDateDate_off];
  extern void fn( set_current_time, (word));

  set_current_time(unixstamp);

  Request_Return(ReplyOK, 0L, 0L);
  use(myco)
}

#endif   /* Clock settable */


/*------------------------------------------------------------------------
--------------------------------------------------------------------------
--
--     The Host device
--
--------------------------------------------------------------------------
------------------------------------------------------------------------*/
/**
*** The Host device is one I do not like very much, but all our customers
*** seem to want it. Its purpose is to provide limited (or maybe not so
*** limited) interaction between Helios and the machine running the host.
*** Each host has a device /pc or /st or whatever that can be opened, and
*** once it is open it can be sent private protocol messages. These result
*** in some machine-dependant operation, followed by a return message.
*** On the PC and the ST the message results in a trap being invoked. On
*** other systems, particularly multi-tasking systems, it might be possible
*** to provide full bi-directional communication between Helios and host
*** programs, though I have no intention of writing the relevant code myself.
**/

#if interaction_supported

void Host_Open(myco)
Conode *myco;
{                                            /* check the device we are opening */
  if (strcmp(IOname, machine_name) )
    { Request_Return(EC_Error + SS_IOProc + EG_Name + EO_Message, 0L, 0L);
      return;
    }

  NewStream(Type_File, Flags_Closeable + Flags_Interactive, NULL, 
            Host_Handlers);
  use(myco)
}

#if (ST || PC)              /* The behaviour varies from machine to machine */
void Host_PrivateStream(myco)
Conode *myco;
{ word   reply_port = mcb->MsgHdr.Reply;    /* Save the reply port */
  word   reply;
  extern word fn( call_a_trap, (MCB *));   /* The machine dependant routine */
                                           /* Probably written in assembler */
  reply = call_a_trap(mcb);

  mcb->MsgHdr.Reply = reply_port;
  Request_Return(reply, (word) mcb->MsgHdr.ContSize,
                 (word) mcb->MsgHdr.DataSize);
  use(myco)
}
#endif  /* ST || PC */

void Host_Close(myco)
Conode *myco;
{ if (mcb->MsgHdr.Reply ne NullPort)
   Request_Return(ReplyOK, 0L, 0L);
  Seppuku();
  use(myco)
}

#endif  /* interaction_supported */

/*------------------------------------------------------------------------
--
-- Network control device
--
------------------------------------------------------------------------*/

/**
*** This server may be used by network server device drivers to reset
*** processors and to configure links. There is a server /NetworkController,
*** type file, which may be opened. Operations are performed by sending
*** private protocol requests to the resulting stream. Sample local
*** routines are in ibm/pclocal.c, which just display messages.
**/

#if Network_supported

extern bool fn( network_reset,      (word));
extern bool fn( network_analyse,    (word));
extern bool fn( network_connect,    (word, word, word, word));
extern bool fn( network_disconnect, (word, word));
extern bool fn( network_enquire,    (word, word));

void Network_Open(myco)
Conode *myco;
{                                            /* check the device we are opening */
  if (strcmp(IOname, myco->name) )
    { Request_Return(EC_Error + SS_IOProc + EG_Name + EO_Message, 0L, 0L);
      return;
    }

  NewStream(Type_File, Flags_Closeable + Flags_Interactive, NULL, 
            Network_Handlers);
  use(myco)
}

void Network_PrivateStream(myco)
Conode *myco;
{ word fncode = mcb->MsgHdr.FnRc & FG_Mask;
  word result = TRUE;
  word *vec;
  uint i;

  Server_errno =  EC_Error + SS_IOProc +EG_Invalid + EO_Subnetwork;
  if (fncode eq NC_Reset)
   { for (i = 0, vec = (word *) mcb->Data;
          i < (mcb->MsgHdr.DataSize >> 2);
          i++, vec++)
      unless(network_reset(swap(*vec)))
       { result = FALSE; break; }
   }
  elif (fncode eq NC_Analyse)
   { for (i = 0, vec = (word *) mcb->Data;
          i < (mcb->MsgHdr.DataSize >> 2);
          i++, vec++)
       unless(network_analyse(swap(*vec)))
        { result = FALSE; break; }
   }
  elif (fncode eq NC_Connect)
   { vec = (word *) mcb->Data;
     result = network_connect(swap(vec[0]), swap(vec[1]), swap(vec[2]),
                            swap(vec[3]));
   }
  elif (fncode eq NC_Disconnect)
   { vec = (word *) mcb->Data;
     result = network_disconnect(swap(vec[0]), swap(vec[1]));
   }
  elif (fncode eq NC_Enquire)
   { vec = (word *) mcb->Data;
     result = network_enquire(swap(vec[0]), swap(vec[1]));
     if (result)
      { for (i = 0 ; i < 3; i++) vec[i] = swap(vec[i]);
        Request_Return(ReplyOK, 0L, 12L);
        return;
      }
   }
  else
   { Request_Return(EC_Error + SS_IOProc + EG_FnCode + EO_Subnetwork, 0L, 0L);
     return;
   }
    
  if (result) Server_errno = ReplyOK;
  Request_Return(Server_errno, 0L, 0L);
  use(myco)
}

void Network_Close(myco)
Conode *myco;
{ if (mcb->MsgHdr.Reply ne NullPort)
   Request_Return(ReplyOK, 0L, 0L);
  Seppuku();
  use(myco)
}

#endif  /* Network_supported */


/*------------------------------------------------------------------------
--
-- Port IO
--
------------------------------------------------------------------------*/


/**
*** Important note : the port I/O code has changed drastically between
*** the 3.69 and 3.74 versions of the server, including the interface
*** between the hardware-dependent and independent parts. I am now a lot
*** happier with the code, so there are unlikely to be any more major
*** changes.
***
*** All the various communication ports, RS232, Centronics, Midi etc., tend
*** to work in much the same way, so I use the same code for most of them.
*** In addition, I treat Printer devices, which may or may not use similar
*** communication stuff, in the same way.
***
*** A given machine may have many different serial and parallel lines,
*** for example a humble PC may have upto 7 serial lines and upto 3 parallel
*** ones. A given line, e.g. com1, may be accessed from Helios in a
*** variety of different ways depending on the hardware and the user's
*** configuration. For example, com1 may be accessed as /rs232/default,
*** /rs232/com1, /printers/default, and /printers/com1, all at the same
*** time by different programs. To keep things working it is essential
*** that there is only one set of attributes describing how the port
*** should behave at present, and that there is a lock guaranteeing that
*** only one program can perform I/O (reading, writing, or reconfiguring)
*** at any one time. Hence there should be one and only one ComsPort
*** structure, defined in structs.h, for a given port. This same ComsPort
*** is used irrespective of whether the user is accessing /rs232/default or
*** /printers/com1.
***
*** How does this work in practice ? When initialising the Server there
*** may or may not be a call to a hardware-specific TestFun, e.g.
*** RS232_TestFun, depending on the defines.h file. Assuming the port
*** server should exists there will be a call to InitServer. This
*** initialises a list to hold all the ports accessible via that server,
*** in the form of Port_node structures. Each Port_node structure has
*** a pointer to a ComsPort structure. /rs232/default and /printers/com1
*** would have different Port_node's, and would be held in different
*** linked lists, but they would have the same ComsPort pointer. The
*** InitServer routine creates the entry default within the list, but
*** cannot assign the ComsPort pointer : that has to be done by the
*** hardware-specific code. Then InitServer() calls initlist() in the
*** local code. Initlist() is responsible for initialising the ComsPort
*** pointer for default, and if there is more than one port initlist()
*** adds those ports to the linked list by calls to add_port_node below.
*** Once initlist() has finished all the Port_nodes are in the linked list
*** and initialised, so InitServer() will set up the default attributes
*** where appropriate and configure the hardware.
***
*** Most messages such as Locate, Rename, reading the directory, and so on,
*** can be handled by machine-independent code. Special code is needed for
*** things like enabling modem interrupt events on a serial line. Otherwise
*** all the real I/O can be done by indirecting through a set of function
*** pointers held in the ComsPort structure. These functions are as follows:
***    configure_fn(ComsPort *)
***    done_fn(ComsPort *)
***    error_fn(ComsPort *)
***    send_fn(ComsPort *, UBYTE *buff, int amount)
***    pollwrite_fn(ComsPort *)
***    abortwrite_fn(ComsPort *)
***    receive_fn(ComsPort *, UBYTE *buff, int amount)
***    pollread_fn(ComsPort *)
***    abortread_fn(ComsPort *)
***
*** All these routines are hardware-specific and have to be filled in within
*** the appropriate ComsPort structures by the local routines. The
*** configure_fn() is called for the initial hardware configuration and
*** whenever there is a change of Attributes for the port : typically it
*** might set some bits in a control register. The done_fn() is called
*** as the server is exiting or rebooting, and is responsible for any
*** tidying-up that is required. The error_fn() is called when an error
*** of some sort has been detected, and default error_fn's are provided
*** in this file.
***
*** Next consider a read from a port. Depending on the hardware, this read
*** may be interrupt-driven or it may involve polling. When the read is
*** started there is a call to the receive_fn(), specifying the port to use,
*** a buffer, and the amount to be read : for an interrupt-driven system this
*** would enable the receive interrupts, and data would start to arrive.
*** The receive_fn() should return TRUE or FALSE to indicate whether or not
*** it is possible to initiate the read. At regular intervals the Server
*** calls the pollread_fn(); in an interrupt system this would check a flag
*** to see whether or not the read has finished; in a polling system the
*** routine would perform the actual polling and reading from the port.
*** pollread_fn() should return -1L to indicate that the read has finished
*** successfully, -2L to indicate that the read is still proceeding, or
*** some other value >= 0 to indicate an error and the amount of data read
*** successfully : following an error the error_fn() will be called, which
*** may be a no-op. If the read times out the abortread_fn() will be
*** called, which should return the amount of data read so far and e.g.
*** disable interrupts.
***
*** Writes are more complicated than reads. The first stage in a write is
*** to get all the data from Helios within a finite time. Following this
*** the Write must be acknowledged, and a separate coroutine must be forked
*** to perform the actual Write with an infinite timeout. This is necessary
*** because the Helios protocols provide no way of indicating a partially-
*** successful write to the client. The port must be locked and the write
*** must be initiated before the write can be acknowledged. The write
*** is initiated by a call to the send_fn(). The new coroutine can then
*** monitor the progress of the write by calls to pollwrite(), which should
*** fail only under very serious circumstances because the error recovery
*** will cause any remaining data to be discarded. The abortwrite() routine
*** is not used at present.
***
*** The main implementation of this port I/O is for the IBM PC, so the
*** ibm/pclocal.c file contains typical local code. The PC serial lines
*** are handled using interrupts, and the parallel lines by polling.
**/

#if Ports_used

void add_port_node(list, name, port)
List *list;
char *name;
ComsPort *port;
{ register Port_node *node = (Port_node *) malloc(sizeof(Port_node));
  if (node eq (Port_node *) NULL)
    { ServerDebug("Insufficient memory to initialise devices.");
/*      longjmp(exit_jmpbuf, 1); */
      longjmp_exit;
    }

  strcpy(node->objnode.direntry.Name, name);
  node->objnode.direntry.Type = Type_File;
  node->objnode.size          = 0L;
  node->objnode.account       = 0L;
  node->port                  = port;
  AddTail(&(node->objnode.node), list);
}

/**
*** Port_TidyServer is responsible for tidying up all the communication
*** ports by calling the hardware-specific done functions and freeing the
*** list.
**/
PRIVATE void port_tidy(node)
Port_node *node;
{ ComsPort *port = node->port;
  (*port->done_fn)(port);
}

void Port_TidyServer(myco)
Conode *myco;
{ List *list = (List *) myco->extra;
  WalkList(list, func(port_tidy));
  FreeList(list);
}
  
/**
*** Rename may be used to change the default port. For example, the user can
*** use "mv /printers/com1 /printers/default" to set the default printer port
*** to be the first serial port. IOname will contain printers/default, and
*** the rename offset will contain default. To set the default printer, all
*** I need to do is change its id in the port node structure. 
**/
void Port_Rename(myco)
Conode *myco;
{ char *data = mcb->Data;
  int  dest    = (int) mcb->Control[RenameToname_off];
  Port_node *source_node;

  if (!strcmp(myco->name, IOname))     /* Cannot rename e.g. /printers */
   { Request_Return(EC_Error + SS_IOProc + EG_WrongFn + EO_Server, 0L, 0L);
     return;
   }

  source_node = (Port_node *) Dir_find_node(myco);
  if (source_node eq (Port_node *) NULL)
   { Request_Return(EC_Error + SS_IOProc + EG_Unknown + EO_File, 0L, 0L);
     return;
   }

  if (strcmp(&(data[dest]), "default"))
   { Request_Return(EC_Error + SS_IOProc + EG_Protected + EO_File, 0L, 0L);
     return;
   } 

  ((Port_node *) ((List *) myco->extra)->head)->port = source_node->port;

  Request_Return(ReplyOK, 0L, 0L);
}

/**
*** Open may be used to read the directory or to open an entry within
*** the directory
**/
void Port_Open(myco)
Conode *myco;
{ Port_node *node;

  if (!strcmp(IOname, myco->name))    /* is it for the server ? */
   NewStream(Type_Directory, Flags_Closeable, (word) myco->extra,
             PortDir_Handlers);
  elif ( (node = (Port_node *) Dir_find_node(myco) ) eq (Port_node *) NULL)
   Request_Return(EC_Error + SS_IOProc + EG_Unknown + EO_File, 0L, 0L);
  else
   { node->objnode.account++;
     NewStream(Type_File, Flags_Closeable + Flags_Interactive, (word) node,
             node->handlers );
   }
}

/**
*** Reads are rather complicated. The first step is to impose a sensible
*** upper limit on the read and obtain a suitable buffer. Then the
*** appropriate coms port is locked, i.e. there can be only one I/O
*** operation active at a time. The read is initiated by a call to
*** the hardware specific receive routine, and monitored by the pollread
*** routines. Once the read has timed-out, finished, or aborted a suitable
*** message is sent to the transputer, the port is unlocked, and the
*** operation has finished.
**/
void Port_Read(myco)
Conode *myco;
{ int       count      = (int) (mcb->Control)[ReadSize_off];
  word      timeout    = (mcb->Control)[ReadTimeout_off];
  word      timelimit;
  word      reply_port = mcb->MsgHdr.Reply;
  Port_node *node      = (Port_node *) myco->extra;
  ComsPort  *port      = node->port;
  UBYTE      *buff;
  word      temp;

  if (count eq 0)
   { Request_Return(ReadRc_EOD, 0L, 0L); return; }

  if (mcb->Control[ReadSize_off] > 1024L)
    count = 1024;  /* impose an upper limit */

  buff = (UBYTE *) malloc(count);
  if (buff eq (UBYTE *) NULL)
   { Request_Return(EC_Warn + SS_IOProc + EG_NoMemory + EO_Server, 0L, 0L);
     return;
   }

  if (timeout eq -1L)
   timelimit = MAXTIME;
  else
   timelimit = Now + divlong( timeout, time_unit);

  myco->timelimit = timelimit;
  Wait(&(port->lock));
  if ((myco->type eq CoSuicide) || (myco->type eq CoTimeout))
   { 
     iofree(buff);
     if (myco->type eq CoTimeout)
      { mcb->MsgHdr.Reply = reply_port;
        Request_Return(EC_Recover + SS_IOProc + EG_InUse + EO_Port, 0L, 0L);
        return;
      }
     else
      Seppuku();
   }

  Server_errno = EC_Recover + SS_IOProc + EG_Timeout + EO_Stream;

  unless((*port->receive_fn)(port, (word) count, &(buff[0])))
   { iofree(buff);
     mcb->MsgHdr.Reply = reply_port;
     Request_Return(Server_errno, 0L, 0L);
     return;
   }

  AddTail(Remove(&(myco->node)), PollingCo);
  myco->type = CoReady;

  forever
   { 
     temp = (*port->pollread_fn)(port);
     if (temp eq -1L)        /* finished */
      { mcb->Control[Reply1_off] = (word) count;
        break;
      }

     if (temp >= 0L)         /* an error occurred, with some data read */
      { (*port->error_fn)(port);
        mcb->Control[Reply1_off] = temp;
        break;
      }

     Suspend();
     if (myco->type eq CoSuicide)
      { (void) (*port->abortread_fn)(port);
	Signal(&(port->lock));
        iofree(buff);
        Seppuku();
      }
     elif (myco->type eq CoTimeout)
      { mcb->Control[Reply1_off] = (*port->abortread_fn)(port);
        break;
      }
   }

  AddTail(Remove(&(myco->node)), WaitingCo);
  mcb->MsgHdr.Reply = reply_port;
  memcpy(&(mcb->Data[0]), &(buff[0]), (int) mcb->Control[Reply1_off]);
  if (mcb->Control[Reply1_off] eq 0L)  /* no data */
   Request_Return(EC_Recover + SS_IOProc + EG_Timeout + EO_Port, 0L, 0L);
  else
   Request_Return(ReadRc_EOD, 0L, mcb->Control[Reply1_off]);
  Signal(&(port->lock));
  iofree(buff);
}

/**
*** Writes are more complicated than reads. It is necessary to accept
*** all of the data, and then fork a separate coroutine to do the write.
*** I impose an upper limit of 32K of data at a time, to avoid
*** excessively large buffers. For small writes the data will have come
*** with the Write message and must be copied into the buffer before the
*** coroutine can be suspended. The coms port must be locked to guarantee
*** unique access to the port. For large writes the data will arrive
*** in subsequent messages. Once all the data has arrived I fork off a new
*** coroutine, and send back a reply. The coroutine Port_Write_aux is
*** responsible for transferring all the data, and freeing the lock
*** afterwards.
**/
PRIVATE void fn( Port_Write_aux, (Conode *));
PRIVATE ComsPort *Port_Write_port;
PRIVATE UBYTE    *Port_Write_buff;

void Port_Write(myco)
Conode *myco;
{ word      timeout       = mcb->Control[WriteTimeout_off];
  word      count         = mcb->Control[WriteSize_off];
  word      timelimit;
  Port_node *node         = (Port_node *) myco->extra;
  ComsPort  *port         = node->port;
  UBYTE     *buff;
  int       data_buffered = 0;
  word      reply_port    = mcb->MsgHdr.Reply;
  Conode    *writer = (Conode *) NULL;

  if (count eq 0L)
   { mcb->Control[Reply1_off] = 0L;
     Request_Return(WriteRc_Done, 1L, 0L);
     return;
   }

  if (count > 0x7fffL)   /* limit of 32K on writes */
   { Request_Return(EC_Error + SS_IOProc + EG_WrongSize + EO_Stream, 0L, 0L);
     return;
   }

  if (timeout eq -1L)
   timelimit = MAXTIME;
  else
   timelimit = Now + divlong(timeout, time_unit);

  buff = (UBYTE *) malloc((uint) count);
  if (buff eq (UBYTE *) NULL)
   { Request_Return(EC_Error + SS_IOProc + EG_NoMemory + EO_Server, 0L, 0L);
     return;
   }

  if (mcb->MsgHdr.DataSize ne 0)
   { memcpy(buff, mcb->Data, mcb->MsgHdr.DataSize);
     data_buffered = 1;
   }

  myco->timelimit = timelimit;
  Wait(&(port->lock));

  if ((myco->type eq CoTimeout) || (myco->type eq CoSuicide))
   { iofree(buff);
     if (myco->type eq CoSuicide)
      Seppuku();
     else
      { mcb->MsgHdr.Reply = reply_port;
        Request_Return(EC_Recover + SS_IOProc + EG_Timeout + EO_Stream, 0L, 0L);
        return;
      }
   }

  unless(data_buffered)
   { word fetched = 0L;
     byte *a_ptr  = buff;

     mcb->Control[Reply1_off] = (count > maxdata) ? maxdata : count;
     mcb->Control[Reply2_off] = maxdata;
     mcb->MsgHdr.Flags        = MsgHdr_Flags_preserve;
     mcb->MsgHdr.Reply        = reply_port;
     Request_Return(WriteRc_Sizes, 2L, 0L);

     while (fetched < count)
      { if (timeout eq -1L)
         myco->timelimit = MAXTIME;
        else
         myco->timelimit = Now + divlong(timeout, time_unit);

        Suspend();
        if ((myco->type eq CoSuicide) || (myco->type eq CoTimeout) ||
            (mcb->MsgHdr.DataSize eq 0) )
         { Signal(&(port->lock)); iofree(buff);
           Seppuku();  /* no sensible way to recover from timeout */
         }

        fetched += (word) mcb->MsgHdr.DataSize;
        memcpy(a_ptr, mcb->Data, mcb->MsgHdr.DataSize);
        a_ptr += mcb->MsgHdr.DataSize;
      }
   }

  Server_errno = EC_Recover + SS_IOProc +EG_InUse + EO_Port;
  unless((*port->send_fn)(port, count, buff))
   { iofree(buff);
     Signal(&(port->lock));
     mcb->MsgHdr.Reply = reply_port;
     Request_Return(Server_errno, 0L, 0L);
     return;
   }

  writer = NewCo(Port_Write_aux);
  if (writer eq (Conode *) NULL)
   { mcb->MsgHdr.Reply = reply_port;
     Request_Return(EC_Error + SS_IOProc +EG_NoMemory + EO_Server, 0L, 0L);
     iofree(buff); Signal(&(port->lock));
     return;
   }
  AddTail(&(writer->node), PollingCo);
  writer->id         = -1L;
  writer->timelimit  = MAXTIME;
  writer->name[0]    = '\0';
  writer->flags      = 0L;
  writer->type       = 0L;

  Port_Write_port   = port;
  Port_Write_buff   = buff;

  mcb->MsgHdr.Reply        = reply_port;
  mcb->Control[Reply1_off] = count;
  Request_Return(WriteRc_Done, 1L, 0L);
  StartCo(writer);
}

/**
*** This routine runs as a separate coroutine, and is responsible for
*** sending the specified amount of data down the line. It can take as
*** long as it has to. Because it runs as a separate coroutine it cannot
*** be passed the arguments in the normal way, instead statics are used.
*** The actual transfer has already been initiated successfully above,
*** but it must be monitored.
**/

PRIVATE void Port_Write_aux(myco)
Conode *myco;
{ ComsPort *port = Port_Write_port;
  UBYTE    *buff = Port_Write_buff;
  word     temp;

  forever
   { temp = (*port->pollwrite_fn)(port);
     if (temp >= 0L)   /* an error occurred, oh dear I have to throw away */
      { (*port->error_fn)(port);  /* the rest of the data */
        break;
      }

     if (temp eq -1L) /* finished successfully */
      break;

     myco->type = CoReady;
     Suspend();
     if ((myco->type eq CoTimeout) || (myco->type eq CoSuicide))
      break;
   }

  iofree(buff);
  Signal(&(port->lock));
  Seppuku();
}
  
void Port_Close(myco)
Conode *myco;
{ Port_node *node = (Port_node *) myco->extra;
  ComsPort  *port = node->port;

  node->objnode.account--;
  if (mcb->MsgHdr.Reply ne 0L)
    Request_Return(ReplyOK, 0L, 0L);
  Seppuku();
}

void Port_GetAttr(myco)
Conode *myco;
{ Port_node *node = (Port_node *) myco->extra;
  ComsPort  *port = node->port;

  CopyAttributes((Attributes *) mcb->Data, &(port->attr));
  Request_Return(ReplyOK, 0L, (word) sizeof(Attributes));
}

/**
*** Setting the attributes may effect current I/O, so the port has to be
*** locked.
**/

void Port_SetAttr(myco)
Conode *myco;
{ Port_node  *node = (Port_node *) myco->extra;
  ComsPort   *port = node->port;
  Attributes attr;
  word       reply_port = mcb->MsgHdr.Reply;

  CopyAttributes(&attr, (Attributes *) mcb->Data);
  myco->timelimit = Now + divlong(5L * OneSec, time_unit);
  Wait(&(port->lock));
  if ((myco->type eq CoSuicide) || (myco->type eq CoTimeout))
   { 
     if (myco->type eq CoTimeout)
      { mcb->MsgHdr.Reply = reply_port;
        Request_Return(EC_Recover + SS_IOProc + EG_InUse + EO_Port, 0L, 0L);
        return;
      }
     else
      Seppuku();
   }
  CopyAttributes(&(port->attr), &attr);
  (*port->configure_fn)(port);
  Signal(&(port->lock));
  Request_Return(ReplyOK, 0L, 0L);
}


#endif /* Ports_used */

/*------------------------------------------------------------------------
--
-- the RS232
--
------------------------------------------------------------------------*/

/**
*** The purpose of the Helios RS232 device is to provide just enough support
*** for kermit and for use with serial printers.
**/
#if RS232_supported

DirHeader   RS232_list;

void RS232_error_handler()
{ 
}

PRIVATE void RS232_init_port(node)
Port_node *node;
{ ComsPort   *port = node->port;
  Attributes *attr;

  InitSemaphore(&(port->lock), 1);

  attr = &(port->attr);
  InitAttributes(attr);
/**
*** Default attributes : 9600 baud on input and output, 8 bits per byte, with
*** one stop bit, parity disabled on output and ignored on input, xon/xoff
*** enabled on input and output, do not hang up on close, ignore modem status
*** lines, generate event on break.
**/
  SetInputSpeed( attr, (word) RS232_B9600);
  SetOutputSpeed(attr, (word) RS232_B9600);
  AddAttribute(attr,   RS232_Csize_8);
  AddAttribute(attr,   RS232_IgnPar);
  AddAttribute(attr,   RS232_IXON);
  AddAttribute(attr,   RS232_IXOFF);
  AddAttribute(attr,   RS232_CLocal);
  AddAttribute(attr,   RS232_BreakInterrupt);

  node->handlers      = RS232_Handlers;

  (*port->configure_fn)(port);
}

void RS232_InitServer(myco)
Conode *myco;
{ register Port_node *node;

  InitList(&RS232_list.list);
  myco->extra = (ptr) &RS232_list.list;

  add_port_node(&RS232_list.list, "default", (ComsPort *) NULL);
  node        = (Port_node *) RS232_list.list.head;
 
  RS232_list.entries = RS232_initlist(&RS232_list.list,
     &(node->port) ) + 1L;

  WalkList(&RS232_list.list, func(RS232_init_port));
}

void RS232_TidyServer(myco)
Conode *myco;
{
  Port_TidyServer(myco);
}

void RS232_EnableEvents(myco)
Conode *myco;
{ word mask = mcb->Control[EnableEventsMask_off] &
             (Event_SerialBreak + Event_ModemRing);
  Port_node *node = (Port_node *) myco->extra;
  ComsPort  *port = node->port;

  if (mask eq 0L)       /* disable a break handler */
    { RS232_disable_events(port);
      mcb->Control[Reply1_off] = 0L;
      Request_Return(ReplyOK, 1L, 0L);
    }
  else                  /* install an event handler */
    {
      RS232_enable_events(port, mask, mcb->MsgHdr.Reply);
      mcb->Control[Reply1_off] = mask;
      mcb->MsgHdr.Flags = MsgHdr_Flags_preserve;
      Request_Return(ReplyOK, 1L, 0L);
    } 
}


#endif   /* RS232_supported */

/*------------------------------------------------------------------------
--
-- the Centronics interface
--
------------------------------------------------------------------------*/

#if Centronics_supported

DirHeader   Centronics_list;

void Centronics_error_handler()
{ if (Centronics_errno & 0x0010)    /* Check the bit reserved for Centronics errors */
   { switch (Centronics_errno)
      { case printer_outofpaper : output("*** SERVER : printer out of paper.\r\n");
                                  break;
        case printer_offline    : output("*** SERVER : printer off line.\r\n");
                                  break;
        case printer_invalid    : output("*** SERVER : printer not connected.\r\n");
                                  break;

        case printer_error      :
        default                 : output("*** SERVER : unknown error when accessing printer.\r\n");
                                  break;
      }
   }
        
  Centronics_errno = 0;
}

PRIVATE void Centronics_init_port(node)
Port_node *node;
{ ComsPort   *port = node->port;
  Attributes *attr = &(port->attr);

  InitSemaphore(&(port->lock), 1);
  InitAttributes(attr);
  node->handlers      = Centronics_Handlers;
  (*port->configure_fn)(port);
}

void Centronics_InitServer(myco)
Conode *myco;
{ register Port_node *node;

  InitList(&Centronics_list.list);
  myco->extra = (ptr) &Centronics_list;

  add_port_node(&Centronics_list.list, "default", (ComsPort *) NULL);
  node = (Port_node *) Centronics_list.list.head;

  Centronics_list.entries = 
    Centronics_initlist(&(Centronics_list.list), &(node->port) ) + 1L;

  WalkList(&Centronics_list.list, func(Centronics_init_port));
}

#endif /* Centronics_supported */

/*------------------------------------------------------------------------
--
-- the Printer device
--
------------------------------------------------------------------------*/

#if Printer_supported

DirHeader   Printer_list;

void Printer_error_handler()
{ if (Printer_errno & 0x0010)    /* Check the bit reserved for Centronics errors */
   { switch (Printer_errno)
      { case printer_outofpaper : output("*** SERVER : printer out of paper.\r\n");
                                  break;
        case printer_offline    : output("*** SERVER : printer off line.\r\n");
                                  break;
        case printer_invalid    : output("*** SERVER : printer not connected.\r\n");
                                  break;

        case printer_error      :
        default                 : output("*** SERVER : unknown error when accessing printer.\r\n");
                                  break;
      }
   }
        
  Printer_errno = 0;
}

PRIVATE void Printer_init_port(node)
Port_node *node;
{ ComsPort *port     = node->port;

  InitSemaphore(&(port->lock), 1);
  node->handlers = Printer_Handlers;
  (*port->configure_fn)(port);
}

void Printer_InitServer(myco)
Conode *myco;
{ register Port_node *node;

  InitList(&Printer_list.list);
  myco->extra = (ptr) &Printer_list.list;

  add_port_node(&Printer_list.list, "default", (ComsPort *) NULL);
  node = (Port_node *) Printer_list.list.head;

  Printer_list.entries = Printer_initlist(&Printer_list.list,
     &(node->port) ) + 1L;

  WalkList(&Printer_list.list, func(Printer_init_port));
}

#endif /* Printer_supported */

/*------------------------------------------------------------------------
--
-- Midi
--
------------------------------------------------------------------------*/

#if Midi_supported

DirHeader   Midi_list;

void Midi_error_handler()
{ output("in MIDI_error_handler()\r\n");
  Midi_errno = 0;
}

PRIVATE void Midi_init_port(node)
Port_node *node;
{ ComsPort *port   = node->port;
  Attributes *attr = &(port->attr);

  InitSemaphore(&(port->lock), 1);
  InitAttributes(attr);
  (*port->configure_fn)(port);
  node->handlers = Midi_Handlers;
}

void Midi_InitServer(myco)
Conode *myco;
{ register Port_node *node;

  InitList(&Midi_list.list);
  myco->extra = (ptr) &Midi_list.list;

  add_port_node(&Midi_list.list, "default", (ComsPort *) NULL);
  node = (Port_node *) Midi_list.list.head;

  Midi_list.entries = Midi_initlist(&Midi_list.list, &(node->port)) + 1L;

  WalkList(&Midi_list.list, func(Midi_init_port));
}

#endif    /* Midi_supported */

/*------------------------------------------------------------------------
-- the Mouse and keyboard
------------------------------------------------------------------------*/

/**
*** The mouse device is fairly simple, because all you can do is Open it,
*** enable mouse events, deal with acknowledgements and negative
*** acknowledgements for the mouse, and close it. Open, Close, InitStream,
*** TidyStream, and EnableEvents are all very similar to the corresponding
*** Console routines.
**/

#if mouse_supported

#define       Mouse_limit 256
IOEvent       Mouse_Table[Mouse_limit];
int           Mouse_head, Mouse_tail;
int           Mouse_Relative;
word          Mouse_count, Mouse_ack;
event_handler mouse_handler;

void Mouse_InitServer(myco)
Conode *myco;
{ Mouse_head  = 0;
  Mouse_tail  = 0;
  Mouse_count = 0L;
  Mouse_ack   = -1L;
  mouse_handler.port = (word) NULL;
  initialise_mouse();
  use(myco)
}

void Mouse_TidyServer(myco)
Conode *myco;
{ tidy_mouse();
  use(myco)
}

/**
*** The mouse may be optional for some implementations.
**/
#if !(Mouse_Always_Available)
void Mouse_Testfun(result)
bool *result;
{ char *config = get_config("Xsupport");

  if (config ne (char *) NULL)
    *result = 1L;
  else
    *result = 0L;
}
#endif /* !Mouse_Always_Available */

/**
*** new_mouse() is called each time the mouse interrupt routine wishes to add
*** an event to the mouse buffer. It is called from an interrupt routine,
*** so I cannot do any library calls. On systems where the mouse is not
*** interrupt-driven, e.g. UNIX machines, there is a separate routine
*** read_mouse_events() which should call new_mouse().
**/

void new_mouse(mouse_x, mouse_y, mouse_buttons)
SHORT mouse_x, mouse_y;
word  mouse_buttons;
{ register IOEvent *curr_mouse;

    /* to avoid ridiculous numbers of mouse events I overwrite previous */
    /* movements. However, I must be careful with the buttons. */

  if ((Mouse_head ne Mouse_tail) &&
      (mouse_buttons eq Buttons_Unchanged) &&
      (Mouse_Table[Mouse_head + (Mouse_limit-1) & 
       (Mouse_limit-1)].Device.Mouse.Buttons eq Buttons_Unchanged))
  { Mouse_head = Mouse_head + (Mouse_limit - 1) & (Mouse_limit - 1);
    Mouse_count--;       /* to keep the counter right */
  }

  curr_mouse = &(Mouse_Table[Mouse_head]);

  curr_mouse->Type                 = Event_Mouse;
  curr_mouse->Counter              = Mouse_count++;
  curr_mouse->Stamp                = Now;
  curr_mouse->Device.Mouse.X       = mouse_x;
  curr_mouse->Device.Mouse.Y       = mouse_y;
  curr_mouse->Device.Mouse.Buttons = mouse_buttons;

  Mouse_head = (Mouse_head + 1) & (Mouse_limit - 1);
  if (Mouse_head eq Mouse_tail)       /* buffer overflow */
    { Mouse_tail = (Mouse_tail + 1) & (Mouse_limit - 1);
      curr_mouse->Type |= Flag_Buffer;
    }
}  

PRIVATE void fn( send_mouse_events, (void));
PRIVATE void poll_the_mouse()
{
#if (UNIX)
  { extern void fn( read_mouse_events, (void));
    read_mouse_events();
  }
#endif
          /* Are there any mouse events outstanding, and is there a handler */
          /* for the mouse events (i.e. has there been an enable_events) ?  */
  if ((mouse_handler.port ne (word) NULL) && (Mouse_tail ne Mouse_head))
    send_mouse_events();
}


/**
*** When poll_the_devices() detects that there are outstanding mouse events
*** and there is a mouse event handler active it calls this routine to send
*** the data off to the event handler on the transputer. There is a limit
*** of IOCDataMax on the amount I can send, so that limits the number of
*** events in the table which I can use up. As usual for event handlers I
*** have to worry about the need for acknowledgements and some data may
*** have got lost due to buffer overflow. The data is copied into a message
*** data area, the message is initialised and sent off, and that is it.
**/

PRIVATE void send_mouse_events()
{ IOEvent *table = (IOEvent *) mcb->Data;
  int     count  = 0;
  word    FnRc   = 0L;
  word    Flags  =  MsgHdr_Flags_preserve + MsgHdr_Flags_sacrifice;
               /* Should the handler acknowledge ? */
  if ((Mouse_count - Mouse_ack) > (Mouse_limit / 2))
    FnRc = EventRc_Acknowledge;

                       /* Copy the events into the message data area,       */
                       /* and check for lost events due to buffer overflow. */
  while ((Mouse_tail ne Mouse_head) && (count < (IOCDataMax / sizeof(IOEvent))))
    { if ((Mouse_Table[Mouse_tail].Type & Flag_Buffer) ne 0L)
        { FnRc |= EventRc_IgnoreLost;
          Mouse_Table[Mouse_tail].Type &= ~Flag_Buffer;
        } 
        
      /* try not to throw buttons away */
        
      if (Mouse_Table[Mouse_tail].Device.Mouse.Buttons != Buttons_Unchanged)
         Flags &= ~MsgHdr_Flags_sacrifice;  
      copy_event(&(table[count++]), &Mouse_Table[Mouse_tail]);
      Mouse_tail = (Mouse_tail + 1) & (Mouse_limit - 1);
    }

  mcb->MsgHdr.Reply = mouse_handler.port;
  mcb->MsgHdr.Dest  = 0L;
  mcb->MsgHdr.Flags = Flags;
                                                             /* and send it */
  Request_Return(FnRc, 0L, (word)(sizeof(IOEvent) * count));
}

/**
*** opening a mouse is very straightforward.
**/

void Mouse_Open(myco)
Conode *myco;
{ 
  if (strcmp(IOname, "mouse") )
    { Request_Return(EC_Error + SS_IOProc + EG_Name + EO_Message, 0L, 0L);
      return;
    }

  Mouse_Relative =  ((mcb->Control)[OpenMode_off] eq O_Truncate);
  NewStream(Type_File, Flags_Closeable + Flags_Interactive, NULL, 
            Mouse_Handlers);
  use(myco)
}

word Mouse_InitStream(myco)
Conode *myco;
{ myco->extra = (word *) NULL;       /* A new stream does not own the event handler */
  return(ReplyOK);
}

word Mouse_TidyStream(myco)
Conode *myco;
{          /* does this stream own mouse events ?*/
           /* The stream must not time out if it still owns the event handler */
  if ((event_handler *) myco->extra ne (event_handler *) NULL)
   return(1L);
  else
   return(0L);
}

void Mouse_Close(myco)
Conode *myco;
{ if (mcb->MsgHdr.Reply ne 0L)
    Request_Return(ReplyOK, 0L, 0L);
  if ((event_handler *) myco->extra ne (event_handler *) NULL)
   { ((event_handler *) myco->extra)->port = 0L;
     stop_mouse();
   }
  Seppuku();
}

void Mouse_EnableEvents(myco)
Conode *myco;
{ word mask = mcb->Control[EnableEventsMask_off] & Event_Mouse;

  if (mask eq 0L)                             /* disable a mouse handler */
    { if (mouse_handler.port ne (word) NULL)  /* is there a mouse handler ? */
       { mouse_handler.port = (word) NULL;
         *mouse_handler.ownedby = (word) NULL;
         stop_mouse();
       }
      mcb->Control[Reply1_off] = 0L;
      Request_Return(ReplyOK, 1L, 0L);
    }
  else                                       /* install an event handler */
    { if (mouse_handler.port ne (word) NULL) /* disable an existing handler */
         *mouse_handler.ownedby = (word) NULL;
      else
        start_mouse();
      mouse_handler.port = mcb->MsgHdr.Reply;
      mouse_handler.ownedby = (word *) &(myco->extra);
      myco->extra  = (ptr) &mouse_handler;
      mcb->Control[Reply1_off] = mask;
      mcb->MsgHdr.Flags = MsgHdr_Flags_preserve;
      Request_Return(ReplyOK, 1L, 0L);
    } 
}

/**
*** Mouse_acknowledge is used to tell me that I can release some of my buffer
*** space. Because my buffers are statically allocated and I allow for buffer
*** overflow anyway it is not essential. However, I suspect that its use
*** reduces the frequency of the Ignore_Lost flag and hence the system will
*** recover better if a message is lost. I do not check the validity of the
*** number acknowledged because this allows the other end to acknowledge
*** upto MaxInt events immediately.
**/

void Mouse_Acknowledge(myco)
Conode *myco;
{ Mouse_ack  = mcb->Control[AcknowledgeCount_off];
  use(myco)
}

/**
*** Mouse_NegAcknowledge is used to indicate that one or more messages has been
*** lost. The mcb provided contains the number of the next message expected,
*** and I have to reset my tail pointer to correspond to that event. Then
*** poll_the_devices() will take care of sending the data again.
***
*** There are severe problems if the buffer is almost full, the negative
*** acknowledgement takes me back to an event that has been overwritten or is
*** about to be overwritten, and a new mouse interrupt occurs... I need
*** semaphore protection and truly parallel processes to do this properly, but
*** I have to make do with an ST or a PC. Hence I can but hope that this routine
*** will never cause a crash, or not regularly enough to trace the problem to
*** me....
**/

void Mouse_NegAcknowledge(myco)
Conode *myco;
{ word count    = mcb->Control[NegAcknowledgeCount_off];
  word curcount = Mouse_Table[Mouse_tail].Counter;

                                   /* validate the event count */
  if (count > curcount) return;    /* cannot negacknowledge an unsent message */

/**
*** Now move backwards through Mouse_table until I reach the current head
*** pointer or until the current event is the one after the one acknowledged.
**/
  while (curcount ne count)
   { if (Mouse_tail eq ((Mouse_head + 1) & (Mouse_limit - 1)))
       break;      /* cannot go back any further */
           /* Yukk, I need semaphores here but I have not got them */
     Mouse_tail = (Mouse_tail - 1) & (Mouse_limit - 1);
     curcount   = Mouse_Table[Mouse_tail].Counter;
   }

  if (curcount ne count)                  /* Not reached correct event */
    Mouse_Table[Mouse_tail].Type |= Flag_Buffer;   /* so buffer overflow */

                /* A negative acknowledgement tells me which message has been */
                /* received. */
  Mouse_ack = count - 1;

  use(myco)
}

#endif       /* mouse_supported */

/*-----------------------------------------------------------------------
--
-- The raw keyboard device
--
-----------------------------------------------------------------------*/

#if keyboard_supported
/**
*** There is no real difference between this device and the above mouse
*** device.
**/

#define       Keyboard_limit 256
IOEvent       Keyboard_Table[Keyboard_limit];
int           Keyboard_head, Keyboard_tail;
word          Keyboard_count, Keyboard_ack;
event_handler keyboard_handler = { 0L, (word *) NULL };

PRIVATE void fn ( send_keyboard_events, (void));

void Keyboard_InitServer(myco)
Conode *myco;
{
  Keyboard_head  = 0;
  Keyboard_tail  = 0;
  Keyboard_count = 0L;
  Keyboard_ack   = -1L;
  keyboard_handler.port = (word) NULL;

  initialise_keyboard();
  use(myco)
}

void Keyboard_TidyServer(myco)
Conode *myco;
{ tidy_keyboard();
  keyboard_handler.port = (word) NULL;
  use(myco)
}
 
#if !(Keyboard_Always_Available)
void Keyboard_Testfun(result)
bool *result;
{ char *config = get_config("Xsupport");

  if (config ne (char *) NULL)
    *result = 1L;
  else
    *result = 0L;
}
#endif /* !Keyboard_Always_Available */

/**
*** new_keyboard() is called to add a new keyboard event to the buffer. The
*** information required is which key and whether the event was key-up or
*** key-down. On machines which do not use interrupts for the raw keyboard,
*** i.e. Unix boxes, there is a separate routine read_keyboard_events()
*** which should call new_keyboard().
**/

void new_keyboard(event, key)
word event, key;
{ register IOEvent *curr_keyboard = &(Keyboard_Table[Keyboard_head]);
  curr_keyboard->Type    = Event_Keyboard;
  curr_keyboard->Counter = Keyboard_count++;
  curr_keyboard->Stamp   = Now;
  curr_keyboard->Device.Keyboard.What = (word) event;
  curr_keyboard->Device.Keyboard.Key  = (word) key;
  Keyboard_head = (Keyboard_head + 1) & (Keyboard_limit - 1);
  if (Keyboard_head eq Keyboard_tail)
    { Keyboard_tail = (Keyboard_tail + 1) & (Keyboard_limit - 1);
      curr_keyboard->Type |= Flag_Buffer;
    }
}  

/**
*** This routine checks whether or not there is a keyboard handler installed.
*** If so and there are outstanding events, these are sent. Otherwise the
*** routine returns 1 to indicate to poll_the_devices above that it should
*** poll every window for input.
**/
PRIVATE int poll_the_keyboard()
{
#if (UNIX)
  { extern void fn( read_keyboard_events, (void));
    read_keyboard_events();
  }
#endif

  if (keyboard_handler.port ne (word) NULL)
    { if (Keyboard_tail ne Keyboard_head)
        send_keyboard_events();
      return(0);
    }
  else
   return(1);
}

/**
*** The code for sending keyboard events is rather similar to that for mouse
*** ones because I copied it from above.
**/

PRIVATE void send_keyboard_events()
{ IOEvent *table = (IOEvent *) mcb->Data;
  int     count = 0;
  word    FnRc  = 0L;

  if ((Keyboard_count - Keyboard_ack) > (Keyboard_limit / 2))
    FnRc = EventRc_Acknowledge;

  while ((Keyboard_tail ne Keyboard_head) &&
         (count < (IOCDataMax / sizeof(IOEvent))))
    { if ((Keyboard_Table[Keyboard_tail].Type & Flag_Buffer) ne 0L)
        { FnRc |= EventRc_IgnoreLost;
          Keyboard_Table[Keyboard_tail].Type &= ~Flag_Buffer;
        } 

      Debug(Keyboard_Flag, ("Sending raw keyboard event : %lx %s", Keyboard_Table[Keyboard_tail].Device.Keyboard.Key, ((Keyboard_Table[Keyboard_tail].Device.Keyboard.What eq Keys_KeyUp) ? "Up" : "Down") ) );

      copy_event(&(table[count++]), &Keyboard_Table[Keyboard_tail]);
      Keyboard_tail = (Keyboard_tail + 1) & (Keyboard_limit - 1);

    }

  mcb->MsgHdr.Reply = keyboard_handler.port;
  mcb->MsgHdr.Dest  = 0L;
  mcb->MsgHdr.Flags = MsgHdr_Flags_preserve;

  Request_Return(FnRc, 0L, (word)(sizeof(IOEvent) * count));
}

/**
*** Here are the GSP handlers for the keyboard.
**/

void Keyboard_Open(myco)
Conode *myco;
{ if (strcmp(IOname, "keyboard") )
    { Request_Return(EC_Error + SS_IOProc + EG_Name + EO_Message, 0L, 0L);
      return;
    }

  NewStream(Type_File, Flags_Closeable + Flags_Interactive, NULL, 
            Keyboard_Handlers);
  use(myco)
}

word Keyboard_InitStream(myco)
Conode *myco;
{ myco->extra = (word *) NULL;  /* stream does not own keyboard event handler */
  return(ReplyOK);
}

word Keyboard_TidyStream(myco)
Conode *myco;
{                                /* does this stream own keyboard events ?*/
  if ((event_handler *)myco->extra ne (event_handler *) NULL)
    return(1L);
  else
    return(0L);
}

void Keyboard_Close(myco)
Conode *myco;
{ if (mcb->MsgHdr.Reply ne 0L)
    Request_Return(ReplyOK, 0L, 0L);
  if ((event_handler *) myco->extra ne (event_handler *) NULL)
   { stop_keyboard();
     ((event_handler *) myco->extra)->port = 0L;
   }
  Seppuku();
}

void Keyboard_EnableEvents(myco)
Conode *myco;
{ word mask = mcb->Control[EnableEventsMask_off] & Event_Keyboard;

  if (mask eq 0L)       /* disable a keyboard handler */
    { if (keyboard_handler.port ne (word)NULL)  /* is there a keyboard handler ? */
       { stop_keyboard();
         keyboard_handler.port = (word) NULL;
         *keyboard_handler.ownedby = (word) NULL;
       }
      mcb->Control[Reply1_off] = 0L;
      Request_Return(ReplyOK, 1L, 0L);
    }
  else                 /* install an event handler */
    { if (keyboard_handler.port ne (word) NULL) /* disable an existing handler */
        *keyboard_handler.ownedby = (word) NULL;
      else
       start_keyboard();
      keyboard_handler.port = mcb->MsgHdr.Reply;
      keyboard_handler.ownedby = (word *) &(myco->extra);
      myco->extra  = (ptr) &keyboard_handler;
      mcb->Control[Reply1_off] = mask;
      mcb->MsgHdr.Flags = MsgHdr_Flags_preserve;
      Request_Return(ReplyOK, 1L, 0L);
    } 
}

void Keyboard_Acknowledge(myco)
Conode *myco;
{ Keyboard_ack  = mcb->Control[AcknowledgeCount_off];
  use(myco)
}

void Keyboard_NegAcknowledge(myco)
Conode *myco;
{ word count    = mcb->Control[NegAcknowledgeCount_off];
  word curcount = Keyboard_Table[Keyboard_tail].Counter;

                                   /* validate the event count */
  if (count > curcount) return;    /* cannot negacknowledge an unsent message */

/**
*** Now move backwards through Mouse_table until I reach the current head
*** pointer or until the current event is the one after the one acknowledged.
**/
  while (curcount ne count)
   { if (Keyboard_tail eq ((Keyboard_head + 1) & (Keyboard_limit - 1)))
       break;      /* cannot go back any further */
           /* Yukk, I need semaphores here but I have not got them */
     Keyboard_tail = (Keyboard_tail - 1) & (Keyboard_limit - 1);
     curcount   = Keyboard_Table[Keyboard_tail].Counter;
   }

  if (curcount ne count)                         /* Not reached correct event */
    Keyboard_Table[Keyboard_tail].Type |= Flag_Buffer;  /* so buffer overflow */

                /* A negative acknowledgement tells me which message has been */
                /* received. */
  Keyboard_ack = count - 1;
  use(myco)
}

#endif /* keyboard_supported */

