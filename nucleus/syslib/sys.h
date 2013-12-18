/*------------------------------------------------------------------------
--                                                                      --
--                     H E L I O S   N U C L E U S                      --
--                     ---------------------------                      --
--                                                                      --
--             Copyright (C) 1987, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- syslib/sys.h								--
--                                                                      --
--	System Library header file					--
--                                                                      --
--	Author:  NHG 16/8/87						--
--                                                                      --
------------------------------------------------------------------------*/
/* SccsId:	 %W%	%G%	Copyright (C) 1987, Perihelion Software Ltd.*/
/* $Id: sys.h,v 1.18 1993/08/06 12:42:03 bart Exp $ */

#ifndef __sys_h
#define __sys_h

#include <helios.h>	/* Standard Header */

#define in_syslib 1	/* flag that we are in this module */

/*------------------------------------------------------------------------
-- Stack handling							--
------------------------------------------------------------------------*/
	/* Stack checking within syslib is controlled			*/
	/* by -DSTACKCHECK on the command line, not by -ps1 or similar	*/
	/* pragmas. Note that there is special code within pipe.c for	*/
	/* some processors.						*/
#if defined(STACKCHECK)
# pragma -s0
#else
# pragma -s1
#endif

#if defined(__TRAN)
# pragma -f0
#endif

/*--------------------------------------------------------
-- 		     Include Files			--
--------------------------------------------------------*/

#include <syslib.h>
#include <gsp.h>
#include <codes.h>
#include <string.h>

#define memclr(p,n) memset(p,0,n)

/* Option bits for CheckObject and CheckStream */

#define C_ReOpen	0x10	/* ReOpen pseudo streams 	*/
#define C_Locate	0x10	/* locate pseudo object		*/
#define C_Close		0x20	/* prepare for close		*/

/* Safety margin on memory allocation for Object and Stream	*/
/* structures, this allows the pathname to grow by the given	*/
/* amount.							*/

#define SafetyMargin	32

/*--------------------------------------------------------
--		Public Data Definitions 		--
--------------------------------------------------------*/

extern Task		*MyTask;			/* pointer to Task */

/* The type modifier STATIC indicates that this is a variable		   */
/* which is accessed from other files in this library, but is not exported */
/* outside the library.							   */

# define STATIC

typedef struct
{
	MCB	mcb;
	word	size;
	word 	control[IOCMsgMax]; 
	byte 	data[IOCDataMax];
} MsgBuf;


extern	Semaphore	BufLock;	/* lock on IOC buffer		*/
extern	MsgBuf		*IOCBuf;	/* IOC message buffer		*/
extern	Semaphore	ObjectLock;	/* lock on Object list		*/
extern	List		Objects; 	/* list of Objects		*/
extern	Semaphore	StreamLock;	/* lock on Stream list		*/
extern	List		Streams;	/* list of Streams		*/
extern	bool		Terminating;	/* TRUE if program is quitting	*/
extern Capability	DefaultCap;	/* Default capability		*/

/* Routines which are private to the library, but which are called	*/
/* from other files must be defined as static, but any prototypes must	*/
/* be extern.								*/

#ifndef _in_misc
extern MCB *NewMsgBuf(int dsize);
extern void FreeMsgBuf( MCB *mcb );
extern void AddObject(Object *o);
extern void AddStream(Stream *s);
extern WORD CheckObject(Object *obj, word options);
extern WORD CheckStream(Stream *str, word options);
extern WORD StreamMsg(MCB *mcb, Stream *stream);
extern WORD IOCMsg(MCB *m, void *data);
#endif

#ifndef _in_ioc
extern bool SetupStream( Stream *stream, MCB *mcb );
#endif

#ifndef _in_pipe
extern word PipeClose(Stream *s);
extern void PipeAbort(Stream *s);
extern word PipeRead(Stream *s, byte *buffer, word size, word timeout);
extern word PipeWrite(Stream *s, byte *buffer, word size, word timeout);
extern word PipeSize(Stream *s);
extern Port PipeSelect(Stream *s, word mode);
#endif

#ifdef SYSDEB
#define SysDebug(flg) if(MyTask->Flags&(Task_Flags_##flg##|Task_Flags_info))_SysDebug
#ifdef _in_misc
static void _SysDebug(char *str, ... );
#else
extern void _SysDebug(char *str, ... );
#endif
#endif

#ifdef __TRAN
# define returnlink_(x) (((word *)(&(x)))[-2])
#else
# ifdef __C40
#  define returnlink_(x)	NULL
# else
#  define returnlink_(x)	NULL
# endif
#endif

#define progname_ ((char *)(MyTask->TaskEntry)+8)

#ifdef PRIVATE
#undef PRIVATE
#endif

#define PRIVATE

PRIVATE char *	  procname( VoidFnPtr );
PRIVATE bool	  CopyPipe( Stream *, Stream * );	  
PRIVATE word	  CloseStream( Stream * stream );
PRIVATE word	  FindPorts(Stream *s, word fn, Port *server, Port *reply);

#endif /* __sys_h */

/* end of sys.h */
