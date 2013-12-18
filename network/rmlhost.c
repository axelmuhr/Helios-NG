/*------------------------------------------------------------------------
--                                                                      --
--           H E L I O S   N E T W O R K I N G   S O F T W A R E	--
--           ---------------------------------------------------	--
--                                                                      --
--             Copyright (C) 1993, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- rmlhost.c								--
--                                                                      --
--	This module contains the required support routines for hosted	--
--	versions of the simpler software, e.g. rmgen			--
--                                                                      --
--	Author:  BLV 5/8/93						--
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Header: /hsrc/network/RCS/rmlhost.c,v 1.3 1993/08/11 10:44:21 bart Exp $*/

#define in_rmlib	1

/*{{{  RmLib variables (before header files!) */
#define NULL ((void *)0)
char	*RmT_Names[12] = {
	"<Unknown>",
	"<Default>",
	"T800",
	"T414",
	"T425",
	"T212",
	"T9000",
	"T400",
	"i860",
	"Arm",
	"680x0",
	"C40"
};

int	 RmProgram		= 0x7F;
void	*RmNetworkServer	= NULL;
void	*RmSessionManager	= NULL;
void	*RmParent		= NULL;
int	 RmStartSearchHere	= 0;
char	*RmVersionNumber	= "1.18";
void	*RmExceptionHandler	= NULL;
int	 RmExceptionStack	= 2000;
char	*RmRootName		= NULL;
int	 RmErrno		= 0;
#undef NULL

/*}}}*/
/*{{{  Header files etc. */
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>

#ifdef SUN4
#include <sys/fcntlcom.h>
#endif

#ifdef RS6000
#include <fcntl.h>
#include <sys/stat.h>
#endif

	/* avoid problems with IOdebug() declarations in Helios headers	*/
#define IOdebug junk_IOdebug
#include <helios.h>
#include <queue.h>
#include "ampp/../memory.h"
#include <syslib.h>
#include <sem.h>
#include <codes.h>
#undef IOdebug

/*}}}*/
/*{{{  list handling */
/*--------------------------------------------------------
-- InitList                                             --
--                                                      --
-- Initialise a list structure                          --
--                                                      --
--------------------------------------------------------*/

void InitList(list)
List *list;
{
	list->Head  = (Node *)&list->Earth;
	list->Earth = NULL;
	list->Tail  = (Node *)list;
}

/*--------------------------------------------------------
-- PreInsert                                            --
--                                                      --
-- Insert a node before another                         --
-- Note that next must not be a List, but may point to  --
-- the Earth/Tail pair of a List structure.             --
--                                                      --
--------------------------------------------------------*/

void PreInsert(Node *next, Node *node)
{
	node->Prev	 = next->Prev;
	node->Next	 = next;
	next->Prev	 = node;
	node->Prev->Next = node;
}

/*--------------------------------------------------------
-- PostInsert                                           --
--                                                      --
-- Insert a node after another                          --
-- Note that pred may be a List structure.              --
--                                                      --
--------------------------------------------------------*/

void PostInsert(Node *prev, Node *node)
{
	node->Next	 = prev->Next;
	node->Prev	 = prev;
	prev->Next	 = node;
	node->Next->Prev = node;
}

/*--------------------------------------------------------
-- Remove                                               --
--                                                      --
-- Remove a node from its current list                  --
-- NOTE: It MUST be on list!!                           --
--                                                      --
--------------------------------------------------------*/

Node *Remove(node)
Node *node;
{
	node->Next->Prev = node->Prev;
	node->Prev->Next = node->Next;
	node->Next = NULL; node->Prev = NULL;
	return node;
}

/*--------------------------------------------------------
-- AddHead                                              --
--                                                      --
-- Add a node to the head of the list                   --
--                                                      --
--------------------------------------------------------*/

void AddHead(List *list, Node *node)
{
	node->Next	 = list->Head;
	node->Prev	 = (Node *)list;
	list->Head->Prev = node;
	list->Head	 = node;
}

/*--------------------------------------------------------
-- AddTail                                              --
--                                                      --
-- Add the node to the tail of the list                 --
--                                                      --
--------------------------------------------------------*/

void AddTail(list, node)
List *list;
Node *node;
{
	node->Prev	 = list->Tail;
	node->Next	 = (Node *)&list->Earth;
	list->Tail->Next = node;
	list->Tail	 = node;
}

/*--------------------------------------------------------
-- RemHead                                              --
--                                                      --
-- Remove the head node from the list, if there is one. --
--                                                      --
--------------------------------------------------------*/

Node *RemHead(List *list)
{
	Node *node = list->Head;
	if( node->Next == NULL ) return NULL;
	node->Next->Prev = (Node *)list;
	list->Head = node->Next;
	node->Next = NULL; node->Prev = NULL;
	return node;
}

/*--------------------------------------------------------
-- RemTail                                              --
--                                                      --
-- Remove tail node from the list, if there is one.     --
--                                                      --
--------------------------------------------------------*/

Node *RemTail(List *list)
{
	Node *node = list->Tail;
	if( node->Next == NULL ) return NULL;
	node->Prev->Next = (Node *)&list->Earth;
	list->Tail = node->Prev;
	node->Next = NULL; node->Prev = NULL;
	return node;
}

/*--------------------------------------------------------
-- WalkList						--
--							--
-- This function scans down the given list applying	--
-- the given function to each Node in turn. Takes care	--
-- to avoid problems is the node if removed from the	--
-- list by the function.				--
--							--
--------------------------------------------------------*/

word WalkList(List *list, WordFnPtr fn, ...)
{
	Node *node = list->Head;
	Node *next = node->Next;
	word sum = 0;
	word arg;
	va_list a;

	va_start(a,fn);
	arg = va_arg(a,word);
	
	while( next != NULL )
	{
		sum += fn(node,arg);
		node = next;
		next = node->Next;
	}
	va_end(a);
	return sum;
}

/*--------------------------------------------------------
-- SearchList						--
--							--
-- Similar to WalkList except that the result of the fn	--
-- is used. If it returns 0 the walk continues,		--
-- otherwise it is terminated. The terminating node, or	--
-- Null is returned.					--
--							--
--------------------------------------------------------*/


Node *SearchList(List *list, WordFnPtr fn, ... )
{
	Node *node = list->Head;
	Node *next = node->Next;
	word arg;
	va_list a;

	va_start(a,fn);
	arg = va_arg(a,word);
		
	while( next != NULL )
	{
		if( fn(node,arg) ) 
		{
			va_end(a);
			return node;
		}
		node = next;
		next = node->Next;
	}
	va_end(a);
	return NULL;
}

/*}}}*/
/*{{{  mystrcmp() */
/**
*** String comparison routine which is not case sensitive. It returns the
*** same result as strcmp, i.e. 0 for identical strings
**/
int mystrcmp(char *ms1, char *ms2)
{ char *s1 = ms1;
  char *s2 = ms2; 
#define ToUpper(x) (islower(x) ? toupper(x) : x)
  
  for (;;)
   { if (*s1 == '\0')
       return((*s2 == '\0') ? 0L : -1L);
     elif (*s2 == '\0')
       return(1L);
     elif(ToUpper(*s1) < ToUpper(*s2))
       return(-1L);
     elif(ToUpper(*s1) > ToUpper(*s2))
       return(1L);
     else
       { s1++; s2++; }
   }
}
/*}}}*/
/*{{{  Malloc() and Free() */
void *Malloc(word x)
{
	return(malloc((int) x));
}

word Free(void *x)
{
	free(x);
	return(0);
}
/*}}}*/
/*{{{  InitSemaphore() */
void InitSemaphore(Semaphore *sem, word count)
{
	sem->Count	= count;
	sem->Head	= NULL;
	sem->Tail	= (struct Id *) sem;
}
/*}}}*/
/*{{{  IOdebug() */
void IOdebug(const char *str, ...)
{
	char	buf[256];
	va_list	args;

	va_start(args, str);
	strcpy(buf, "+++ ");
	vsprintf(&(buf[4]), str, args);
	strcat(buf, "\n");
	fputs(buf, stderr);
}
/*}}}*/
/*{{{  capabilities etc */
int _cputime(void)
{
	return(0);
}

char	*DecodeCapability(char *s, Capability *cap)
{
	return(NULL);
}
/*}}}*/
/*{{{  GetDate() */
Date	GetDate(void)
{
	time_t	x;
	x = time(NULL);
	return((Date) x);
}
/*}}}*/
/*{{{  getenviron() */
static	Environ	 environ;
static	char	 current_directory[512];
static	Object	*objv[2];

Environ	*getenviron()
{
	environ.Objv	= objv;
	environ.Argv	= NULL;
	environ.Envv	= NULL;
	environ.Strv	= NULL;
	environ.Objv[0]	= (Object *) &current_directory;
	environ.Objv[1]	= NULL;
	getwd(environ.Objv[0]->Name);
	return(&environ);
}

/*}}}*/
/*{{{  Result2() */
/*
 * Within RmLib2.c Result2() is used after a failed open. This
 * routine maps Posix error codes onto Helios ones. It has been
 * mostly lifted from the I/O Server
 */
word (Result2)(Object *o)
{

 switch(errno)
   {
#ifdef    EPERM 
     case EPERM        :       /* Not owner */
           return(EC_Error + SS_RmLib + EG_Protected + EO_File);
#endif
#ifdef    ENOENT
     case ENOENT       :       /* No such file or directory */
           return(EC_Error + SS_RmLib + EG_Unknown + EO_File);
#endif
#ifdef    ENINTR
     case EINTR        :       /* Interrupted system call */
           return(EC_Warn + SS_RmLib + EG_Exception + EO_Stream);
#endif
#ifdef    EIO
     case EIO          :       /* I/O error */
           return(EC_Error + SS_RmLib + EG_Broken + EO_File);
#endif
#ifdef    ENXIO
     case ENXIO        :       /* No such device or address */
           return(EC_Error + SS_RmLib + EG_Unknown + EO_File);
#endif
#ifdef    EBADF
     case EBADF        :       /* Bad file number */
           return(EC_Error + SS_RmLib + EG_Unknown + EO_Stream);
#endif
#ifdef    ENOMEM
     case ENOMEM       :       /* Not enough core */
           return(EC_Error + SS_RmLib + EG_NoMemory + EO_Server);
#endif
#ifdef    EACCES
     case EACCES       :       /* Permission denied */
           return(EC_Error + SS_RmLib + EG_Protected + EO_File);
#endif
#ifdef    EFAULT
     case EFAULT       :       /* Bad address */
           return(EC_Error + SS_RmLib + EG_Broken + EO_Server);
#endif
#ifdef    EBUSY
     case EBUSY        :       /* Mount device busy */
           return(EC_Warn + SS_RmLib + EG_InUse + EO_Object);
#endif
#ifdef    EEXIST
     case EEXIST       :       /* File exists */
           return(EC_Error + SS_RmLib + EG_Create + EO_Object);
#endif
#ifdef    EXDEV
     case EXDEV        :       /* Cross-device link */
           return(EC_Error + SS_RmLib + EG_WrongFn + EO_Object);
#endif
#ifdef    ENOTDIR
     case ENOTDIR      :       /* Not a directory*/
           return(EC_Error + SS_RmLib + EG_WrongFn + EO_File);
#endif
#ifdef    EISDIR
     case EISDIR       :       /* Is a directory */
           return(EC_Error + SS_RmLib + EG_WrongFn + EO_Directory);
#endif
#ifdef    EINVAL
     case EINVAL       :       /* Invalid argument */
           return(EC_Error + SS_RmLib + EG_Broken + EO_Server);
#endif
#ifdef    EFBIG
     case EFBIG        :       /* File too large */
           return(EC_Error + SS_RmLib + EG_WrongSize + EO_File);
#endif
#ifdef    ENFILE
     case ENFILE       :       /* File table overflow */
           return(EC_Error + SS_RmLib + EG_NoMemory + EO_Stream);
#endif
#ifdef    EMFILE
     case EMFILE       :
           return(EC_Error + SS_RmLib + EG_NoResource + EO_Server);
#endif
#ifdef    ENOSPC
     case ENOSPC       :       /* No space left on device */
           return(EC_Error + SS_RmLib + EG_NoMemory + EO_Route);
#endif
#ifdef    ESPIPE
     case ESPIPE       :       /* Illegal seek */
           return(EC_Error + SS_RmLib + EG_WrongFn + EO_Fifo);
#endif
#ifdef     EROFS
     case EROFS        :       /* Read-only file system */
           return(EC_Error + SS_RmLib + EG_Protected + EO_Route);
#endif
#ifdef    EMLINK
     case EMLINK       :       /* Too many links */
           return(EC_Error + SS_RmLib + EG_NoMemory + EO_Link);
#endif
#ifdef    EPIPE
     case EPIPE        :       /* Broken pipe */
           return(EC_Error + SS_RmLib + EG_Broken + EO_Fifo);
#endif
#ifdef    EWOULDBLOCK
     case EWOULDBLOCK  :       /* Operation would block */
           return(EC_Warn + SS_RmLib + EG_InUse + EO_Object);
#endif
#ifdef    ELOOP
     case ELOOP        :       /* Too many levels of symbolic links */
           return(EC_Error + SS_RmLib + EG_WrongSize + EO_Link);
#endif
#ifdef    ENAMETOOLONG
     case ENAMETOOLONG :       /* File name too long */
           return(EC_Error + SS_RmLib + EG_Name + EO_File);
#endif
#ifndef RS6000
#ifdef    ENOTEMPTY
     case ENOTEMPTY    :       /* Directory not empty */
           return(EC_Error + SS_RmLib + EG_InUse + EO_Directory);
#endif
#endif

#ifdef    EDQUOT
     case EDQUOT       :       /* Disc quota exceeded */
           return(EC_Error + SS_RmLib + EG_NoMemory + EO_Route);
#endif
      default : return(0);
   }
}

/*}}}*/
/*{{{  Helios stream I/O and Locate() */
/*
 * These I/O routines all assume that all I/O is relative to the
 * current directory.
 */

	/* Largest file descriptor that open() is likely to return	*/
#define MAXFD	63

/* 
 * Locate() is used to check whether or not a particular file
 * already exists.
 */
Object *Locate(Object *o, char	*name)
{
	struct	stat	 buf;
	Object		*result;

	if (stat(name, &buf) != 0)
		return(NULL);

	result	= (Object *) malloc(512);
	if (result == NULL) return(NULL);

	strcpy(result->Name, name);
	return(result);
}

/*
 * Close() may be used on an Object returned by Locate() or on a stream.
 */
word (Close)(Stream *s)
{
	int	fd	= (int) s;

	if ((fd < 0) || (fd > MAXFD))
		/* Must be an Object					*/
		free(s);
	else
		/* Must be a stream.					*/
		close(fd);
	return(Err_Null);
}

/*
 * Open(): the file name may be embedded in the object.
 */
Stream *Open(Object *o, char *name, word mode)
{
	int	real_mode;
	int	fd;

	real_mode	= 0;
	if (mode & O_ReadOnly)		real_mode |= O_RDONLY;
	if (mode & O_WriteOnly)		real_mode |= O_WRONLY;
	if (mode & O_Create)		real_mode |= O_CREAT;
	if (mode & O_Exclusive)		real_mode |= O_EXCL;
	if (mode & O_Truncate)		real_mode |= O_TRUNC;
	if (mode & O_NonBlock)		real_mode |= O_NONBLOCK;
	if (mode & O_Append)		real_mode |= O_APPEND;

	if (name != NULL)
		fd = open(name, real_mode, 0666);
	else if (o != NULL)
		fd = open(o->Name, real_mode, 0666);
	else
		return(NULL);

	if (fd < 0)
		return(NULL);
	else
		return((Stream *)fd);
}


word Read(Stream *s, byte *buffer, word size, word timeout)
{
	int	fd	= (int) s;

	return((word) read(fd, buffer, size));
}

word Write(Stream *s, byte *buffer, word size, word timeout)
{
	int	fd	= (int) s;

	return((word) write(fd, buffer, size));
}

Stream *fdstream(int x)
{
	return((Stream *) x);
}
/*}}}*/
