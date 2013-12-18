/*------------------------------------------------------------------------
--                                                                      --
--           H E L I O S   N E T W O R K I N G   S O F T W A R E	--
--           ---------------------------------------------------	--
--                                                                      --
--             Copyright (C) 1990 - 1993, Perihelion Software Ltd.      --
--                        All Rights Reserved.                          --
--                                                                      --
-- rmlib1.c								--
--                                                                      --
--	The construction module of the Resource Management library.	--
--	This module contains all the code needed to build networks,	--
--	taskforces, and the like.					--
--                                                                      --
--	Author:  BLV 1/5/90						--
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Header: /hsrc/network/RCS/rmlib1.c,v 1.27 1994/03/10 17:14:04 nickc Exp $*/

#define in_rmlib	1

/*{{{  Header files */

#include <helios.h>
#if defined __SUN4 || defined RS6000
#include </hsrc/include/memory.h>
#include </hsrc/include/link.h>
#define _link_h
#endif
#include <stddef.h>
#include <syslib.h>
#include <root.h>
#include <gsp.h>
#include <nonansi.h>
#include <module.h>
#include <posix.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include "exports.h"
#include "private.h"
#include "rmlib.h"

#ifdef Malloc		/* courtesy of servlib.h */
#undef Malloc
#endif

/*}}}*/
/*{{{  Compile-time options */

#ifdef __TRAN
#pragma -f0		/* 0 == disable vector stack			*/
#pragma -g0		/* remove names from code			*/
#endif

#ifdef STACKCHECK
#ifdef __TRAN
#pragma -s1
static void _stack_error(Proc *p)
{ IOdebug("RmLib: stack overflow in %s at %x",p->Name,&p);
  Exit(0x0080 | SIGSTAK);
}
#endif
#pragma	-s0
#else
#pragma -s1
#endif

#ifdef RS6000
extern Environ * getenviron( void );
#endif

/*}}}*/
/*{{{  Library version numbers */
/**
*** Library revision versions.
***	1.10	first numbered version
***	1.11	change of communication to sockets
***	1.12	execute functions plus clean-up
***	1.13	cleaned up comms code, re. resetting pointers on receipt
***	1.14	added RmRegisterTask(), RmConvertTaskToTaskforce()
***	1.15	bug fixes
***	1.16	fixed problem with EOF conditions in PipeGuardian
***	1.17	now partially compiles on Unix hosts
***	1.18	reduced memory requirements for C40 1.3.1 release
**/
#define	VersionNumber	"1.18"

/*
 * N.B. Please update rmlhost.c as well.
 */
/*}}}*/
/*{{{  Library initialisation routine */
/*
 * This routine must be kept in step with rmlhost.c
 */
void _RmLib_Init()
{
#if 0	
  Object *lib = Locate(Null(Object), "/loader/RmLib");
  if (lib eq Null(Object))
   IOdebug("rmlib : failed to locate library");
  else
   { (void) Delete(lib, Null(char)); (void) Close(lib); }
#endif

  RmT_Names[RmT_Unknown]= "<Unknown>";
  RmT_Names[RmT_Default]= "<Default>";
  RmT_Names[RmT_T800]	= "T800";
  RmT_Names[RmT_T414]	= "T414";
  RmT_Names[RmT_T425]	= "T425";
  RmT_Names[RmT_T212]	= "T212";
  RmT_Names[RmT_T9000]	= "T9000";
  RmT_Names[RmT_T400]	= "T400";
  RmT_Names[RmT_i860]	= "i860";
  RmT_Names[RmT_Arm]	= "Arm";
  RmT_Names[RmT_680x0]	= "680x0";
  RmT_Names[RmT_C40]	= "C40";
  RmProgram		= Rm_User;
  RmNetworkServer	=
  RmSessionManager	=
  RmParent		= (RmServer) NULL;
  RmStartSearchHere	= 0;
  RmVersionNumber	= VersionNumber;
  RmExceptionHandler	= (VoidFnPtr) NULL;
#ifdef STACKEXTENSION
  RmExceptionStack	= 1000;
#else
  RmExceptionStack	= 2000;
#endif
  RmRootName		= NULL;
}
/*}}}*/
/*{{{  Processor manipulation */
/**-----------------------------------------------------------------------------
*** Processor manipulation. These routines deal with a single processor at
*** a time.
***
*** RmNewProcessor : allocate space for a new structure, fill it in
*** with default values, and return a pointer to the new structure.
**/
RmProcessor	RmNewProcessor(void)
{ RmProcessor	result = (RmProcessor) Malloc(sizeof(RmProcessorStruct));
  Date		date = GetDate();
  
  if (result eq (RmProcessor) NULL)
   { RmErrno = RmE_NoMemory; return((RmProcessor) NULL); }

	/* Most of the contents is 0, the rest is set explicitly */
  memset(result, 0, sizeof(RmProcessorStruct));
  result->ObjNode.Type		= Type_Processor;
  result->ObjNode.Matrix	= 0x010101c3;	/* darw:r:r:r */
  InitSemaphore(&(result->ObjNode.Lock), 1);	/* As per InitNode() */
  result->ObjNode.Key		= date + _cputime();
  result->ObjNode.Parent	= Null(struct DirNode);
  result->ObjNode.Dates.Creation =
  result->ObjNode.Dates.Access   =
  result->ObjNode.Dates.Modified = date;
  result->ObjNode.Account	= RmM_NoOwner;
  result->Type			= RmT_Default;
  result->StructType		= RmL_New;
  result->Purpose		= RmP_Helios | RmP_User;
  InitList(&(result->MappedTasks));
  result->SessionId		= -1;
  result->ApplicationId		= -1;
  return(result);
}

/**
*** Free'ing a processor structure. This will fail if the argument is
*** NULL, if the argument does not appear to be a processor, or if the
*** processor is currently in a network. Any mapped tasks have to be
*** unmapped.
**/
int RmFreeProcessor(RmProcessor processor)
{
  CheckProcessor(processor);

  if (processor->ObjNode.Parent ne Null(struct DirNode))
   return(RmErrno = processor->Errno = RmE_InUse);

  if (!EmptyList_(processor->MappedTasks))
   { Node	*x, *y;
     RmTask	 task;
     for (x = Head_(Node, processor->MappedTasks); !EndOfList_(x); x = y)
      { y = Next_(Node, x);
 	task = (RmTask) (((BYTE *) x) - offsetof(RmTaskStruct, MappedNode));
        task->MappedTo = RmL_NoUid;
        Remove(x);
      }
   }

  if (processor->OtherLinks ne Null(RmLink))
   if (Free(processor->OtherLinks) ne Err_Null)
    return(RmErrno = processor->Errno = RmE_Corruption);

  if (processor->AttribData ne Null(char))
   if (Free(processor->AttribData) ne Err_Null)
    return(RmErrno = processor->Errno = RmE_Corruption);

  if (processor->PAttribData ne Null(char))
   if (Free(processor->PAttribData) ne Err_Null)
    return(RmErrno = processor->Errno = RmE_Corruption);
    
	/* This is true iff the routine is called courtesy of RmFreeNetwork() */
	/* when a whole network is being deleted in one go. */
  if (processor->Root ne (RmNetwork) NULL)
   unless(RmReleaseUid((RmObject)processor))
    return(RmErrno = processor->Errno = RmE_Corruption);

  processor->ObjNode.Type	= 0;             
  if (Free(processor) ne Err_Null)
   return(RmErrno = processor->Errno = RmE_Corruption);
  return(RmE_Success);
}

/**
*** The basic operations for manipulating processors.
**/
unsigned long RmGetProcessorMemory(RmProcessor processor)
{ CheckProcessorFail(processor, 0L);
  return(processor->MemorySize);
}

int RmSetProcessorMemory(RmProcessor processor, unsigned long size)
{ CheckProcessor(processor);
  processor->MemorySize = size;
  return(RmE_Success);
}

const char *RmGetProcessorId(RmProcessor processor)
{ CheckProcessorFail(processor, (const char *) NULL);
  return(processor->ObjNode.Name);
}

int RmSetProcessorId(RmProcessor processor, char *name)
{ 
  CheckProcessor(processor);
	/* It is illegal to rename an existing processor */
  if (processor->StructType ne RmL_New)
    return(RmErrno = processor->Errno = RmE_ReadOnly);

  if (strlen(name) >= NameMax)
    return(RmErrno = processor->Errno = RmE_TooLong);
  strcpy(processor->ObjNode.Name, name);
  return(RmE_Success);
}

int RmGetProcessorPurpose(RmProcessor processor)
{ CheckProcessorFail(processor, -1);
  return(processor->Purpose);
}

int RmSetProcessorPurpose(RmProcessor processor, int Purpose)
{ CheckProcessor(processor);
  processor->Purpose = Purpose;
  return(RmE_Success);
}

int RmGetProcessorState(RmProcessor processor)
{ CheckProcessorFail(processor, -1);
  return((int)processor->ObjNode.Size);
}

int RmSetProcessorState(RmProcessor processor, int state)
{ CheckProcessor(processor);
  processor->ObjNode.Size = state;
  return(RmE_Success);
}

int RmGetProcessorType(RmProcessor processor)
{ CheckProcessorFail(processor, -1);
  return(processor->Type);
}

int RmSetProcessorType(RmProcessor processor, int type)
{ CheckProcessor(processor);
  processor->Type = type;
  return(RmE_Success);
}

int RmGetProcessorOwner(RmProcessor processor)
{ CheckProcessorFail(processor, -1);
  return((int)processor->ObjNode.Account);
}

int RmSetProcessorPrivate(RmProcessor processor, int x)
{ CheckProcessor(processor);
  processor->Private = x;
  return(RmE_Success);
}

int RmSetProcessorPrivate2(RmProcessor processor, int x)
{ CheckProcessor(processor);
  processor->Private2 = x;
  return(RmE_Success);
}

int RmGetProcessorPrivate(RmProcessor processor)
{ CheckProcessorFail(processor, -1);	
  return(processor->Private);
}

int RmGetProcessorPrivate2(RmProcessor processor)
{ CheckProcessorFail(processor, -1);
  return(processor->Private2);
}

int RmGetProcessorError(RmProcessor processor)
{ CheckProcessor(processor);	
  return(processor->Errno);
}

int RmClearProcessorError(RmProcessor processor)
{ CheckProcessor(processor);	
  return(processor->Errno = RmE_Success);
}

RmUid RmGetProcessorUid(RmProcessor processor)
{ CheckProcessorFail(processor, 0);	
  return(processor->Uid);
}

Capability *RmGetProcessorCapability(RmProcessor processor, bool real)
{ if (real)
   return(&(processor->RealCap));
  else
   return(&(processor->NsCap));
}

int RmGetProcessorControl(RmProcessor processor)
{ CheckProcessorFail(processor, -1);
  return(processor->Control);
}
/*}}}*/
/*{{{  Attribute handling */

/**-----------------------------------------------------------------------------
*** Attribute handling. This is the same for processors and for tasks.
***
*** Adding an attribute is valid provided there is enough memory
*** and the string is real. This means it must contain at least one
*** character. The data field is variable-length, initially
*** 32 bytes, which should suffice for a few attributes. If the
*** object does not have one yet, a data vector is allocated.
*** Strings are held as a single byte for the string length, BCPL style,
*** followed by the string itself including the NULL terminator.
**/
typedef	struct	RmAttributes {
	int	Size;
	int	Free;
	char	*Data;
} RmAttributes;

int RmAddObjectAttribute(RmObject obj, char *attr, bool bPrivate)
{ int		length;
  RmAttributes	*attrib;
  
  if (attr eq Null(char)) return(RmE_BadArgument);
  if ((length = strlen(attr)) < 1) return(RmE_BadArgument);
  
  if (bPrivate)
   attrib = (RmAttributes *) &(obj->PAttribSize);
  else
   attrib = (RmAttributes *) &(obj->AttribSize);

  if (attrib->Size eq 0)
   { attrib->Data	= (char *) Malloc(32);
     if (attrib->Data eq Null(BYTE))
      return(RmE_NoMemory);
     attrib->Size	= 32; 
     attrib->Free = 0;
   }

   	/* Re-alloc the current data structure if necessary */
  while ((attrib->Size - attrib->Free) < (length + 2))
   { char *newdata = (char *)Malloc(2 * (word)(attrib->Size));
     if (newdata eq Null(char))
      return(RmE_NoMemory);

     if (attrib->Free > 0)
      memcpy((void *) newdata, attrib->Data, attrib->Free); 
     attrib->Size = 2 * attrib->Size;
     if (Free(attrib->Data) ne Err_Null) return(RmE_Corruption);
     attrib->Data = newdata;
   }

  	/* Fill in the data structure */
  { char *datavec = attrib->Data;
    datavec[attrib->Free] = length + 2;
    strcpy(&(datavec[attrib->Free+1]), attr);
    attrib->Free += (length + 2);
  }
  
  return(RmE_Success);
}

int RmAddProcessorAttribute(RmProcessor processor, char *attr)
{ int x;

  CheckProcessor(processor);

  if ((x = RmAddObjectAttribute((RmObject) processor, attr, FALSE)) ne RmE_Success)
   RmErrno = processor->Errno = x;
  return(x);
}

int RmAddTaskAttribute(RmTask task, char *attr)
{ int x;

  CheckTask(task);

  if ((x = RmAddObjectAttribute((RmObject) task, attr, FALSE)) ne RmE_Success)
   RmErrno = task->Errno = x;
  return(x);  
}

/**
*** This routine is useful to locate an attribute. It returns an index
*** into the data vector of the RmAttributes structure, or -1 for failure.
**/
static int RmFindObjectAttribute(RmAttributes *attrib, char *attr)
{ int index = 0;
  char	*datavec = attrib->Data;
  
  while (index < attrib->Free)
   { if (!strcmp(&(datavec[index+1]), attr))
      return(index);
     index += datavec[index];
   }
  return(-1);
}

/**
*** Removing an attribute. Again the arguments have to be valid. If the
*** attribute exists it is removed from the table, which is readjusted.
**/
int RmRemoveObjectAttribute(RmObject object, char *attr, bool bPrivate)
{ RmAttributes *attrib;
  int		index;
  char		*datavec;
  int		length;
    
  if (attr eq Null(char)) return(RmE_BadArgument);
  if (strlen(attr) < 1) return(RmE_BadArgument);
  if (bPrivate)
   attrib = (RmAttributes *) &(object->PAttribSize);
  else
   attrib = (RmAttributes *) &(object->AttribSize);

  index = RmFindObjectAttribute(attrib, attr);

  if (index eq -1) return(RmE_NotFound);
  datavec = attrib->Data;
  length = datavec[index];
  if (index + length < attrib->Free)
   memcpy((void *) &(datavec[index]), (void *) &(datavec[index+length]), 
          attrib->Free - (index + length));
  attrib->Free -= length;
  if (attrib->Free eq 0)
   { char 	*temp = attrib->Data;
     attrib->Size = 0;
     attrib->Data = Null(char);
     if (Free(temp) ne Err_Null)
      return(RmE_Corruption);
   }
  return(RmE_Success);
}

int RmRemoveProcessorAttribute(RmProcessor processor, char *attr)
{ int x;

  CheckProcessor(processor);

  if ((x = RmRemoveObjectAttribute((RmObject) processor, attr, FALSE)) ne RmE_Success)
   RmErrno = processor->Errno = x;
  return(x);
}

int RmRemoveTaskAttribute(RmTask task, char *attr)
{ int x;

  CheckTask(task);

  if ((x = RmRemoveObjectAttribute((RmObject) task, attr, FALSE)) ne RmE_Success)
   RmErrno = task->Errno = x;
  return(x);
}

/**
*** Internal attributes can be of type xx=yy. This routine will return the yy
*** part.
**/
char *RmGetObjectAttribute(RmObject obj, char *attr, bool bPrivate)
{ RmAttributes	*attrib;
  int		length	= strlen(attr);
  int		index = 0;
  char		*datavec;

  if (bPrivate)
   attrib = (RmAttributes *) &(obj->PAttribSize);
  else
   attrib = (RmAttributes *) &(obj->AttribSize);  
  datavec = attrib->Data;
  while (index < attrib->Free)
   { 
     if (!strncmp(&(datavec[index+1]), attr, length))
      { if (datavec[index+1+length] eq '=')
         return(&(datavec[index+1+length+1]));
      }
     index += datavec[index];
   }
  return(Null(char));
}

const char *RmGetProcessorAttribute(RmProcessor processor, char *text)
{ CheckProcessorFail(processor, (const char *) NULL);
  if (text eq Null(char))
   return(RmErrno = RmE_BadArgument, (const char *) NULL);
   
  return((const char *)RmGetObjectAttribute((RmObject) processor, text, FALSE));
}

const char *RmGetTaskAttribute(RmTask task, char *text)
{ CheckTaskFail(task, (const char *) NULL);
  if (text eq Null(char))
   return(RmErrno = RmE_BadArgument, (const char *) NULL);
   
  return((const char *) RmGetObjectAttribute((RmObject) task, text, FALSE));
}

static int RmIsAnObjectAttribute(RmObject obj, char *attr, bool bPrivate)
{ RmAttributes 	*attrib;

  if (attr eq Null(char)) return(RmE_BadArgument);
  if (strlen(attr) < 1) return(RmE_BadArgument);
  if (bPrivate)
   attrib = (RmAttributes *) &(obj->PAttribSize);
  else
   attrib = (RmAttributes *) &(obj->AttribSize);
  if (RmFindObjectAttribute(attrib, attr) ne -1)
   return(RmE_Success);
  else
   return(RmE_NotFound);
}

int RmTestProcessorAttribute(RmProcessor processor, char *attr)
{ int x;

  CheckProcessor(processor);

  if ((x = RmIsAnObjectAttribute((RmObject) processor, attr, FALSE)) ne RmE_Success)
   RmErrno = processor->Errno = x;
  return(x);
}

int RmTestTaskAttribute(RmTask task, char *attr)
{ int x;

  CheckTask(task);

  if ((x = RmIsAnObjectAttribute((RmObject) task, attr, FALSE)) ne RmE_Success)
   RmErrno = task->Errno = x;
  return(x);
}

/**
*** Counting the number of attributes is easy, just loop until the Free
*** index is found.
**/
static int RmCountObjectAttributes(RmObject obj, bool bPrivate)
{ int		 result = 0;
  RmAttributes	*attrib;
  int		 index;
  char		*datavec;

  if (bPrivate)
   attrib = (RmAttributes *) &(obj->PAttribSize);
  else
   attrib = (RmAttributes *) &(obj->AttribSize);

  index = 0;
  datavec = attrib->Data;
  while (index < attrib->Free)
   { result++;
     index += datavec[index];
   }
  return(result);
}

int RmCountProcessorAttributes(RmProcessor processor)
{ CheckProcessorFail(processor, -1);
  return(RmCountObjectAttributes((RmObject) processor, FALSE));
}

int RmCountTaskAttributes(RmTask task)
{ CheckTaskFail(task, -1);
  return(RmCountObjectAttributes((RmObject) task, FALSE));
}

/**
*** Listing a set of attributes is similarly easy, assuming the user is
*** sensible enough to provide an adequate table.
**/

static int RmListObjectAttributes(RmObject obj, char **table, bool bPrivate)
{ RmAttributes	*attrib;
  int		i = 0;
  int		index;
  char		*datavec;
  
  if (table eq Null(char *)) return(RmE_BadArgument);
  if (bPrivate)
   attrib = (RmAttributes *) &(obj->PAttribSize);
  else
   attrib = (RmAttributes *) &(obj->AttribSize);

  index = 0;
  datavec = attrib->Data;
  while (index < attrib->Free)
   { table[i++] = &(datavec[index + 1]);
     index += datavec[index];
   }
  return(RmE_Success);
}
     
int RmListProcessorAttributes(RmProcessor processor, char **table)
{ int x;

  CheckProcessor(processor);

  if ((x = RmListObjectAttributes((RmObject) processor, table, FALSE)) ne RmE_Success)
    RmErrno = processor->Errno = x;
  return(x);
}

int RmListTaskAttributes(RmTask task, char **table)
{ int x;

  CheckTask(task);

  if ((x = RmListObjectAttributes((RmObject) task, table, FALSE)) ne RmE_Success)
   RmErrno = task->Errno = x;
  return(x);
}

/**
*** Getting the current processor nucleus is always legal. The most likely
*** result is an empty string, indicating that the default nucleus should be
*** used. Setting the nucleus only makes sense when generating resource maps.
**/
const char *RmGetProcessorNucleus(RmProcessor processor)
{ char	*result;

  CheckProcessorFail(processor, (const char *) NULL);

  result = RmGetObjectAttribute((RmObject) processor, "nucleus", TRUE);
  if (result  eq Null(char))
   return("");
  else
   return(result);
}

/**
*** Setting the processor nucleus involves adding a private attribute,
*** of the form nucleus=/helios/lib/nucleus.dbg. If the nucleus passed
*** is null then the application is simply trying to eliminate the
*** existing nucleus, rather than install one. Either way the routine
*** constructs a suitable attribute string, eliminates the existing
*** attribute if possible, and installs the new string. 
**/
int RmSetProcessorNucleus(RmProcessor processor, char *nucleus)
{ char	*buffer = NULL;
  int	length;
  char	*temp;
  int	rc;

  CheckProcessor(processor);      

	/* Construct an attribute string for the new nucleus */
  if (nucleus ne Null(char))
   { length = strlen(nucleus) + 9;	/* nucleus=xxx\0 */
     buffer = (char *)Malloc(length);
     if (buffer eq Null(BYTE)) return(RmE_NoMemory);
     strcpy(buffer, "nucleus=");
     strcat(buffer, nucleus);
   }     

	/* If a nucleus is already defined, get rid of it */
  temp = RmGetObjectAttribute((RmObject) processor, "nucleus", TRUE);
  if (temp ne Null(char))
   { temp = &(temp[-8]);
     if ((rc = RmRemoveObjectAttribute((RmObject) processor, temp, TRUE)) ne RmE_Success)
      return(RmErrno = processor->Errno = RmE_Corruption);
   }

  if (nucleus ne Null(char))
   { rc = RmAddObjectAttribute((RmObject) processor, buffer, TRUE);
     Free(buffer);
     if (rc ne RmE_Success)
      return(RmErrno = processor->Errno = rc);
   }
  return(RmE_Success);
}

/*}}}*/
/*{{{  Task manipulation */

/**
*** RmNewTask() creates a new task structure and fills it in with default
*** values.
**/

RmTask RmNewTask(void)
{ RmTask result = (RmTask) Malloc(sizeof(RmTaskStruct));
  Date	 date	= GetDate();

  if (result eq (RmTask) NULL)
   return(RmErrno = RmE_NoMemory, (RmTask) NULL);

  memset(result, 0, sizeof(RmTaskStruct));
  result->ObjNode.Type		= Type_Task;
  result->ObjNode.Matrix	= 0x010101c3;	/* darw:r:r:r */
  InitSemaphore(&(result->ObjNode.Lock), 1);	/* As per InitNode() */
  result->ObjNode.Key		= date + _cputime();
  result->ObjNode.Dates.Creation =
  result->ObjNode.Dates.Access   =
  result->ObjNode.Dates.Modified = date;
  result->Type			= RmT_Default;
  result->StructType		= RmL_New;
  result->IsNative		= FALSE;
  result->NextArgIndex		= 1;
  result->ReturnCode		= -1;
  return(result);
}

/**
*** Free'ing a task is essentially the same as freeing a processor.
**/
int RmFreeTask(RmTask task)
{
  CheckTask(task);	

  if (task->ObjNode.Parent ne Null(struct DirNode))
   return(RmErrno = task->Errno = RmE_InUse);

  if (task->MappedTo ne RmL_NoUid)
   { Remove(&(task->MappedNode));
     task->MappedTo = RmL_NoUid;
   }

  if (task->OtherChannels ne Null(RmChannel))
   if (Free(task->OtherChannels) ne Err_Null)
    return(RmErrno = task->Errno = RmE_Corruption);

  if (task->AttribData ne Null(char))
   if (Free(task->AttribData) ne Err_Null)
    return(RmErrno = task->Errno = RmE_Corruption);

  if (task->PAttribData ne Null(char))
   if (Free(task->PAttribData) ne Err_Null)
    return(RmErrno = task->Errno = RmE_Corruption);

  if (task->ArgIndex ne Null(int))
   if (Free(task->ArgIndex) ne Err_Null)
    return(RmErrno = task->Errno = RmE_Corruption);

  if (task->ArgStrings ne Null(char))
   if (Free(task->ArgStrings) ne Err_Null)
    return(RmErrno = task->Errno = RmE_Corruption);
    
	/* This case is true iff the task is being freed as a	*/
	/* result of an RmFreeTaskforce call.			*/
  if (task->Root ne (RmTaskforce) NULL)
   unless(RmReleaseUid((RmObject)task))
    return(RmErrno = task->Errno = RmE_Corruption);

  task->ObjNode.Type	= 0;
  if (Free(task) ne Err_Null)
   return(RmErrno = RmE_Corruption);
  return(RmE_Success); 
}

unsigned long RmGetTaskMemory(RmTask task)
{ CheckTaskFail(task, 0L);	
  return(task->MemorySize);
}

int RmSetTaskMemory(RmTask task, unsigned long size)
{ CheckTask(task);
  task->MemorySize = size; 
  return(RmE_Success);
}

const char *RmGetTaskId(RmTask task)
{ CheckTaskFail(task, (const char *) NULL);
  return(task->ObjNode.Name);
}

int RmSetTaskId(RmTask task, char *name)
{ CheckTask(task);
	/* It is illegal to rename an existing task */
  if (task->StructType ne RmL_New)
   return(RmErrno = task->Errno = RmE_ReadOnly);
  if (strlen(name) >= NameMax)
   return(RmErrno = task->Errno = RmE_TooLong);
  strcpy(task->ObjNode.Name, name);
  return(RmE_Success);
}

int RmGetTaskType(RmTask task)
{ CheckTaskFail(task, -1); 
  return(task->Type);	/* processor type, e.g. T800 */
}

int RmSetTaskType(RmTask task, int type)
{ CheckTask(task);
  if ((type < RmT_Unknown) || (type >= RmT_Known))
   return(RmErrno = task->Errno = RmE_BadArgument);
  task->Type = type;
  return(RmE_Success);
}

int RmGetTaskState(RmTask task)
{ CheckTaskFail(task, -1);
  return((int)task->ObjNode.Size);
}

int RmSetTaskState(RmTask task, int state)
{ CheckTask(task);
  task->ObjNode.Size	= state;
  return(RmE_Success);
}

const char *RmGetTaskCode(RmTask task)
{ char	*result;

  CheckTaskFail(task, (const char *) NULL);
  result = RmGetObjectAttribute((RmObject) task, "code", TRUE);
  if (result eq Null(char))
   return("");
  else
   return(result);
}

int		RmSetTaskCode(RmTask task, char *file)
{ char *buffer;
  int	length;
  int	rc;

  CheckTask(task);     

  if (file eq Null(char))
   return(RmErrno = task->Errno = RmE_BadArgument);

  length = 5 + strlen(file) + 1;	/* code=xxx\0 */
  buffer = (char *) Malloc(length);
  if (buffer eq Null(char))
   return(RmErrno = task->Errno = RmE_NoMemory);
  strcpy(buffer, "code=");
  strcat(buffer, file);
  { char *temp = RmGetObjectAttribute((RmObject) task, "code", TRUE);
     if (temp ne Null(char))
      { temp -= 5;	/* get back to the start of the string, code=xx */
        rc = RmRemoveObjectAttribute((RmObject) task, temp, TRUE);
        if (rc ne RmE_Success)
         return(RmErrno = task->Errno = rc);
      }
  }

  rc = RmAddObjectAttribute((RmObject) task, buffer, TRUE);
  if (Free(buffer) ne Err_Null)
   if (rc eq RmE_Success) rc = RmE_Corruption;

  if (rc ne RmE_Success)
   RmErrno = task->Errno = rc;
      
  return(rc);
}

int RmSetTaskPrivate(RmTask task, int x)
{ CheckTask(task);
  task->Private = x;
  return(RmE_Success);
}

int RmSetTaskPrivate2(RmTask task, int x)
{ CheckTask(task); 
  task->Private2 = x;
  return(RmE_Success);
}

int RmGetTaskPrivate(RmTask task)
{ CheckTaskFail(task, -1);
  return(task->Private);
}

int RmGetTaskPrivate2(RmTask task)
{ CheckTaskFail(task, -1); 
  return(task->Private2);
}

int RmGetTaskError(RmTask task)
{ CheckTask(task);
  return(task->Errno);
}

int RmClearTaskError(RmTask task)
{ CheckTask(task);
  return(task->Errno = RmE_Success);
}

RmUid RmGetTaskUid(RmTask task)
{ CheckTaskFail(task, 0);
  return(task->Uid);
}

/**
BLV These should be replaced by RmSetTaskPurpose() etc., and should
BLV include things like broadcast tasks etc.
**/
int RmSetTaskNative(RmTask task)
{ CheckTask(task);
  task->IsNative = TRUE;
  return(RmE_Success);
}

bool RmIsTaskNative(RmTask task)
{ CheckTaskFail(task, FALSE);	
  return(task->IsNative);
}

bool RmIsTaskNormal(RmTask task)
{ CheckTaskFail(task, FALSE);
  return(!task->IsNative);
}

int RmSetTaskNormal(RmTask task)
{ CheckTask(task); 
  task->IsNative = FALSE;
  return(RmE_Success);
}

/**
*** Argument handling. There are two tables pointed at by the RmTask
*** structure. First there is a big buffer to hold all the strings
*** consecutively. Second there is a table of indexes for this buffer.
***
*** NOTE : valid argument numbers for these routines start at 1.
*** argv[0] is always reserved for the component name.
**/

static int	RmAddTaskArgumentAux1(RmTask task, int number, char *str);

int RmAddTaskArgument(RmTask task, int number, char *str)
{
  CheckTask(task);

  if ((number < 1) || (number > task->NextArgIndex) || (str eq Null(char)))
   return(RmErrno = task->Errno = RmE_BadArgument);
  
	/* Changing an existing argument is difficult and is handled by */
	/* a separate routine.						*/
  if (number ne task->NextArgIndex)
   return(RmAddTaskArgumentAux1(task, number, str));

	/* OK, the simple case of simply setting the next argument.	*/
	/* First time around the buffers have to be allocated.		*/
  if (task->ArgIndex eq Null(int))
   { task->ArgIndex	= (int *) Malloc(8 * sizeof(int));
     if (task->ArgIndex eq Null(int))
      return(RmErrno = task->Errno = RmE_NoMemory);
     task->MaxArgIndex	= 8;
     task->ArgStrings	= (char *) Malloc(64);
     if (task->ArgStrings eq Null(char))
      return(RmErrno = task->Errno = RmE_NoMemory);
     task->MaxArgStrings= 64;
   }

	/* On occasion one of the two tables may be too small.		*/
  if (number > task->MaxArgIndex)
   { int *temp = (int *) Malloc(2 * (word)(task->MaxArgIndex) * sizeof(int));
     word rc;
     if (temp eq Null(int)) 
      return(RmErrno = task->Errno = RmE_NoMemory);
     memcpy(temp, task->ArgIndex, task->MaxArgIndex * sizeof(int));
     rc = Free(task->ArgIndex);
     task->ArgIndex	= temp;
     task->MaxArgIndex	= 2 * task->MaxArgIndex;
     if (rc ne Err_Null)
      return(RmErrno = task->Errno = RmE_Corruption);
   }

  while ((task->NextArgStrings + strlen(str) + 1) > task->MaxArgStrings)
   { char	*temp = (char *) Malloc(2L * (word)(task->MaxArgStrings));
     word	 rc;
     if (temp eq Null(char))
      return(RmErrno = task->Errno = RmE_NoMemory);
     if (task->NextArgStrings > 0)
      memcpy(temp, task->ArgStrings, task->NextArgStrings);
     rc = Free(task->ArgStrings);
     task->ArgStrings		= temp;
     task->MaxArgStrings	= 2 * task->MaxArgStrings;
     if (rc ne Err_Null)
      return(RmErrno = task->Errno = RmE_Corruption);
   }

	/* Everything seems to be OK. Put the string into the table */
  task->ArgIndex[number - 1] 	 = task->NextArgStrings;
  task->NextArgIndex		+= 1;
  { char *temp = &(task->ArgStrings[task->NextArgStrings]);
    strcpy(temp, str);
    task->NextArgStrings	+= (strlen(temp) + 1);
  }
 return(RmE_Success);
}

static int RmAddTaskArgumentAux1(RmTask task, int number, char *str)
{	/* This routine has the unfortunate chore of rearranging the	*/
	/* existing arguments to change one of them.			*/
  char	*old_buffer		= task->ArgStrings;
  int	*arg_index		= task->ArgIndex;
  char	*new_buffer;
  int	buffer_size		= task->MaxArgStrings;
  int	new_index, old_index, i, length;
    
	/* Allocate a new buffer to do the copying. This may or may not	*/
	/* have to be bigger than the current buffer.			*/
  while ((task->NextArgStrings + strlen(str) + 1)  > buffer_size)
   buffer_size += buffer_size;
  new_buffer	= (char *) Malloc(buffer_size);
  if (new_buffer eq Null(char))
   return(RmErrno = task->Errno = RmE_NoMemory);

	/* Put the current strings into the buffer, up to the new one	*/
	/* The ArgIndex table remains unchanged.			*/
  new_index = 0; old_index = 0;
  for (i = 1; i < number; i++)
   { strcpy(&(new_buffer[new_index]), &(old_buffer[new_index]));
     length	 = strlen(&(new_buffer[new_index])) + 1;
     new_index	+= length;
     old_index  += length;
   }
	/* Substitute in the new string. */
  strcpy(&(new_buffer[new_index]), str);
  new_index	+= (strlen(str) + 1);
  old_index	+= (strlen(&(old_buffer[old_index])) + 1);

	/* And copy the remaining arguments, this time updating the	*/
	/* ArgIndex table.						*/
  for (i = number + 1; i < task->NextArgIndex; i++)
   { arg_index[i - 1]	= new_index;
     strcpy(&(new_buffer[new_index]), &(old_buffer[old_index]));
     length	= strlen(&(new_buffer[new_index])) + 1;
     new_index += length;
     old_index += length;
   }

	/* Free the old buffer, and update the fields in the task structure */
  Free(old_buffer);
  task->ArgStrings	= new_buffer;
  task->MaxArgStrings	= buffer_size;
  task->NextArgStrings	= new_index;
  return(RmE_Success);
}

const char *RmGetTaskArgument(RmTask task, int argc)
{
  CheckTaskFail(task, (const char *) NULL);

  if (argc eq 0) return(RmGetTaskId(task));
  if ((argc < 1) || (argc >= task->NextArgIndex))
   return(RmErrno = RmE_BadArgument, Null(char));
  return(&(task->ArgStrings[task->ArgIndex[argc-1]]));
}

int RmCountTaskArguments(RmTask task)
{ CheckTaskFail(task, -1);
  return(task->NextArgIndex);
}

/**
*** There is no processor equivalent for this routine. Its purpose is to
*** examine a binary file and figure out what type of processor it is
*** supposed to run on. Under Helios this information should be stored at
*** the start of a file. Again the type can be a complex object rather
*** than just a simple number.
**/
int RmGetProgramType(char *file)
{ file = file;
  return(RmT_Default);
}

/*}}}*/
/*{{{  Network manipulation */
/**-----------------------------------------------------------------------------
*** Here are the basic operations for a network, at least the ones that
*** do not change the size of the network. Things like AddTailProcessor()
*** require more work than you might expect, because of the problems of
*** maintaining uid's and adding subnets and so on.
***
*** RmNewNetwork() allocates space for a network structure and fills in
*** the usual defaults. For now this is the root network of itself, so the
*** root is filled in to point to itself.
**/
RmNetwork RmNewNetwork(void)
{ RmNetwork	result = (RmNetwork) Malloc(sizeof(RmNetworkStruct));
  Date		date = GetDate();
  
  if (result eq (RmNetwork) NULL)
   return(RmErrno = RmE_NoMemory, (RmNetwork) NULL);

  memset(result, 0, sizeof(RmNetworkStruct));
  result->DirNode.Type		= Type_Network; 
  result->DirNode.Matrix	= 0x211109c7;  /* darwv:rx:ry:rz */
  InitSemaphore(&(result->DirNode.Lock), 1);
  result->DirNode.Key		= date + _cputime();
  result->DirNode.Dates.Creation =
  result->DirNode.Dates.Access	 =
  result->DirNode.Dates.Modified = date;
  InitList(&(result->DirNode.Entries));
  result->Root			= (RmSet) result;
  result->StructType		= RmL_New;
  InitList(&(result->Hardware));
  return(result);
}

/**
*** Free'ing a network means removing and freeing all subnets and processors.
*** Any mapped tasks are unmapped. The reset and configuration facilities are
*** freed, as are the Uid tables if this is the root.
**/
static int FreeNetwork_aux(RmProcessor processor, ...)
{
  (void) Remove((Node *) processor);
  processor->ObjNode.Parent = Null(struct DirNode);
  if (RmIsNetwork(processor))
   return(RmFreeNetwork((RmNetwork) processor));
  else
   return(RmFreeProcessor(processor));
} 

static int FreeHardware(RmHardwareFacility *reset, ...)
{ (void) Remove(&(reset->Node));
  if (Free(reset) ne Err_Null)
   return(RmErrno = RmE_Corruption);
  else
   return(RmE_Success);
}

int RmFreeNetwork(RmNetwork network)
{ int result, i;

  CheckNetwork(network); 

  if (network->DirNode.Parent ne Null(struct DirNode))
   return(RmErrno = network->Errno = RmE_InUse);

  if (network->DirNode.Nentries ne 0)
   { result = RmApplyNetwork(network, &FreeNetwork_aux);  
     if (result ne RmE_Success)
      return(RmErrno = network->Errno = result);
   }

  if ((result = RmApplyHardwareFacilities(network, &FreeHardware))
       ne RmE_Success)
   return(RmErrno = network->Errno = result);

  if (network->NoTables > 0)
   { for (i = 0; i < network->NoTables; i++)
      if (Free(network->Tables[i]) ne Err_Null)
       return(RmErrno = network->Errno = RmE_Corruption);
     if (Free(network->Tables) ne Err_Null)
       return(RmErrno = network->Errno = RmE_Corruption);
   }
   
  network->DirNode.Type = 0;
  if (Free(network) ne Err_Null)
   return(RmErrno = RmE_Corruption);
   
  return(RmE_Success);
}

const char *RmGetNetworkId(RmNetwork network)
{ CheckNetworkFail(network, (const char *) NULL);
  return(network->DirNode.Name);
}

int RmSetNetworkId(RmNetwork network, char *name)
{ CheckNetwork(network);
  if (network->StructType ne RmL_New)
   return(RmErrno = network->Errno = RmE_ReadOnly);
  if (strlen(name) >= NameMax) 
   return(RmErrno = network->Errno = RmE_TooLong);
  strcpy(network->DirNode.Name, name);
  return(RmE_Success);
}

int RmSetNetworkPrivate(RmNetwork network, int x)
{ CheckNetwork(network);
  network->Private = x;
  return(RmE_Success);
}

int RmSetNetworkPrivate2(RmNetwork network, int x)
{ CheckNetwork(network);	
  network->Private2 = x;
  return(RmE_Success);
}

int RmGetNetworkPrivate(RmNetwork network)
{ CheckNetworkFail(network, -1);	
  return(network->Private);
}

int RmGetNetworkPrivate2(RmNetwork network)
{ CheckNetworkFail(network, -1);	
  return(network->Private2);
}

int RmGetNetworkError(RmNetwork network)
{ CheckNetwork(network);
  return(network->Errno);
}

int RmClearNetworkError(RmNetwork network)
{ CheckNetwork(network);
  return(network->Errno = RmE_Success);
}

/**
*** The first and last processors are easy to get hold of, because there
*** are pointers in the list structure.
**/
RmProcessor RmFirstProcessor(RmNetwork network)
{ CheckNetworkFail(network, (RmProcessor) NULL);
  if (network->DirNode.Nentries < 1)
   return((RmProcessor) NULL);
  return((RmProcessor) network->DirNode.Entries.Head);
}

RmProcessor RmLastProcessor(RmNetwork network)
{ CheckNetworkFail(network, (RmProcessor) NULL); 
  if (network->DirNode.Nentries < 1) return((RmProcessor) NULL);
  return((RmProcessor) network->DirNode.Entries.Tail);
}

/**
*** The next and previous routines are only valid if the processor is
*** currently in a network. N.B. do not validate Type_Processor for these,
*** since they may be subnets.
**/
RmProcessor RmNextProcessor(RmProcessor processor)
{ 
  if (processor eq (RmProcessor) NULL)
   return(RmErrno = RmE_NotProcessor, (RmProcessor) NULL);
  if ((processor->ObjNode.Type ne Type_Processor) &&
      (processor->ObjNode.Type ne Type_Network))
   return(RmErrno = RmE_NotProcessor, (RmProcessor) NULL);

  if (processor->ObjNode.Parent eq Null(struct DirNode))
   return(RmErrno = processor->Errno = RmE_NotNetwork, (RmProcessor) NULL);
  processor = (RmProcessor) processor->ObjNode.Node.Next;
  if (processor->ObjNode.Node.Next eq Null(struct Node))
   return((RmProcessor) NULL);
  else
   return(processor);
}

RmProcessor RmPreviousProcessor(RmProcessor processor)
{ RmProcessor	result;
  RmNetwork	parent;

  if (processor eq (RmProcessor) NULL)
   return(RmErrno = RmE_NotProcessor, (RmProcessor) NULL);
  if ((processor->ObjNode.Type ne Type_Processor) &&
      (processor->ObjNode.Type ne Type_Network))
   return(RmErrno = RmE_NotProcessor, (RmProcessor) NULL);

  parent = (RmNetwork) processor->ObjNode.Parent;
  if (parent eq (RmNetwork) NULL)
   return(RmErrno = processor->Errno = RmE_NotNetwork, (RmProcessor) NULL); 
  result = (RmProcessor) processor->ObjNode.Node.Prev;
  if (result eq (RmProcessor) &(parent->DirNode.Entries.Head))
   return((RmProcessor) NULL);
  return(result);
}

/**
*** An empty network has 0 Nentries. The size of a network is Nentries,
*** not the number of processors in all the subnets. The parent network
*** is null if this is the root, otherwise it is held in the DirNode.
**/

bool RmIsNetworkEmpty(RmNetwork network)
{ CheckNetworkFail(network, TRUE);
  return(network->DirNode.Nentries eq 0);
}

int RmSizeofNetwork(RmNetwork network)
{ CheckNetworkFail(network, -1);
  return((int)network->DirNode.Nentries);
}

int RmCountProcessors(RmNetwork network)
{ int		result;
  RmProcessor	current;

  CheckNetworkFail(network, -1);  

  if (network->NoSubnets eq 0) return((int)network->DirNode.Nentries);
  result = (int)(network->DirNode.Nentries - network->NoSubnets);
  for (current = RmFirstProcessor(network);
       current ne (RmProcessor) NULL;
       current = RmNextProcessor(current))
   if (current->ObjNode.Type eq Type_Network)
    result += RmCountProcessors((RmNetwork) current);
  return(result);
}

/**
*** Do not validate the type, it may be a subnet. Any object in a network
*** has a root, so if the root is NULL then the object is not in a network.
*** It is necessary to allow for the special case of the root network,
*** which has a pointer to itself but is not its own parent.
**/
RmNetwork RmParentNetwork(RmProcessor processor)
{ if (processor eq (RmProcessor) NULL)
   return(RmErrno = RmE_NotProcessor, (RmNetwork) NULL);
  if (processor->Root eq (RmNetwork) NULL) 
   return(RmErrno = processor->Errno = RmE_NotNetwork, (RmNetwork) NULL);
  if (processor->Root eq (RmNetwork) processor) return((RmNetwork) NULL);
  return((RmNetwork) processor->ObjNode.Parent);
}

int RmApplyNetwork(RmNetwork network, int (*fn)(RmProcessor, ...), ...)
{ va_list	args;
  int		arg1, arg2, arg3;
  int		result = 0;
  RmProcessor	processor, next;
  
  va_start(args, fn);
  arg1 = va_arg(args, int);
  arg2 = va_arg(args, int);
  arg3 = va_arg(args, int);
  va_end(args);

  CheckNetworkFail(network, -1);  

  processor	= (RmProcessor) network->DirNode.Entries.Head;
  next		= (RmProcessor) processor->ObjNode.Node.Next;
  while (next ne (RmProcessor) NULL)
   { result	+= (*fn)(processor, arg1, arg2, arg3);
     processor	 = next;
     next	 = (RmProcessor) processor->ObjNode.Node.Next;
   }
  return(result);
}

int RmSearchNetwork(RmNetwork network, int (*fn)(RmProcessor, ...), ...)
{ va_list	args;
  int		arg1, arg2, arg3;
  int		result = 0;
  RmProcessor	processor, next;
  
  va_start(args, fn);
  arg1 = va_arg(args, int);
  arg2 = va_arg(args, int);
  arg3 = va_arg(args, int);
  va_end(args);

  CheckNetworkFail(network, -1);  

  processor	= (RmProcessor) network->DirNode.Entries.Head;
  next		= (RmProcessor) processor->ObjNode.Node.Next;
  while ((next ne (RmProcessor) NULL) && (result eq 0))
   { result	+= (*fn)(processor, arg1, arg2, arg3);
     processor	 = next;
     next	 = (RmProcessor) processor->ObjNode.Node.Next;
   }
  return(result);
}


int RmApplyProcessors(RmNetwork network, int (*fn)(RmProcessor, ...), ...)
{ va_list	args;
  int		arg1, arg2, arg3;
  int		result = 0;
  RmProcessor	processor, next;
  
  va_start(args, fn);
  arg1 = va_arg(args, int);
  arg2 = va_arg(args, int);
  arg3 = va_arg(args, int);
  va_end(args);

  CheckNetworkFail(network, -1);  

  processor	= (RmProcessor) network->DirNode.Entries.Head;
  next		= (RmProcessor) processor->ObjNode.Node.Next;
  while (next ne (RmProcessor) NULL)
   { if (processor->ObjNode.Type eq Type_Network)
      result += RmApplyProcessors((RmNetwork) processor, fn, arg1, arg2, arg3);
     else   
      result	+= (*fn)(processor, arg1, arg2, arg3);
     processor	 = next;
     next	 = (RmProcessor) processor->ObjNode.Node.Next;
   }
  return(result);
}

int RmSearchProcessors(RmNetwork network, int (*fn)(RmProcessor, ...), ...)
{ va_list	args;
  int		arg1, arg2, arg3;
  int		result = 0;
  RmProcessor	processor, next;
  
  va_start(args, fn);
  arg1 = va_arg(args, int);
  arg2 = va_arg(args, int);
  arg3 = va_arg(args, int);
  va_end(args);

  CheckNetworkFail(network, -1);  

  processor	= (RmProcessor) network->DirNode.Entries.Head;
  next		= (RmProcessor) processor->ObjNode.Node.Next;
  while ((next ne (RmProcessor) NULL) && (result eq 0))
   { if (processor->ObjNode.Type eq Type_Network)
      result += RmSearchProcessors((RmNetwork) processor, fn, arg1, arg2, arg3);
     else
      result	+= (*fn)(processor, arg1, arg2, arg3);
     processor	 = next;
     next	 = (RmProcessor) processor->ObjNode.Node.Next;
   }
  return(result);
}

/**
*** Do not validate the type, it may be a subnet. Note that the root
*** network is its own root.
**/
RmNetwork RmRootNetwork(RmProcessor processor)
{ if (processor eq (RmProcessor) NULL)
   return(RmErrno = RmE_NotProcessor, (RmNetwork) NULL);
  return(processor->Root);
}

bool RmIsNetwork(RmProcessor processor)
{ if (processor eq (RmProcessor)NULL)
   return(RmErrno = RmE_NotProcessor, FALSE);
  return(processor->ObjNode.Type eq Type_Network);
}

bool RmIsProcessor(RmProcessor processor)
{ if (processor eq (RmProcessor)NULL) 
   return(RmErrno = RmE_NotProcessor, FALSE);
  return(processor->ObjNode.Type eq Type_Processor);
}

/**
*** RmFindMatchingProcessor(). This serves two purposes:
*** a) when considering two existing networks or subnets, for example the
***    user's domain and the global network, it allows the programmer
***    to switch from one to the other.
*** b) when using a constructed template to obtain a network of processors
***    it allowsa similar mapping.
**/
RmProcessor RmFindMatchingProcessor(RmProcessor processor, RmNetwork network)
{ RmProcessor	result;

  CheckProcessorFail(processor, (RmProcessor) NULL);
  CheckNetworkFail(network, (RmProcessor) NULL);

  if ((processor->StructType eq RmL_Existing) || 
      (processor->StructType eq RmL_Obtained))
   result = RmFindProcessor(network, processor->Uid);
  else
   { if (processor->MappedTo eq 0) 
      return(processor->Errno = RmErrno = RmE_BadProcessor, (RmProcessor) NULL);
     result = RmFindProcessor(network, processor->MappedTo);
   }
  if (result eq RmM_ExternalProcessor) result = NULL;
  if (result eq (RmProcessor) NULL)
   processor->Errno = RmErrno = RmE_NotFound;
  return(result);   
}
/*}}}*/
/*{{{  Hardware facilities */

/**-----------------------------------------------------------------------------
*** Adding a hardware facility. These contain pointers to real processors,
*** not uid's, since applications must be able to examine hardware
*** facilities without knowing about uids. This means that conversions
*** are required when communicating networks. There are also problems if
*** a processor is removed from a network, because a pointer to that
*** processor can remain inside one or more hardware facility structures.
*** I am not too worried about all this since hardware facilities are not
*** really an important part of the library.
**/

int RmAddHardwareFacility(RmNetwork network, RmHardwareFacility *Reset)
{ RmHardwareFacility	*NewReset;

  CheckNetwork(network);  

  if (Reset eq Null(RmHardwareFacility))
   return(RmErrno = network->Errno = RmE_BadArgument);

  NewReset = (RmHardwareFacility *) Malloc(
   sizeof(RmHardwareFacility) + ((word)(Reset->NumberProcessors) * sizeof(RmProcessor *)));
  if (NewReset eq Null(RmHardwareFacility))
   return(RmErrno = network->Errno = RmE_NoMemory);

  memcpy((void *) NewReset, (void *) Reset, sizeof(RmHardwareFacility));
  NewReset->Processors = (RmProcessor *)
                ( &(((BYTE *)NewReset)[sizeof(RmHardwareFacility)]));
  NewReset->Device	= Null(void);
  NewReset->Essential	= (RmProcessor) NULL;
  memcpy((void *) NewReset->Processors, (void *) Reset->Processors,
         Reset->NumberProcessors * sizeof(RmProcessor *));
#ifdef SYSDEB
  NewReset->Node.Next = NewReset->Node.Prev = &NewReset->Node;
#endif
  AddTail(&(network->Hardware), &(NewReset->Node));
  return(RmE_Success);
}

int RmApplyHardwareFacilities(RmNetwork network,
		int (*fn)(RmHardwareFacility *, ...), ...)
{ va_list args;
  RmHardwareFacility *current, *next;
  int	arg1, arg2, arg3;
  int   result = 0;

  CheckNetworkFail(network, -1);  
  
  va_start(args, fn);
  arg1 = va_arg(args, int);
  arg2 = va_arg(args, int);
  arg3 = va_arg(args, int);
  va_end(args);
  
  current = (RmHardwareFacility *) network->Hardware.Head;
  next    = (RmHardwareFacility *) current->Node.Next;
  while (next ne Null(RmHardwareFacility))
   { result += (*fn)(current, arg1, arg2, arg3);
     current = next;
     next    = (RmHardwareFacility *) current->Node.Next;
   }
  return(result);
}

int RmSearchHardwareFacilities(RmNetwork network,
		int (*fn)(RmHardwareFacility *, ...), ...)
{ va_list args;
  RmHardwareFacility *current, *next;
  int	arg1, arg2, arg3;
  int   result = 0;

  CheckNetworkFail(network, -1);  
  
  va_start(args, fn);
  arg1 = va_arg(args, int);
  arg2 = va_arg(args, int);
  arg3 = va_arg(args, int);
  va_end(args);
  
  current = (RmHardwareFacility *) network->Hardware.Head;
  next    = (RmHardwareFacility *) current->Node.Next;
  while (next ne Null(RmHardwareFacility))
   { if ((result = (*fn)(current, arg1, arg2, arg3)) ne 0)
      return(result);
     current = next;
     next    = (RmHardwareFacility *) current->Node.Next;
   }
  return(0);
}

/*}}}*/
/*{{{  Taskforce manipulation */
RmTaskforce RmNewTaskforce(void)
{ RmTaskforce	result = (RmTaskforce) Malloc(sizeof(RmTaskforceStruct));
  Date		date = GetDate();
  
  if (result eq (RmTaskforce) NULL)
   return(RmErrno = RmE_NoMemory, (RmTaskforce) NULL);

  memset(result, 0, sizeof(RmTaskforceStruct));   
  result->DirNode.Type		= Type_Taskforce; 
  result->DirNode.Matrix	= 0x211109c7;	/* darwv:rx:ry:rz */
  InitSemaphore(&(result->DirNode.Lock), 1);
  result->DirNode.Key		= date + _cputime();
  result->DirNode.Parent	= Null(struct DirNode);
  result->DirNode.Dates.Creation =
  result->DirNode.Dates.Access	 =
  result->DirNode.Dates.Modified = date;
  InitList(&(result->DirNode.Entries));
  result->Root			= (RmSet) result;
  result->StructType		= RmL_New;
  result->ReturnCode		= -1;
  return(result);
}

static int FreeTaskforce_aux(RmTask task, ...)
{
  (void) Remove((Node *) task);
  task->ObjNode.Parent = Null(struct DirNode);

  if (RmIsTaskforce(task))
   return(RmFreeTaskforce((RmTaskforce) task));
  else
   return(RmFreeTask(task));
} 

int RmFreeTaskforce(RmTaskforce taskforce)
{ int result, i;

  CheckTaskforce(taskforce); 

  if (taskforce->DirNode.Parent ne Null(struct DirNode))
   return(RmErrno = taskforce->Errno = RmE_InUse);

  if (taskforce->DirNode.Nentries ne 0)
   { result = RmApplyTaskforce(taskforce, &FreeTaskforce_aux);  
     if (result ne RmE_Success)
      return(RmErrno = taskforce->Errno = result);
   }

  if (taskforce->NoTables > 0)
   { for (i = 0; i < taskforce->NoTables; i++)
      if (Free(taskforce->Tables[i]) ne Err_Null)
       return(RmErrno = taskforce->Errno = RmE_Corruption);
     if (Free(taskforce->Tables) ne Err_Null)
       return(RmErrno = taskforce->Errno = RmE_Corruption);
   }

  taskforce->DirNode.Type	= 0;
  Free(taskforce);
  return(RmE_Success);
}

const char *RmGetTaskforceId(RmTaskforce taskforce)
{
  CheckTaskforceFail(taskforce, (const char *) NULL);
  return(taskforce->DirNode.Name);
}

int RmSetTaskforceId(RmTaskforce taskforce, char *name)
{
  CheckTaskforce(taskforce);	
  if (taskforce->StructType ne RmL_New)
   return(RmErrno = taskforce->Errno = RmE_ReadOnly);
  if (strlen(name) >= NameMax)
   return(RmErrno = taskforce->Errno = RmE_TooLong);
  strcpy(taskforce->DirNode.Name, name);
  return(RmE_Success);
}

int RmSetTaskforcePrivate(RmTaskforce taskforce, int x)
{ CheckTaskforce(taskforce);	
  taskforce->Private = x;
  return(RmE_Success);
}

int RmSetTaskforcePrivate2(RmTaskforce taskforce, int x)
{ CheckTaskforce(taskforce);	
  taskforce->Private2 = x;
  return(RmE_Success);
}

int RmGetTaskforcePrivate(RmTaskforce taskforce)
{ CheckTaskforceFail(taskforce, -1);	
  return(taskforce->Private);
}

int RmGetTaskforcePrivate2(RmTaskforce taskforce)
{ CheckTaskforceFail(taskforce, -1);
  return(taskforce->Private2);
}

int RmGetTaskforceError(RmTaskforce taskforce)
{ CheckTaskforce(taskforce);	
  return(taskforce->Errno);
}

int RmClearTaskforceError(RmTaskforce taskforce)
{ CheckTaskforce(taskforce);
  return(taskforce->Errno = RmE_Success);
}

int RmGetTaskforceState(RmTaskforce taskforce)
{ CheckTaskforceFail(taskforce, -1);
  return(taskforce->State);
}

int RmSetTaskforceState(RmTaskforce taskforce, int state)
{ CheckTaskforce(taskforce);
  taskforce->State = state;
  return(RmE_Success);
}


RmTask RmFirstTask(RmTaskforce taskforce)
{ CheckTaskforceFail(taskforce, (RmTask) NULL);
  if (taskforce->DirNode.Nentries < 1)
   return((RmTask) NULL);
  return((RmTask) taskforce->DirNode.Entries.Head);
}

RmTask RmLastTask(RmTaskforce taskforce)
{ CheckTaskforceFail(taskforce, (RmTask) NULL); 
  if (taskforce->DirNode.Nentries < 1)
   return((RmTask) NULL);
  return((RmTask) taskforce->DirNode.Entries.Tail);
}

RmTask RmNextTask(RmTask task)
{ 
  if (task eq (RmTask) NULL)
   return(RmErrno = RmE_NotTask, (RmTask) NULL);
  if ((task->ObjNode.Type ne Type_Task) &&
      (task->ObjNode.Type ne Type_Taskforce))
   return(RmErrno = RmE_NotTask, (RmTask) NULL);

  if (task->ObjNode.Parent eq Null(struct DirNode))
   return(RmErrno = task->Errno = RmE_NotTaskforce, (RmTask) NULL);
  task = (RmTask) task->ObjNode.Node.Next;
  if (task->ObjNode.Node.Next eq Null(struct Node))
   return((RmTask) NULL);
  else
   return(task);
}

RmTask RmPreviousTask(RmTask task)
{ RmTask        result;
  RmTaskforce   parent;

  if (task eq (RmTask) NULL)
   return(RmErrno = RmE_NotTask, (RmTask) NULL);
  if ((task->ObjNode.Type ne Type_Task) &&
      (task->ObjNode.Type ne Type_Taskforce))
   return(RmErrno = RmE_NotTask, (RmTask) NULL);

  parent = (RmTaskforce) task->ObjNode.Parent;
  if (parent eq (RmTaskforce) NULL)
   return(RmErrno = task->Errno = RmE_NotTaskforce, (RmTask) NULL); 
  result = (RmTask) task->ObjNode.Node.Prev;
  if (result eq (RmTask) &(parent->DirNode.Entries.Head))
   return((RmTask) NULL);
  return(result);
}

bool RmIsTaskforceEmpty(RmTaskforce taskforce)
{ CheckTaskforceFail(taskforce, TRUE);
  return(taskforce->DirNode.Nentries eq 0);
}

int RmSizeofTaskforce(RmTaskforce taskforce)
{ CheckTaskforceFail(taskforce, -1);    
  return((int)taskforce->DirNode.Nentries);
}

int RmCountTasks(RmTaskforce taskforce)
{ int           result;
  RmTask        current;

  CheckTaskforceFail(taskforce, -1);  

  if (taskforce->NoSubsets eq 0) return((int)taskforce->DirNode.Nentries);
  result = (int)(taskforce->DirNode.Nentries - taskforce->NoSubsets);
  for (current = RmFirstTask(taskforce);
       current ne (RmTask) NULL;
       current = RmNextTask(current))
   if (current->ObjNode.Type eq Type_Taskforce)
    result += RmCountTasks((RmTaskforce) current);
  return(result);
}

RmTaskforce RmParentTaskforce(RmTask task)
{
  if (task eq (RmTask) NULL)
   return(RmErrno = RmE_NotTask, (RmTaskforce) NULL);
  if (task->Root eq (RmTaskforce) NULL)
   return(RmErrno = RmE_NotTask, (RmTaskforce) NULL);
  if (task->Root eq (RmTaskforce) task)
   return((RmTaskforce) NULL);
  return((RmTaskforce) task->ObjNode.Parent);
}

int RmApplyTaskforce(RmTaskforce taskforce, int (*fn)(RmTask, ...), ...)
{ va_list       args;
  int           arg1, arg2, arg3;
  int           result = 0;
  RmTask        task, next;
  
  va_start(args, fn);
  arg1 = va_arg(args, int);
  arg2 = va_arg(args, int);
  arg3 = va_arg(args, int);
  va_end(args);

  CheckTaskforceFail(taskforce, -1);  

  task          = (RmTask) taskforce->DirNode.Entries.Head;
  next          = (RmTask) task->ObjNode.Node.Next;
  while (next ne (RmTask) NULL)
   { 
     result     += (*fn)(task, arg1, arg2, arg3);
     task        = next;
     next        = (RmTask) task->ObjNode.Node.Next;
   }
  return(result);
}

int RmSearchTaskforce(RmTaskforce taskforce, int (*fn)(RmTask, ...), ...)
{ va_list       args;
  int           arg1, arg2, arg3;
  int           result = 0;
  RmTask        task, next;
  
  va_start(args, fn);
  arg1 = va_arg(args, int);
  arg2 = va_arg(args, int);
  arg3 = va_arg(args, int);
  va_end(args);

  CheckTaskforceFail(taskforce, -1);  
   
  task          = (RmTask) taskforce->DirNode.Entries.Head;
  next          = (RmTask) task->ObjNode.Node.Next;
  while ((next ne (RmTask) NULL) && (result eq 0))
   { result     += (*fn)(task, arg1, arg2, arg3);
     task        = next;
     next        = (RmTask) task->ObjNode.Node.Next;
   }
  return(result);
}

int RmApplyTasks(RmTaskforce taskforce, int (*fn)(RmTask, ...), ...)
{ va_list       args;
  int           arg1, arg2, arg3;
  int           result = 0;
  RmTask        task, next;
  
  va_start(args, fn);
  arg1 = va_arg(args, int);
  arg2 = va_arg(args, int);
  arg3 = va_arg(args, int);
  va_end(args);

  CheckTaskforceFail(taskforce, -1);  

  task          = (RmTask) taskforce->DirNode.Entries.Head;
  next          = (RmTask) task->ObjNode.Node.Next;
  while (next ne (RmTask) NULL)
   { if (task->ObjNode.Type eq Type_Taskforce)
      result += RmApplyTasks((RmTaskforce) task, fn, arg1, arg2, arg3);
     else
      result    += (*fn)(task, arg1, arg2, arg3);
     task        = next;
     next        = (RmTask) task->ObjNode.Node.Next;
   }
  return(result);
}

int RmSearchTasks(RmTaskforce taskforce, int (*fn)(RmTask, ...), ...)
{ va_list       args;
  int           arg1, arg2, arg3;
  int           result = 0;
  RmTask        task, next;
  
  va_start(args, fn);
  arg1 = va_arg(args, int);
  arg2 = va_arg(args, int);
  arg3 = va_arg(args, int);
  va_end(args);

  CheckTaskforceFail(taskforce, -1);  
   
  task          = (RmTask) taskforce->DirNode.Entries.Head;
  next          = (RmTask) task->ObjNode.Node.Next;
  while ((next ne (RmTask) NULL) && (result eq 0))
   { if (task->ObjNode.Type eq Type_Taskforce)
      result    += RmSearchTasks((RmTaskforce) task, fn, arg1, arg2, arg3);
     else
      result    += (*fn)(task, arg1, arg2, arg3);
     task        = next;
     next        = (RmTask) task->ObjNode.Node.Next;
   }
  return(result);
}

RmTaskforce RmRootTaskforce(RmTask task)
{ if (task eq (RmTask) NULL)
   return(RmErrno = RmE_NotTask, (RmTaskforce) NULL);
  return(task->Root);
}

bool RmIsTaskforce(RmTask task)
{ if (task eq (RmTask)NULL)
   return(RmErrno = RmE_NotTask, FALSE);
  return(task->ObjNode.Type eq Type_Taskforce);
}

bool RmIsTask(RmTask task)
{ if (task eq (RmTask)NULL)
   return(RmErrno = RmE_NotTask, FALSE);
  return(task->ObjNode.Type eq Type_Task);
}

RmTask RmFindMatchingTask(RmTask task, RmTaskforce taskforce)
{ RmTask result;

  CheckTaskFail(task, (RmTask) NULL);
  CheckTaskforceFail(taskforce, (RmTask) NULL);

  result = RmFindTask(taskforce, task->Uid);
  if (result eq RmM_ExternalTask) result = NULL;
  if (result eq (RmTask) NULL)
   task->Errno = RmErrno = RmE_NotFound;
  return(result);   
}
/*}}}*/
/*{{{  Task <-> Processor mapping */

int RmMapTask(RmProcessor processor, RmTask task)
{
  CheckProcessor(processor);
  CheckTask(task);
  
  if (task->MappedTo ne RmL_NoUid)
   return(RmErrno = task->Errno = RmE_InUse);
  AddTail(&(processor->MappedTasks), &(task->MappedNode));
  task->MappedTo = processor->Uid;
  return(RmE_Success);
}

static int RmCheckMappedTask(RmProcessor processor, RmTask task)
{ Node  *current;
  Node  *real;

  CheckProcessor(processor);
  CheckTask(task);  

  real = &(task->MappedNode);
  for (current = Head_(Node, processor->MappedTasks);
       !EndOfList_(current);
       current = Next_(Node, current))
   if (current eq real)
    return(RmE_Success);
    
  return(RmErrno = task->Errno = RmE_NotFound);       
}

bool RmIsMappedTask(RmProcessor processor, RmTask task)
{ if (RmCheckMappedTask(processor, task) eq RmE_Success)
   return(TRUE);
  else
   return(FALSE);
}

int RmUnmapTask(RmProcessor processor, RmTask task)
{ int   x = RmCheckMappedTask(processor, task);
  if (x eq RmE_Success)
   { Remove(&(task->MappedNode));
     task->MappedTo = RmL_NoUid;
   }
  return(x);
}

RmProcessor RmFollowTaskMapping(RmNetwork network, RmTask task)
{ 
  CheckNetworkFail(network, (RmProcessor) NULL);
  CheckTaskFail(task, (RmProcessor) NULL);

  if (task->MappedTo eq RmL_NoUid) return((RmProcessor) NULL);
  return(RmFindProcessor(network, task->MappedTo));
}

int RmApplyMappedTasks(RmProcessor processor, int (*fn)(RmTask, ...), ...)
{ Node		*current, *next;
  RmTask         temp;
  va_list        args;
  int            arg1, arg2, arg3;
  int            result = 0; 

  CheckProcessorFail(processor, -1);

  va_start(args, fn);
  arg1 = va_arg(args, int);
  arg2 = va_arg(args, int);
  arg3 = va_arg(args, int);
  va_end(args);
  
  for (current = Head_(Node, processor->MappedTasks);
       !EndOfList_(current);
       current = next )
   { next = Next_(Node, current);
     temp = (RmTask) (((BYTE *) current) - offsetof(RmTaskStruct, MappedNode));
     result += (*fn)(temp, arg1, arg2, arg3);
   }
  return(result);
}

int RmSearchMappedTasks(RmProcessor Processor, int (*fn)(RmTask, ...), ...)
{ Node		*current, *next;
  RmTask         temp;
  va_list        args;
  int            arg1, arg2, arg3;
  int            result = 0; 

  CheckProcessorFail(Processor, -1);

  va_start(args, fn);
  arg1 = va_arg(args, int);
  arg2 = va_arg(args, int);
  arg3 = va_arg(args, int);
  va_end(args);
  
  for (current = Head_(Node, Processor->MappedTasks);
       !EndOfList_(current);
       current = next )
   { next = Next_(Node, current);
     temp = (RmTask) (((BYTE *) current) - offsetof(RmTaskStruct, MappedNode));
     result = (*fn)(temp, arg1, arg2, arg3);
     if (result ne 0) return(result);
   }
  return(result);
}

int RmCountMappedTasks(RmProcessor processor)
{ int   result = 0;
  Node  *temp;

  CheckProcessorFail(processor, -1);  
  
  for (temp = Head_(Node, processor->MappedTasks);
       !EndOfList_(temp);
       temp = Next_(Node, temp))
   result++;
  return(result);
}

/*}}}*/
/*{{{  Uid manipulation */

/**-----------------------------------------------------------------------------
*** Before considering the structure manipulation routines, here are the
*** routines for manipulating Uid's.
***
*** RmFindTableEntry takes as arguments the root of a network or taskforce
*** and a uid, and returns a pointer to the RmUidTableEntry. It validates
*** the table number and the index within the table, but not the cycle
*** number.
**/

RmUidTableEntry *RmFindTableEntry(RmSet root, RmUid uid)
{ int             tabno, tabindex;
  RmUidTableEntry *tab;
  
  if ((root eq (RmSet)NULL) || (uid  eq RmL_NoUid) )
   return(Null(RmUidTableEntry));
   
  tabno    = (uid >> 8) & 0x0000FFFF;
  tabindex = uid & 0x000000FF;
  if ((tabindex >= RmL_SlotsPerTable) ||
      (tabno    >= root->NoTables))
   return(Null(RmUidTableEntry));
  tab = &(((root->Tables)[tabno])[tabindex]);
  return(tab);
}

/**
*** RmFindUid() is similar to RmFindTableEntry(), but returns a pointer
*** to the target object rather the TableEntry. In addition it checks
*** the cycle number.
**/
RmObject RmFindUid(RmSet root, RmUid uid)
{ RmUidTableEntry *tab = RmFindTableEntry(root, uid);

  if (tab eq Null(RmUidTableEntry)) return(RmL_NoObject);
  if (RmGetCycle(uid) ne tab->Cycle)
   return(RmL_ExtObject);
  return((RmObject) tab->Target);
}

/**
*** This routine extends the table of tables by 1, to allow for overflows.
**/  
bool RmExtendFreeQ(RmSet root)
{ RmUidTableEntry *newtab;
  RmUidTableEntry **tabs;

    /* Obtain space for another 64 TableEntry's */
  newtab = (RmUidTableEntry *)
      Malloc(RmL_SlotsPerTable * sizeof(RmUidTableEntry));     
  if (newtab eq Null(RmUidTableEntry))
   return(FALSE);

    /* Realloc the table of tables of TableEntries */
  tabs = (RmUidTableEntry **) 
          Malloc(((word)(root->NoTables) + 1) * sizeof(RmUidTableEntry *));
  if (tabs eq Null(RmUidTableEntry *))
   { Free(newtab);
     return(FALSE);
   }
  if (root->NoTables > 0)
   memcpy(tabs, root->Tables, (size_t)(root->NoTables * sizeof(RmUidTableEntry *)));
  tabs[root->NoTables] = newtab;
  
    /* Set all the new slots to free */
  { int i; 
    for (i = 0; i < RmL_SlotsPerTable; i++)
     { newtab[i].Cycle  = 0;
       newtab[i].Free   = TRUE;
       newtab[i].Target = (void *) RmL_NoObject;
     }
  }
  
  if (root->NoTables eq 0)      /* Very first one */
   { newtab[0].Free     = FALSE;        /* Ensures that Uid of 0 is invalid */
     newtab[0].Target   = (void *) RmM_NoProcessor;
     newtab[1].Free     = FALSE;
     newtab[1].Target   = (void *) RmM_ExternalProcessor;
   }
   
   /* Update the Root structure */     
  if (root->NoTables > 0)
   Free(root->Tables);
  root->Tables  = tabs;
  root->NoTables++;
  return(TRUE);
}

/**
*** The routine picks the next free Uid that is available, and assigns
*** the given object that Uid. This involves filling in the table of
*** Uid's and filling in the Uid field of the object. The routine may
*** recurse once, to allow for having to extend the queue.
**/
bool RmNextFreeUid(RmSet root, RmObject obj)
{ int tabno, slot;

  if ((root eq (RmSet)NULL) || (obj eq (RmObject)NULL))
   return(FALSE);
  if (obj->Uid ne RmL_NoUid) return(FALSE);

  for (tabno = 0; tabno < root->NoTables; tabno++)
   { RmUidTableEntry *table = (root->Tables)[tabno];
     for (slot = 0; slot < RmL_SlotsPerTable; slot++)
      { if (table[slot].Free)
         { table[slot].Free = FALSE;
           obj->Uid = (table[slot].Cycle << 24) + (tabno << 8) + slot;
           table[slot].Target = (void *) obj;
           return(TRUE);
         }
       }
   }

  unless(RmExtendFreeQ(root))
   return(FALSE);
  return(RmNextFreeUid(root, obj));
}

/**
*** This routine releases the Uid associated with the object. It is
*** called when free'ing an object that is still inside a network or
*** taskforce.
***/
bool RmReleaseUid(RmObject obj)
{ RmSet root;
  RmUidTableEntry *tab;
  int uid;
    
  if (obj eq (RmObject)NULL) return(FALSE);
  root = obj->Root;
  uid  = obj->Uid;
  
  if (root eq (RmSet)NULL) return(FALSE);
  if (uid eq RmL_NoUid) return(FALSE);
  tab = RmFindTableEntry(root, uid);
  if (tab eq Null(RmUidTableEntry)) return(FALSE);
  if (tab->Target ne (void *) obj) return(FALSE);
  if (RmGetCycle(uid) ne tab->Cycle)
   return(FALSE);
   
  tab->Target   = (void *) RmL_NoObject;
  tab->Cycle    = tab->Cycle + 1;
  tab->Free     = TRUE;
  obj->Uid      = RmL_NoUid;
  return(TRUE);
}

/**
*** This routine is similar to RmNextFreeUid(), but if the object already
*** has a Uid of some sort then the system will attempt to reuse it.
*** For example, when adding an obtained processor to an obtained network
*** it is very desirable to keep the same Uid.
**/
bool RmObtainUid(RmSet root, RmObject obj)
{ int uid;
  RmUidTableEntry *tab;
  
  if ((root eq (RmSet)NULL) || (obj eq (RmObject)NULL)) return(FALSE);

  uid = obj->Uid;
  if (uid eq RmL_NoUid)
   return(RmNextFreeUid(root, obj));

  tab = RmFindTableEntry(root, uid);
  if (tab eq Null(RmUidTableEntry))
   { int tabno = (uid >> 8) & 0x00FFFF;
     if (tabno >= root->NoTables)
      { while(tabno >= root->NoTables)
         unless(RmExtendFreeQ(root))
          return(FALSE);
        return(RmObtainUid(root, obj));
      }
     else
      return(FALSE);
   }
  if (tab->Free)
   { tab->Free   = FALSE;
     tab->Cycle  = RmGetCycle(uid);
     tab->Target = (void *) obj;
     return(TRUE);
   }
  else
   return(FALSE);
}

/*}}}*/
/*{{{  Adding objects (including subsets and merging) */

/**-----------------------------------------------------------------------------
*** List manipulation routines. These are much more complicated than they
*** appear, because of the need to keep Uid's in a sensible state. 
***
*** The final routine is RmAddObject. By now both objects are believed
*** to exist, and it is legal to add the object to the set. The object may
*** be terminal, or a subset. The routine can be used irrespective of
*** whether or not it is already in the network, by use of a flag argument.
*** This means that it can be used by AddTail, AddHead, PreInsert, and
*** PostInsert. Note that the routine does not put the object into the list.
***
*** RmAddObject() is called by RmAddProcessor() or RmAddTail(), which
*** do the necessary type validation.
***
*** RmAddProcessor() is called by RmAddTailProcessor(), RmAddHeadProcessor(),
*** RmPreInsertProcessor(), and RmPostInsertProcessor().
**/
static RmObject    RmAddObject(RmSet set, RmObject obj, bool bNew);
static RmProcessor RmAddProcessor(RmNetwork, RmProcessor, bool);
static RmTask      RmAddTask(RmTaskforce, RmTask, bool);


/**
*** This routine is called to add an obtained processor to an obtained network
*** First check that these conditions have been met. Then determine the right
*** subnet in which to put the processor. Then call RmAddProcessor.
**/
RmProcessor     RmInsertProcessor(RmNetwork network, RmProcessor processor)
{ char          *netname, *temp;
  RmProcessor   cur_proc;

  CheckNetworkFail(network, (RmProcessor) NULL);
  CheckProcessorFail(processor, (RmProcessor) NULL);

  netname = RmGetObjectAttribute((RmObject) processor, "PUID", TRUE);
  if (netname eq Null(char))
   return(RmErrno = network->Errno = processor->Errno = RmE_NoAccess,
          (RmProcessor) NULL);

        /* Do not validate the root of the network, this might be /domain */
  for (temp = &(netname[1]); (*temp ne '/') && (*temp ne '\0'); temp++);
  if (*temp eq '\0')            /* just /Net ?? */
   return(RmErrno = network->Errno = processor->Errno = RmE_WrongNetwork,
          (RmProcessor) NULL);

  forever 
   { 
loop:
     netname = ++temp;          /* now root or Cluster/Root */
     if (netname eq '\0')               /* /Net/ ??? */
      return(RmErrno = network->Errno = processor->Errno = RmE_WrongNetwork,
             (RmProcessor)NULL);

     for (temp = netname; (*temp ne '/') && (*temp ne '\0'); temp++);
     if (*temp eq '\0') /* we are now in the target network */
      { 
        if (RmAddProcessor(network, processor, TRUE) ne (RmProcessor) NULL)
         { if (network->NoSubnets eq 0)
            AddTail(&(network->DirNode.Entries), &(processor->ObjNode.Node));
           elif (network->NoSubnets eq network->DirNode.Nentries)
            AddHead(&(network->DirNode.Entries), &(processor->ObjNode.Node));
           else
            { RmProcessor head = RmFirstProcessor(network);
              until(RmIsNetwork(head)) head = RmNextProcessor(head);
              PreInsert(&(head->ObjNode.Node), &(processor->ObjNode.Node));
            }
           return(processor);
         }
        else
         return((RmProcessor) NULL);
      }


     *temp = '\0';              /* netname is Cluster/root, temp is /root */
     for (cur_proc = RmFirstProcessor(network);
          cur_proc ne (RmProcessor) NULL;
          cur_proc = RmNextProcessor(cur_proc))
      if (RmIsNetwork(cur_proc))
       if (!strcmp(netname, cur_proc->ObjNode.Name))
        { network       = (RmNetwork) cur_proc;
          *temp = '/';
          goto  loop;
        }
                /* specified subnet Cluster is not in this network */
     *temp = '/';
     return((RmProcessor) NULL);
  }
}

RmProcessor     RmAddtailProcessor(RmNetwork network, RmProcessor processor)
{
  if (RmAddProcessor(network, processor, TRUE) ne (RmProcessor) NULL)
   { AddTail(&(network->DirNode.Entries), &(processor->ObjNode.Node));
     return(processor);
   }
  else
   return((RmProcessor) NULL);
}

RmProcessor     RmAddheadProcessor(RmNetwork network, RmProcessor processor)
{
  if (RmAddProcessor(network, processor, TRUE) ne (RmProcessor) NULL)
   { AddHead(&(network->DirNode.Entries), &(processor->ObjNode.Node));
     return(processor);
   }
  else
   return((RmProcessor) NULL);
}

RmProcessor     RmPreinsertProcessor(RmProcessor procA, RmProcessor procB)
{ if ((procA eq (RmProcessor) NULL) || (procB eq (RmProcessor) NULL))
   return((RmProcessor) NULL);

  if (procB->ObjNode.Parent eq Null(struct DirNode))
   { if (RmAddProcessor((RmNetwork) procA->ObjNode.Parent, procB, TRUE))
      { PreInsert(&(procA->ObjNode.Node), &(procB->ObjNode.Node));
        return(procB);
      }
     else
      return((RmProcessor) NULL);
   }
  else
   { if (RmAddProcessor((RmNetwork) procA->ObjNode.Parent, procB, FALSE))
      { Remove(&(procB->ObjNode.Node));
        PreInsert(&(procA->ObjNode.Node), &(procB->ObjNode.Node));
        return(procB);
      }
     else
      return((RmProcessor) NULL);
   }
}

RmProcessor     RmPostinsertProcessor(RmProcessor procA, RmProcessor procB)
{ if ((procA eq (RmProcessor) NULL) || (procB eq (RmProcessor) NULL))
   return((RmProcessor) NULL);

  if (procB->ObjNode.Parent eq Null(struct DirNode))
   { if (RmAddProcessor((RmNetwork)procA->ObjNode.Parent, procB, TRUE))
      { PostInsert(&(procA->ObjNode.Node), &(procB->ObjNode.Node));
        return(procB);
      }
     else
      return((RmProcessor) NULL);
   }
  else
   { if (RmAddProcessor((RmNetwork) procA->ObjNode.Parent, procB, FALSE))
      { Remove(&(procB->ObjNode.Node));
        PostInsert(&(procA->ObjNode.Node), &(procB->ObjNode.Node));
        return(procB);
      }
     else
      return((RmProcessor) NULL);
   }
}

RmTask  RmAddtailTask(RmTaskforce taskforce, RmTask task)
{
  if (RmAddTask(taskforce, task, TRUE) ne (RmTask) NULL)
   { AddTail(&(taskforce->DirNode.Entries), &(task->ObjNode.Node));
     return(task);
   }
  else
   return((RmTask) NULL);
}

RmTask  RmAddheadTask(RmTaskforce taskforce, RmTask task)
{
  if (RmAddTask(taskforce, task, TRUE) ne (RmTask) NULL)
   { AddHead(&(taskforce->DirNode.Entries), &(task->ObjNode.Node));
     return(task);
   }
  else
   return((RmTask) NULL);
}

RmTask  RmPreinsertTask(RmTask taskA, RmTask taskB)
{ if ((taskA eq (RmTask) NULL) || (taskB eq (RmTask) NULL))
   return((RmTask) NULL);

  if (taskB->ObjNode.Parent eq Null(struct DirNode))
   { if (RmAddTask((RmTaskforce) taskA->ObjNode.Parent, taskB, TRUE))
      { PreInsert(&(taskA->ObjNode.Node), &(taskB->ObjNode.Node));
        return(taskB);
      }
     else
      return((RmTask) NULL);
   }
  else
   { if (RmAddTask((RmTaskforce) taskA->ObjNode.Parent, taskB, FALSE))
      { Remove(&(taskB->ObjNode.Node));
        PreInsert(&(taskA->ObjNode.Node), &(taskB->ObjNode.Node));
        return(taskB);
      }
     else
      return((RmTask) NULL);
   }
}

RmTask  RmPostinsertTask(RmTask taskA, RmTask taskB)
{ if ((taskA eq (RmTask) NULL) || (taskB eq (RmTask) NULL))
   return((RmTask) NULL);

  if (taskB->ObjNode.Parent eq Null(struct DirNode))
   { if (RmAddTask((RmTaskforce) taskA->ObjNode.Parent, taskB, TRUE))
      { PostInsert(&(taskA->ObjNode.Node), &(taskB->ObjNode.Node));
        return(taskB);
      }
     else
      return((RmTask) NULL);
   }
  else
   { if (RmAddTask((RmTaskforce) taskA->ObjNode.Parent, taskB, FALSE))
      { Remove(&(taskB->ObjNode.Node));
        PostInsert(&(taskA->ObjNode.Node), &(taskB->ObjNode.Node));
        return(taskB);
      }
     else
      return((RmTask) NULL);
   }
}

/**
*** These two routines validate the data types of the main arguments,
*** and then call RmAddObject() to do most of the real work. The
*** target object is not added to the list, that is done by the calling
*** routine.
**/

static RmTask   RmAddTask(RmTaskforce taskforce, RmTask task, bool bNew)
{ RmTask result;

  CheckTaskforceFail(taskforce, (RmTask) NULL);

  if (task eq (RmTask) NULL)
   return(RmErrno = RmE_NotTask, (RmTask) NULL);
  if ((task->ObjNode.Type ne Type_Task) && 
      (task->ObjNode.Type ne Type_Taskforce))
   return(RmErrno = RmE_NotTask, (RmTask) NULL);
  result = (RmTask) RmAddObject((RmSet) taskforce, (RmObject) task, bNew);
  if (result eq (RmTask) NULL) taskforce->Errno = RmErrno;
  return(result);
}

static RmProcessor      RmAddProcessor(RmNetwork network, RmProcessor processor,
                                        bool    bNew)
{ RmProcessor result;

  CheckNetworkFail(network, (RmProcessor) NULL);

  if (processor eq (RmProcessor)NULL)
   return(RmErrno = RmE_NotProcessor, (RmProcessor) NULL);
  if ((processor->ObjNode.Type ne Type_Processor) &&
      (processor->ObjNode.Type ne Type_Network))
   return(RmErrno = RmE_NotProcessor, (RmProcessor) NULL);

  result = (RmProcessor)RmAddObject((RmSet) network, (RmObject) processor, bNew);
   
  if (result eq (RmProcessor) NULL)
   network->Errno = RmErrno;
  return(result);  
}

/**
*** O.K, this is where things get tricky. RmAddObject()...
***
*** The following is known at this point. Set really is a directory of
*** some sort, possibly the root, possibly not. Obj can be either
*** a subset or a leaf object. The third argument indicates whether or
*** not this object is being added, or whether the list is being
*** rearranged. There are several more tests that have to be performed.
*** Otherwise, there is another routine call, depending on whether the
*** object is terminal or a subset.
**/

static RmObject RmAddTerminal(RmSet, RmObject, bool);
static RmSet    RmAddSubset(RmSet, RmSet, bool);

static RmObject RmAddObject(RmSet set, RmObject obj, bool bNew)
{ 
  if (set eq (RmSet) obj)
   return(RmErrno = RmE_BadArgument, (RmObject) NULL);

  if (bNew)      /* then the object cannot have a parent */
   { if (obj->ObjNode.Parent ne Null(struct DirNode))
      return(RmErrno = RmE_InUse, (RmObject) NULL);
   }
   
  if ((obj->ObjNode.Type & Type_Flags) eq Type_Directory)
   return((RmObject) RmAddSubset(set, (RmSet) obj, bNew));
  else
   return(RmAddTerminal(set, obj, bNew));
}

/**
*** If the subset is already part of the set, very little has to be done.
*** If the subset is empty, very little has to be done. However,
*** if the subset contains stuff then it is necessary to do quite a bit
*** of work remapping Uid's. A similar remapping has to take place when
*** merging two networks together.
**/
static bool  RmRemapUids(RmSet target, RmSet current);

static int change_root(RmProcessor obj, ...)
{ va_list args;
  RmSet   *root;
  
  va_start(args, obj);
  root = va_arg(args, RmSet *);
  va_end(args);
  if ((obj->ObjNode.Type & Type_Flags) eq Type_Directory)
   return(RmApplyNetwork((RmNetwork) obj, &change_root, root));
  obj->Root = (RmNetwork) root;
  return(0);
}
  
static RmSet RmAddSubset(RmSet set, RmSet subset, bool bNew)
{ int	i;

  if (bNew && (subset->DirNode.Nentries > 0))
   unless(RmRemapUids(set, subset))
    return(RmErrno = RmE_NoMemory, (RmSet) NULL);

  (void) RmApplyNetwork((RmNetwork) subset, &change_root, set->Root);

  subset->Root = set->Root;
  set->DirNode.Nentries++;
  unless(bNew)
   { RmSet parent = (RmSet) subset->DirNode.Parent;
     parent->DirNode.Nentries--;
   }  
  subset->DirNode.Parent = (struct DirNode *) set;
  set->NoSubsets++;

	/* The subset no longer has or needs uid info			*/
  for (i = 0; i < subset->NoTables; i++)
   Free(subset->Tables[i]);
  Free(subset->Tables);
  subset->Tables = NULL;
  subset->NoTables = 0;

  return(subset);
}

static RmObject RmAddTerminal(RmSet set, RmObject obj, bool bNew)
{ 
  if (bNew)
   unless(RmObtainUid(set->Root, obj))
     return(RmErrno = RmE_NoMemory, (RmObject) NULL);

  obj->Root = (RmSet) set->Root;
  set->DirNode.Nentries++;
  unless(bNew)
   { RmSet parent = (RmSet) obj->ObjNode.Parent;
     parent->DirNode.Nentries--;
   }
  obj->ObjNode.Parent = (struct DirNode *) set;
  return(obj);  
}

/**
*** Merging networks. This is only really used by the Taskforce Manager to
*** add an obtained network to the domain, but conceivably other applications
*** might want to do the same.
**/

static int RmMergeNetworksAux2(RmProcessor, ...);

int             RmMergeNetworks(RmNetwork domain, RmNetwork obtained)
{ int   rc;

  CheckNetwork(domain);
  CheckNetwork(obtained);
  
  if ((rc = RmSearchNetwork(obtained, &RmMergeNetworksAux2, domain))
      ne RmE_Success)
   RmErrno = domain->Errno = obtained->Errno = rc;
  return(rc);
}

static int RmMergeNetworksAux2(RmProcessor processor, ...)
{ RmNetwork     target;
  va_list       args;
  RmProcessor   current;
  
  va_start(args, processor);
  target = va_arg(args, RmNetwork);
  va_end(args);

  if (processor->ObjNode.Type ne Type_Processor)
   { for (current = RmFirstProcessor(target);
          current ne (RmProcessor) NULL;
          current = RmNextProcessor(current))
      if (current->ObjNode.Type eq Type_Network)
       if (!strcmp(current->ObjNode.Name, processor->ObjNode.Name))
        return(RmSearchNetwork((RmNetwork) processor, &RmMergeNetworksAux2, current));
      return(RmE_BadArgument);
   }

  { RmNetwork   parent = (RmNetwork) processor->ObjNode.Parent;
    parent->DirNode.Nentries--;
    Remove(&(processor->ObjNode.Node));
  }

  unless(RmObtainUid((RmSet) target->Root, (RmObject) processor))
   return(RmE_InUse);

  if (target->NoSubnets eq 0)
   AddTail(&(target->DirNode.Entries), &(processor->ObjNode.Node));
  elif (target->NoSubnets eq target->DirNode.Nentries)
   AddHead(&(target->DirNode.Entries), &(processor->ObjNode.Node));
  else
   { for (current = RmFirstProcessor(target);
          current->ObjNode.Type ne Type_Network;
          current = RmNextProcessor(current));
     PreInsert( &(current->ObjNode.Node), &(processor->ObjNode.Node));
   }
  target->DirNode.Nentries++;
  processor->ObjNode.Parent = (DirNode *) target;
  processor->Root           = (RmNetwork) target->Root;
  return(RmE_Success);
}
/**
*** For every terminal object in SetB, allocate a new Uid inside SetA.
*** Then walk through all the connections inside SetB and change them
*** for the new Uids. The routine works in four stages.
***
*** 1) walk down the terminals and obtain a new Uid. This will invalidate
***    the current Uid's, but not the Uid tables for SetB. The old Uid is
***    stored in the RmLib field, for restoring purposes.
***
*** 2) On an error, restore all the old Uid's and free the slots in the
***    new slot.
***
*** 3) Check whether any of the Uid's have changed. If not, finished.
***
*** 4) walk down the terminals again, and for every connection use the old
***    Uid tables to get to the right pointer and fill in the new Uid
***
*** The routine is called only from inside the library, with all the
*** arguments validated.
**/

static int      RmRemapAux1(RmProcessor processor, ...)
{ va_list args;
  RmNetwork network;
  
  va_start(args, processor);
  network = va_arg(args, RmNetwork);
  va_end(args);
  
  processor->Private = processor->Uid;
  
  if (RmIsNetwork(processor))
   return(RmApplyNetwork((RmNetwork) processor, &RmRemapAux1, network));
   
  if (RmObtainUid((RmSet) network, (RmObject) processor))
   return(0);

        /* The uid is already in use, so get a different one */
  processor->Uid = RmL_NoUid;
  if (RmNextFreeUid((RmSet) network, (RmObject) processor))
   return(0);
  
        /* Failed to get a Uid of any sort. This is an error */
  return(1);
}

        /* This routine undoes any damage done by the previous routine */
static int      RmRemapAux2(RmProcessor processor, ...)
{ va_list args;
  RmNetwork network, save;
  
  va_start(args, processor);
  network = va_arg(args, RmNetwork);
  va_end(args);
  if (RmIsNetwork(processor))
   return(RmApplyNetwork((RmNetwork) processor, &RmRemapAux2, network));
  if (processor->Uid eq RmL_NoUid)
   { processor->Uid = processor->Private;
     return(0);
   }
  save = processor->Root;
  processor->Root = network;
  if (!RmReleaseUid((RmObject) processor))
   { processor->Root = save;
     return(1);
   }
  processor->Root = save;
  processor->Uid  = processor->Private;
  return(0);
}

static int      RmRemapAux3(RmProcessor processor, ...)
{ if (RmIsNetwork(processor))
   return(RmApplyNetwork((RmNetwork) processor, &RmRemapAux3));
  if (processor->Uid ne processor->Private)
   return(1);
  return(0);
}

static int      RmRemapAux4(RmProcessor processor, ...)
{ int   count, i;

  if (RmIsNetwork(processor))
   return(RmApplyNetwork((RmNetwork) processor, &RmRemapAux4));

  count = RmCountLinks(processor);
  for (i = 0; i < count; i++)
   { RmLink *link = RmFindLink(processor, i);
     RmObject obj;
     if ((link->Target eq RmL_NoUid) || (link->Target eq RmL_ExtUid))
      continue;
     obj = RmFindUid((RmSet) processor->Root, link->Target);
     if ((obj eq RmL_NoObject) || (obj eq RmL_ExtObject))
      continue;
     link->Target = obj->Uid;
   } 
  return(0);
}

static bool     RmRemapUids(RmSet setA, RmSet setB)
{
  if (RmApplyNetwork((RmNetwork) setB, &RmRemapAux1, setA) ne 0)
   { if (RmApplyNetwork((RmNetwork) setB, &RmRemapAux2, setA) ne 0)
      IOdebug("RmLib internal error : failed to undo RemapUids");
     return(FALSE);
   }
  if (RmSearchNetwork((RmNetwork) setB, &RmRemapAux3) ne 0)
   return(RmApplyNetwork((RmNetwork) setB, &RmRemapAux4) eq 0);
  return(TRUE);
}

/*}}}*/
/*{{{  Removing objects (including subsets) */
/**
*** RmRemoveProcessor() and RmRemoveTask(), these verify the argument
*** and call routines RmRemoveSet() and RmRemoveObject() to do the real
*** work.
**/
static RmSet    RmRemoveSet(RmSet set);
static RmObject RmRemoveObject(RmObject obj);

RmProcessor     RmRemoveProcessor(RmProcessor processor)
{ RmProcessor result;

  if (processor eq (RmProcessor) NULL)
   return(RmErrno = RmE_NotProcessor, (RmProcessor) NULL);
  if (processor->ObjNode.Type eq Type_Processor)
   { if ((result = (RmProcessor) RmRemoveObject((RmObject) processor))
         eq (RmProcessor) NULL)
      processor->Errno = RmErrno;
     return(result);
   }
  elif(processor->ObjNode.Type eq Type_Network)
   { if ((result = (RmProcessor) RmRemoveSet((RmSet) processor))
         eq (RmProcessor) NULL)
      { RmNetwork network = (RmNetwork) processor;
        network->Errno    = RmErrno;
      }
     return(result);
   }
  else
   return(RmErrno = RmE_NotProcessor, (RmProcessor) NULL);
}

RmTask          RmRemoveTask(RmTask task)
{ RmTask        result;

  if (task eq (RmTask)NULL)
   return(RmErrno = RmE_NotTask, (RmTask) NULL);
  if (task->ObjNode.Type eq Type_Task)
   { if ((result = (RmTask) RmRemoveObject((RmObject) task))
         eq (RmTask) NULL)
      task->Errno = RmErrno;
     return(result);
   }
  elif (task->ObjNode.Type eq Type_Taskforce)
   { if ((result = (RmTask) RmRemoveSet((RmSet) task))
         eq (RmTask) NULL)
      { RmTaskforce taskforce = (RmTaskforce) task;
        taskforce->Errno      = RmErrno;
      }
     return(result);
   }
  else
   return(RmErrno = RmE_NotTask, (RmTask) NULL);
}

/**
*** RemoveObject(). This is only legal if the object is currently in a set.
*** The following steps are required.
***
*** 1) releasing the Uid. Unless the object is new, i.e. a structure created
***    by the user's application, the Uid should be restored afterwards.
***
*** 2) removing the object from the parent, updating all the fields
***
*** 3) setting the parent and root fields in the object
***
*** 4) if the object has been created locally get rid of the connections. 
***    It is almost impossible to maintain a user's
***    connections in a sensible state once an object has been removed,
***    so the connections are deleted
**/
static RmObject RmRemoveObject(RmObject obj)
{ int  Uid;
  RmSet parent;
  
  parent = (RmSet) obj->ObjNode.Parent;
  if (parent eq (RmSet)NULL)
   { RmErrno = (obj->ObjNode.Type eq Type_Processor) ? RmE_NotNetwork :
               RmE_NotTaskforce;
     return((RmObject) NULL);
   }

  Uid = obj->Uid;
  unless(RmReleaseUid(obj))
   return(RmErrno = RmE_Corruption, (RmObject) NULL);

  if (obj->StructType ne RmL_New) obj->Uid = Uid;
  
  (void) Remove(&(obj->ObjNode.Node));
  parent->DirNode.Nentries--;
    
  obj->ObjNode.Parent = Null(struct DirNode);
  obj->Root = (RmSet) NULL;

  if (obj->StructType eq RmL_New)  
   { int i;
     if (obj->Connections > 4)
      { (void) Free(obj->OtherLinks);
        obj->OtherLinks = Null(RmLink);
      }
     obj->Connections = 0;
     for (i = 0; i < 4; i++)
      { obj->DefaultLinks[i].Flags              = 0;
        obj->DefaultLinks[i].Destination        = 0;
        /* obj->Defaultlinks[i].Spare           = 0; */
        obj->DefaultLinks[i].Target             = RmL_NoUid;
      }
   }
  return(obj);
}

/**
*** RmRemoveSet(). If the set is empty then it is easy, the job is
*** equivalent to removing an object and setting the Root pointer
*** appropriately. Otherwise some work is needed.
***
*** 1) update the parent and current structures.
*** 2) allocate space for UidTables, using the same number of tables as in
***    the root. This may be more than necessary.
*** 3) update very processor in the target subnet, releasing the current Uid,
***    obtaining a uid using the new tables, and zapping the root pointer
*** 4) check every connection of every processor in the network, to see if
***    the connection now leads to the outside world. If there used to be
***    a connection but it is no longer valid, it is set to external
**/
static int      RmRemoveSetAux(RmProcessor, ...);
static int      RmRemoveSetAux2(RmProcessor, ...);

static RmSet    RmRemoveSet(RmSet set)
{ RmSet parent, root;
  int   i;

  parent = (RmSet) set->DirNode.Parent;
  root   = (RmSet) set->Root;
  if ((parent eq (RmSet) NULL) || (root eq (RmSet) NULL))
   { RmErrno = (set->DirNode.Type eq Type_Network) ? RmE_NotNetwork :
               RmE_NotTaskforce;
     return((RmSet) NULL);
   }

  if (set->DirNode.Nentries eq 0)
   { (void) Remove(&(set->DirNode.Node));
     parent->DirNode.Nentries--;
     parent->NoSubsets--;
     set->Root = (struct RmSetStruct *) set;
     set->DirNode.Parent = NULL;
     return(set);
   }

  for(i = 0; i < root->NoTables; i++)
   if (!RmExtendFreeQ(set))
    { int j;
      for (j = 0; j < i; j++)
       (void) Free(set->Tables[j]);
      (void) Free(set->Tables);
      set->NoTables = 0;
      return(RmErrno = RmE_NoMemory, (RmSet) NULL);
    }

  (void) Remove(&(set->DirNode.Node));
  parent->DirNode.Nentries--;
  parent->NoSubsets--;
  set->DirNode.Parent = Null(struct DirNode);
  set->Root = (struct RmSetStruct *) set;

  if (RmApplyNetwork((RmNetwork) set, &RmRemoveSetAux, set))
   return(RmErrno = RmE_NoMemory, (RmSet) NULL);

  (void) RmApplyNetwork((RmNetwork) set, &RmRemoveSetAux2);
  return(set);
}

static int RmRemoveSetAux(RmProcessor obj, ...)
{ va_list       args;
  RmNetwork     root;
  int           temp;

  va_start(args, obj);
  root = va_arg(args, RmNetwork);
  va_end(args);

  if ((obj->ObjNode.Type & Type_Flags) eq Type_Directory)
   { RmSet tmp = (RmSet) obj;
     tmp->Root = (RmSet) root;
     return(RmApplyNetwork((RmNetwork) obj, &RmRemoveSetAux, root));
   }
   
  temp       = obj->Uid;
  unless(RmReleaseUid((RmObject) obj)) return(1);
  obj->Uid   = temp;
  obj->Root  = (RmNetwork) root;
  unless(RmObtainUid((RmSet) root, (RmObject) obj)) return(1);
  return(0);
}

static int      RmRemoveSetAux2(RmProcessor processor, ...)
{ int i;

  if (RmIsNetwork(processor))
   return(RmApplyNetwork((RmNetwork) processor, &RmRemoveSetAux2));

  for (i = 0; i < processor->Connections; i++)
   { RmLink *link = RmFindLink(processor, i);

     if ((link->Target ne RmL_NoUid) && (link->Target ne RmL_ExtUid))
      { RmUidTableEntry *entry = 
         RmFindTableEntry((RmSet) processor->Root, link->Target);
        if (entry->Free)
         link->Target = RmL_ExtUid;
      }
   }
  return(0); 
}
/*}}}*/
/*{{{  Links and channels */

/**-----------------------------------------------------------------------------
*** Manipulating links and channels. By a strange coincidence the same
*** code can be used for manipulating links between processors and channels
*** between tasks. Both are implemented in terms of RmObjects, which happen
*** to use links. Links are numbered from 0 onwards. 
***
*** The first routine, RmFindLink(), simply returns a pointer to the
*** appropriate RmLink structure given the object.
**/

RmLink  *RmFindLink(RmProcessor processor, int linkno)
{ if (linkno >= processor->Connections)
   return(RmErrno = RmE_BadArgument, Null(RmLink));
  elif (linkno < 4)
   return(&(processor->DefaultLinks[linkno]));
  else
   return(&(processor->OtherLinks[linkno - 4]));
}

/**
*** There are routines RmMakeConn(), RmFollowConn(), and so on, that
*** get called by RmMakeLink(), RmMakeChannel(), etc. following
*** basic validation tests. Some of the routines call themselves
*** recursively to deal with the other end of the connection, so there
*** is an extra argument to control this.
**/
static int      RmMakeConn(RmObject, int, RmObject, int, int recurse);
static int      RmBreakConn(RmObject, int, int recurse);
static RmObject RmFollowConn(RmObject, int, int *);
static void      build_fd_string(char *buf, int channel);

int RmMakeLink(RmProcessor source, int sourcelink, 
                   RmProcessor dest,   int destlink)
{ int rc;

  CheckProcessor(source);

  unless ((dest eq RmM_NoProcessor) || (dest eq RmM_ExternalProcessor))
   { CheckProcessor(dest);
   }
  if ((rc = RmMakeConn((RmObject) source, sourcelink, (RmObject) dest, destlink, TRUE))
      ne RmE_Success)
   RmErrno = source->Errno = rc;
  return(rc);
}

int RmMakeChannel(RmTask source, int sourcechan,
                      RmTask dest, int destchan)
{ int rc;

  CheckTask(source);

  unless ((dest eq RmM_NoTask) || (dest eq RmM_ExternalTask))
   { CheckTask(dest);
   }
  if ((rc = RmMakeConn((RmObject) source, sourcechan, (RmObject) dest, destchan, TRUE))
      ne RmE_Success)
   RmErrno = source->Errno = rc;
  return(rc);
}


int RmBreakLink(RmProcessor processor, int link)
{ int rc;

  CheckProcessor(processor);

  if ((link < 0) || (link >= processor->Connections))
   return(RmErrno = processor->Errno = RmE_BadArgument);
  if ((rc = RmBreakConn((RmObject) processor, link, TRUE))
      ne RmE_Success)
   RmErrno = processor->Errno = rc;
  return(rc);
}

/**
*** This has to cope with the special case of external channels, i.e.
*** streams redirected to and from files.
**/
int RmBreakChannel(RmTask task, int channel)
{ int rc;

  CheckTask(task);

  if ((channel < 0) || (channel >= task->Connections))
   return(RmErrno = task->Errno = RmE_BadArgument);

  if (RmFollowChannel(task, channel, Null(int)) eq RmM_ExternalTask)
   { char	buf[9];
     char	*result;
     build_fd_string(buf, channel);
     result	= RmGetObjectAttribute((RmObject) task, buf, TRUE);
     if (result ne Null(char))
      { result	= &(result[-9]);   /* fd000000=/helios/tmp/xx */
        (void) RmRemoveObjectAttribute((RmObject) task, result, TRUE);
      }
   }
  if ((rc = RmBreakConn((RmObject) task, channel, TRUE))
      ne RmE_Success)
   RmErrno = task->Errno = rc;
  return(rc);
}

int RmCountLinks(RmProcessor processor)
{ CheckProcessorFail(processor, -1);
  return(processor->Connections);
}

int RmCountChannels(RmTask task)
{ CheckTaskFail(task, -1);	
  return(task->Connections);
}

RmProcessor RmFollowLink(RmProcessor processor, int link, int *dlink)
{ RmProcessor result;
  int junk;
  if (dlink eq Null(int)) dlink = &junk;
  
  if (processor eq (RmProcessor) NULL)
   return(RmErrno = *dlink = RmE_NotProcessor, RmM_NoProcessor); 
  if (processor->ObjNode.Type ne Type_Processor)
   return(RmErrno = *dlink = RmE_NotProcessor, RmM_NoProcessor);
  
  if ((result = (RmProcessor) RmFollowConn((RmObject) processor, link, dlink))
      eq (RmProcessor) NULL)
   processor->Errno = RmErrno;
  return(result);
}

RmTask RmFollowChannel(RmTask task, int channel, int *dchannel)
{ RmTask result;
  int junk;
  if (dchannel eq Null(int)) dchannel = &junk;

  if (task eq (RmTask) NULL)
   return(RmErrno = *dchannel = RmE_NotTask, RmM_NoTask);
  if (task->ObjNode.Type ne Type_Task)
   return(RmErrno = *dchannel = RmE_NotTask, RmM_NoTask);
  if ((result = (RmTask) RmFollowConn((RmObject) task, channel, dchannel))
      eq (RmTask) NULL)
   task->Errno = RmErrno;
  return(result);
}

int RmGetLinkFlags(RmProcessor processor, int link)
{ RmLink *res;

  CheckProcessorFail(processor, -1);

  if ((link < 0) || (link >= processor->Connections))
   return(RmErrno = processor->Errno = RmE_BadArgument, -1);
  res = RmFindLink(processor, link);
  if (res eq Null(RmLink))
   return(RmErrno = RmE_BadLink, -1);
  else
   return(res->Flags);
}

int RmGetChannelFlags(RmTask task, int channel)
{ RmLink *res;

  CheckTaskFail(task, -1);

  if ((channel < 0) || (channel >= task->Connections))
   return(RmErrno = task->Errno = RmE_BadArgument, -1);
  res = RmFindLink((RmProcessor) task, channel);
  if (res eq Null(RmLink))
   return(RmErrno = task->Errno = RmE_BadChannel, -1);
  else
   return(res->Flags);
}

/**
*** Special routines to deal with redirection to and from files. These
*** are treated as external links, with the file name stored in the
*** private attributes list. The destlink field is overloaded with the
*** file open mode.
***
*** Fill in an attribute name depending on the channel number. For example,
*** if the channel number is 3 then the attribute name will be
*** fdaaaaad.
**/
static void build_fd_string(char *buf, int channel)
{ buf[0] = 'f';
  buf[1] = 'd';
  buf[7] = (channel & 0x0F) + 'a'; channel >>= 4;
  buf[6] = (channel & 0x0F) + 'a'; channel >>= 4;
  buf[5] = (channel & 0x0F) + 'a'; channel >>= 4;
  buf[4] = (channel & 0x0F) + 'a'; channel >>= 4;
  buf[3] = (channel & 0x0F) + 'a'; channel >>= 4;
  buf[2] = (channel & 0x0F) + 'a'; channel >>= 4;
  buf[8] = '\0';  
}

/**
*** RmConnectChannelToFile(). There are two cases. If the filename
*** is absolute then there is no way of getting an extra capability
*** for it. Otherwise the name stored should include a capability
*** for the current directory, in the form @xxxx/helios/tmp:filename
*** This has the disadvantage of potentially confusing RmFollowChannelToFile()
***
*** N.B. the current CDL compiler does not store any capabilities.
**/

int RmConnectChannelToFile(RmTask task, int channelno, char *name, int mode)
{ int	rc;
  int	length;
  BYTE	*buffer;

	/* Validate all the arguments */    
  CheckTask(task);

  if ((name eq Null(char)) || (channelno eq RmM_AnyLink) || (channelno < 0))
   return(RmErrno = task->Errno = RmE_BadArgument);
  if (name[0] eq '\0') 
   return(RmErrno = task->Errno = RmE_BadArgument);

  if (name[0] eq '/')
   {	/* Create the attribute string fdaaaaad=/helios/temp */
     length = 10 + strlen(name);
     buffer = (BYTE *) Malloc(length);
     if (buffer eq Null(BYTE))
      return(RmErrno = task->Errno = RmE_NoMemory);
     build_fd_string(buffer, channelno);
     buffer[8] = '=';
     strcpy(&(buffer[9]), name);
   }
  else
   {    /* Creating a name relative to the current directory	*/
   	/* Format is aaaa=@xxxxxxxxxxxxxxxx/helios/tmp:filename	*/
     Environ	*env = getenviron();
     Object	*context;
     if (env eq Null(Environ))
      return(RmErrno = task->Errno = RmE_NoMemory);
     context = env->Objv[OV_Cdir];

     length = 9 + 17 + strlen(context->Name) + 1 + strlen(name) + 1;
     buffer = (BYTE *) Malloc(length);
     if (buffer eq Null(BYTE))
      return(RmErrno = task->Errno = RmE_NoMemory);
     build_fd_string(buffer, channelno);
     buffer[8] = '=';
     (void) DecodeCapability(&(buffer[9]), &(context->Access));
     strcpy(&(buffer[26]), context->Name);
     strcat(&(buffer[26]), ":");
     strcat(&(buffer[26]), name);
   }
	/* Try to make this connection */    
  rc = RmMakeConn((RmObject) task, channelno, RmL_ExternalObject, mode, FALSE);
  if (rc ne RmE_Success)
   { Free(buffer);
     return(RmErrno = task->Errno = rc);
   }
	/* Add the string to the attributes, so that I can figure out */
	/* what the file name is */
  rc = RmAddObjectAttribute((RmObject) task, buffer, TRUE);
  if (rc ne RmE_Success)
   { (void) RmBreakConn((RmObject) task, channelno, FALSE);
     RmErrno = task->Errno = rc;
   }
  Free(buffer);
  return(rc);
}

const char *RmFollowChannelToFile(RmTask task, int channelno, int *mode_ptr)
{ int	temp;
  int	rc = RmE_NotTask;
  char	*result = NULL;
  char	buffer[9];
   
  if (mode_ptr eq Null(int)) mode_ptr = &temp;
  
  if (task eq (RmTask) NULL) 
   return(RmErrno = *mode_ptr = RmE_NotTask, Null(char));
  if (task->ObjNode.Type ne Type_Task)
   return(RmErrno = *mode_ptr = RmE_NotTask, Null(char));
  if ((channelno eq RmM_AnyLink) || (channelno < 0))
   { rc = RmE_BadArgument; goto done; }

  if (RmFollowChannel(task, channelno, mode_ptr) ne RmM_ExternalTask)
   { rc = RmE_BadFile; goto done; }
   
  build_fd_string(buffer, channelno); 
  result = RmGetObjectAttribute((RmObject) task, buffer, TRUE);
  if (result eq Null(char))
   rc = RmE_Corruption;
  else
   rc = RmE_Success;
   
done:
  if (rc ne RmE_Success)
   { *mode_ptr = RmErrno = task->Errno = rc; return(Null(char)); }
  else
   return(result); 
}

/**
*** The fill_in_link() routine takes a pointer to a
*** the target object and the number of the link to fill in, and
*** details of the destination. The relevant RmLink structure is known
*** to exist. 
**/
static int fill_in_link(RmObject source, int linkno,
			 RmObject dest, int destlink)
{ RmLink *curr_link = RmFindLink((RmProcessor) source, linkno);

  curr_link->Destination = destlink;

  if (dest eq RmL_NoObject)
   curr_link->Target		= RmL_NoUid;
  elif (dest eq RmL_ExternalObject)
   curr_link->Target		= RmL_ExtUid;
  else
   curr_link->Target		= dest->Uid;
  return(RmE_Success);
}

/**
*** add_link() is called when it is necessary to extend the number of
*** extra links held within an object. linkno is guaranteed to be
*** >= 4. Essentially it is a case of realloc'ing, without
*** using the C library, and then filling in the link.
**/
static int add_link(RmObject processor, int linkno, RmObject dest,
                    int destlink)
{ RmLink *new_links;
  int    i;
  
  new_links = (RmLink *) Malloc(((word)linkno - 3) * sizeof(RmLink));
  if (new_links eq Null(RmLink))
   return(RmErrno = RmE_NoMemory);
  for (i = (processor->Connections <= 4) ? 0 : processor->Connections - 4;
       i < (linkno - 3);
       i++)
   { new_links[i].Target       = RmL_NoUid;
     new_links[i].Flags	       = 0;
     new_links[i].Destination  = 0;
   }
  if (processor->Connections > 4)
   { memcpy((void *) new_links, (void *) processor->OtherLinks,
          (processor->Connections - 4) * sizeof(RmLink));
     Free(processor->OtherLinks);
   }

  processor->OtherLinks  = new_links;
  processor->Connections = linkno + 1;
  return(fill_in_link(processor, linkno, dest, destlink));
}


/**
*** This routine is responsible for making connections between objects.
*** The following things are known: source is a real object, either a
*** processor or a task; dest can be NoObject, ExtObject, or a real
*** object; sourcelink and destlink can be anything. This is not completely
*** true. In a recursive call source may well be NoObject or ExtObject,
*** in which case the routine is a no-op. The routine recurses if and only
*** if the link arguments are symmetrical: if both links are defined, or
*** neither link is defined. If only one link is defined then there is
*** no recursion.
***
*** 1) Check for source being NoObject or ExtObject
*** 2) It is illegal to connect AnyLink to NoObject or ExtObject
*** 3) If dest is a real object, it must be in the same set as source
*** 4) If sourcelink can be any link, look for a free slot, fill it in,
***    and mark it as AnyLink. See comment below.
*** 5) if sourcelink is a particular link, decide what should be done with
***    it. See comment below.
**/

static int RmMakeConn(RmObject source, int sourcelink, 
		   RmObject dest, int destlink, int recurse)
{
  if ((source eq RmL_NoObject) || (source eq RmL_ExternalObject))
   return(RmE_Success);
   
  if ((sourcelink eq RmM_AnyLink) && 
      ((dest eq RmL_NoObject) || (dest eq RmL_ExternalObject)))
   return(RmErrno = RmE_BadArgument);
   
  if ((dest ne RmL_NoObject) && (dest ne RmL_ExternalObject))
   { if (source->Root ne dest->Root)
      return(RmErrno = (source->ObjNode.Type eq Type_Processor) ? 
      		RmE_WrongNetwork : RmE_WrongTaskforce);
   }

  if (sourcelink eq RmM_AnyLink)
   { int i;
	/* If the source link can be any link, it is necessary to walk	*/
	/* through the table of known links to find a free slot. This	*/
	/* slot can now be used for this connection. If there are no	*/
	/* free slots it may be possible to fill in one of the four	*/
	/* default slots, if less than four links have been allocated,	*/
	/* otherwise the table of other connections has to be extended.	*/
     for (i = 0; i < source->Connections; i++)
      { RmLink *curr_link = RmFindLink((RmProcessor) source, i);

        if (curr_link->Target eq RmL_NoUid)
         { int result = fill_in_link(source, i, dest, destlink);
           if (result ne RmE_Success) return(RmErrno = result);
           curr_link->Flags |= RmF_AnyLink;
           goto recurse;
         }
      }

	/* There are no gaps in the current table of connections. This may */
	/* be because not all of the default connections have been used.   */
	/* If so I can use the next free default link.			   */
     if (source->Connections < 4)
      { int linkno = source->Connections++;
        RmLink	*link = RmFindLink((RmProcessor) source, linkno);
        int	result = fill_in_link(source, linkno, dest, destlink);
        if (result ne RmE_Success) return(RmErrno = result);
        link->Flags |= RmF_AnyLink;
	goto recurse;
      }   

	/* All the links including the default ones have been used, so it */
	/* is necessary to expand the table. */
      { int linkno = source->Connections;
        int result = add_link(source, linkno, dest, destlink);
        RmLink *link;
	if (result ne RmE_Success) return(RmErrno = result);
        link = RmFindLink((RmProcessor) source, linkno);
        link->Flags |= RmF_AnyLink;
	goto recurse;
      }

  }

	/* Special case, any negative link is invalid */
  if (sourcelink < 0) return(RmErrno = RmE_BadArgument);
       
	/* If a specific link number has been supplied, and this number is */
	/* greater than that of any existing link, then the link table must */
	/* be expanded to match. */
  if (sourcelink >= source->Connections)
   { if (sourcelink < 4)
      { int result;
        source->Connections = sourcelink + 1;
        result = fill_in_link(source, sourcelink, dest, destlink);
        if (result ne RmE_Success) return(RmErrno = result);
	goto recurse;
       }         
      else
       { int result = add_link(source, sourcelink, dest, destlink);
         if (result ne RmE_Success) return(RmErrno = result);
         goto recurse;
       }
   }

	/* A real link number has been supplied, and the link tables	*/
	/* contain this link. If the link is currently in use as AnyLink*/
	/* some rearranging is necessary. If the link is in		*/
	/* use as a real connection then this would have to be broken	*/
	/* first.							*/
   { RmLink *link = RmFindLink((RmProcessor) source, sourcelink);
   
     if (link->Target eq RmL_NoUid)
      { int result = fill_in_link(source, sourcelink, dest, destlink);
        if (result ne RmE_Success) return(RmErrno = result);
  	goto recurse;
      }

	/* If the connection already exists, there is no problem. */
    if ((dest ne RmL_NoObject) && (link->Target eq dest->Uid))
     { link->Destination = destlink;
       link->Flags	&= ~RmF_AnyLink;
       goto recurse;
     }
     
    if (link->Flags & RmF_AnyLink)
     { RmObject	obj	= RmFindUid(source->Root, link->Target);
       int	destlink = link->Destination;
       int	result;
       result = fill_in_link(source, sourcelink, dest, destlink);
       if (result ne RmE_Success) return(RmErrno = result);
       if (obj ne (RmObject)NULL)
        { result = RmMakeConn(source, RmM_AnyLink, obj, destlink, FALSE);
          if (result ne RmE_Success) return(RmErrno = result);
        }
     }
    return(RmErrno = RmE_InUse);
   }

recurse:
  unless(recurse)
   return(RmE_Success);
  if (((sourcelink ne RmM_AnyLink) && (destlink ne RmM_AnyLink)) ||
      ((sourcelink eq RmM_AnyLink) && (destlink eq RmM_AnyLink)))
   return(RmMakeConn(dest, destlink, source, sourcelink, FALSE));
  else
   return(RmE_Success);
}

/**
*** Breaking connections. At this point the source object has been
*** validated, but nothing else. Breaking a connection is equivalent
*** to connecting it to NoObject.
**/
static int RmBreakConn(RmObject source, int sourcelink, int recurse)
{ RmObject	dest;
  int		destlink;
  RmLink	*link;
  
  if ((sourcelink eq RmM_AnyLink) || (sourcelink < 0) ||
      (sourcelink >= source->Connections))
   return(RmErrno = RmE_BadArgument);
   
  link = RmFindLink((RmProcessor) source, sourcelink);
  if (link eq Null(RmLink)) return(RmErrno = RmE_Corruption);
  if (recurse && (link->Target ne RmL_NoUid) && (link->Target ne RmL_ExtUid))
   { dest = RmFindUid(source->Root, link->Target);
     destlink = link->Destination;
     if ((dest ne RmL_NoObject) && (dest ne RmL_ExtObject) &&
         (destlink ne RmM_AnyLink))
      (void) RmBreakConn(dest, destlink, FALSE);
   }
   
  link->Target  = RmL_NoUid;
  link->Destination = 0;
  link->Flags &= ~(RmF_AnyLink + RmF_ToFile);
  return(RmE_Success);
}

/**
*** Following a connection to its destination. At this point Processor
*** is guaranteed to be a valid object and dlink a valid pointer,
*** possibly pointing to a local somewhere in the calling routine's
*** stackframe. The value of link has not been validated. If reasonable,
*** the specified connection may be external, non-existant, or some
*** real object.
**/
static RmObject RmFollowConn(RmObject processor, int linkno, int *dlink)
{ RmLink   *link;
  RmObject Target;
  
  if ((linkno eq RmM_AnyLink) || (linkno < 0) || (linkno >= processor->Connections))
   { RmErrno = *dlink = RmE_BadArgument; return((RmObject) NULL); }
  link = RmFindLink((RmProcessor) processor, linkno);
  if (link->Target eq RmL_ExtUid)
   { *dlink = link->Destination; return(RmL_ExternalObject); }
  elif (link->Target eq RmL_NoUid)
   { *dlink = 0; return(RmL_NoObject); }

  Target = RmFindUid(processor->Root, link->Target);
  if (Target eq RmL_NoObject)
   *dlink = 0;
  else
   *dlink = link->Destination;
  if (Target eq RmL_NoObject)
   return(RmL_ExternalObject);
  else
   return(Target); 
}

/*}}}*/
/*{{{  Emacs Customisation */

/* Local variables: */
/* folded-file: t */
/* end: */

/*}}}*/
