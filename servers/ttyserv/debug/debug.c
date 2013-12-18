/*************************************************************************
**									**
**		     S Y S T E M   D E B U G G I N G			**
**		     -------------------------------			**
**									**
**		    Copyright (C) 1989, Parsytec GmbH			**
**			  All Rights Reserved.				**
**									**
**									**
** debug.c								**
**									**
**	- Debug macro definitions and functions				**
**									**
**************************************************************************
** HISTORY   :								**
** -----------								**
** Author    :	29/11/89 : C. Fleischer					**
*************************************************************************/

#ifdef	DEBUG
#ifndef	SYSDEBUG
#include <stdio.h>
#endif
#include <stdarg.h>
#include <sem.h>
#include <string.h>
#include <codes.h>
#include <syslib.h>
#include <servlib.h>

#include "../debug/debug.h"

#ifdef	DEBUG
int			DEBUG_LEVEL	= DEBUG;
#else
int			DEBUG_LEVEL	= 0;
#endif

#ifndef	SYSDEBUG
FILE			*DEBUG_FILE;
#endif
static Semaphore	DEBUG_LOCK;
static char		DEBUG_BUFFER[2048];

void
Do_Diags (ServInfo *srvinfo)
{
    MCB		*m	= srvinfo->m;
    word	fnrc	= srvinfo->FnCode;
    struct ReinitReq
    {
        IOCCommon	Common;
        word		Level;
    }		*req	= (struct ReinitReq *) m->Control;

    if ( fnrc == FG_SetDiags )
    {
    	DEBUG_LEVEL = req->Level;
    	DoDebug ("SetDiag to %08x", DEBUG_LEVEL);
    	if ( m->MsgHdr.Reply == NullPort )
    	    return;
    	InitMCB ( m, 0, m->MsgHdr.Reply, NullPort, fnrc );
    	PutMsg ( m );
    }
    elif ( fnrc == FG_GetDiags )
    {
    	if ( m->MsgHdr.Reply == NullPort )
    	    return;
    	InitMCB ( m, 0, m->MsgHdr.Reply, NullPort, fnrc );
    	MarshalWord ( m, DEBUG_LEVEL );
    	DoDebug ("GetDiag is %08x", DEBUG_LEVEL);
    	PutMsg ( m );
    }
    else				/* Other private calls are not	*/
	ErrorMsg (m, EC_Error + EG_FnCode + EO_Server);	/* supported.	*/
}

static char *
writenum1 (char *dest, unsigned int value, int base, int width)
{
    static char *digits = "0123456789abcdef";

    if (width > 0 || value != 0)
    {
	dest = writenum1 (dest, value / base, base, width - 1);
	*dest++ = digits [value % base];
    }
    return dest;
}

static char *
writenum (char *dest, int i, int base, int width)
{
    if (i < 0 && base == 10)
    {
    	i = -i; 
    	*dest++ = '-';
    }
    if (i == 0 && width <= 0) 
    	*dest++ = '0';
    else 
    	dest = writenum1 (dest, i, base, width - 1);
    return dest;
}

static char *
writestr (char *dest, char *s, int width)
{
    if (width > 0)
    {
    	while (*s && width-- > 0)
    	    *dest++ = *s++;
    	while (width-- > 0)
    	    *dest++ = ' ';
    }
    else
        while (*s)
    	    *dest++ = *s++;
    return dest;
}

static void
dprint (char *buf, char *fmt, va_list a)
{
    int		base, width, *p, i;
    char	*s;

    while (*fmt)
    {
	if (*fmt == '%')		/* format specifier found	*/
	{
	    fmt++;
	    width = 0;
	    while (*fmt >= '0' && *fmt <= '9')	/* width given ?	*/
	    {					/* yes, collect it	*/
	    	width = width * 10 + (*fmt++ - '0');
	    }

	    switch (*fmt)		/* decode format specifier	*/
	    {
	    case 'c':			/* char				*/
		i = va_arg (a, int);
		*buf++ = (char) i;
		break;

	    case 'o':
	    	base = 8;
	    	goto putnum;
	    case 'p':
	    case 'x': 			/* hex	      			*/
		base = 16;
		goto putnum;
	    case 'd':		 	/* decimal 			*/
	    	base = 10;
	    putnum:
		i = va_arg (a, int);
		buf = writenum (buf, i, base, width);
		break;

	    case 'N':			/* pathname			*/
	    case 's':		 	/* string    			*/
		s = va_arg (a, char *);
		if (s == NULL) 
		    s = "<NULL string>";
	    putstr:
		buf = writestr (buf, s, width);
		break;

	    case 'L':			/* line with special chars	*/
		s = va_arg (a, char *);
		if (s == NULL) 
		{
		    s = "<NULL string>";
		    buf = writestr (buf, s, 0);
		}
		else
		    while ( *s )
		    {
		    	if ( *s >= 32 && *s < 127 )
		    	    *buf++ = *s++;
		    	else
		    	{
		    	    *buf++ = '\\';
			    buf = writenum (buf, ( word ) *s++, 8, 0);
			}
		    }
		break;
		
/* The following are special formats for various Helios-specific things	*/

	    case 'A':		 	/* access mask			*/
	    case 'E':		 	/* error code			*/
	    case 'P':		 	/* pointer    			*/
	    case 'T':		 	/* type     			*/
	    case 'X':		 	/* matrix    			*/
		i = va_arg (a, int);
		buf = writenum (buf, i, 16, 8);
		break;

	    case 'C':		 	/* capability	  		*/
		p = va_arg (a, int *);
		buf = writenum (buf, p[0], 16, 8);
		*buf++ = ' ';
		buf = writenum (buf, p[1], 16, 8);
		break;

	    case 'M':		 	/* mcb        			*/
		p = va_arg (a, int *);
		buf = writenum (buf, p[0], 16, 8);
		*buf++ = ' ';
		buf = writenum (buf, p[1], 16, 8);
		*buf++ = ' ';
		buf = writenum (buf, p[2], 16, 8);
		*buf++ = ' ';
		i = p[3]; 
		goto putfn;

	    case 'O':		 	/* object    			*/
	    {
		Object *o = va_arg (a, Object *);
		buf = writestr (buf, "<Object: ", 0);
		if (o == NULL) 
		    buf = writestr (buf, "NULL", 0);
		else
		{
 		    buf = writenum (buf, o->Type, 16, 8);
		    *buf++ = ' ';
		    buf = writestr (buf, (char *) &o->Name, 0);
		}
		*buf++ = '>';
		break;
	    }

	    case 'S':		 	/* stream    			*/
	    {
		Stream *sp = va_arg (a, Stream *);
		buf = writestr (buf, "<Stream: ", 0);
		if (sp == NULL) 
		    buf = writestr (buf, "NULL", 0);
		else
		{
		    buf = writenum (buf, sp->Type, 16, 8);
		    *buf++ = ' ';
		    buf = writenum (buf, sp->Server, 16, 8);
		    *buf++ = ' ';
		    buf = writestr (buf, (char *) &sp->Name, 0);
		}
		*buf++ = '>';
		break;
	    }

	    case 'F':		 	/* function code		*/

		i = va_arg (a, int);
	    putfn:
		switch (i & FG_Mask)
		{
		case FG_Open:		s = "FG_Open";	  	  goto putstr;
		case FG_Create:		s = "FG_Create";	  goto putstr;
		case FG_Locate:		s = "FG_Locate";	  goto putstr;
		case FG_ObjectInfo:    	s = "FG_ObjectInfo";	  goto putstr;
		case FG_ServerInfo:    	s = "FG_ServerInfo";	  goto putstr;
		case FG_Delete:     	s = "FG_Delete";	  goto putstr;
		case FG_Rename:     	s = "FG_Rename";	  goto putstr;
		case FG_Link:	    	s = "FG_Link";		  goto putstr;
		case FG_Protect:    	s = "FG_Protect";	  goto putstr;
		case FG_SetDate:    	s = "FG_SetDate";	  goto putstr;
		case FG_Refine:     	s = "FG_Refine";	  goto putstr;
		case FG_CloseObj:    	s = "FG_CloseObj";	  goto putstr;
		case FG_Read:	     	s = "FG_Read";		  goto putstr;
		case FG_Write:	      	s = "FG_Write";	  	  goto putstr;
		case FG_GetSize:    	s = "FG_GetSize";	  goto putstr;
		case FG_SetSize:    	s = "FG_SetSize"; 	  goto putstr;
		case FG_Close:	      	s = "FG_Close";    	  goto putstr;
		case FG_Seek:	     	s = "FG_Seek"; 		  goto putstr;
		case FG_GetInfo:    	s = "FG_GetInfo"; 	  goto putstr;
		case FG_SetInfo:    	s = "FG_SetInfo"; 	  goto putstr;
		case FG_EnableEvents:	s = "FG_EnableEvents"; 	  goto putstr;
		case FG_Acknowledge:	s = "FG_Acknowledge";	  goto putstr;
		case FG_NegAcknowledge:	s = "FG_NegAcknowledge";  goto putstr;
		case FG_Select:		s = "FG_Select";	  goto putstr;
		case FG_Search:     	s = "FG_Search"; 	  goto putstr;
		case FG_FollowTrail:	s = "FG_FollowTrail";	  goto putstr;
		case FG_MachineName:	s = "FG_MachineName"; 	  goto putstr;
		case FG_Debug:	      	s = "FG_Debug";		  goto putstr;
		case FG_Alarm:	      	s = "FG_Alarm"; 	  goto putstr;
		case FG_Reconfigure:	s = "FG_Reconfigure"; 	  goto putstr;
		case FG_SetFlags:    	s = "FG_SetFlags"; 	  goto putstr;
		case FG_SendEnv:    	s = "FG_SendEnv"; 	  goto putstr;
		case FG_Signal:     	s = "FG_Signal"; 	  goto putstr;
		case FG_ProgramInfo:	s = "FG_ProgramInfo"; 	  goto putstr;
		case FG_RequestEnv:	s = "FG_RequestEnv";	  goto putstr;
		case FG_BootLink:    	s = "FG_BootLink"; 	  goto putstr;
		case FG_Format:		s = "FG_Format"; 	  goto putstr;
		case FG_WriteBoot:	s = "FG_WriteBoot"; 	  goto putstr;
		case FG_Terminate:	s = "FG_Terminate"; 	  goto putstr;
		default:
		    buf = writenum (buf, i, 16, 8);
		    break;
		}

	    default:
		*buf++ = *fmt;		/* "%%" or similar, copy it	*/
		break;
	    }
	    fmt++;
	}
	else 				/* No format specifier, copy it	*/
	    *buf++ = *fmt++;
    }

    *buf = '\0';			/* and the terminating '\0'.	*/
}

#endif

void
DoDebug (char *fmt, ...)
{
#ifdef	DEBUG
    va_list	args;

    va_start (args, fmt);
    Wait (&DEBUG_LOCK);
    dprint (DEBUG_BUFFER, fmt, args);
#ifdef	SYSDEBUG
    IOdebug ("%s", DEBUG_BUFFER);
#else
    fprintf (DEBUG_FILE, "[%s]\n", DEBUG_BUFFER);
    fflush (DEBUG_FILE);
    Delay (1000);
#endif
    Signal (&DEBUG_LOCK);
    va_end (args);
#else
    fmt = fmt;				/* keep the compiler happy...	*/
#endif
}

void
DebugInit (void)
{
#ifdef	DEBUG
#ifndef	SYSDEBUG
#ifdef	WINDEBUG
    DEBUG_FILE = fopen ("/IO/window/debug", "w");
#else
    DEBUG_FILE = stderr;
#endif
#endif
    InitSemaphore (&DEBUG_LOCK, 1);
#endif
}

/*--- end of debug.c ---*/
