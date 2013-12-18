
/* $Header: /hsrc/cmds/shell/RCS/dodebug.c,v 1.4 1993/08/04 14:45:02 bart Exp $ */

#ifdef DEBUGGING
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

#include <stdio.h>
#include <stdarg.h>
#include <sem.h>
#include <string.h>
#include <codes.h>
#include <syslib.h>

FILE			*DEBUG_FILE;
static Semaphore	DEBUG_LOCK;
static char		DEBUG_BUFFER[2048];

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
    	dest = writenum1 (dest, i, base, width);
    return dest;
}

static char *
writestr (char *dest, char *s, int width)
{
    if (width > 0)
    {
    	while (*s && width-- > 0)
    	    if ( *s < ' ' )
    	    {
    	    	*dest++ = '\\';
    	    	dest = writenum (dest, *s++, 16, 2);
    	    }
    	    else
    	    	*dest++ = *s++;
    	while (width-- > 0)
    	    *dest++ = ' ';
    }
    else
        while (*s)
    	    if ( *s < ' ' )
    	    {
    	    	*dest++ = '\\';
    	    	dest = writenum (dest, *s++, 16, 2);
    	    }
    	    else
    	    	*dest++ = *s++;
    return dest;
}

static void
dprint (char *buf, char *fmt, va_list a)
{
    int		base, width, *p, i;
    char	*s;
    char	**v;

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
	    case '%':			/* no linefeed			*/
	    	goto nonl;
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
		case FG_Terminate:	s = "FG_Terminate"; 	  goto putstr;
		default:
		    buf = writenum (buf, i, 16, 8);
		    break;
		}
	    case 'V':			/* a string vector...		*/
		v = va_arg (a, char **);
		if (v == NULL || *v == NULL) 
		{
		    buf = writestr (buf, "<NULL vector>",0);
		    break;
		}   
		i = 0;
		while (*v)
		{
		    buf = writestr (buf, *v++, 0);
		    if (*v)
		    	*buf++ = ' ';
		}
		break;

	    default:
		*buf++ = *fmt;		/* "%%" or similar, copy it	*/
		break;
	    }
	    fmt++;
	}
	else 				/* No format specifier, copy it	*/
	    *buf++ = *fmt++;
    }
    *buf++ = '\n';
nonl:
    *buf = '\0';			/* and the terminating '\0'.	*/
}

void
DoDebug (char *fmt, ...)
{
    va_list	args;

    va_start (args, fmt);
    Wait (&DEBUG_LOCK);
    dprint (DEBUG_BUFFER, fmt, args);
    fputs (DEBUG_BUFFER, DEBUG_FILE);
    fflush (DEBUG_FILE);
    Delay (1000);
    Signal (&DEBUG_LOCK);
    va_end (args);
}

void
DebugInit (void)
{
    DEBUG_FILE = fopen ("/window/debug", "w");
    InitSemaphore (&DEBUG_LOCK, 1);
}

/*--- end of debug.c ---*/
#endif
