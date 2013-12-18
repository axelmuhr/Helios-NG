/*------------------------------------------------------------------------
--                                                                      --
--                 H E L I O S   A N S I   E M U L A T O R		--
--                 ---------------------------------------              --
--                                                                      --
--             Copyright (C) 1988, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
--      keymap.h                                                        --
--                                                                      --
--  Author:  BLV 26/9/88                                                --
--                                                                      --
------------------------------------------------------------------------*/

/* RcsId: $Id: keymap.h,v 1.1 1990/10/17 13:41:12 bart Exp $ (C) Copyright 1988, Perihelion Software Ltd. */ 
/* SccsId: 1.3 27/2/90  Copyright (C) 1988, Perihelion Software Ltd. */

/**
*** A header file containing the main ANSI key sequences.
**/

#define CSI			 0x009B

#define Help			 0
#define F1			 1
#define F2			 2
#define F3			 3
#define F4			 4
#define F5			 5
#define F6			 6
#define F7			 7
#define F8			 8
#define F9			 9
#define F10			10
#define ShiftF1			11
#define ShiftF2			12
#define ShiftF3			13
#define ShiftF4			14
#define ShiftF5			15
#define ShiftF6			16
#define ShiftF7			17
#define ShiftF8			18
#define ShiftF9			19
#define ShiftF10		20
#define AltF1			21
#define AltF2			22
#define AltF3			23
#define AltF4			24
#define AltF5			25
#define AltF6			26
#define AltF7			27
#define AltF8			28
#define AltF9			29
#define AltF10			30
#define AltShiftF1		31
#define AltShiftF2		32
#define AltShiftF3		33
#define AltShiftF4		34
#define AltShiftF5		35
#define AltShiftF6		36
#define AltShiftF7		37
#define AltShiftF8		38
#define AltShiftF9		39
#define AltShiftF10		40
#define CursorUp		41
#define CursorDown		42
#define CursorLeft		43
#define CursorRight		44
#define ShiftCursorUp		45
#define ShiftCursorDown		46
#define ShiftCursorLeft		47
#define ShiftCursorRight	48
#define Home			49
#define End			50
#define InsertKey		51
#define Undo			52
#define PageUp			53
#define PageDown		54
#define Delete			55
#define Unknown1		56
#define Unknown2		57
#define Unknown3		58
#define Unknown4		59
#define Unknown5		60
#define Unknown6		61
#define Unknown7		62
#define Unknown8		63
#define Unknown9		64
#define Unknown10		65
#define Unknown11		66
#define Unknown12		67
#define Unknown13		68
#define Unknown14		69
#define Unknown15		70
#define MaxKey			71

static char keymap[MaxKey][5] = {
{ CSI, '?', '~', '\0' },
{ CSI, '0', '~', '\0' },
{ CSI, '1', '~', '\0' },
{ CSI, '2', '~', '\0' },
{ CSI, '3', '~', '\0' },
{ CSI, '4', '~', '\0' },
{ CSI, '5', '~', '\0' },
{ CSI, '6', '~', '\0' },
{ CSI, '7', '~', '\0' },
{ CSI, '8', '~', '\0' },
{ CSI, '9', '~', '\0' },
{ CSI, '1', '0', '~', '\0' },
{ CSI, '1', '1', '~', '\0' },
{ CSI, '1', '2', '~', '\0' },
{ CSI, '1', '3', '~', '\0' },
{ CSI, '1', '4', '~', '\0' },
{ CSI, '1', '5', '~', '\0' },
{ CSI, '1', '6', '~', '\0' },
{ CSI, '1', '7', '~', '\0' },
{ CSI, '1', '8', '~', '\0' },
{ CSI, '1', '9', '~', '\0' },
{ CSI, '2', '0', '~', '\0' },
{ CSI, '2', '1', '~', '\0' },
{ CSI, '2', '2', '~', '\0' },
{ CSI, '2', '3', '~', '\0' },
{ CSI, '2', '4', '~', '\0' },
{ CSI, '2', '5', '~', '\0' },
{ CSI, '2', '6', '~', '\0' },
{ CSI, '2', '7', '~', '\0' },
{ CSI, '2', '8', '~', '\0' },
{ CSI, '2', '9', '~', '\0' },
{ CSI, '3', '0', '~', '\0' },
{ CSI, '3', '1', '~', '\0' },
{ CSI, '3', '2', '~', '\0' },
{ CSI, '3', '3', '~', '\0' },
{ CSI, '3', '4', '~', '\0' },
{ CSI, '3', '5', '~', '\0' },
{ CSI, '3', '6', '~', '\0' },
{ CSI, '3', '7', '~', '\0' },
{ CSI, '3', '8', '~', '\0' },
{ CSI, '3', '9', '~', '\0' },
{ CSI, 'A', '\0' },
{ CSI, 'B', '\0' },
{ CSI, 'D', '\0' },
{ CSI, 'C', '\0' },
{ CSI, 'T', '~', '\0' },
{ CSI, 'S', '~', '\0' },
{ CSI, ' ', 'A', '~', '\0' },
{ CSI, ' ', '@', '~', '\0' },
{ CSI, 'H', '\0' },
{ CSI, '2', 'z', '\0' },
{ CSI, '@', '\0' },
{ CSI, '1', 'z', '\0' },
{ CSI, '3', 'z', '\0' },
{ CSI, '4', 'z', '\0' },
{ 0x7F, '\0' },
{ CSI, '5', 'z', '\0' },
{ CSI, '6', 'z', '\0' },
{ CSI, '7', 'z', '\0' },
{ CSI, '8', 'z', '\0' },
{ CSI, '9', 'z', '\0' },
{ CSI, '1', '0', 'z', '\0' },
{ CSI, '1', '1', 'z', '\0' },
{ CSI, '1', '2', 'z', '\0' },
{ CSI, '1', '3', 'z', '\0' },
{ CSI, '1', '4', 'z', '\0' },
{ CSI, '1', '5', 'z', '\0' },
{ CSI, '1', '6', 'z', '\0' },
{ CSI, '1', '7', 'z', '\0' },
{ CSI, '1', '8', 'z', '\0' },
{ CSI, '1', '9', 'z', '\0' }
};
	
