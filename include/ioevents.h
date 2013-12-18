#ifndef __ioevents_h
#define __ioevents_h
/*------------------------------------------------------------------------
--                                                                      --
--                     H E L I O S   N U C L E U S                      --
--                     ---------------------------                      --
--                                                                      --
--             Copyright (C) 1987, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- ioevents.h								--
--                                                                      --
--		raw device events					--
--									--
--	Author:  BLV 08/02/88						--
--                                                                      --
------------------------------------------------------------------------*/
/* SccsId: 1.2	25/11/88 Copyright (C) 1987, Perihelion Software Ltd.	*/
/* RcsId: $Id: ioevents.h,v 1.3 90/11/15 13:08:24 paul Exp $ */

/* These are valid types for the Type field of an Event */
#define Event_Mouse	 0x01L
#define Event_Keyboard	 0x02L
#define Event_Break	 0x04L
#define Event_RS232Break 0x08L
#define Event_ModemRing  0x10L
#define Event_Stylus	 0x20L

#define Flag_Buffer	0x80000000L

/* additional reply codes */
#define EventRc_Acknowledge	0x1L   /* the other side should acknowledge   */
#define EventRc_IgnoreLost	0x2L   /* unimportant messages have been lost */


typedef struct { SHORT	X;
		 SHORT	Y;
		 WORD	Buttons;
} Mouse_Event;

#define Buttons_Unchanged	0x00000000L
#define Buttons_Button0_Down	0x00000001L
#define Buttons_Button0_Up	0x00008001L
#define Buttons_Button1_Down	0x00000002L
#define Buttons_Button1_Up	0x00008002L
#define Buttons_Button2_Down	0x00000004L
#define Buttons_Button2_Up	0x00008004L
#define Buttons_Button3_Down	0x00000008L
#define Buttons_Button3_Up	0x00008008L
#define Buttons_Button4_Down	0x00000010L
#define Buttons_Button4_Up	0x00008010L
#define Buttons_Button5_Down	0x00000020L
#define Buttons_Button5_Up	0x00008020L

/* Helpful names for mouse buttons (standard Helios numbering) */

#define Buttons_Left_Down	Buttons_Button0_Down
#define Buttons_Left_Up	  	Buttons_Button0_Up
#define Buttons_Right_Down  	Buttons_Button1_Down
#define Buttons_Right_Up	Buttons_Button1_Up
#define Buttons_Middle_Down 	Buttons_Button2_Down
#define Buttons_Middle_Up	Buttons_Button2_Up


typedef struct { SHORT	X;
		 SHORT	Y;
		 WORD	Buttons;
		 SHORT  XTilt;
		 SHORT  YTilt;
} Stylus_Event;

/* Helpful names for stylus  buttons.				        */
/* The `proximity' signal is also treated as a button.			*/

#define Buttons_Tip_Down	Buttons_Button3_Down
#define Buttons_Tip_Up		Buttons_Button3_Up
#define Buttons_Barrel_Down	Buttons_Button4_Down
#define Buttons_Barrel_Up	Buttons_Button4_Up
#define Buttons_Into_Prox	Buttons_Button5_Down /* Into proximity range */
#define Buttons_OutOf_Prox	Buttons_Button5_Up   /* Out of proximity     */


typedef struct { WORD	Key;
		 WORD	What;
} Keyboard_Event;

#define Keys_KeyUp	1L
#define Keys_KeyDown	2L

typedef struct { WORD	junk1;
		 WORD	junk2;
} Break_Event;			/* this is for ctrl-C etc. */

typedef struct { WORD   junk1;
                 WORD   junk2;
} RS232Break_Event;

typedef struct { WORD   junk1;
                 WORD   junk2;
} ModemRing_Event;

typedef struct IOEventHdr {
	word Type;
	word Counter;
	word Stamp;
} IOEventHdr;

/* Do not assume sizeof(IOEvent), as events from different servers can vary */
/* Currently each server should only return one size of event */

#define Stylus_EventSize (sizeof(IOEventHdr) + sizeof(Stylus_Event))
#define Mouse_EventSize (sizeof(IOEventHdr) + sizeof(Mouse_Event))
/* Keyboard server includes both break and keyup/down events */
#define Keyboard_EventSize (sizeof(IOEventHdr) + sizeof(Keyboard_Event))
/* RS232 servers include both RS232 break and modem ring */
#define RS232_EventSize (sizeof(IOEventHdr) + sizeof(RS232Break_Event))

typedef struct IOEvent {
	word Type;
	word Counter;
	word Stamp;
	/* @@@ in 1.3/2.0 we will have to add: 'word Size' for multiple event types */
	union {
		Mouse_Event	 Mouse;
		Stylus_Event	 Stylus;
		Keyboard_Event	 Keyboard;
		Break_Event 	 Break;
                RS232Break_Event RS232Break;
                ModemRing_Event  ModemRing;
	} Device;
} IOEvent;

#endif

/* end of ioevents.h */
