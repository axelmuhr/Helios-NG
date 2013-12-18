/*------------------------------------------------------------------------
--                                                                      --
--           H E L I O S   N E T W O R K I N G   S O F T W A R E	--
--           ---------------------------------------------------	--
--                                                                      --
--             Copyright (C) 1990, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- netutils : nuobjs.c							--
--	This code keeps track of Objects and Streams that have been	--
--	allocated by an application, generating IOdebug()'s as		--
--	appropriate.							--
--									--
--	Author:  BLV 10/4/92						--
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Header: /hsrc/network/RCS/nuobjs.c,v 1.2 1993/08/11 10:39:21 bart Exp $*/

#include <helios.h>
#include <task.h>
#include <syslib.h>
#include <queue.h>
#include <memory.h>

#ifdef __TRAN
	/* This code only works with the transputer version, using the	*/
	/* module table layout for a trannie.				*/
static WordFnPtr	real_Open;
static WordFnPtr	real_Locate;
static WordFnPtr	real_Create;
static WordFnPtr	real_NewStream;
static WordFnPtr	real_NewObject;
static WordFnPtr	real_CopyObject;
static WordFnPtr	real_PseudoStream;
static WordFnPtr	real_Execute;
static WordFnPtr	real_Load;
static WordFnPtr	real_Close;

static word my_Open(Object *a, char *b, int c)
{ word	result = (real_Open)(a,b,c);
  if (result != 0) IOdebug("Open : %x", result);
  return(result);
}

static word my_Locate(Object *a, char *b)
{ word result = (real_Locate)(a,b);
  if (result != 0) IOdebug("Locate : %x", result);
  return(result);
}

static word my_Create(Object *a, char *b, int c, int d, void *e)
{ word result = (real_Create)(a,b,c,d,e);
  if (result != 0) IOdebug("Create : %x", result);
  return(result);
}

static word	my_Close(Object *a)
{ word	result = (real_Close)(a);
  IOdebug("Close : %x", a);
  return(result);
}

static word	my_NewObject(char *a, void *b)
{ word result = (real_NewObject)(a,b);
  if (result != 0) IOdebug("NewObject : %x", result);
  return(result);
}

static word	my_NewStream(char *a, void *b, int c)
{ word result = (real_NewStream)(a,b,c);
  if (result != 0) IOdebug("NewStream : %x", result);
  return(result);
}

static word	my_CopyObject(Object *a)
{ word result = (real_CopyObject)(a);
  if (result != 0) IOdebug("CopyObject : %x", result);
  return(result);
}

static word	my_PseudoStream(Object *a, word mode)
{ word	result = (real_PseudoStream)(a,mode);
  if (result != 0) IOdebug("PseudoStream : %x", result);
  return(result);
}

static word	my_Execute(Object *a, Object *b)
{ word result = (real_Execute)(a,b);
  if (result != 0) IOdebug("Execute : %x", result);
  return(result);
}

static word	my_Load(Object *a, Object *b)
{ word result = (real_Load)(a,b);
  if (result != 0) IOdebug("Load : %x", result);
  return(result);
}

void	PatchObjects(void)
{ int	*table = (int *) &MyTask;

  IOdebug("Installing own versions of Open, Locate, Create and Close");
  real_Open		= (WordFnPtr) table[1];
  real_Locate		= (WordFnPtr) table[2];
  real_Create		= (WordFnPtr) table[3];
  real_Close		= (WordFnPtr) table[21];
  real_NewObject	= (WordFnPtr) table[13];
  real_NewStream	= (WordFnPtr) table[14];
  real_CopyObject 	= (WordFnPtr) table[12];
  real_PseudoStream	= (WordFnPtr) table[78];
  real_Execute		= (WordFnPtr) table[23];
  real_Load		= (WordFnPtr) table[22];
  table[1]		= (int) &my_Open;
  table[2]		= (int) &my_Locate;
  table[3]		= (int) &my_Create;
  table[21]		= (int) &my_Close;
  table[13]		= (int) &my_NewObject;
  table[14]		= (int) &my_NewStream;
  table[12]		= (int) &my_CopyObject;
  table[78]		= (int) &my_PseudoStream;
  table[23]		= (int) &my_Execute;
  table[22]		= (int) &my_Load;
}

#else
void PatchObjects(void)
{
}
#endif
