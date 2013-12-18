/*------------------------------------------------------------------------
--                                                                      --
--                     H E L I O S   S E R V E R S			--
--                     ---------------------------			--
--                                                                      --
--             Copyright (C) 1991, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- familiar.h								--
--                                                                      --
--	Header file defining the protocol between the familiar and the	--
--	Inmos witch board. The protocol has been designed to be		--
--	compatible with the Miniserver-ServTask protocol wherever	--
--	possible.							--
--                                                                      --
--	Author:  BLV 22/3/91						--
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Id: familiar.h,v 1.1 1991/03/23 16:32:49 bart Exp $ (C) Copyright 1988, Perihelion Software Ltd. */  

/**
*** Protocol bytes, for the different protocols across the links.
**/
#define	Pro_IOServ	  1	/* request-reply pairs */
#define Pro_Poll	  4	/* witch->familiar events */

/**
*** Function codes for requests sent to the witch board
**/
#define	Fun_GetClock	 17	/* Get the real-time clock value	  */
#define	Fun_SetClock	 18	/* and change it			  */
				/* A single integer is transferred, which */
				/* is a standard Unix time stamp	  */
/**
*** Reply codes sent by the witch board following replies
**/
#define	Rep_Success	128
#define Rep_Failure	129

/**
*** The different types of events that may be sent by the witch board as
*** part of the polling protocol.
**/
#define Poll_RawKeyboard   2
#define Poll_Mouse	   3

/**
*** Packet sent by familiar to witch
**/
typedef struct FullHead {
	BYTE	Protocol;
	BYTE	FnCode;
	BYTE	Extra;
	UBYTE	HighSize;
	UBYTE	LowSize;
} FullHead;
#define sizeofFullHead		5

/**
*** Reply sent by witch board as part of IOServer protocol.
**/
typedef	struct Head	{
	BYTE	FnCode;
	BYTE	Extra;
	UBYTE	HighSize;
	UBYTE	LowSize;
} Head;
#define sizeofHead	4

/**
*** The following structures are used by for mouse and keyboard events
*** sent by the witch board. These are always the same size as the Head
*** structure. The mouse event contains a button change field, which should
*** be one of the codes listed below. It also contains two deltas, signed
*** bytes, allowing changes in the range -128 to 127. The PC I/O Server never
*** seems to involve changes of more than +/- 20, so this range should be
*** OK. The keyboard event only needs two bits of data, the scancode and
*** whether it is up or down. Note that a single detected change may
*** generate two separate events, e.g. if two buttons are released
*** simultaneously.
**/

typedef struct	witch_mouse {
	BYTE		FnCode;		/* always Poll_Mouse */
	BYTE		ButtonChange;
	signed char	DX;
	signed char	DY;
} witch_mouse;

#define	But_Unchanged	0x00
#define But_LeftDown	0x01
#define But_LeftUp	0x81
#define But_RightDown	0x02
#define But_RightUp	0x82
#define But_MiddleDown	0x04
#define But_MiddleUp	0x84

typedef struct witch_keyboard {
	BYTE		FnCode;
	BYTE		Up;		/* 0 if down, non-0 if up */
	BYTE		Scancode;
	BYTE		Spare;
} witch_keyboard;

