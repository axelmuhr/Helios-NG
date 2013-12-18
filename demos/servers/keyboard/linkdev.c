/*------------------------------------------------------------------------
--                                                                      --
--                     H E L I O S			                --
--                     -----------                                      --
--                                                                      --
--             Copyright (C) 1990, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- linkdev.c								--
--                                                                      --
--	link-based device driver for the raw keyboard server.		--
--                                                                      --
--	Author:  BLV 21.2.90						--
--                                                                      --
------------------------------------------------------------------------*/

#include <helios.h>
#include <link.h>
#include <device.h>
#include <codes.h>
#include <nonansi.h>
#include <stdlib.h>

#include "keyboard.h"

	/* Suppress stack checking		*/
#pragma -s1

static word	startup_hardware(KeyboardDCB *dcb);
static void	shutdown_hardware(KeyboardDCB *dcb);
static word	DeviceClose(KeyboardDCB *dcb);
static void	DeviceOperate(KeyboardDCB *dcb, VoidFnPtr fn);

KeyboardDCB *
DevOpen(
	MPtr	dev,
	void *	info )
{ KeyboardDCB	*dcb		= (KeyboardDCB *)Malloc(sizeof(KeyboardDCB));
  dcb->DCB.Device		= dev;
  dcb->DCB.Operate		= &DeviceOperate;
  dcb->DCB.Close		= &DeviceClose;
  dcb->new_keyboard		= NULL;
  dcb->Device.Link.running	= FALSE;
  return(dcb);
}

static word	DeviceClose(KeyboardDCB *dcb)
{ shutdown_hardware(dcb);
  return(Err_Null);
}

static void DeviceOperate(KeyboardDCB *dcb, VoidFnPtr fn)
{
  if (dcb->new_keyboard != NULL)
    {
#ifdef DEBUG
#ifdef __TRAN
      IOdebug("/keyboard,linkdev.d: fatal, double initialisation");
#else
      char buff[ 4 ];
      SetString_( buff, 0, 'K', ':', '!' : '\0' );      
      IOdebug( buff );      
#endif
#endif
      Exit(EXIT_FAILURE << 8);
   } 

  dcb->new_keyboard		= fn;
  dcb->Device.Link.running	= TRUE;
  dcb->rc = startup_hardware(dcb);
}

/**-----------------------------------------------------------------------------
*** Hardware specific code.
**/

#define KeyboardLink	3
static void	link_monitor(KeyboardDCB *dcb);

static word	startup_hardware(KeyboardDCB *dcb)
{ LinkInfo	info;
  LinkConf	conf;

	/* 1) reconfigure link 3 to dumb so that it can be used	*/
  if (LinkData(KeyboardLink, &info) < Err_Null)
   return(EC_Error + EG_Invalid + EO_Link);

  conf.Flags	= info.Config.Flags;
  conf.Id	= info.Config.Id;
  conf.Mode 	= Link_Mode_Dumb;
  conf.State	= Link_State_Dumb;
  if (Configure(conf) < Err_Null)
   return(EC_Error + EG_Broken + EO_Link);

	/* 2) Obtain sole access to the link			*/
  if (AllocLink(KeyboardLink) < Err_Null)
   return(EC_Error + EG_InUse + EO_Link);

	/* 3) Fork off a thread to monitor the link		*/
  unless(Fork(1000, &link_monitor, 4, dcb))
   { FreeLink(KeyboardLink);
     return(EC_Error + EG_NoMemory + EO_Link);
   }

  return(Err_Null);
}

static void link_monitor(KeyboardDCB *dcb)
{ BYTE	data[1];

  while(dcb->Device.Link.running)
   if (LinkIn(1, KeyboardLink, data, 2 * OneSec) >= Err_Null)
    { bool up = ((data[0] & 0x0080) == 0);
      (*(dcb->new_keyboard))(up, data[0] & 0x007F);
    }
}

static void shutdown_hardware(KeyboardDCB *dcb)
{ dcb->Device.Link.running = FALSE;
  Delay(3 * OneSec);	/* to let the monitor exit */
  FreeLink(KeyboardLink);
}

