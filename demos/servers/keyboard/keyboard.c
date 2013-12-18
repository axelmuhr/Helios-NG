/*------------------------------------------------------------------------
--                                                                      --
--                     H E L I O S			                --
--                     -----------                                      --
--                                                                      --
--             Copyright (C) 1990, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- keyboard.c								--
--                                                                      --
--	Raw keyboard server.						--
--                                                                      --
--	Author:  BLV 21.2.90						--
--                                                                      --
------------------------------------------------------------------------*/

/*--------------------------------------------------------
-- 		     Include Files			--
--------------------------------------------------------*/
#include <helios.h>	/* standard header */
#include <string.h>
#include <codes.h>
#include <syslib.h>
#include <servlib.h>
#include <gsp.h>
#include <root.h>
#include <link.h>
#include <message.h>
#include <protect.h>
#include <ioevents.h>
#include <nonansi.h>
#include <device.h>
#include <fault.h>

#include <stdio.h>
#include <stdlib.h>

#include "keyboard.h"

static void	init_hardware(char *);
static void	tidy_hardware(void);
static void	do_open(ServInfo *);
static void	usage(void);

static ObjNode		KeyboardRoot;
static DispatchInfo	KeyboardInfo = {
	(DirNode *) &KeyboardRoot,
	NullPort,
	SS_Keyboard,
	NULL,
	{ NULL, 0 },
	{
		{ do_open,	5000 },
		{ InvalidFn,	2000 },		/* Create	*/
		{ DoLocate,	2000 },
		{ DoObjInfo,	2000 },
		{ InvalidFn,	2000 },		/* ServerInfo	*/
		{ InvalidFn,	2000 },		/* Delete	*/
		{ InvalidFn,	2000 },		/* Rename	*/
		{ InvalidFn,	2000 },		/* Link		*/
		{ InvalidFn,	2000 },		/* Protect	*/
		{ InvalidFn,	2000 },		/* SetDate	*/
		{ InvalidFn,	2000 },		/* Refine	*/
		{ InvalidFn,	2000 },		/* CloseObj	*/
	}
};

int main(int argc, char **argv)
{ char	*driver_name;

  if (argc > 2)
   usage();
  elif (argc == 2)
   driver_name = argv[1];
  else
   driver_name = "keyboard.d";

	/* Do the hardware initialisation	*/
  init_hardware(driver_name);
  atexit(&tidy_hardware);

	/* Initialise the root of this server to be a Stream	*/
	/* instead of a directory. Read-only access suffices.	*/
  InitNode(&KeyboardRoot, "/keyboard", Type_Stream, Flags_Interactive,
		0x01010101);

	/* Set the Root object's parent to be a LinkNode to	*/
	/* the processor, create a name table entry, and	*/
	/* call the dispatcher as usual.			*/
  { char	processor_name[IOCDataMax];
    Object	*this_processor;
    Object	*nametable_entry;
    NameInfo	nameinfo;
    LinkNode	*parent;

    MachineName(processor_name);
    this_processor = Locate(Null(Object), processor_name);
    if (this_processor == Null(Object))
     { fprintf(stderr, "/keyboard: failed to locate own processor %s\n",
		processor_name);
       exit(EXIT_FAILURE);
     }

    parent = (LinkNode *) Malloc(sizeof(LinkNode) + (long) strlen(processor_name));
    if (parent == NULL)
     { fprintf(stderr, "/keyboard: out of memory during initialisation\n");
       exit(EXIT_FAILURE);
     }

    InitNode(&parent->ObjNode, "..", Type_Link, 0, DefLinkMatrix);
    parent->Cap = this_processor->Access;
    strcpy(parent->Link, this_processor->Name);
    KeyboardRoot.Parent = (DirNode *) parent;

    nameinfo.Port	= KeyboardInfo.ReqPort = NewPort();
    nameinfo.Flags	= Flags_StripName;    
    nameinfo.Matrix	= DefNameMatrix;
    nameinfo.LoadData	= NULL;

    nametable_entry = Create(this_processor, "keyboard", Type_Name,
	sizeof(NameInfo), (byte *) &nameinfo);
    Close(this_processor);

    if (nametable_entry == Null(Object))
     { fprintf(stderr, "/keyboard: failed to enter name in name table\n");
       exit(EXIT_FAILURE);
     }

    Dispatch(&KeyboardInfo);
    Delete(nametable_entry, Null(char));
  }

 return(EXIT_SUCCESS);
}

static	void usage(void)
{ fputs("/keyboard: usage, keyboard [driver]\n", stderr);
  exit(EXIT_FAILURE);
}

/**-----------------------------------------------------------------------------
*** The Do_Open routine
**/
static	Port		KeyboardPort	= NullPort;
static	Semaphore	KeyboardLock;

static void do_open(ServInfo *servinfo)
{ MCB		*m		= servinfo->m;
  MsgBuf	*r;
  ObjNode	*f;
  BYTE		*data		= m->Data;
  char		*pathname	= servinfo->Pathname;
  Port		stream_port;
  Port		my_event_port	= NullPort;

  f = GetTarget(servinfo);
  if (f == Null(ObjNode))
   { ErrorMsg(m, EO_File); return; }

  unless(f == &KeyboardRoot)
   { ErrorMsg(m, EC_Error + EG_WrongFn + EO_Object); return; }

  r = New(MsgBuf);
  if (r == Null(MsgBuf))
   { ErrorMsg(m, EC_Error + EG_NoMemory + EO_Message); return; }

  FormOpenReply(r, m, f, Flags_Closeable + Flags_Interactive, pathname);
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

     errcode		= GetMsg(m);
     m->MsgHdr.FnRc	= SS_Keyboard;

     if (errcode < Err_Null)
      {	/* Event streams cannot time out if an event is enabled	*/
	if (errcode == EK_Timeout)
	 { Wait(&KeyboardLock);
	   if ((KeyboardPort == my_event_port) &&
	       (KeyboardPort != NullPort))
	     { Signal(&KeyboardLock); continue; }
	   Signal(&KeyboardLock);
	   break;
	 }

	errcode &= EC_Mask;
	if ((errcode == EC_Error) || (errcode == EC_Fatal))
	 break;
	else
	 continue;
      }

     if ((errcode & FC_Mask) != FC_GSP)
      { ErrorMsg(m, EC_Error + EG_WrongFn + EO_Stream); continue; }

     switch(errcode & FG_Mask)
      { case	FG_Close :
	  if (m->MsgHdr.Reply != NullPort)
	   { m->MsgHdr.FnRc = 0; ErrorMsg(m, Err_Null); }
	  goto done;

	case	FG_EnableEvents :
	  { WORD	mask = m->Control[0] & Event_Keyboard;

	    Wait(&KeyboardLock);
	    if (mask == 0)	/* disable event */
	     { if ((KeyboardPort == my_event_port) &&
		   (KeyboardPort != NullPort))
		{ AbortPort(KeyboardPort, EC_Error);
		  KeyboardPort = my_event_port = NullPort;
		}
	       InitMCB(m, 0, m->MsgHdr.Reply, NullPort, Err_Null);
	       MarshalWord(m, 0);
	       PutMsg(m);
	     }
	    else	/* install new event port */
	     { if (KeyboardPort != NullPort)
		AbortPort(KeyboardPort, EC_Error);
	       KeyboardPort = my_event_port = m->MsgHdr.Reply;
	       InitMCB(m, MsgHdr_Flags_preserve, m->MsgHdr.Reply,
			NullPort, Err_Null);
	       MarshalWord(m, mask);
	       PutMsg(m);
	     }
	    Signal(&KeyboardLock);
	    break;
	  }

        default :
		ErrorMsg(m, EC_Error + EG_WrongFn + EO_Stream);
		break;
      }
   }

done:
  f->Account--;
  FreePort(stream_port);
  Wait(&KeyboardLock);
  if ((KeyboardPort == my_event_port) && (KeyboardPort != NullPort))
   { AbortPort(KeyboardPort, EC_Error);
     KeyboardPort = NullPort;
   }
  Signal(&KeyboardLock);
}

/**-----------------------------------------------------------------------------
*** Interacting with the hardware
**/

#define	KeytabSize	32
static	word		KeyboardCounter	= 1;
static	word		KeyboardHead	= 0;
static	IOEvent		Keytab[KeytabSize];
static	KeyboardDCB	*Keyboard_DCB;
static	void		new_keyboard(bool up, int scancode);

static void init_hardware(char *device_driver)
{ int	i;

  InitSemaphore(&KeyboardLock, 1);
  for (i = 0; i < KeytabSize; i++)
   { Keytab[i].Type	= Event_Keyboard;
     Keytab[i].Stamp	= 0;
   }

  Keyboard_DCB = (KeyboardDCB *) OpenDevice(device_driver, NULL);
  if (Keyboard_DCB == Null(KeyboardDCB))
   { fprintf(stderr, "/keyboard: failed to load driver %s\n", device_driver);
     exit(EXIT_FAILURE);
   }

  Operate(&(Keyboard_DCB->DCB), &new_keyboard);
  if (Keyboard_DCB->rc < Err_Null)
   { char buf[128];
     Fault(Keyboard_DCB->rc, buf, 128);
     fprintf(stderr, "/keyboard: error from device driver\n");
     fprintf(stderr, "         : %s\n", buf);
     exit(EXIT_FAILURE);
   }
}

static void tidy_hardware()
{ CloseDevice(&(Keyboard_DCB->DCB));
}

	/* This routine is invoked asynchronously by a thread	*/
	/* running inside the device driver.			*/
static void	new_keyboard(bool up, int scancode)
{ MCB	m;

  Wait(&KeyboardLock);
	/* has an event port been installed ?	*/
  if (KeyboardPort == NullPort) goto done;
  Keytab[KeyboardHead].Counter			= KeyboardCounter++;
  Keytab[KeyboardHead].Device.Keyboard.Key	= scancode;
  Keytab[KeyboardHead].Device.Keyboard.What	= (up) ? Keys_KeyUp : Keys_KeyDown;

  InitMCB(&m, MsgHdr_Flags_preserve, KeyboardPort, NullPort, 0);
  m.Data		= (BYTE *) &(Keytab[KeyboardHead]);
  m.MsgHdr.DataSize	= Keyboard_EventSize;
  m.Timeout		= 5 * OneSec;
  (void) PutMsg(&m);

  KeyboardHead = (KeyboardHead + 1) & (KeytabSize - 1);

done:
  Signal(&KeyboardLock);
}
