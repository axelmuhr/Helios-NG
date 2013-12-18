/*------------------------------------------------------------------------
--                                                                      --
--           H E L I O S   N E T W O R K I N G   S O F T W A R E	--
--           ---------------------------------------------------	--
--                                                                      --
--             Copyright (C) 1991, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- netutils : nuioc.c							--
--									--
--	Author:  BLV 18/12/91						--
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Header: /hsrc/network/RCS/nuioc.c,v 1.3 1993/08/11 10:38:38 bart Exp $*/

#ifdef __TRAN

#include <helios.h>
#include <task.h>
#include <syslib.h>

/**
*** This unbelievably gruesome code zaps the module table entries for
*** Wait(), Delay(), Locate(), Open(), ServerInfo(), Execute(), Read(), Write()
*** and Close(),replacing these with private calls that store some information
*** and then call the system ones. This should allow me to work out where a
*** program deadlocks.
**/

static	WordFnPtr	real_Wait;
#define Wait(a)		(*real_Wait)(a)
static	WordFnPtr	real_Delay;
static	WordFnPtr	real_Locate;
static	WordFnPtr	real_Open;
static	WordFnPtr	real_ServerInfo;
static	WordFnPtr	real_Execute;
static	WordFnPtr	real_Read;
static	WordFnPtr	real_Write;
static	WordFnPtr	real_Close;
static	Semaphore	Lock;

static	int		WaitCount	= 0;
static	int		DelayCount	= 0;
static	int		LocateCount	= 0;
static	int		OpenCount	= 0;
static	int		ServerInfoCount = 0;
static	int		ExecuteCount	= 0;
static	int		ReadCount	= 0;
static	int		WriteCount	= 0;
static	int		CloseCount	= 0;


static	void	my_Wait(Semaphore *x)
{
  Wait(&Lock);
  WaitCount++;
  Signal(&Lock);
  Wait(x);
  Wait(&Lock);
  WaitCount--;
  Signal(&Lock);
}

static	void	my_Delay(int delay)
{
  Wait(&Lock);
  DelayCount++;
  Signal(&Lock);
  (*real_Delay)(delay);
  Wait(&Lock);
  DelayCount--;
  Signal(&Lock);
}

static	Object	*my_Locate(Object *context, string name)
{ Object	*result;

  Wait(&Lock);
  LocateCount++;
  Signal(&Lock);
  result = (Object *) (*real_Locate)(context, name);
  Wait(&Lock);
  LocateCount--;
  Signal(&Lock);
  return(result);
}

static	Stream	*my_Open(Object *context, string name, int mode)
{ Stream	*result;

  Wait(&Lock);
  OpenCount++;
  Signal(&Lock);
  result = (Stream *)(*real_Open)(context, name, mode);
  Wait(&Lock);
  OpenCount--;
  Signal(&Lock);
  return(result);
}

static	word	my_ServerInfo(Object *context, BYTE *buffer)
{ int	result;

  Wait(&Lock);
  ServerInfoCount++;
  Signal(&Lock);
  result = (word) (*real_ServerInfo)(context, buffer);
  Wait(&Lock);
  ServerInfoCount--;
  Signal(&Lock);
  return(result);
}

static	Object	*my_Execute(Object *procman, Object *image)
{ Object	*result;

  Wait(&Lock);
  ExecuteCount++;
  Signal(&Lock);
  result = (Object *)(*real_Execute)(procman, image);
  Wait(&Lock);
  ExecuteCount--;
  Signal(&Lock);
  return(result);
}

static	word	my_Read(Stream *s, BYTE *buffer, int size, int timeout)
{ word	result;

  Wait(&Lock);
  ReadCount++;
  Signal(&Lock);
  result = (word)(*real_Read)(s, buffer, size, timeout);
  Wait(&Lock);
  ReadCount--;
  Signal(&Lock);
  return(result);
}

static	word	my_Write(Stream *s, BYTE *buffer, int size, int timeout)
{ word	result;

  Wait(&Lock);
  WriteCount++;
  Signal(&Lock);
  result = (word) (*real_Write)(s, buffer, size, timeout);
  Wait(&Lock);
  WriteCount--;
  Signal(&Lock);
  return(result);
}

static	word	my_Close(Stream *s)
{ word	result;

  Wait(&Lock);
  CloseCount++;
  Signal(&Lock);
  result = (word) (*real_Close)(s);
  Wait(&Lock);
  CloseCount--;
  Signal(&Lock);
  return(result);
}

void	PatchIOC(int x)
{
  WORD	**modtab;
  WORD	*kernel_slot;
  WORD	*syslib_slot;

  InitSemaphore(&Lock, 1);

  modtab	= (WORD **) (*(WORD *)(*(WORD *)(&x - 1)));
  kernel_slot	= modtab[1];
  syslib_slot	= modtab[2];

  real_Wait		= (WordFnPtr) kernel_slot[18];
#if 0		/* This seems to crash the world	*/
  kernel_slot[18]	= (WORD) &my_Wait;
#endif
  real_Delay		= (WordFnPtr) kernel_slot[36];
  kernel_slot[36]	= (WORD) &my_Delay;
  real_Open		= (WordFnPtr) syslib_slot[1];
  syslib_slot[1]	= (WORD) &my_Open;
  real_Locate		= (WordFnPtr) syslib_slot[2];
  syslib_slot[2]	= (WORD) &my_Locate;
  real_ServerInfo	= (WordFnPtr) syslib_slot[5];
  syslib_slot[5]	= (WORD) &my_ServerInfo;
  real_Read		= (WordFnPtr) syslib_slot[16];
  syslib_slot[16]	= (WORD) &my_Read;
  real_Write		= (WordFnPtr) syslib_slot[17];
  syslib_slot[17]	= (WORD) &my_Write;
  real_Close		= (WordFnPtr) syslib_slot[21];
  syslib_slot[21]	= (WORD) &my_Close;
  real_Execute		= (WordFnPtr) syslib_slot[23];
  syslib_slot[23]	= (WORD) &my_Execute;
}

void	ShowIOC(void)
{
  IOdebug("ShowIOC: Wait %d, Delay %d, Locate %d, Open %d, ServerInfo %d",
		WaitCount, DelayCount, LocateCount, OpenCount,
		ServerInfoCount);
  IOdebug("ShowIOC: Execute %d, Read %d, Write %d, Close %d",
		ExecuteCount, ReadCount, WriteCount, CloseCount);
}

#else
void PatchIOC( int x ) {}
void ShowIOC( void ) {}
#endif
