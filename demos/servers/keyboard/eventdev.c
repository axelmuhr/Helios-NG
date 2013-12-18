/*------------------------------------------------------------------------
--                                                                      --
--                     H E L I O S			                --
--                     -----------                                      --
--                                                                      --
--             Copyright (C) 1990, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- eventdev.c								--
--                                                                      --
--	Event-based device driver for the raw keyboard server		--
--                                                                      --
--	Author:  BLV 21.2.90						--
--                                                                      --
------------------------------------------------------------------------*/

#include <helios.h>
#include <queue.h>
#ifdef __TRAN
#include <process.h>
#include <event.h>
#else
#include <thread.h>
#include <intr.h>
#endif
#include <codes.h>
#include <device.h>
#include <stdlib.h>

#include "keyboard.h"

	/* Suppress stack checking	*/
#pragma -s1

static word	startup_hardware(KeyboardDCB *dcb);
static void	shutdown_hardware(KeyboardDCB *dcb);
static word	DeviceClose(KeyboardDCB *dcb);
static void	DeviceOperate(KeyboardDCB *dcb, VoidFnPtr fn);

KeyboardDCB *
DevOpen(
	MPtr 	dev,
	void *	info )
{
  KeyboardDCB	*dcb	= (KeyboardDCB *)Malloc(sizeof(KeyboardDCB));
  dcb->DCB.Device	= dev;
  dcb->DCB.Operate	= &DeviceOperate;
  dcb->DCB.Close	= &DeviceClose;
  dcb->new_keyboard	= NULL;
  return(dcb);
}

static word	DeviceClose(KeyboardDCB *dcb)
{ shutdown_hardware(dcb);
  return(Err_Null);
}

static void	DeviceOperate(KeyboardDCB *dcb, VoidFnPtr fn)
{
  if (dcb->new_keyboard != NULL)
   {
#ifdef DEBUG
#ifdef __TRAN
     IOdebug("/keyboard,eventdev.d: fatal, double initialisation");
#else
     char buff[ 4 ];
     SetString_( buff, 0, 'K', ':', '!', '\0' );     
     IOdebug( buff );     
#endif
#endif
     Exit(EXIT_FAILURE << 8);
   } 

  dcb->new_keyboard	= fn;
  dcb->rc = startup_hardware(dcb);
}  

/**-----------------------------------------------------------------------------
*** Hardware specific code.
**/

static void event_handler(KeyboardDCB *dcb);
static void keyboard_monitor(KeyboardDCB *dcb);

static word startup_hardware(KeyboardDCB *dcb)
{
#ifdef __TRAN
  Event	*event = &(dcb->Device.Event.event);
#else
  IntrHandler	*event = &(dcb->Device.Event.handler);
#endif
  
  InitSemaphore(&(dcb->Device.Event.wait), 0);
  event->Pri	= StandardPri;
#ifdef __TRAN
  event->Code	= &event_handler;
#else
  event->Code	= (bool (*)(void *, int))&event_handler;
#endif
  event->Data	= dcb;

  if (
#ifdef __TRAN
      SetEvent(event)
#else
      SetIntrHandler(event)
#endif
      < Err_Null)
   return(EC_Error + EG_UnknownError + EO_Processor);

  unless(Fork(1000, &keyboard_monitor, 4, dcb))
   {
#ifdef __TRAN
     RemEvent(event);
#else
     RemIntrHandler(event);
#endif
     return(EC_Error + EG_NoMemory + EO_Processor);
   }

  return(Err_Null);
}

static void event_handler(KeyboardDCB *dcb)
{ Signal(&(dcb->Device.Event.wait));
}

static void keyboard_monitor(KeyboardDCB *dcb)
{ BYTE	key;
  bool	up;

  forever
   { Wait(&(dcb->Device.Event.wait));
     key = *((BYTE *) 0x00006000);
     up  = ((key & 0x0080) == 0);
     (*(dcb->new_keyboard))(up, key & 0x007F);
   }
}

static void shutdown_hardware(KeyboardDCB *dcb)
{
#ifdef __TRAN
  RemEvent(&(dcb->Device.Event.event));
#else
  RemIntrHandler(&(dcb->Device.Event.handler));
#endif
}

