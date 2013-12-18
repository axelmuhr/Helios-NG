/**
*
* Title:        Debug Library Header File.
*
* Author:       Andy England
*
* Date:         March 1989
*
* Modified By:  Nick Clifton
*
* Date:         October 1992
*
*         (c) Copyright 1988 - 1992, Perihelion Software Ltd.
*
*         All Rights Reserved.
*
* $Header: /hsrc/cmds/debugger/lib/RCS/dlib.h,v 1.6 1993/05/18 11:57:12 nickc Exp $
*
**/

#ifndef __dlib_h
#define __dlib_h

#ifdef __C40
#include <c40.h>	/* for wordptr */
#endif

#define AND 		&&
#define OR		||

typedef int		BOOL;
typedef Node		NODE;
typedef List		LIST;

#define FrameStackSize	100

#ifdef	MYDEBUG
#define DLib_Slot 	18
#else
#define DLib_Slot 	19
#endif

#define HashMax 	211

#define RealGetMsg	(debug->GetMsg)
#define RealPutMsg	(debug->PutMsg)

#ifdef __C40

#define CODE_IN_HIGH_MEMORY
#ifdef CODE_IN_HIGH_MEMORY

#define WordPtr	MPtr

#define MP_StructGet( type, field, ptr )	MP_GetWord( (ptr), (offsetof( type, field ) / sizeof (word) ) )
#define MP_memcpy( cptr, wptr, woff, wrds )	MP_GetData( (char *)cptr, wptr, woff, wrds )

#else /* code not in high memory */

#define WordPtr	word *

#define MP_StructGet( type, field, ptr )	(((type *)ptr)->field)
#define MP_memcpy( cptr, wptr, woff, wrds )	memcpy( (char *)cptr, (char *)(wptr + woff * sizeof (word)), (int)wrds * sizeof (word) )

#endif /* not CODE_IN_HIGH_MEMORY */

#define MP_RTOA( ptr )				(ptr + MP_GetWord( (ptr), 0 ))
#define GetProcInfo( ptr, field )		MP_GetWord( (ptr), (offsetof( ProcInfo, field ) / sizeof (word) ) )

#define T_SourceInfo	  T_FileName

typedef struct
  {
    word	Type;			/* T_SourceInfo flag (=> T_FileName)     */
    word	Modnum;			/* module number of compiled source file */
    byte	Name[ Variable ];	/* name of source file                   */
  }
SourceInfo;

#else /* not __C40 */

#define GetProcInfo(p) 	((ProcInfo *)((char *)(p) - sizeof(ProcInfo)))

#endif /* __C40 */

typedef struct
  {
    int 	Type;
    int		Id;
  }
Source;

typedef struct
  {
    NODE        node;
#ifdef __C40
    MPtr     procinfo;
#else
    Proc *	proc;    
#endif
    word        flags;
    word        calls;
    word        time;
  }
FUNC;

typedef struct
  {
    LIST 	threadlist;
    LIST	breakpointlist;
    LIST 	watchpointlist;
    Stream * 	stream;
    Port	port;
    Port 	reply;
    Semaphore	lock;
    LIST	functable[ HashMax ];
    word (*	GetMsg)(MCB *);
    word (*	PutMsg)(MCB *);
    word (*	Fork)(word, VoidFnPtr, word, ...);
  }
DEBUG;

typedef struct
  {
    word 	modnum;
    word	line;
  }
LOCATION;

#ifdef __C40
typedef struct
  {
    MPtr	procinfo;
    WordPtr	frameptr;
    WordPtr	stackptr;
    word	line;
  }
FRAME;
#else
typedef struct
  {
    Proc *	proc;
    byte *	wptr;
    word	line;
  }
FRAME;
#endif /* not __C40 */
	  
typedef struct
  {
    NODE 	node;
    BOOL	watchstop;
    Semaphore	sync;
    FRAME *	framestk;
    FRAME *	frameptr;
    FRAME *	stopframe;
    BOOL	profiling;
    BOOL	stopping;
    BOOL	tracing;
    Port	port;
    word	flags;
#ifdef __C40
    void (*	notify_entry)(   MPtr procinfo, MPtr frame_ptr, MPtr stack_ptr );
    void (*	notify_command)( word line_number, MPtr sourceinfo );
    word (*	notify_return)(  MPtr procinfo, word return_val  );
#else
    void (*	notify_entry)(   Proc *, byte *   );
    void (*	notify_command)( int,    Module * );
    word (*	notify_return)(  Proc *, word     );
#endif
  }
THREAD;

typedef struct breakpoint
  {
    NODE 	node;
    LOCATION	loc;
    word	threshold;
    int		count;
#ifdef V1_1
    BOOL 	temp;      /* tempory breakpoint for until commands */
#endif
  }
BREAKPOINT;

typedef struct
  {
    void *	addr;
    word	size;
  }
MEMLOCATION;

typedef struct watchpoint
  {
    NODE 	node;
    MEMLOCATION	loc;
    int		usage;
#ifdef OLD_CODE
    byte 	copy[];
#else
    byte * 	copy;	/* fix for ANSI compilation */  
#endif
  }
WATCHPOINT;

#ifdef __C40
PUBLIC void _notify_entry(	MPtr procinfo, MPtr frame_ptr, MPtr stack_ptr );
PUBLIC word _notify_return(	MPtr procinfo, word return_value );
PUBLIC void _notify_command(	word line_number, MPtr sourceinfo );
#else
PUBLIC void _notify_entry(	Proc *, byte *   );
PUBLIC word _notify_return(	Proc *, word     );
PUBLIC void _notify_command(	int,    Module * );
#endif

#endif /* __dlib_h */
