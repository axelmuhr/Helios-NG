/*
 * File:	gdebug.c
 * Subsystem:	Generic Helios executive
 * Author:	P.A.Beskeen
 * Date:	Jan '92
 *
 * Description: Generic Helios executive KDebug() debugging function.
 *
 *		Three debugging message strategies for different
 *		stages of the kernel port are available:
 *
 *		KERNELDEBUG1
 *		This level simply throws text at the link and should be used
 *		with early debugging harnesses. It will block until the text
 *		is sent.
 *
 *		KERNELDEBUG2
 *		This sends IODebug 'look alike' messages directly down the link.
 *		This is for use *before* the link guardians get going. It will
 *		block until the message is sent out.
 *
 *		KERNELDEBUG3
 *		This last form of debugging is for when the link guardians are
 *		up and going, It will not interfere with other link traffic
 *		and it will not cause the caller to block - Hence it can be
 *		used by all parts of the kernel. This strategy can use large
 *		amounts	of memory for its buffer though, so shouldn't be
 *		enabled in production systems.
 *
 *		To stop debug recursion, No KDebug()'s are allowed in _PutMsg
 *		and _Signal and all functions called from these.
 *
 *		The code is based on the IOdebug code in /hsrc/util/misc.c. The
 *		KDebug() function taking the same parameters.
 *
 *		@@@ The implementation of KERNELDEBUG3 should be changed so
 *		there is no shared buffer. The use of HIGHPRI is Ok, but by
 *		removing the use of KDBuffer[], we could also call KDebug()
 *		from within interrupt routines.
 *
 * RcsId: $Id: gdebug.c,v 1.9 1993/08/12 08:43:26 nickc Exp $
 *
 * (C) Copyright 1991 Perihelion Software Ltd.
 * 
 * RcsLog: $Log: gdebug.c,v $
 * Revision 1.9  1993/08/12  08:43:26  nickc
 * fixed compile time warning
 *
 * Revision 1.8  1993/08/11  09:43:59  nickc
 * added bigendian support and improved format interpretation
 *
 * Revision 1.7  1992/11/12  16:58:36  paul
 * updated parameters to c40wordAddress and fixed KDebug as a permenent, not just
 * SYSDEB feature
 *
 * Revision 1.6  1992/09/22  13:13:34  paul
 * changed name of ExecRoot to GetExecRoot()
 *
 * Revision 1.5  1992/09/21  10:37:11  paul
 * moved the generic executive from the C40 directory to the main kernel
 * dir, so that it can be shared with the ARM implementation
 *
 * Revision 1.4  1992/06/23  19:55:26  paul
 * removed some uneeded #ifs
 *
 * Revision 1.3  1992/05/20  16:59:53  nickc
 * minior cosmetic change
 *
 * Revision 1.2  1992/05/20  12:54:36  paul
 * uses #define for size of KDebug buffer in ExecRoot structure
 *
 * Revision 1.1  1992/04/21  09:54:56  paul
 * Initial revision
 *
 *
 */

/* Include Files: */
#include "kernel.h"
#include <stdio.h>
#include <syslib.h>
#include <stdarg.h>
#include <helios.h>
#include <root.h>
#include <codes.h>
#include <message.h>


#ifdef KERNELDEBUG1
void KDebugSend(char *txt, word size)
{
	/* Simple debug, just uses low level Output() fn */
	/* cannot be used once nucleus is run with IOServer */
	txt[size] = 0;
	Output(txt);
}
#endif


#ifdef KERNELDEBUG2

# ifdef __C40
#  define DEBUGLINK	0x100070
# elif defined(__ARM)
#  if 0
#   define DEBUGLINK	0x03342000	/* Archi slot 0 link adapter */
#  else
#   define DEBUGLINK	0x02000020	/* PID serial */
#  endif
# else
#  error DEBUGLINK link intentifier not set!
# endif

void KDebugSend(char *txt, word size)
{
	/* fairly simple debug, can be used when IOServer is up and */
	/* running, but before full link guardians are up and running */
	/* assumes _LinkTx() writes and blocks on link */

	MsgHdr	mh;
	word	proto = Proto_Msg;

	mh.DataSize = (short)size;
	mh.ContSize = 0;
	mh.Flags = 0;
	mh.Dest = 0x12345678;
	mh.Reply = 0xeeee;
	mh.FnRc = 0x22222222;

#  ifdef __C40
	_LinkTx(4, DEBUGLINK, &proto);
	_LinkTx(sizeof(MsgHdr), DEBUGLINK, &mh);
	_LinkTx((size + 3) & ~3, DEBUGLINK, txt);
#  else
#   ifdef __BIGENDIAN
	_LinkTx(1, (Channel)DEBUGLINK, ((char *)&proto) + 3);
#   else
	_LinkTx(1, (Channel)DEBUGLINK, &proto);
#   endif
	_LinkTx(sizeof(MsgHdr), (Channel)DEBUGLINK, &mh);
	_LinkTx(size, (Channel)DEBUGLINK, txt);
#  endif
}
#endif


#ifdef KERNELDEBUG3
/* A Fully featured Implementation of KDebug.  It sends out debug messages */
/* that queue properly for use of the link like any other message. This debug */
/* can be used safely once the link guardians have been started, and can be */
/* called without fear of blocking the calling thread. */
/* As KDebugSend() is running at high pri, the manipulation of the text dump */
/* area pointers is safe */

void KDebugSend(char *txt, word size)
{
	ExecRoot	*xroot = GetExecRoot();

	txt[size] = 0;

	while (*txt) {
		*xroot->KD3EndM++ = *txt++;

		if (xroot->KD3EndM >= xroot->KD3EndB) {
			xroot->KD3EndM = xroot->KD3StartB;
			if (xroot->KD3EndM == xroot->KD3StartM) {
				xroot->KD3EndM = xroot->KD3EndB - 1;
				*(xroot->KD3EndM - 1) = '?';
				break;
			}
		}
		else if (xroot->KD3EndM == xroot->KD3StartM) {
			--xroot->KD3EndM;
			*(xroot->KD3EndM - 1) = '?';	/* note overflow */
			break;
		}
	}

	_Signal(&xroot->KD3Sem);
}

/* Package and send IOdebug look alike message to debug port */
void DbgMsg(char *buf, word count) {
	RootStruct *root = GetRoot();
	MCB mcb;
	LinkInfo **linkp;
	LinkInfo *debug = NULL;
	Port port;

	/* From V1.2 there is a port in the root structure, which if not */
	/* NullPort, can intercept all KD/IOdebugs. If it is null we look */
	/* for a debugging link.					 */
	if( (port = root->IODebugPort) == NullPort ) {
		linkp = root->Links;
		while( (*linkp != NULL) && ((*linkp)->Flags & Link_Flags_debug) == 0)
			linkp++;
		if( *linkp != NULL ) {
			debug = *linkp;
			port = debug->RemoteIOCPort;
		}
	}

	if ( port == NullPort ) {
		_Trace(0xdebbdebb, 0, (word)GetExecRoot()->KDBuffer);
		return; /* if no debugging link then simply return */
	}

	*(word *)&mcb = 0;
	mcb.MsgHdr.Flags = MsgHdr_Flags_preserve;
	mcb.MsgHdr.Dest = port;
	mcb.MsgHdr.Reply = NullPort;
	mcb.MsgHdr.FnRc = 0x22222222;
	mcb.Timeout = -1;
	mcb.MsgHdr.DataSize = (short)count;
	mcb.Data = buf;

	/* Only place where no KDebug()'s are allowed is in _PutMsg and all */
	/* functions that it calls */
	_PutMsg(&mcb);
}


/* DbgSend is a separate fn from Dbg Worker to stop the compiler cacheing */
/* the buffer pointer state */
void DbgSend(void) {
	char	*start;
	word	size;

	IntsOff();
	if (GetExecRoot()->KD3StartM != GetExecRoot()->KD3EndM) {
		if (GetExecRoot()->KD3EndM > GetExecRoot()->KD3StartM) {
			start = GetExecRoot()->KD3StartM;
			size = (word)(GetExecRoot()->KD3EndM - GetExecRoot()->KD3StartM);
			IntsOn();

			DbgMsg(start, size);
			GetExecRoot()->KD3StartM += size;
		} else {
			start = GetExecRoot()->KD3StartM;
			size = (word)GetExecRoot()->KD3EndB - (word)GetExecRoot()->KD3StartM;
			IntsOn();

			DbgMsg(start, size);
			/* This routine is the only one that alters KD3StartM */
			/* so no need to guard it */
			GetExecRoot()->KD3StartM = GetExecRoot()->KD3StartB;

			/* going over boundary, so force another call */
			_Signal(&GetExecRoot()->KD3Sem);
		}
		return;
	}
	IntsOn();
}

/* Run at High Pri, the thread sends out messages that are put in its buffer */
/* This is the only place where KD3StartM can be changed */

void DbgWorker(void)
{
	Semaphore *kickme = &GetExecRoot()->KD3Sem;

	forever {
		_Wait(kickme);
		DbgSend();
	}
}


/* Init called once at system init */
void InitKDebug(void)
{
	ExecRoot *xroot = GetExecRoot();

	InitSemaphore(&xroot->KD3Sem, 0);

	/* a KDEBUG3SIZE'ed area has been set aside before the trace buffer */
	xroot->KD3StartB = ((char *)GetRoot()->TraceVec) - KDEBUG3SIZE;
	xroot->KD3EndB = xroot->KD3StartB + KDEBUG3SIZE;

	xroot->KD3StartM = xroot->KD3EndM = xroot->KD3StartB;
	NewWorker(DbgWorker);
	xroot->KDebugEnable = TRUE;	/* allow debugging messages now */
}

#endif


#if defined(KERNELDEBUG1) || defined(KERNELDEBUG2) || defined(KERNELDEBUG3)
/* strlen support fn */
int strlen2(char *str)
{
	int i = 0;

	while (*str++)
		i++;

	return i;
}

/* Generic KDebug code: */

void InitKDBuf()
{
	GetExecRoot()->KDBufPos = 0;
}

void FlushKDBuf()
{
	ExecRoot *xroot = GetExecRoot();

	KDebugSend(xroot->KDBuffer, xroot->KDBufPos);

	xroot->KDBufPos = 0;
}

void BufKDputc(byte c)
{
	ExecRoot *xroot = GetExecRoot();

	if (xroot->KDBufPos >= KDSIZE - 3) /* if at end of buffer, force flush */
		FlushKDBuf();

	if( c == '\n' )
		xroot->KDBuffer[xroot->KDBufPos++] = '\r';

	xroot->KDBuffer[xroot->KDBufPos++] = c;
}

void BufKDputs(byte *s)
{
	while( *s )
		BufKDputc(*s++);
}

static void writenum1(unsigned int i, int base )
{
	char *digits = "0123456789abcdef";

	if( i == 0 ) return;

	writenum1(i/base,base);
	BufKDputc(digits[i % base]);
}

static void writenum(int i, int base)
{
	if( i < 0 && base == 10 ) {
		i=-i;
		BufKDputc('-');
	}
	if( i == 0 )
		BufKDputc('0');
	else
		writenum1(i,base);
}

static void writestr(char *s)
{
	BufKDputs(s);
}

void KDebug(const char *str, ... )
{
	int base, i, *p;
	char *s, *t;
	va_list a;
	Object *o;
	Stream *sp;
	word oldpri;

	if (GetExecRoot()->KDebugEnable == FALSE)
		return;

	/* don't allow any slicing! */
	oldpri = SetPhysPri(HIGHPRI);

	InitKDBuf();

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
				BufKDputc(*str); 
				break;

			case 'c':			/* char		*/
				i = va_arg(a,int);
				BufKDputc(i);
				break;
#ifdef __C40
			case 'a':
				base = 16;		/* hex 		*/
				i = va_arg(a,int);
				BufKDputc('0');
				BufKDputc('x');
				writenum(i,base);
				i = (int)C40WordAddress((char *)i);
				BufKDputc(' ');
				BufKDputc('[');
				BufKDputc('W');
				BufKDputc('P');
				BufKDputc(':');
				writenum(i,base);
				BufKDputc(']');
				break;

#endif
			case 'x': base = 16;		/* hex 		*/
				BufKDputc('0');
				BufKDputc('x');

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
				t = s + strlen2(s);
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
				writenum(p[0],16); BufKDputc(' ');
				writenum(p[1],16);
				break;
				
			case 'M':			/* mcb		*/
				p = va_arg(a,int *);
				writenum(p[0],16); BufKDputc(' ');
				writenum(p[1],16); BufKDputc(' ');
				writenum(p[2],16); BufKDputc(' ');
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
						BufKDputc(' ');
						writestr((char *) &o->Name);
					}
					BufKDputc('>');
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
						BufKDputc(' ');
						writenum((int)sp->Flags,16);
						BufKDputc(' ');
						writestr((char *)&sp->Name);
					}
					BufKDputc('>');				
					break;
				}

			case 'F':			/* function code*/
				i = va_arg(a,int);
			putfn:
				switch( i & FG_Mask ) {
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
			}
			str++;
		}
		else BufKDputc(*str++);
	}
	
	va_end(a);

	FlushKDBuf();

	SetPhysPri(oldpri);
}
#endif /* KERNELDEBUG1 | KERNELDEBUG2 | KERNELDEBUG3 */

#if defined __ARM && defined SYSDEB
/* syncronous KDebug - Temporary executive debug. */
/* This is only used for fatal executive errors at the start of a port */

# ifdef __C40
#  define DEBUGLINK	0x100070
# elif defined(__ARM)
#  if 0
#   define DEBUGLINK	0x03342000	/* Archi slot zero link adapter */
#  else
#   define DEBUGLINK	0x02000020	/* PID serial */
#  endif
# else
#  error "DEBUGLINK not defined"
# endif

void s_KDebugSend(char *txt, word size)
{
	/* Fairly simple debug, can be used when IOServer is up and */
	/* running, but before full link guardians are up and running */
	/* assumes _LinkTx() writes and blocks on link. */

	MsgHdr	mh;
	word	proto = Proto_Msg;

	mh.DataSize = (short)size;
	mh.ContSize = 0;
	mh.Flags = 0;
	mh.Dest = 0x12345678;
	mh.Reply = 0xeeee;
	mh.FnRc = 0x22222222;

#  ifdef __C40
	_LinkTx(4, (Channel)DEBUGLINK, &proto);
	_LinkTx(sizeof(MsgHdr), (Channel)DEBUGLINK, &mh);
	_LinkTx((size + 3) & ~3, DEBUGLINK, txt);
#  else
#   ifdef __BIGENDIAN
	_LinkTx(1, (Channel)DEBUGLINK, ((char *)&proto) + 3);
#   else
	_LinkTx(1, (Channel)DEBUGLINK, &proto);
#   endif
	_LinkTx(sizeof(MsgHdr), (Channel)DEBUGLINK, &mh);
	_LinkTx(size, (Channel)DEBUGLINK, txt);
#  endif
}

void s_FlushKDBuf()
{
	ExecRoot *xroot = GetExecRoot();

	s_KDebugSend(xroot->KDBuffer, xroot->KDBufPos);

	xroot->KDBufPos = 0;
}

void s_BufKDputc(byte c)
{
	ExecRoot *xroot = GetExecRoot();

	if (xroot->KDBufPos >= KDSIZE - 3) /* if at end of buffer, force flush */
		s_FlushKDBuf();

	if( c == '\n' )
		xroot->KDBuffer[xroot->KDBufPos++] = '\r';

	xroot->KDBuffer[xroot->KDBufPos++] = c;
}

void s_BufKDputs(byte *s)
{
	while( *s )
		s_BufKDputc(*s++);
}

static void s_writenum1(unsigned int i, int base )
{
	char *digits = "0123456789abcdef";

	if( i == 0 ) return;

	s_writenum1(i/base,base);
	s_BufKDputc(digits[i % base]);
}

static void s_writenum(int i, int base)
{
	if( i < 0 && base == 10 ) {
		i=-i;
		s_BufKDputc('-');
	}
	if( i == 0 )
		s_BufKDputc('0');
	else
		s_writenum1(i,base);
}

static void s_writestr(char *s)
{
	s_BufKDputs(s);
}

void sKDebug(const char *str, ... )
{
	int base, i;
	char *s, *t;
	va_list a;
	word oldpri;
	extern word _pcreg( void );
	
#ifdef __ARM /* @@@ tmp */
	if ((_pcreg() & ModeMask) == UserMode) {
		KDebug("ARGHHH! entered sKDebug in User Mode");
		backtrace();
		Stop();
	}
#endif
		
	/* Unblock any messages on the link */
	if( GetRoot()->Links[0]->TxUser != NULL ) {
		base = Proto_ReSync;
		for (i = 0xffff; i > 0; i--) {
			_LinkTx(1, (Channel)DEBUGLINK, &base);
		}

		/* Zap further link traffic and need to re-sync */
		GetRoot()->Links[0]->TxUser = NULL;
	}

	/* don't allow any slicing! */
	oldpri = SetPhysPri(HIGHPRI);

	InitKDBuf();

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
				s_BufKDputc(*str); 
				break;

			case 'c':			/* char		*/
				i = va_arg(a,int);
				s_BufKDputc(i);
				break;
#ifdef __C40
			case 'a':
				base = 16;		/* hex 		*/
				i = va_arg(a,int);
				s_BufKDputc('0');
				s_BufKDputc('x');
				s_writenum(i,base);
				i = (int)C40WordAddress(i);
				s_BufKDputc(' ');
				s_BufKDputc('[');
				s_BufKDputc('W');
				s_BufKDputc('P');
				s_BufKDputc(':');
				s_writenum(i,base);
				s_BufKDputc(']');
				break;

#endif
			case 'x': base = 16;		/* hex 		*/
				s_BufKDputc('0');
				s_BufKDputc('x');

			case 'd':			/* decimal	*/
				i = va_arg(a,int);
				s_writenum(i,base);
				break;
				
			case 'N':			/* pathname	*/
			case 's':			/* string	*/
				s = va_arg(a,char *);
				if( s == NULL ) s = "<NULL string>";
			putstr:
				s_writestr(s);
				break;

			case 'o':
				s = va_arg(a,char *);
				t = s + strlen2(s);
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
				s_writenum(i,16);
				break;
				
			case 'C':			/* capability	*/
			{
				int *p = va_arg(a,int *);
				s_writenum(p[0],16); s_BufKDputc(' ');
				s_writenum(p[1],16);
				break;
			}				
			case 'M':			/* mcb		*/
			{
				int *p = va_arg(a,int *);
				s_writenum(p[0],16); s_BufKDputc(' ');
				s_writenum(p[1],16); s_BufKDputc(' ');
				s_writenum(p[2],16); s_BufKDputc(' ');
				i = p[3]; goto putfn;
			}				
			case 'O':			/* object	*/
			case 'S':			/* stream	*/
			{
				Object *o = va_arg(a, Object *);

				if( (o == NULL && fch == 'O') ||
				    (o->Flags & Flags_Stream) == 0 )
				{
					s_writestr("<Object: ");
					if( o == NULL ) s_writestr("NULL");
					else 
					{
						s_writenum((int)o->Type, 16);
						s_BufKDputc(' ');
						s_writestr((char *) &o->Name);
					}
					s_BufKDputc('>');
					break;
				}
				else
				{
					Stream *sp = (Stream *)o;

					s_writestr("<Stream: ");
					if( sp == NULL ) s_writestr("NULL");
					else 
					{
						s_writenum((int)sp->Type,16);
						s_BufKDputc(' ');
						s_writenum((int)sp->Flags,16);
						s_BufKDputc(' ');
						s_writestr((char *)&sp->Name);
					}
					s_BufKDputc('>');				
					break;
				}
			}
			case 'F':			/* function code*/
				i = va_arg(a,int);
			putfn:
				switch( i & FG_Mask ) {
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
					s_writenum(i,16);
					break;
				}
			}
			str++;
		}
		else s_BufKDputc(*str++);
	}
	
	va_end(a);

	s_FlushKDBuf();

	SetPhysPri(oldpri);
}
#endif

/* end of gdebug.c */

