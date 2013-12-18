/*------------------------------------------------------------------------
--                                                                      --
--                     H E L I O S   N U C L E U S                      --
--                     ---------------------------                      --
--                                                                      --
--             Copyright (C) 1987, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- syslib.h								--
--                                                                      --
--	System library header.						--	
--                                                                      --
--	Author:  NHG 16/8/87						--
--                                                                      --
------------------------------------------------------------------------*/
/* SccsId: %W%	%G% Copyright (C) 1987, Perihelion Software Ltd.	*/
/* $Id: syslib.h,v 1.7 1993/08/18 16:16:54 nickc Exp $ */

#ifndef __syslib_h
#define __syslib_h

#ifndef __helios_h
#include <helios.h>
#endif

#include <queue.h>

#ifndef __sem_h
#include <sem.h>
#endif
#ifndef __protect_h
#include <protect.h>
#endif
#ifndef __task_h
#include <task.h>
#endif
#ifndef __message_h
typedef uword		Port;		/* message port			*/
#endif

/*----------------------------------------------------------------------*/
/* Date stamp 								*/
/*----------------------------------------------------------------------*/

typedef unsigned int Date;		/* in seconds since 1/1/70	*/

/* common set of dates used by most objects */
struct DateSet 
{
	Date		Creation;	/* time object was created	*/
	Date		Access;		/* time object last accessed	*/
	Date		Modified;	/* time object last modified	*/
};

#ifndef __cplusplus
typedef struct DateSet DateSet;
#endif

/*----------------------------------------------------------------------*/
/* Passive object description structure 				*/
/*----------------------------------------------------------------------*/

struct Object 
{
        Node		Node;           /* link in housekeeping list    */
        word            Type;           /* object type                  */
        word            Flags;          /* control flag bits            */
        word            Result2;        /* second result or error       */
	word		FnMod;		/* function code modifier	*/
	word		Timeout;	/* seed for timeouts		*/
        Port            Reply;          /* port for object replies      */
        Capability      Access;         /* access capability            */
        byte            Name[Variable]; /* object pathname	        */
};

#ifndef __cplusplus
typedef struct Object Object;
#endif
 
/*----------------------------------------------------------------------*/
/* Active object structure 						*/
/*----------------------------------------------------------------------*/

struct Stream 
{
        Node		Node;           /* link in housekeeping list    */
        word            Type;           /* stream type                  */
        word            Flags;          /* flag bits                    */
        word            Result2;        /* second result or error       */
	word		FnMod;		/* function code modifier	*/
	word		Timeout;	/* seed for timeouts		*/
        Port            Reply;          /* reply port                   */
        Capability      Access;         /* access capability            */
        word            Pos;            /* current object position      */
        Port            Server;         /* server message port          */
	Semaphore	Mutex;		/* protect against multi-access */
        char            Name[Variable]; /* full object path name        */
};

#ifndef __cplusplus
typedef struct Stream Stream;
#endif
 
/*----------------------------------------------------------------------*/
/* Directory Entry structure						*/
/*----------------------------------------------------------------------*/

struct DirEntry 
{
	word		Type;		/* entry type			*/
	word		Flags;		/* flag bits			*/
	Matrix		Matrix;		/* access matrix		*/
	char		Name[32];	/* entry name			*/
};

#ifndef __cplusplus
typedef struct DirEntry DirEntry;
#endif
 
/*----------------------------------------------------------------------*/
/* Generic Object Info structure					*/
/*----------------------------------------------------------------------*/

struct ObjInfo 
{
	DirEntry	DirEntry;	/* re-iteration of common info	*/
	word		Account;	/* servers accounting info	*/
	word		Size;		/* file size in bytes		*/
	DateSet		Dates;		/* object dates			*/
};

#ifndef __cplusplus
typedef struct ObjInfo ObjInfo;
#endif
 
/*----------------------------------------------------------------------*/
/* Link Info returned from ObjInfo					*/
/*----------------------------------------------------------------------*/

struct Link_Info 
{
	struct DirEntry	DirEntry;	/* re-iteration of common info	*/
	Capability	Cap;		/* capability for target	*/
	char		Name[Variable];	/* full target path name	*/
};

#ifndef __cplusplus
typedef struct Link_Info Link_Info;
#endif
 
/*----------------------------------------------------------------------*/
/* Task Creation information						*/
/*----------------------------------------------------------------------*/

struct TaskInfo 
{
	RPTR		Name;		/* name of program file		*/
	Capability	Cap;		/* capability for it		*/
	Matrix		Matrix;		/* matrix for new task		*/
};

#ifndef __cplusplus
typedef struct TaskInfo TaskInfo;
#endif
 
/*----------------------------------------------------------------------*/
/* Environment description structure					*/
/*----------------------------------------------------------------------*/

struct Environ 
{
	string		*Argv;		/* argument strings		*/
	string		*Envv;		/* environment strings		*/
	Object		**Objv;		/* objects to be passed		*/
	Stream		**Strv;		/* streams to be passed		*/
};

#ifndef __cplusplus
typedef struct Environ Environ;
#endif
 
/*----------------------------------------------------------------------*/
/* Marshalled environment descriptors					*/
/*----------------------------------------------------------------------*/

struct ObjDesc 
{
	Capability	Cap;		/* capability for object	*/
	char		Name[Variable];	/* object path name		*/
};

#ifndef __cplusplus
typedef struct ObjDesc ObjDesc;
#endif
 
struct StrDesc 
{
	word		Mode;		/* mode stream was opened in	*/
	word		Pos;		/* position to start at		*/
	Capability	Cap;		/* capability for stream	*/
	char		Name[Variable];	/* name of stream object	*/
};

#ifndef __cplusplus
typedef struct StrDesc  StrDesc;
#endif
  

/*----------------------------------------------------------------------*/
/* Layout of Environment Objv						*/
/*----------------------------------------------------------------------*/

/* All entries must be non-NULL, although ((Object *)MinInt) may be	*/
/* inserted in place of undefined entries.				*/

/* entries 0..4 mandatory for all programs				*/
#define OV_Cdir		0	/* current directory			*/
#define OV_Task		1	/* ProcMan task entry			*/
#define OV_Code		2	/* Loader code entry			*/
#define OV_Source	3	/* original program source file		*/
#define OV_Parent	4	/* this task's parent			*/

/* entries 5..7 present for logged on users, otherwise optional		*/
#define OV_Home		5	/* home directory			*/
#define OV_Console	6	/* control console			*/
#define OV_CServer	7	/* control console/window server	*/

/* entries 8..9 present only for logged on users			*/
#define OV_Session	8	/* user's session manager entry		*/
#define OV_TFM		9	/* user's task force manager		*/

/* entry 10 present only for programs run via TFM			*/
#define OV_TForce	10	/* TFM entry for parent task force	*/

#define OV_End		11	/* NULL at end of Objv			*/

/*----------------------------------------------------------------------*/
/* Program Load information						*/
/*----------------------------------------------------------------------*/

struct LoadInfo 
{
	Capability	Cap;		/* access permission		*/
	Matrix		Matrix;		/* new access matrix		*/
	word		Pos;		/* position of image in file	*/
	char		Name[Variable];	/* file name			*/
}; 

#ifndef __cplusplus
typedef struct LoadInfo LoadInfo;
#endif
 
/*----------------------------------------------------------------------*/
/* Modes for Open							*/
/*----------------------------------------------------------------------*/

#define O_Mask		0x00ff		/* mask for access bits		*/
#define O_ReadOnly	0x0001		/* open for reading only	*/
#define O_WriteOnly	0x0002		/* open for writing only	*/
#define O_ReadWrite	0x0003		/* open for reading & writing	*/
#define O_Execute	0x0004		/* open for execution		*/
#define O_Private	0x0008		/* open for private interface	*/
#define O_Create	0x0100		/* create if does not exist	*/
#define O_Exclusive	0x0200		/* get exclusive access		*/
#define O_Truncate	0x0400		/* truncate if already exists	*/
#define O_NonBlock	0x0800		/* Do not block on read/write	*/
#define O_Append	0x1000		/* append data			*/

/* The flags array of the Select call contains the standard bits in the	*/
/* bottom 2 bits, plus the following bits...				*/

#define O_Exception	0x04		/* select for exception		*/
#define O_Selected	0x10		/* set if stream ready		*/

/*----------------------------------------------------------------------*/
/* Modes for Seek							*/
/*----------------------------------------------------------------------*/

#define S_Beginning	0		/* relative to start of file	*/
#define S_Relative	1		/* relative to current pos	*/
#define S_End		2		/* relative to end of file	*/
#define S_Last		3		/* relative to last operation	*/

/*----------------------------------------------------------------------*/
/* Flag Bits								*/
/*----------------------------------------------------------------------*/

/* First column of comments indicates whose responsibility the state 	*/
/* of the bit is: S = Server, L = Syslib, A = Application.		*/

#define Flags_Mode		0x0000000f	/* L copy of open mode		*/
#define Flags_More		0x00000010	/* S More info available	*/
#define Flags_Seekable		0x00000020	/* S stream is seekable		*/
#define Flags_Remote		0x00000040	/* L server is non-local	*/
#define Flags_StripName		0x00000080	/* S names are stripped before pass on */
#define Flags_CacheName 	0x00000100	/* S name is cached		*/
#define Flags_LinkName  	0x00000200	/* S name is for h/w link	*/
#define Flags_NoIData		0x00000400	/* S do not send data in write request */
#define Flags_ResetContext 	0x00000800	/* S set for remote servers	*/
#define Flags_Append		0x00001000	/* L append data to file		*/
#define Flags_CloseOnSend	0x00002000	/* S close in SendEnv		*/
#define Flags_OpenOnGet		0x00004000	/* S open in GetEnv		*/
#define Flags_Selectable	0x00008000	/* S can be used in Select	*/
#define Flags_Interactive 	0x00010000	/* S if stream is interactive	*/
#define Flags_MSdos	  	0x00020000	/* S MSdos format files		*/
#define Flags_Extended		0x00040000	/* S use extended Read/Write protocol */
#define Flags_NoReOpen		0x00080000	/* S cannot be ReOpened		*/
#define Flags_Application	0x00F00000	/* A for use by application code */
#define Flags_Fast		0x01000000	/* S timeouts can be short	*/

#define Flags_Reserved		0x1E000000	/* Unused bits			*/

						/* bits to save from open mode	*/
#define Flags_SaveMode		(Flags_Append|Flags_Mode)

#define Flags_CloseMask 0xe0000000	/*   mask for following flags	*/
#define Flags_CloseShift	29	/*   shift value		*/
#define Flags_Closeable	0x20000000	/* S set if needs closing	*/
#define Flags_Server	0x40000000	/* S set if served stream	*/
#define Flags_Stream	0x80000000	/* L set if stream structure	*/

#define closebits_(x)    ((((UWORD)(x))>>Flags_CloseShift)&0x7)

/*----------------------------------------------------------------------*/
/* Codes for TaskData							*/
/*----------------------------------------------------------------------*/

#define TD_Set		1		/* set rather than read data	*/
#define TD_Program	2		/* base of program code		*/
#define TD_Port		4		/* own port			*/
#define TD_Parent	6		/* parents port			*/
#define TD_HeapBase	8		/* base of heap			*/
#define TD_Flags	10		/* Task flags			*/
#define TD_Pool		12		/* task memory pool		*/

/*----------------------------------------------------------------------*/
/* Program status codes							*/
/*----------------------------------------------------------------------*/

#define PS_Terminate	0x100		/* wait for program to stop	*/
#define PS_Suspend	0x200		/* wait for program pause	*/
#define PS_Restart	0x400		/* wait for program restart	*/
#define PS_Status	0x800		/* update for all components	*/

/*----------------------------------------------------------------------*/
/* Public Variables							*/
/*----------------------------------------------------------------------*/

extern Task *MyTask;			/* pointer to task struct	*/

/*----------------------------------------------------------------------*/
/* Function Prototypes							*/
/*----------------------------------------------------------------------*/

/* Object operations */
extern Stream *	Open		(Object *context, string name, word mode);
extern Object *	Locate		(Object *context, string name);
extern Object *	Create		(Object *context, string name, word type, word infosize, byte *info);
extern word	ObjectInfo	(Object *context, string name, byte *info);
extern word	Link		(Object *context, string name, Object *object);
extern word	SetDate		(Object *context, string name, DateSet *dates);
extern word	Protect		(Object *context, string name, Matrix matrix);
extern word	Revoke		(Object *context);
extern word	Delete		(Object *context, string name);
extern word	Rename		(Object *context, string name, string newname);
extern word	Refine		(Object *context,	       AccMask mask);
extern word	ServerInfo	(Object *context,              byte *info);
extern word	ReLocate	(Object *context);

/* Stream operations */
extern word	Read		(Stream *stream, byte *buf, word size, word timeout);
extern word	Write		(Stream *stream, byte *buf, word size, word timeout);
extern word	Seek		(Stream *stream, word whence, word pos);
extern word	GetFileSize	(Stream *stream);
extern word	SetFileSize	(Stream *stream, word size);
extern word	ReOpen		(Stream *stream);
extern word	GetInfo		(Stream *stream, byte *info);
extern word	SetInfo		(Stream *stream, byte *info, word infosize);
extern Port	EnableEvents	(Stream *stream, word mask);
extern word	SelectStream	(word nstreams, Stream **streams, word *flags, word timeout);

/* Pipe operations */
extern word	GrabPipe	(Stream *stream, Port *ports);
extern word	UnGrabPipe	(Stream *stream);

/* Socket operations */
extern Stream *	Socket		(char *domain, word type, word protocol);
extern word	Bind		(Stream *stream, byte *addr, word len);
extern word	Listen		(Stream *stream, word len);
extern Stream * Accept		(Stream *stream, byte *addr, word *len);
extern word	Connect		(Stream *stream, byte *addr, word len);
extern word	SendMessage	(Stream *stream, word flags, ... );
extern word	RecvMessage	(Stream *stream, word flags, ... );
extern word	GetSocketInfo	(Stream *stream, word level, word option, void *optval, word *optlen);
extern word	SetSocketInfo	(Stream *stream, word level, word option, void *optval, word optlen);

/* Program Loading and Execution */
extern Object *	(Load)		(Object *loader, Object *image);
extern Object *	Execute		(Object *procman, Object *image);
extern word	SendSignal	(Stream *task, word signum);
extern word	InitProgramInfo (Stream *task, word mask);
extern word	GetProgramInfo	(Stream *task, word *status_vec, word timeout);
extern word	DefaultException(word signum, void *data);

/* Heap management */
extern void *	Malloc		(word size);
extern word 	Free		(void *block);
extern word	MemSize		(void *block);

/* Polymorphic operations */
extern word 	(Close)		(Stream *stream);
extern word	(Result2)	(Object *object);
extern word	(Abort)		(Object *object);

#ifndef in_syslib
/* These macros prevent compiler warnings */
#define Close(x)	Close((Stream *)(x))
#define Result2(x)	Result2((Object *)(x))
#define Abort(x)	Abort((Object *)(x))
#define Load(x,y)	Load(x,(Object *)(y))
#endif

/* Miscellaneous */
extern Date 	GetDate		(void);
extern void 	Exit		(word code);
extern void 	TidyUp		(void);
extern Stream *	NewStream	(string path, Capability *cap, word mode);
extern Stream *	PseudoStream	(Object *object, word mode);
extern Stream * CopyStream	(Stream *stream);
extern Object *	NewObject	(string path, Capability *cap);
extern Object * CopyObject	(Object *object);
extern void 	SendIOC		(MCB *mcb);
extern word 	MachineName	(byte *name);
extern word	Alarm		(word seconds);
extern word	TaskData	(word act, void *result);
extern Port	SetSignalPort	(Port port);

/* Port management */
extern Port 	NewPort		(void);
extern word	FreePort	(Port port);

/* Parameter Marshalling */
extern void 	InitMCB		(MCB *mcb, byte flags, Port dest, Port reply, word fnrc);
extern void 	MarshalString	(MCB *mcb, string str);
extern void 	MarshalWord	(MCB *mcb, word value);
extern void 	MarshalOffset	(MCB *mcb);
extern void 	MarshalCap	(MCB *mcb, Capability *cap);
extern void 	MarshalDate	(MCB *mcb, Date date);
extern void 	MarshalCommon	(MCB *mcb, Object *object, string name);
extern void 	MarshalData	(MCB *mcb, word size, byte *data);
extern void 	MarshalStream	(MCB *mcb, Stream *stream);
extern void 	MarshalObject	(MCB *mcb, Object *object);

/* Matrix & Capability En/Decoding */
extern Matrix 	EncodeMatrix	(string str, word type);
extern string 	DecodeMask	(string str1, AccMask mask, string str2);
extern void 	DecodeMatrix	(string str, Matrix matrix, word type);
extern string 	getbitchars	(word type);
extern char	*EncodeCapability(char *s, Capability *cap);
extern char	*DecodeCapability(char *s, Capability *cap);

/* Environment manipulation */
extern word 	SendEnv		(Port dest, Environ *env);
extern word 	GetEnv		(Port port, Environ *env);

/* File name manipulation */
extern int 	splitname	(char *prefix, char ch, char *str);

/* DES Encryption support */
extern void DES_KeySchedule	(uword *key, uword *ks);
extern void DES_Inner		(bool encrypt, uword *source, uword *dest, uword *ks);
extern void DES_ECB		(bool encrypt, char *key, char *text);
extern void DES_CFB		(bool encrypt, char *key, char *text, int size);

/* This is really in the kernel */
extern void 	Delay		(word micros);

/* For compatability... */
extern Object *cdobj(void);	/* actually in POSIX library */
#define CurrentDir cdobj()

#endif

/* -- End of syslib.h */
