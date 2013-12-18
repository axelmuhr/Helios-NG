/*------------------------------------------------------------------------
--                                                                      --
--                     H E L I O S			                --
--                     -----------                                      --
--                                                                      --
--             Copyright (C) 1990, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- keyboard.h								--
--                                                                      --
--	Header file defining Keyboard DCB structure.			--
--                                                                      --
--	Author:  BLV 21.2.90						--
--                                                                      --
------------------------------------------------------------------------*/

#ifndef __helios_h
#include <helios.h>
#endif

#ifdef __TRAN
#ifndef __event_h
#include <event.h>
#endif
#else
#ifndef __intr_h
#include <intr.h>
#endif
#endif

#ifndef __link_h
#include <link.h>
#endif

typedef struct EventStruct
  {
    Semaphore	wait;
#ifdef __TRAN
    Event	event;
#else
    IntrHandler	handler;
#endif
  }
EventStruct;

typedef struct LinkStruct
  {
    bool		running;
  }
LinkStruct;

typedef struct KeyboardDCB
  {
    DCB			DCB;
    VoidFnPtr		new_keyboard;
    word		rc;
    union
      {
	LinkStruct	Link;
	EventStruct	Event;
      }
    Device;
  }
KeyboardDCB;

