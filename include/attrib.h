/*------------------------------------------------------------------------
--                                                                      --
--                     H E L I O S   N U C L E U S                      --
--                     ---------------------------                      --
--                                                                      --
--             Copyright (C) 1987, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- attrib.h								--
--                                                                      --
--	    Stream attributes						--	
--                                                                      --
--	Author:  BLV 25/01/88						--
--                                                                      --
------------------------------------------------------------------------*/
/* SccsId: %W%	%G% Copyright (C) 1987, Perihelion Software Ltd.	*/
/* $Id: attrib.h,v 1.1 90/09/05 11:05:15 nick Exp $ */

#ifndef __attrib_h
#define __attrib_h

#ifndef __helios_h
#include <helios.h>
#endif

#ifndef __syslib_h
#include <syslib.h>
#endif

typedef struct Attributes {
	WORD	Input;
	WORD	Output;
	WORD	Control;
	WORD	Local;
	short	Min;
	short	Time;
} Attributes;

typedef WORD Attribute;

extern WORD GetAttributes(Stream *, Attributes *);
extern WORD SetAttributes(Stream *, Attributes *);
extern WORD IsAnAttribute(Attributes *, Attribute);
extern void AddAttribute(Attributes *, Attribute);
extern void RemoveAttribute(Attributes *, Attribute);
extern WORD GetInputSpeed(Attributes *);
extern WORD GetOutputSpeed(Attributes *);
extern void SetInputSpeed(Attributes *, WORD);
extern void SetOutputSpeed(Attributes *, WORD);

#define ConsoleEcho			0x00000007
#define ConsoleIgnoreBreak		0x00000100
#define ConsoleBreakInterrupt		0x00000200
#define ConsolePause			0x00000400
#define ConsoleRawInput			0x0000000b
#define ConsoleRawOutput		0x00000101

#define RS232_IgnPar          0x00000800
#define RS232_ParMrk          0x00001000
#define RS232_InPck           0x00002000
#define RS232_IXON            0x00004000
#define RS232_IXOFF           0x00008000
#define RS232_Istrip          0x00010000
#define RS232_IgnoreBreak     0x00000100
#define RS232_BreakInterrupt  0x00000200
#define RS232_Cstopb          0x00000102
#define RS232_Cread           0x00000202
#define RS232_ParEnb          0x00000402
#define RS232_ParOdd          0x00000802
#define RS232_HupCl           0x00001002
#define RS232_CLocal          0x00002002
#define RS232_Csize           0x0003C000   /* Mask for the sizes */
#define RS232_Csize_5         0x00004002
#define RS232_Csize_6         0x00008002
#define RS232_Csize_7         0x00010002
#define RS232_Csize_8         0x00020002

#define RS232_B0              0
#define RS232_B50             1
#define RS232_B75             2
#define RS232_B110            3
#define RS232_B134            4
#define RS232_B150            5
#define RS232_B200            6
#define RS232_B300            7
#define RS232_B600            8
#define RS232_B1200           9
#define RS232_B1800          10
#define RS232_B2400          11
#define RS232_B4800          12
#define RS232_B9600          13
#define RS232_B19200         14
#define RS232_B38400         15


#endif
