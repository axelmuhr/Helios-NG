

/* Stand-Alone Syslib stubs */

/* $Id: sasyslib.c,v 1.3 1990/11/26 19:06:50 nick Exp $ */


#include <syslib.h>
#include <codes.h>
#include <message.h>
#include <config.h>
#include <link.h>
#include <memory.h>
#include <attrib.h>
#include <device.h>
#include <asm.h>

#define Err_WrongFn (EC_Error|SS_SysLib|EG_WrongFn)
#define Err_BadPort (EC_Error|SS_SysLib|EG_Invalid|EO_Port)
#define Err_BadLink (EC_Error|SS_SysLib|EG_Invalid|EO_Link)

#if 0
void _SysLib_Init(void) {}

/* Object operations */
extern Stream *	Open		(Object *context, string name, word mode) { return NULL; }
extern Object *	Locate		(Object *context, string name)  { return NULL; }
extern Object *	Create		(Object *context, string name, word type, word infosize, byte *info)  { return NULL; }
extern word	ObjectInfo	(Object *context, string name, byte *info) { return EC_Error|SS_SysLib|EG_WrongFn; }
extern word	Link		(Object *context, string name, Object *object) { return EC_Error|SS_SysLib|EG_WrongFn; }
extern word	SetDate		(Object *context, string name, DateSet *dates) { return EC_Error|SS_SysLib|EG_WrongFn; }
extern word	Protect		(Object *context, string name, Matrix matrix) { return EC_Error|SS_SysLib|EG_WrongFn; }
extern word	Delete		(Object *context, string name) { return EC_Error|SS_SysLib|EG_WrongFn; }
extern word	Rename		(Object *context, string name, string newname) { return EC_Error|SS_SysLib|EG_WrongFn; }
extern word	Refine		(Object *context,	       AccMask mask)  { return EC_Error|SS_SysLib|EG_WrongFn; }
extern word	ServerInfo	(Object *context,              byte *info)  { return EC_Error|SS_SysLib|EG_WrongFn; }

/* Stream operations */
extern word	Read		(Stream *stream, byte *buf, word size, word timeout) { return EC_Error|SS_SysLib|EG_WrongFn; }
extern word	Write		(Stream *stream, byte *buf, word size, word timeout) { return EC_Error|SS_SysLib|EG_WrongFn; }
extern word	Seek		(Stream *stream, word whence, word pos) { return EC_Error|SS_SysLib|EG_WrongFn; }
extern word	GetFileSize	(Stream *stream) { return EC_Error|SS_SysLib|EG_WrongFn; }
extern word	SetFileSize	(Stream *stream, word size) { return EC_Error|SS_SysLib|EG_WrongFn; }
extern void	ReOpen		(Stream *stream) { return; }
extern word	GetInfo		(Stream *stream, byte *info) { return EC_Error|SS_SysLib|EG_WrongFn; }
extern word	SetInfo		(Stream *stream, byte *info, word infosize) { return EC_Error|SS_SysLib|EG_WrongFn; }
extern Port	EnableEvents	(Stream *stream, word mask) { return NullPort; }
extern word	SelectStream	(word nstreams, Stream **streams, word *flags, word timeout) { return EC_Error|SS_SysLib|EG_WrongFn; }

/* Pipe operations */
extern word	GrabPipe	(Stream *stream, Port *ports) {return EC_Error|SS_SysLib|EG_WrongFn; }
extern word	UnGrabPipe	(Stream *stream) {return EC_Error|SS_SysLib|EG_WrongFn; }

/* Program Loading and Execution */
extern Object *	(Load)		(Object *loader, Object *image) { return NULL; }
extern Object *	Execute		(Object *procman, Object *image) {return NULL; }
extern word	SendSignal	(Stream *task, word signum) { return EC_Error|SS_SysLib|EG_WrongFn; }
extern word	InitProgramInfo (Stream *task, word mask) { return 0; }
extern word	GetProgramInfo	(Stream *task, word *status_vec, word timeout) { return EC_Error|SS_SysLib|EG_WrongFn; }
extern word	DefaultException(word signum, void *data) { return EC_Error|SS_SysLib|EG_WrongFn; }
#endif

/* Heap management */
extern void *	Malloc		(word size) 
{ 
	word *m;
	size = (size + 15) & ~(7);	
	m = AllocMem(size,&MyTask->MemPool);
	if( m == NULL ) return NULL;
	m[0] = -size;
	return (void *)(m+2);
}

extern word 	Free		(void *block)
{
	word *m = (word *)block - 2;
	FreeMem(m);
	return Err_Null;
}

extern void FreeStop(void *mem)
{
	word *m = (word *)m - 2;
	FreeMemStop(m);
}

extern word MemSize(void *mem)
{
	word *m = mem;
	m -= 2;
	return m[0];
}

#if 0
/* Polymorphic operations */
extern word 	(Close)		(Stream *stream) { return EC_Error|SS_SysLib|EG_WrongFn; }
extern word	(Result2)	(Object *object) { return EC_Error|SS_SysLib|EG_WrongFn; }
extern word	(Abort)		(Object *object) { return EC_Error|SS_SysLib|EG_WrongFn; }

/* Miscellaneous */
extern Date 	GetDate		(void) { return 0; }
extern void 	Exit		(word code) { start_(); }
extern void 	TidyUp		(void) {}
extern Stream *	NewStream	(string path, Capability *cap, word mode) { return NULL; }
extern Stream *	PseudoStream	(Object *object, word mode)  { return NULL; }
extern Object *	NewObject	(string path, Capability *cap)  { return NULL; }
extern Object * CopyObject	(Object *object)  { return NULL; }
extern void 	SendIOC		(MCB *mcb)  {}
extern word 	MachineName	(byte *name) { *name=0;  return Err_Null; }
extern word	Alarm		(word seconds)  { return Err_Null; }
extern word	TaskData	(word act, void *result)  { return EC_Error|SS_SysLib|EG_Invalid; }

/* Parameter Marshalling */
extern void 	InitMCB		(MCB *mcb, byte flags, Port dest, Port reply, word fnrc) {}
extern void 	MarshalString	(MCB *mcb, string str)  {}
extern void 	MarshalWord	(MCB *mcb, word value)  {}
extern void 	MarshalOffset	(MCB *mcb) {}
extern void 	MarshalCap	(MCB *mcb, Capability *cap) {}
extern void 	MarshalDate	(MCB *mcb, Date date) {}
extern void 	MarshalCommon	(MCB *mcb, Object *object, string name) {}
extern void 	MarshalData	(MCB *mcb, word size, byte *data) {}
extern void 	MarshalStream	(MCB *mcb, Stream *stream) {}
extern void 	MarshalObject	(MCB *mcb, Object *object) {}

/* Matrix & Capability En/Decoding */
extern Matrix 	EncodeMatrix	(string str, word type) { return 0; }
extern string 	DecodeMask	(string str, AccMask mask, string str1) { return str; }
extern void 	DecodeMatrix	(string str, Matrix matrix, word type) {}
extern string 	getbitchars	(word type) {return FileChars; }
extern char	*EncodeCapability(char *s, Capability *cap) { return s; }
extern char	*DecodeCapability(char *s,Capability *cap) {return s;}

#endif

/* Environment manipulation */
extern word 	SendEnv		(Port dest, Environ *env) { return FreePort(dest); }
extern word 	GetEnv (Port port, Environ *env) 
{
	word *dummy = Malloc(4);
	
	*dummy = NULL;
	
	env->Argv = (char **)dummy; 
	env->Envv = (char **)dummy; 
	env->Objv = (Object **)dummy; 
	env->Strv = (Stream **)dummy; 

	return Err_Null;
}

#if 0
/* File name manipulation */
extern int 	splitname	(char *prefix, char ch, char *str) {return 0;}

extern word SendMsg(word f,...) { return Err_BadPort; }
extern word XchMsg(MCB *m) { return Err_BadPort; }
extern Port _SysNewPort(void) { return NullPort; }
extern word _SysFreePort(Port p) { return Err_BadPort; }
extern word GetAttributes(Stream *s, Attributes *a) { return GetInfo(s,(void *)a); }
extern word SetAttributes(Stream *s, Attributes *a) { return SetInfo(s,(void *)a,1); }
extern word IsAnAttribute(Attributes *a, Attribute i) { return FALSE; }
extern void AddAttribute(Attributes *a, Attribute i) {}
extern void RemoveAttribute(Attributes *a, Attribute i) {}
extern word GetInputSpeed(Attributes *a) { return -1; }
extern void SetInputSpeed(Attributes *a, word s) {}
extern word GetOutputSpeed(Attributes *a) { return -1; }
extern void SetOutputSpeed(Attributes *a, word s) { }
extern word SetException(WordFnPtr f, byte *data, word size) { return Err_WrongFn; }
extern void NegAcknowledge(Stream *s, word c) {}
extern void Acknowledge(Stream *s, word c) {}
extern word BootLink(word l, void *i, Config *c, word s) { return Err_BadLink; }
extern DCB *OpenDevice(string n, void *info) { return NULL; }
extern word CloseDevice(DCB *d) { return Err_Null; }

#endif
