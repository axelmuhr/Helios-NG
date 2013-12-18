/*------------------------------------------------------------------------
--                                                                      --
--                     H E L I O S   S E R V E R  T A S K               --
--                     ----------------------------------               --
--                                                                      --
--             Copyright (C) 1988, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- servtask.h								--
--                                                                      --
--	Author:  BLV 15/8/88						--
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Id: servtask.h,v 1.1 1990/10/17 13:43:12 bart Exp $ (C) Copyright 1988, Perihelion Software Ltd. */ 

GLOBAL WORD Host;
#define PC		1
#define DP2		2
GLOBAL WORD Mode;
#define Mode_Foreground	 1
#define Mode_Background  2
#define Config_flags_Background 0x02
#define Config_flags_Nopop	0x04

#define eq 		==
#define ne 		!=
#define ReplyOK	        0
#define Name_Max        256
#define Command_Max     512
#define Console_Max     40
#define preserve	1
#define release		2
#define Stacksize	3000
#define BootLink	0
#define LongTimeout	20 * OneSec

#define memmove mymemmove	/* To avoid name clash with string.h */
GLOBAL void memmove(UBYTE *, UBYTE *, WORD);

GLOBAL BYTE Machine_Name[256];
GLOBAL WORD Message_Limit;
GLOBAL WORD windows_nopop;

extern void _Trace(int, ...);

GLOBAL void write_to_screen(STRING data, int *timedout);
GLOBAL void Return(MCB *, WORD FnRc, WORD ContSize, WORD DataSize,
		    WORD Preserve);
GLOBAL void SendError(MCB *, WORD, WORD Preserve);
GLOBAL void SendOpenReply(MCB *, string name, WORD type, WORD flags,
			   Port reply);

