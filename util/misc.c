/*------------------------------------------------------------------------
--                                                                      --
--                     H E L I O S   N U C L E U S                      --
--                     ---------------------------                      --
--                                                                      --
--             Copyright (C) 1987, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- misc.c								--
--                                                                      --
--	Util Library, bits that dont logically fit anywhere else!	-- 
--	Simplified process creation and IOdebug functions		--
--                                                                      --
--	Author:  NHG 16/8/87						--
--                                                                      --
------------------------------------------------------------------------*/
/* RCSId: $Id: misc.c,v 1.34 1993/08/08 12:25:03 paul Exp $ */
/* Copyright (C) 1987, Perihelion Software Ltd.*/

#ifdef __TRAN
# define STACKUSE 1
#else
# define STACKUSE 0
#endif

#include <stdio.h>
#include <stdarg.h>
#include <helios.h>
#include <syslib.h>
#include <message.h>
#include <task.h>
#include <root.h>
#include <link.h>
#ifdef __TRAN
#include <asm.h>
#endif
#include <codes.h>
#include <module.h>
#include <process.h>
#include <string.h>
#ifndef __TRAN
# include <string.h>
# include <cpustate.h>
# include <setjmp.h>
void _Halt(void);
#endif
#ifdef __C40
#include <signal.h>
#endif

#if defined(STACKCHECK)
#ifdef __TRAN
extern void _Trace(...);
#    pragma -s1

static void _stack_error(Proc *p)
{
	_Trace(0xaaaaaaaa,p);
	IOdebug("Util stack overflow in %s at %x",p->Name,&p);	
}
#endif

	/* Enable stack checking if -DSTACKCHECK			*/
#    pragma -s0

#else

	/* Otherwise disable stack checking.				*/
# ifdef __ARM
#  pragma no_check_stack
# else
#  pragma -s1
# endif

#endif

#ifdef __TRAN
# pragma -f0		/* switch off vector stack	*/
# pragma -g0		/* switch off procedure names	*/
#endif

#if defined(__ARM) || defined(__C40)
#define __IODBUF	/* + define in <root.h> */
#endif


void IOputc(byte c)
{
	byte s[3];
	void IOputs(byte *s);
	if( c == '\n' ) s[0] = '\r',s[1] = '\n',s[2] = 0;
	else s[0] = c, s[1] = 0;
	IOputs(s);
}

void IOputs(byte *s)
{
	MCB mcb;
	RootStruct *root = GetRoot();
	LinkInfo **linkp;
	LinkInfo *debug = NULL;
	Port port;

#ifdef __TRAN
	/* This delay prevents processors sending too many messages	*/
	/* and congesting those nearer to the IO server or logger	*/
	Delay(1000);
#endif

retry:
	/* From V1.2 there is a port in the root structure, which if not */
	/* NullPort, can intercept all IOdebugs. If it is null we look	 */
	/* for a debugging link.					 */
	if( (port = root->IODebugPort) == NullPort )
	{
		linkp = root->Links;
		while( (*linkp != NULL) && ((*linkp)->Flags & Link_Flags_debug) == 0)
			linkp++;
		if( *linkp != NULL ) 
		{
			debug = *linkp;
			port = debug->RemoteIOCPort;
		}
	}

	if ( port == NullPort )
#ifndef __ABC
		return; /* if no debugging link then simply return */
#else
	{
	Object *obj;
	Stream *f;

	if ((obj = Locate(NULL,"/logger")) == NULL)
		return;
	if ((f = Open(obj,NULL,O_WriteOnly)) == NULL)
		return;
	Write(f, s, strlen(s), -1);

	Close(f);
	Close(obj);
	return ;
	}
#endif

	InitMCB(&mcb,MsgHdr_Flags_preserve,port,NullPort,0x22222222);
		
	mcb.Timeout = -1;
	mcb.MsgHdr.DataSize = strlen(s);
	mcb.Data = s;

	/* To support disconnecting IOservers */
	if (PutMsg(&mcb) < 0 )
	{
		if( debug != NULL )
		{
			/* perhaps we should also try to clean up the remoteIOC port? */
			debug->Flags &= ~Link_Flags_ioproc;
			debug->Flags &= ~Link_Flags_debug;
		} else root->IODebugPort = NullPort;
		goto retry;
	}
}

#ifdef __IODBUF
/* Buffered (Fast) IOdebug code: */

void InitIODBuf()
{
	GetRoot()->IODBufPos = 0;
}

void FlushIODBuf()
{
	RootStruct *root = GetRoot();
	
	root->IODBuffer[root->IODBufPos] = '\0';
	IOputs(root->IODBuffer);
	root->IODBufPos = 0;
}

void BufIOputc(byte c)
{
	RootStruct *root = GetRoot();

	if (root->IODBufPos >= 78) /* if at end of buffer, force flush */
		FlushIODBuf();

	if( c == '\n' )
		root->IODBuffer[root->IODBufPos++] = '\r';

	root->IODBuffer[root->IODBufPos++] = c;
}

void BufIOputs(byte *s)
{
	while( *s )
		BufIOputc(*s++);
}

#else
#define BufIOputc IOputc
#define BufIOputs IOputs
#endif


#if 0

word Fork(word stsize, VoidFnPtr fn, word argsize, ... )
{
	byte *args = ((byte *)&argsize) + sizeof(argsize);
	byte *process = (byte *)NewProcess(stsize, fn, argsize);

	if( process == Null(byte) ) return false;

	memcpy(process,args,argsize);
#ifdef __TRAN
	RunProcess((void *)(((word)process)|1)); /* oring with one sets lopri */
#else
	/* run child at parents priority */
	ExecProcess((void *)process,GetPriority());
#endif
	return true;
}

#else

void _ProcExit(void);		/* only part in assembler */
void FreeStop(void *);

/* stack tracking is specific to transputer at present */
/* Helios/ARM support is incomplete */
#if STACKUSE && defined(__TRAN)

# ifdef __TRAN
void _ProcHalt(void)
{
	_Halt();
}
#  define StackInitVal	((int)_ProcHalt)
# else
#  define StackInitVal	(0xf0000000)
# endif

# ifdef __TRAN
/* why, as its already saved in display? */
void __ProcExit(word *stackbase, word fn)
# else
void __ProcExit(word *stackbase)
# endif
{
# if 1
	if( MyTask->Flags & Task_Flags_fork )
	{
		int vused = 0;
		word sused = 0;
		int unused = 0;
		word *v = stackbase;
		word *s = &sused;
		word *p;
		word *display = (&stackbase)[-1]; /* addr of parameter??? */
		word *stackend = (word *)display + 6;
#  ifndef __TRAN
		char *proc = (char *)(display[2]);
#  endif
		while( s > stackbase && *s != StackInitVal )s--;
			
		while( v < s && *v != StackInitVal )v++;
		
		for( p = v; p < s; p++ )
		{
			if( *p != StackInitVal )
			{
				if( p-v < s-p ) v=p;
				else s=p;
			}
		}

		sused  = stackend - s;
		unused = s - v;
		vused  = v - stackbase;

#  if defined(__ARM)
		proc -= 4; /* point to RPTR */
		/* make proc point at functions ASCII name */
#   ifdef __ARM
		proc += *(word *)proc;
#   else	
		proc -= (*(word *)proc) & 0x00ffffff;
#   endif
		IOdebug("%s: %d %d %d",proc,vused,unused,sused);
#  else
		IOdebug("ProcExit: thread stack use %d %d %d",vused,unused,sused);
#  endif
	}
# endif
	FreeStop((void *)stackbase);

	return;

#ifdef __TRAN
	fn = fn;
#endif
}

#else /* STACKUSE and __TRAN */

void __ProcExit(word *stackbase)
{
	FreeStop((void *)stackbase);
}

#endif /* not STACKUSE and __TRAN */


#if defined(__ARM) || defined(__C40)
word *_GetModTab(void);		/* kernel asm */
#endif

void *NewProcess(word stacksize, VoidFnPtr fn, word nargs)
{
#ifdef __TRAN
	word *stack   = (word *)Malloc(stacksize);
	word *display = stack + (stacksize/sizeof(word)) - 6; /* display is 6 words */
#else
	/* add sizeof(SaveState) to give a level of conformity in stack sizes */
	word *stack   = (word *)Malloc(stacksize + sizeof(SaveState));
	word *display = stack + (stacksize/sizeof(word)) - 2; /* to save modtab and stack ptr */
#endif
	word *p;
#if STACKUSE && defined (__TRAN)
	int i;
#endif
	if( stack == NULL )
#ifdef SYSDEB
	{
		IOdebug("Malloc in NewProcess failed to allocate stacksize %x", stacksize);
		return NULL;
	}
#else
	return NULL;
#endif
#if STACKUSE && defined (__TRAN)
	for( i = 0; i < stacksize/sizeof(word); i++ ) stack[i] = StackInitVal;
#endif

#ifdef __TRAN
	display[0] = (((word **)&stacksize)[-1])[0]; 	/* transputer magic */
#else
	display[0] = (word)_GetModTab();
#endif
	display[1] = (word)stack;
#ifdef __TRAN
	display[2] = (word)fn;
#endif

	if( nargs < 8 ) nargs = 8;	/* leave at least 2 words */

	p = InitProcess(display,fn,_ProcExit,display,nargs);

#if 0
	/* useful if used in conjunction with PFork and FreeStack to find */
	/* threads stack has been corrupted */
	IOdebug("NewProc stack %a", stack);
#endif
	return p;
}

void ZapProcess(void *p)
{
#ifdef __TRAN
	/* The following is transputer magic	*/
	word **pp = (word **)p;
	Free((int *)((pp[-1])[1]));

#elif defined(__C40) || defined(__ARM)
	/* assume less (but less portable) */
	/* CreateProcess seeds v1(r4) with pointer to stack base */

	Free((void *)(((SaveState *) ((word *)p-1) - 1) ->CPUcontext.R_V1));
#else
#	error "Unknown processor"

	/* could try: */
	word *sb = (word *)p;
	/* assumes argsize at (word)p[-1] (placed by Create/InitProcess */
	/* then adds argsize to this to point at top of stack */
	/* the top of stack[1] holds a pointer to the base */
	/* assumes argsize is a word count */
	Free((void *) sb[ sb[-1] + 1 ]); /* +2 if save fn in display */
#endif
}

void ExecProcess(void *p, word logpri)
{
	StartProcess((word *)p, LogToPhysPri(logpri));
}


/* compatability fn - superceeded by ExecProcess */
void RunProcess(void *p)
{
/* N.B. on the transputer oring 1 with *p sets the priority level to low */

	StartProcess((word *)p, LogToPhysPri(StandardPri));
}


/* N.B. in Helios/ARM/C40 caller should realise argsize must be defined in */
/* terms of word multiples, even though specified in bytes */
word Fork(word stsize, VoidFnPtr fn, word argsize, ... )
{
	byte *process = (byte *)NewProcess(stsize, fn, argsize);
#ifdef __TRAN
	byte *args = ((byte *)&argsize) + sizeof(argsize);
#else

#define FORKMAXARGSIZE (32*4)	/* allows 32 word args max */

	word args[FORKMAXARGSIZE/sizeof(word)]; /* max size of args in non transputer version */
	va_list	argp;
	int i;

#ifdef SYSDEB
	if (process == NULL)
		IOdebug("NewProcess failed in Fork");
#endif
	if ( argsize > FORKMAXARGSIZE)
		return false;

	va_start(argp, argsize);

	for (i = 0;  i < (argsize / sizeof(word)) ; i++)
		args[i] = va_arg(argp, word); /* get args */

	va_end(argp);
#endif

	if( process == NULL ) return false;

	memcpy(process,args,(int)argsize);
	
	/* run child at same priority as parent */
	StartProcess((word *)process, LogToPhysPri(GetPriority()));

	return true;
}

#endif

/* Only transputer and ABC Hercules version of ARM have fast RAM */
/* @@@ will need special code for the C40 to copy between code and data spaces */
#if defined(__TRAN) || defined (__ABC)
word AccelerateCode(VoidFnPtr p)
{
	Module *m;
	Module *module;
	word *x;
	Carrier *c;
	int i;
	fncast ansibodge; /* HELIOSARM fix */
	word *curinit;

	/* get address of function */
	ansibodge.vfn = p;
	x = ansibodge.wp;

	/* first find start of module */
	until( *x == T_Module ) x--;
	m = (Module *)x;

	/* allocate space for module */
	c = AllocFast(m->Size,&MyTask->MemPool);

	if( c == NULL ) return EC_Error|SS_Kernel|EG_NoMemory;
	
	/* copy whole module into fast RAM */
	memcpy(c->Addr,m,m->Size);
	
	module = (Module *)c->Addr;
	
	/* now call init routines in module to install new procedure	*/
	/* pointers in module table.					*/
	
#ifdef __SMT
	/* Just do code initialisation (init param = 2) */
	curinit = &module->Init;
	while( *curinit != 0 )
	{
		curinit = (word *)RTOA(*curinit);
		ansibodge.wp = curinit+1;
		(*ansibodge.wfn)(2,0);
	}
#else
	for( i = 0; i <= 1 ; i++ )
	{
		curinit = &module->Init;
		
		while( *curinit != 0 )
		{
			curinit = (word *)RTOA(*curinit);
			ansibodge.wp = curinit+1;
			(*ansibodge.wfn)(i,0);
		}
	}
#endif

	return 0;
}
#else /* not __TRAN or __ARM */
#if defined __C40
word
AccelerateCode( VoidFnPtr p )
{
  extern MPtr	_grab_module( VoidFnPtr, Module * );
  extern void	_copy_module( MPtr, MPtr );
  extern void	_init_module( MPtr, int );
  
  MPtr		m;  
  Module	module;
  Carrier *	c;
  

  /*
   * make sure that we have been passed a valid pionter
   */
  
  if (p == NULL)
    {
      return EC_Error | SS_Kernel | EG_Parameter;
    }      

  /*
   * Get a copy of the module structure associated
   * with function pointer 'p'
   */
  
  m = _grab_module( p, &module );

  /*
   * Allocate space for a copy of the module.
   */
  
  c = AllocFast( module.Size, &MyTask->MemPool );
  
  if (c == NULL)
    {
      return EC_Error | SS_Kernel | EG_NoMemory;
    }
  
  /*
   * Copy whole module into the allocated fast RAM.
   */

  _copy_module( m, c->Addr );

  /*
   * Call the init routines in the copied module to install
   * a new set of function pointers in the module table.
   */

  _init_module( c->Addr, 2 ); 
  
  /*
   * Finished.
   */
  
  return 0;
  
} /* AcclerateCode */
#endif /* __C40 */
#endif /* __ARM, __TRAN */

/*---------------------------------------------------------------------------*/

#if defined(__ABC)
/* "AccelerateCode" copies the function into the FastRAM. This is only useful
 * for very small functions/modules that will fit in the limited FastRAM space.
 * This function "SpeedUpCode" will copy a function into normal RAM
 * (duplicating the function code if it is already RAM based). This allows
 * large modules to be copied into normal RAM.
 * These functions will stay RAM bound until the task terminates.
 */
word SpeedUpCode(VoidFnPtr p)
{
	Module *	m;
	Module *	module;
	word *		x;
	byte *		c;
	fncast		ansibodge;
	word *		curinit = &module->Init;
	
	ansibodge.vfn = p;
	x = ansibodge.wp;

	/* first find start of module */
	until (*x == T_Module)
		x--;
	m = (Module *)x;

	/* allocate space for module */
	c = Malloc(m->Size);

	if (c == NULL)
		return(EC_Error | SS_Kernel | EG_NoMemory);
	
	/* copy whole module into fast RAM */
	memcpy(c,m,m->Size);
	
	module = (Module *)c;
	
	/* now call init routines in module to install new procedure	*/
	/* pointers in module table.					*/
	
#ifdef __SMT
	/* Just do code initialisation (init param = 2) */
	curinit = &module->Init;
	while (*curinit != 0) {
		curinit = (word *)RTOA(*curinit);
		ansibodge.wp = curinit + 1;
		(*ansibodge.wfn)(2,0);
	}
#else
	{
		int	i;

		for (i = 0; (i <= 1); i++) {
			*curinit = &module->Init;
	
			while (*curinit != 0) {
				curinit = (word *)RTOA(*curinit);
				ansibodge.wp = curinit + 1;
				(*ansibodge.wfn)(i,0);
			}
		}
	}
#endif

	return(0);
}
#endif

/*---------------------------------------------------------------------------*/

/* static array moved outside of fn to get around Helios/ARM compiler bug */
static char *digits = "0123456789abcdef"; 

static void writenum1(unsigned int i, int base )
{
	if( i == 0 ) return;
	writenum1(i/base,base);
	BufIOputc(digits[i%base]);
}

static void writenum(int i, int base)
{
	if( i<0 && base==10 ) { i=-i; BufIOputc('-'); }
	if( i == 0 ) BufIOputc('0');
	else writenum1(i,base);
}

static void writestr(char *s)
{
	BufIOputs(s);
}

void IOdebug(const char *str, ... )
{
	int base, i, *p;
	char *s, *t;
	va_list a;
	RootStruct *root = GetRoot();
	Object *o;
	Stream *sp;

	Wait(&root->IODebugLock);		

#ifdef __IODBUF
	InitIODBuf();
#endif
	va_start(a,str);

	while( *str ) 
	{
		if( *str == '%' )
		{
			char fch = *(++str);
			base = 10;
			switch( fch )
			{
			default:
				BufIOputc(*str); 
				break;

			case 0:				/* "...%" = no \n */
				goto nonl;
								
			case 'c':			/* char		*/
				i = va_arg(a,int);
				BufIOputc(i);
				break;

#ifdef __C40
			case 'a':
				base = 16;		/* hex 		*/
				i = va_arg(a,int);
				BufIOputc('0');
				BufIOputc('x');
				writenum(i,base);
				i = (int)C40WordAddress((char *)i);
				BufIOputc(' ');
				BufIOputc('[');
				BufIOputc('W');
				BufIOputc('P');
				BufIOputc(':');
				writenum(i,base);
				BufIOputc(']');
				break;

#endif
			case 'x': base = 16;		/* hex 		*/
							/* @@@ add # support */
			case 'd':			/* decimal	*/
				i = va_arg(a,int);
				writenum(i,base);
				break;
				
			case 'N':			/* pathname	*/
			case 's':			/* string	*/
				s = va_arg(a,char *);
				if( s == NULL ) s = "<NULL string>";
			putstr:
				writestr(s);
				break;

			case 'o':
				s = va_arg(a,char *);
				t = s + strlen(s);
				while( t > s && *(t-1) != '/') t--;
				s = t;
				goto putstr;
				
			/* The following are special formats for various*/
			/* Helios-specific things			*/
			
			case 'A':			/* access mask	*/
			case 'E':			/* error code	*/
			case 'P':			/* pointer	*/
			case 'T':			/* type  	*/
			case 'X':			/* matrix	*/
				i = va_arg(a,int);
				writenum(i,16);
				break;
				
			case 'C':			/* capability	*/
				p = va_arg(a,int *);
				writenum(p[0],16); BufIOputc(' ');
				writenum(p[1],16);
				break;
				
			case 'M':			/* mcb		*/
				p = va_arg(a,int *);
				writenum(p[0],16); BufIOputc(' ');
				writenum(p[1],16); BufIOputc(' ');
				writenum(p[2],16); BufIOputc(' ');
				i = p[3]; goto putfn;
							
			case 'O':			/* object	*/
			case 'S':			/* stream	*/
				o = va_arg(a,Object *);
				if( (o == NULL && fch == 'O') ||
				    (o->Flags & Flags_Stream) == 0 )
				{
					writestr("<Object: ");
					if( o == NULL ) writestr("NULL");
					else 
					{
						writenum((int)o->Type, 16);
						BufIOputc(' ');
						writestr((char *) &o->Name);
					}
					BufIOputc('>');
					break;
				}
				else
				{
					sp = (Stream *)o;
					writestr("<Stream: ");
					if( sp == NULL ) writestr("NULL");
					else 
					{
						writenum((int)sp->Type,16);
						BufIOputc(' ');
						writenum((int)sp->Flags,16);
						BufIOputc(' ');
						writestr((char *)&sp->Name);
					}
					BufIOputc('>');				
					break;
				}

			case 'F':			/* function code*/
			{
				i = va_arg(a,int);
			putfn:
#ifdef SYSDEB
				switch( i & FG_Mask )
				{
				case FG_Open: 		s = "FG_Open"; goto putstr;
				case FG_Create: 	s = "FG_Create"; goto putstr;
				case FG_Locate: 	s = "FG_Locate"; goto putstr;
				case FG_ObjectInfo: 	s = "FG_ObjectInfo"; goto putstr;
				case FG_ServerInfo: 	s = "FG_ServerInfo"; goto putstr;
				case FG_Delete: 	s = "FG_Delete"; goto putstr;
				case FG_Rename: 	s = "FG_Rename"; goto putstr;
				case FG_Link: 		s = "FG_Link"; goto putstr;
				case FG_Protect: 	s = "FG_Protect"; goto putstr;
				case FG_SetDate: 	s = "FG_SetDate"; goto putstr;
				case FG_Refine: 	s = "FG_Refine"; goto putstr;
				case FG_CloseObj: 	s = "FG_CloseObj"; goto putstr;
				case FG_Read: 		s = "FG_Read"; goto putstr;
				case FG_Write: 		s = "FG_Write"; goto putstr;
				case FG_GetSize: 	s = "FG_GetSize"; goto putstr;
				case FG_SetSize: 	s = "FG_SetSize"; goto putstr;
				case FG_Close: 		s = "FG_Close"; goto putstr;
				case FG_Seek: 		s = "FG_Seek"; goto putstr;
				case FG_GetInfo: 	s = "FG_GetInfo"; goto putstr;
				case FG_SetInfo: 	s = "FG_SetInfo"; goto putstr;
				case FG_EnableEvents: 	s = "FG_EnableEvents"; goto putstr;
				case FG_Acknowledge: 	s = "FG_Acknowledge"; goto putstr;
				case FG_NegAcknowledge:	s = "FG_NegAcknowledge"; goto putstr;
				case FG_Search: 	s = "FG_Search"; goto putstr;
				case FG_MachineName: 	s = "FG_MachineName"; goto putstr;
				case FG_Debug: 		s = "FG_Debug"; goto putstr;
				case FG_Alarm: 		s = "FG_Alarm"; goto putstr;
				case FG_Reconfigure: 	s = "FG_Reconfigure"; goto putstr;
				case FG_SetFlags: 	s = "FG_SetFlags"; goto putstr;
				case FG_SendEnv: 	s = "FG_SendEnv"; goto putstr;
				case FG_Signal: 	s = "FG_Signal"; goto putstr;
				case FG_ProgramInfo: 	s = "FG_ProgramInfo"; goto putstr;
				case FG_BootLink: 	s = "FG_BootLink"; goto putstr;
				case FG_FollowTrail: 	s = "FG_FollowTrail"; goto putstr;
				default:
					writenum(i,16);
					break;
				} 
#else
					writenum(i,16);
					break;
#endif
			}
			
			}
			str++;
		}
		else BufIOputc(*str++);
	}
	
	BufIOputc('\n');
nonl:
	va_end(a);

#ifdef __IODBUF
	FlushIODBuf();
#endif
	Signal(&root->IODebugLock);
}


#if 0
/* experimental C based setjmp/longjmp fns */
int setjmp(jmp_buf env)
{
	return(SaveCPUState((SaveState *)env));
}

/* Danger: The following code will probably jump back through a corrupt */
/* stack frame as the original call to setjmp will have returned */
void longjmp(jmp_buf env, int ret)
{
	/* Following statement is ok as spec says that RestoreCPUState */
	/* mustn't touch reg contents */
	((SaveState *)env)->a1 = ret;
	RestoreCPUState((SaveState *)env);
}
#endif /* NEVER */


#ifdef __C40
#pragma no_stack_checks

/*
 * Function:
 *	back_trace
 *
 * Arguments:
 *	none 
 *
 * Description:
 *	prints out a trace of all the functions on the stack
 *	starting with the parent of the function calling this function
 *
 * Returns:
 *	nothing
 */

void
back_trace( void )
{
  extern int 	_backtrace( char *, int );
  char		func_name[ 80 ];	/* XXX - beware of assumption of maximum size of function name */
  int		fp;
  

  IOdebug( "Execution back trace ..." );

  /* get name of our parent, and initialise backtrace */

  if ((fp = _backtrace( func_name, NULL )) == NULL)
    {
      IOdebug( "<Failed to locate initial parent function>" );

      return;
    }

  /* then for every parent function ... */
  
  forever
    {
      int *	ptr;

      
      /* announce ourselves */

      IOdebug( func_name );
            
      /* if we have reached main we have finished */
      /* Fork()ed functions terminate at _ProcExit() */

      ptr = (int *)func_name;

      if (( ptr[ 0 ]             == ('_' | 'm' << 8 | 'a' << 16 | 'i' << 24) &&
	   (ptr[ 1 ] & 0xFFFF)   == ('n' | '\0' << 8))
	  ||
	  ( ptr[ 0 ]             == ('m' | 'a' << 8 | 'i' << 16 | 'n' << 24) && 
	   (ptr[ 1 ] & 0xFF)     == ('\0'))
	  ||
	  ( ptr[ 0 ]             == ('.' | '_' << 8 | 'P' << 16 | 'r' << 24) &&
	    ptr[ 1 ]             == ('o' | 'c' << 8 | 'E' << 16 | 'x' << 24) &&
	   (ptr[ 2 ] & 0xFFFFFF) == ('i' | 't' << 8 | '\0' << 16)))
	break;
            
      /* discover name of parent */
      
      if ((fp = _backtrace( func_name, fp )) == NULL)
	{
	  IOdebug( "<Failed to locate parent function>" );

	  break;
	}
    }
  
  IOdebug( "Execution back trace finished" );
  
  /* finished */
  
  return;	

} /* back_trace */


/*
 * --------------------- Memory Access Check Functions --------------------
 *
 *
 * These functions are called as a result of placing
 *
 *   #pragma memory_access_checks
 *
 * in a file.  They test the pointer that is about to
 * be used for memory access to see if it is legal.
 */

 
#pragma no_check_memory_accesses

#include <memory.h>
#include <config.h>
#include <task.h>


static void *
check_access(
	     void *	ptr,		/* the pointer to be checked			*/
	     int	alignment,	/* mask of zero bits at bottom of pointer	*/
	     bool	write,		/* true if pointer is used for a write	*/
	     char *	ptr_type )	/* string describing the thing pointed to	*/
{
  extern word *		GetExecRoot( void );
  int			return_address;
  char *		start = "Attempting to "; /* helps reduce code size */
  
  
  /*
   * Please ignore this horrible piece of code ...
   */
  
  return_address = *(&return_address + 6);
  
  if (ptr == NULL)
    {
      IOdebug( "%s%s a %s via a NULL pointer", start,
	       write ? "write" : "read", ptr_type );
    }
  else if ((signed int)ptr >= -128 && (int)ptr <= 128)
    {
      /* catch magic UNIX pointers */
      
      IOdebug( "%s%s a %s via a very small pointer (%x)", start,
	       write ? "write" : "read", ptr_type, ptr );
    }
#if 0	/* for unknown reasons this test crashes Helios ... */
  else if (ptr < (void *)(GetConfig() + 1))
    {
      IOdebug( "%s%s a %s with a pointer (%x) below the start of memory", start,
	       write ? "write" : "read", ptr_type, ptr );
    }
#endif
  else if (alignment && ((int)ptr & alignment) != 0)
    {
      IOdebug( "%s%s a %s with a non aligned pointer (%x)", start,
	       write ? "write" : "read", ptr_type, ptr );
    }
  else if (!write)
    {
      /* for now do not bother with any further checks if we are reading */
      
      return ptr;
    }
  else if (C40WordAddress(ptr) >= GetNucleusBase() &&
	   C40WordAddress(ptr) <  (GetNucleusBase() + MP_GetWord(GetNucleusBase(),0) / sizeof (word)))
    {
      if (write)
	IOdebug( "%swrite a %s into the nucleus! (at %x)", start, ptr_type, ptr );
      else
	return ptr;
    }
  else if (ptr >= (void *)GetRoot() &&		/* access to root struct */
	   ptr <  (void *)(GetRoot() + 1))
    {
      return ptr;
    }
  else if (ptr >= (void *)GetConfig() &&	/* access to config struct */
	   ptr <  (void *)(GetConfig() + 1))
    {
      return ptr;
    }  
#if 1 /* while we check the kernel */
  else if (ptr >= (void *)GetExecRoot() &&	/* access to executives root struct */
	   ptr <  (void *)(GetExecRoot() + 1))
    {
      return ptr;
    }  
#endif
  else if (MyTask != NULL                       &&
	   !InPool( ptr, &(MyTask -> MemPool) ) &&
#if 0
	   (write || !InPool( ptr, GetRoot()->LoaderPool )) &&
#else /* while we check the nucleus! */
	   (!InPool( ptr, GetRoot()->LoaderPool )) &&
	   (!InPool( ptr, &(GetRoot()->SysPool) )) &&
#endif
	   (ptr <  (void *)MyTask ||
	    ptr >= (void *)(MyTask + 1)))
    {
      IOdebug( "%s%s a %s with a pointer (%x) outside of valid memory", start,
	       write ? "write" : "read", ptr_type, ptr );

#if 0
	{
	  Memory * block = Head_(Memory, MyTask->MemPool.Memory);

	  
	  while ( !EndOfList_(block) )
	    {
	      void *	start;
	      void *	end;

	      
	      start = (void *)block;
	      end   = (void *)((byte *)block + (block->Size & ~Memory_Size_BitMask));

	      IOdebug( "start = %x, end = %x", start, end );
	  
	      block = Next_(Memory,block);
	    }
	}
#endif
    }
  else
    {
      return ptr;
    }

#ifdef SYSDEB
  IOdebug("memory access violation MIGHT have occured at %x", return_address );
#endif
  
  back_trace();

  /* generate a segmentation violation signal */

  if (MyTask != NULL)
    {
#ifdef SYSDEB
      IOdebug("sending exception" );
#endif
      
      CallException( MyTask, SIGSEGV );

#ifdef SYSDEB
      IOdebug("sent exception" );
#endif
    }
  else
    {
      IOdebug( "task pointer is NULL!" );
    }  

  return ptr;  
  
} /* check_access */

#pragma enable_backtrace


/*
 * Function:
 *	_wr1chk
 *
 * Arguments:
 *	Byte pointer to be checked 
 *
 * Description:
 *	Checks that the pionter argument is a valid pointer for write operations.
 *	Prints out an execution backtrace if the pointer is invalid.
 *
 * Returns:
 *	Validated pointer
 */

char *
_wr1chk( char * ptr )
{
#if 1
  return (char *)check_access( ptr, 0, TRUE, "byte" );
#else
  char *	c;
  word		a;

  
  a = SetPhysPri( 0 );
  c = check_access( ptr, 0, TRUE, "byte" );
  SetPhysPri( a );

  return c;
#endif

} /* _wr1chk */


/*
 * Function:
 *	_rd1chk
 *
 * Arguments:
 *	Byte pointer to be checked 
 *
 * Description:
 *	Checks that the pionter argument is a valid pointer for read operations.
 *	Prints out an execution backtrace if the pointer is invalid.
 *
 * Returns:
 *	Validated pointer
 */

char *
_rd1chk( char * ptr )
{
  return (char *) check_access( ptr, 0, FALSE, "byte" );
  
} /* _rd1chk */


/*
 * Function:
 *	_wr2chk
 *
 * Arguments:
 *	Short pointer to be checked 
 *
 * Description:
 *	Checks that the pointer argument is a valid pointer for write operations.
 *	Prints out an execution backtrace if the pointer is invalid.
 *
 * Returns:
 *	Validated pointer
 */

short int *
_wr2chk( short int * ptr )
{
  return (short int *)check_access( ptr, 0x1, TRUE, "short" );
  
} /* _wr2chk */


/*
 * Function:
 *	_rd2chk
 *
 * Arguments:
 *	Short pointer to be checked 
 *
 * Description:
 *	Checks that the pointer argument is a valid pointer for read operations.
 *	Prints out an execution backtrace if the pointer is invalid.
 *
 * Returns:
 *	Validated pointer
 */

short int *
_rd2chk( short int * ptr )
{
  return (short int *) check_access( ptr, 0x1, FALSE, "short" );
  
} /* _rd2chk */


/*
 * Function:
 *	_wr4chk
 *
 * Arguments:
 *	Word pointer to be checked 
 *
 * Description:
 *	Checks that the pointer argument is a valid pointer for write operations.
 *	Prints out an execution backtrace if the pointer is invalid.
 *
 * Returns:
 *	Validated pointer
 */

long int *
_wr4chk( long int * ptr )
{
  return (long int *)check_access( ptr, 0x3, TRUE, "word" );
  
} /* _wr4chk */


/*
 * Function:
 *	_rd4chk
 *
 * Arguments:
 *	Word pointer to be checked 
 *
 * Description:
 *	Checks that the pointer argument is a valid pointer for read operations.
 *	Prints out an execution backtrace if the pointer is invalid.
 *
 * Returns:
 *	Validated pointer
 */

long int *
_rd4chk( long int * ptr )
{
  return (long int *)check_access( ptr, 0x3, FALSE, "word" );
  
} /* _rd4chk */

#endif /* __C40 */


static char procnamebuf[ 256 ];

#ifdef __C40

char *
procname( VoidFnPtr fn )
{
  if (fn == NULL)
    {
      int x = _backtrace( procnamebuf, NULL );

      
      /* find the name of the function that called our parent */
      
      if (x == NULL || _backtrace( procnamebuf, x ) == NULL)
	return "Unknown Function";
    }
  else
    {
      fncast 	fc;
      MPtr	x;
      word	s;

      
      /* find the name of the function that is passed as arg */
      
      fc.vfn = fn;
      x      = (MPtr)fc.w;
      
      x      = MInc_(  x, -4 );
      s      = MWord_( x,  0 );

      /* check to see if function names have been compiled in */
      
      if ((s & 0xFF000000) == 0xFF000000)
	{
	  x      = MInc_(  x, -s );
      
	  MData_( procnamebuf, x, 0, sizeof (procnamebuf) );
	}
      else
	{
	  char *	ptr;

	  
	  /* scan backwards for Module structure */
	  
	  while (MWord_( --x, 0 ) != T_Module)
	    ;

	  /* NB/ string MUST be a word multiple in length */
	  /*                    1234123412341234 */
	  strcpy( procnamebuf, "<unknown fn> in " );

	  ptr = procnamebuf + strlen( procnamebuf );
	  
#ifdef PARANOIA
	  /* catch buffer overflow */
	  
	  if (ptr - procnamebuf < 33)
	    {
	      ptr = procnamebuf;
	    }
#endif
	  /* copy in name of module */
	  
	  ModuleName_( ptr, x );

	  /* ensure that buffer is terminated */
	  
	  ptr[ 32 ] = '\0';	  
	}      
    }

  return procnamebuf;
  
} /* procname */

#else /* not __C40 */

char *
procname( VoidFnPtr fn )
{
	MPtr x;
	int  deadman = 1024 * 64;

# ifdef __ARM
	x = (MPtr)(((int)fn)&~0xfc000003); /* remove status bits */
# else
	x = (MPtr)(((int)fn)&~3);
# endif	
	while( ((MWord_(x,0) & T_Mask) != T_Valid) && --deadman && x)
		x = MInc_(x,-4);

	if (!deadman)
		return ("<name not found>");

	switch( MWord_(x,0) )
	{
	case T_Proc:
		MData_(procnamebuf,x,offsetof(Proc,Name),sizeof(procnamebuf));
		return procnamebuf;
	case T_Module:
	case T_Program:
		ModuleName_(procnamebuf,x);	
		return procnamebuf;
	}

	return("<Unknown type>");
}
#endif /* not __C40 */


/* end of misc.c */
