                                                                                /*
  []-------------------------------------------------------------------------[]
   |                                                                         |
   |                    (c) 1991 by parsytec GmbH, Aachen                    |
   |                          All rights reserved.                           |
   |                                                                         |
   |-------------------------------------------------------------------------|
   |                                                                         |
   |                               Utilities                                 |
   |                                                                         |
   |-------------------------------------------------------------------------|
   |                                                                         |
   |  error.c							             |
   |                                                                         |
   |    Routines to put out messages to stdout, stderr and to the debug      |
   |    screen WITHOUT using the CLIB.
   |                                                                         |
   |-------------------------------------------------------------------------|
   |                                                                         |
   |  History:                                                               |
   |    1 - O.Imbusch - 27 March 1991 - Basic version                        |
   |                                                                         |
  []-------------------------------------------------------------------------[]
                                                                                */

#include <codes.h>
#include <helios.h>
#include <sem.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <syslib.h>
#include <time.h>

#define DEBUG    0
#define GEPDEBUG 0
#define FLDEBUG  0

#include "error.h"
#include "misc.h"
#include "procname.h"

#define ToDebug     0x01
#define ToStdErr    0x02
#define ToStdIO     0x03

#define MsgOnly     0x00
#define WithTime    0x01
#define WithContext 0x02

int       ContextLine;
char      ContextFileName [PATH_MAX],
          ContextFuncName [64],
          ContextCallName [64];
Semaphore PESem;
bool      SemSet = FALSE;
word      NOfPorts = 0;
     
#if IN_NUCLEUS
extern /* main */ Environ Env;
#else
#include <stdio.h>
#endif

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
cf_bprint (char *buf, char *fmt, va_list a)
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

char *Fvsprintf (char *Dest,
                 char *Format,
                 ...)
{
  va_list Args;
  
  va_start (Args, Format);
  cf_bprint (Dest, Format, Args);
  va_end (Args);
  return (Dest);
}                 

struct tm *localtime(const time_t *timer)
{
    static int monlen[13] = { 31,29,31,30,31,30,31,31,30,31,30,31,0x40000000 };
	
    time_t t = *timer;
    static struct tm _tms;
    int i = 0, yr;
/* treat unset dates as 1-Jan-1900 - any better ideas? */
    if (t == (time_t)-1) memset(&_tms, 0, sizeof(_tms)), _tms.tm_mday = 1;
    else
    {   /* unix time already in seconds (since 1-Jan-1970) ... */
        _tms.tm_sec = t % 60; t /= 60;
        _tms.tm_min = t % 60; t /= 60;
        _tms.tm_hour = t % 24; t /= 24;
/* The next line converts *timer arg into days since 1-Jan-1900 from t which
   now holds days since 1-Jan-1970.  Now there are really only 17 leap years
   in this range 04,08,...,68 but we use 18 so that we do not have to do
   special case code for 1900 which was not a leap year.  Of course this
   cannot give problems as pre-1970 times are not representable in *timer. */
        t += 70*365 + 18;
        _tms.tm_wday = t % 7;               /* it just happens to be so */
        yr = 4 * (t / (365*4+1)); t %= (365*4+1);
        if (t >= 366) yr += (t-1) / 365, t = (t-1) % 365;
        _tms.tm_year = yr;
        _tms.tm_yday = t;
        if ((yr & 3) != 0 && t >= 31+28) t++;
        while (t >= monlen[i]) t -= monlen[i++];
        _tms.tm_mday = t+1;
        _tms.tm_mon = i;
        _tms.tm_isdst = -1;                  /* unavailable */
    }
    return &_tms;
}

char *asctime(const struct tm *timeptr)
{
  static char _timebuf[26+(8+3*9+7)];  /* slop in case illegal args */

   Fvsprintf (_timebuf, "%3s %3s %3d %2d:%2d:%2d %d",
               "SunMonTueWedThuFriSat" + (timeptr -> tm_wday)*3,
               "JanFebMarAprMayJunJulAugSepOctNovDecBad" + (timeptr -> tm_mon)*3,
               timeptr -> tm_mday,
               timeptr -> tm_hour,
               timeptr -> tm_min,
               timeptr -> tm_sec,
               timeptr -> tm_year + 1900);
               return _timebuf;
}

char *ctime(const time_t *timer)
{
  return asctime (localtime (timer));
}

int _PrintError (int      Mode,
                  int      Dest,
                  char    *Format,
                  va_list  Args)
{
  char      UserMsg    [K],
            TimeMsg    [K], 
            ContextMsg [K],
            Msg        [K];
  time_t    CurrentDate;

  if (!SemSet)
  {
    IOdebug ("Semaphore...");
    Exit (0);
  }

  Wait (&PESem);

  cf_bprint (UserMsg, Format, Args);

  if (Mode & WithTime)
  {
    CurrentDate = GetDate ();
    Fvsprintf (TimeMsg, "\t\t\t%s\n", ctime (&CurrentDate));
  }
  else
    *TimeMsg = '\0';

  if (Mode & WithContext)
  {
    Fvsprintf (ContextMsg, "\t\t\t\"%s\", #%d, %s ():\n", ContextFileName, ContextLine, ContextFuncName);
/*
    strcpy (ContextMsg, CalledBy (Mode));
    strcat (ContextMsg, " called ");
    strcat (ContextMsg, ThisFunc (Mode));
    strcat (ContextMsg, " ():");
*/
  }
  else

    *ContextMsg = '\0';

  strcat (strcat (strcpy (Msg, TimeMsg), ContextMsg), UserMsg);

  switch (Dest)
  {
    case ToDebug:
      IOdebug (Msg);
      break;
    case ToStdErr:
#if IN_NUCLEUS    
      Write (Env.Strv [2], Msg,  strlen (Msg),  -1);
      Write (Env.Strv [2], "\n", strlen ("\n"), -1);
#else
      fprintf (stderr, "%s\n", Msg);
#endif           
      break;
    case ToStdIO:
#if IN_NUCLEUS    
      Write (Env.Strv [1], Msg,  strlen (Msg),  -1);
      Write (Env.Strv [1], "\n", strlen ("\n"), -1);
#else
      printf ("%s\n", Msg);
#endif           
      break;
    default:
      Error ("Unknown destination for error message.");
      break;  
  }
  
Signal (&PESem); 

}                 

#define PrintError(FirstVar,Mode,Dest)  va_list Args; \
                                        strcpy      (ContextFuncName, CalledBy (FirstVar)); \
                                        va_start    (Args, Format); \
                                        _PrintError (Mode, Dest, Format, Args); \
                                        va_end      (Args); 

int _Debug (char *Format, ...)
{
  PrintError (Format, WithContext, ToDebug);
}

int _Report (char *Format, ...)
{
  PrintError (Format, MsgOnly, ToDebug);
}

int _Error (char *Format, ...)
{
  PrintError (Format, WithContext | WithTime, ToDebug);
}

int _Serious (char *Format, ...)
{
  PrintError (Format, MsgOnly, ToStdErr);
}

int _Fatal (int   ExitCode,
             char *Format, ...)
{
  PrintError (ExitCode, MsgOnly, ToStdErr);
  Exit (ExitCode);
}

#ifdef __MSDOS__

  void IOdebug (const char *Format,
                ...)
  {
     va_list Args;

     va_start (Args, Format);
     vfprintf (stderr, Format, Args);
     va_end   (Args);
  }

  void DoNothing (char *Format, ...)
  {
  }

#endif

/*******************************************************************************
**
**  error.c
**
*******************************************************************************/
