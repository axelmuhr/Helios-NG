/**
*
* Title:  Debug command header.
*
* Author: Andy England
*
* Date:   March 1989
*
*         (C) Copyright 1989 - 1992, Perihelion Software Limited.
*
*         All Rights Reserved.
*
* $Header: /hsrc/cmds/debugger/lib/RCS/dmsg.h,v 1.3 1992/11/04 14:59:09 nickc Exp $
*
**/

#define FG_DebugCmd	0x00008010

#define TraceEntry	0x00000001
#define TraceCommand	0x00000002
#define TraceReturn	0x00000004
#define TraceOff	0x00000008
#define Profile		0x00000010

typedef struct
  {
    word action;
    word thread;
    word modnum;
    word offset;
    word size;
  }
DBGCMD;

typedef struct
  {
    Port 	port;
    byte *	data;
    DBGCMD	cmd;
  }
DBG;

/**
*
* Execution control commands.
*
**/
#define DBG_Ready		0x01
#define DBG_Stop		0x02
#define DBG_StopAll		0x03
#define DBG_Step		0x04
#define DBG_StepAll		0x05
#define DBG_Fork		0x06
#define DBG_Free		0x07
#define DBG_FreeAll		0x08
#define DBG_Kill		0x09
#define DBG_KillAll		0x0a
#define DBG_Go			0x0b
#define DBG_GoAll		0x0c
#define DBG_Goto		0x0d
#define DBG_GotoAll		0x0e
#define DBG_GotoFrame		0x0f
#define DBG_GotoFrameAll	0x10
#define DBG_Timeout		0x11
#define DBG_TimeoutAll		0x12

/**
*
* Monitor commmands.
*
**/
#define DBG_AddBreak		0x13
#define DBG_RemBreak		0x14
#define DBG_AddWatch		0x15
#define DBG_RemWatch		0x16
#define DBG_Trace		0x17
#define DBG_TraceAll		0x18
#define DBG_Profile		0x19
#define DBG_ProfileAll		0x1a

/**
*
* Memory manipulation commands.
*
**/
#define DBG_PeekMem		0x1b
#define DBG_PokeMem		0x1c
#define DBG_LocateData		0x1d
#define DBG_PokeData		0x1e
#define DBG_PeekData		0x1f
#define DBG_LocateStack		0x20
#define DBG_PokeStack		0x21
#define DBG_PeekStack		0x22
#ifdef __C40
#define DBG_LocateRegister	0x23
#define DBG_LocateFrame		0x24
#endif

/**
*
* Stack frame manipulation comamnds
*
**/
#define DBG_Call		0x25
#define DBG_Where		0x26

/**
*
* Replies
*
**/
#define DBG_Stopped		0x27
#define DBG_Changed		0x28
#define DBG_Dump		0x29
#define DBG_Address		0x2a
#define DBG_Continue		0x2b
#define DBG_Return		0x2c
#define DBG_Position		0x2d
#define DBG_EndThread		0x2e
#define DBG_Traced		0x2f
#define DBG_Entered		0x30
#define DBG_Returned		0x31

/* -- crf : 05/08/91 - additional reply */
#define DBG_DelWatchIds		0x32 
