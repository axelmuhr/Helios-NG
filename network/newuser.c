/*------------------------------------------------------------------------
--                                                                      --
--           H E L I O S   N E T W O R K I N G   S O F T W A R E	--
--           ---------------------------------------------------	--
--                                                                      --
--             Copyright (C) 1990, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- newuser.c								--
--		This is a strange little Helios program that registers	--
--	a window with the Session Manager, causing the latter to	--
--	run logins repeatedly inside that window. The real work is	--
--	done by the the RmRegisterWindow() routine in session.lib	--
--                                                                      --
--	Author:  BLV 21/6/90						--
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Header: /hsrc/network/RCS/newuser.c,v 1.6 1993/08/11 10:37:17 bart Exp $*/

#include <string.h>
#include <stdarg.h>
#include <posix.h>
#include <syslib.h>
#include <codes.h>
#include <nonansi.h>
#include "session.h"
#include "exports.h"

#ifndef eq
#define eq ==
#define ne !=
#endif

static	Stream	*DiagnosticsStream;
static	void 	report(char *, ...);
	
int	main(void)
{ char		*UserName;
  Environ	env;
  Object	*WindowServer;
  WORD		error;
  int		argc;
  
  if (GetEnv(MyTask->Port, &env) < Err_Null)
   { IOputs("newuser: failed to receive environment");
     Exit(0x100);
   }

  if ((env.Strv[0] eq Null(Stream)) || (env.Strv[1] eq Null(Stream)) ||
      (env.Strv[2] eq Null(Stream)) || (env.Strv[2] eq (Stream *) MinInt))
   { IOputs("newuser: failed to receive error stream in environment");
     Exit(0x100);
   }
  DiagnosticsStream = env.Strv[2];
  
  if ((env.Strv[0] eq (Stream *) MinInt) || 
      ((env.Strv[0]->Flags & Flags_Interactive) eq 0))
   { report("must be run interactively");
     Exit(0x100);
   }
   
  for (argc = 0; env.Argv[argc] ne Null(char); argc++);
  if (argc > 2)
   { report("usage, newuser <username>\n");
     Exit(0x100);
   }
   
  { int i;
    for (i = 0; i <= OV_CServer; i++)
     if (env.Objv[i] eq Null(Object))
      { report("incomplete environment");
        Exit(0x100);
      }
  }
  WindowServer  = env.Objv[OV_CServer];
 
  if (argc eq 1)
   UserName = Null(char);
  else
   UserName = env.Argv[1];

  unless(RmTestSessionManager())
   { report("failed to locate Session Manager.");
     Exit(0x100);
   }
   
  if (RmRegisterWindow(WindowServer, env.Strv[0], UserName, &error))
   Exit(0x100);
  else
   { if ((error & SS_Mask) eq 0)
      switch(error)
       { case (EC_Error + EG_Unknown + EO_Server) :
       	  report("failed to locate session manager");
       	  break;
       	 case EC_Error + EG_WrongSize + EO_Name :
       	  report("cannot cope with window name %s", env.Strv[0]->Name);
       	  break;
       	 case EC_Error + EG_NoMemory + EO_Message :
       	  report("not enough memory");
       	  break;
       	 default :
       	  report("failed to register window, fault %x", error); 
       }
     else 
      report("failed to register window, fault %x", error);
     Exit(0x100);   
   }
}

/**
*** Usual formatted output routines
**/
static char	output_buffer[256];
static char	*int_to_string(char *buffer, int x);

static int	process_format(char *init, char *format, va_list args)
{ char	*dest;

  strcpy(output_buffer, init);
  
  for (dest = output_buffer + strlen(output_buffer); *format ne '\0'; format++)
   { if (*format ne '%')
      { *dest++ = *format; continue; }
     switch (*(++format))
      { case	'\0': *dest++ = '%'; format--; break;
        case	'%' : *dest++ = '%'; break;
        case    'c' : *dest++ = (char) va_arg(args, int); break;
        case	's' : { char	*temp = va_arg(args, char *);
			if (temp eq Null(char))
			 { *dest++ = '<'; *dest++ = 'n'; *dest++ = 'u';
			   *dest++ = 'l'; *dest++ = 'l'; *dest++ = '>';
			   break;
			 }
                        while (*temp ne '\0') *dest++ = *temp++;
                        break;
                      }
        case	'x' : { int	x = va_arg(args, int);
        		int	shift;
        		*dest++ = '0'; *dest++ = 'x';
        		for (shift = 28; shift >= 0; shift -= 4)
        		 { int temp = (x >> shift) & 0x0F;
        		   if (temp <= 9)
        		    *dest++ = '0' + temp;
        		   else 
        		    *dest++ = 'a' + temp - 10;
        		 }
        		break;
        	      }
	case	'd' : { int	temp = va_arg(args, int);
	   		dest = int_to_string(dest, temp);
	   		break;
		      }  

	default	    : *dest++ = '%'; *dest++ = *format; break;
      }
    }
  *dest++ = '\n';
  return(dest - output_buffer);
}

static char	*int_to_string_aux(char *buffer, unsigned int i)
{ if (i > 9) buffer = int_to_string_aux(buffer, i / 10);
  *buffer++	= (i % 10) + '0';
  return(buffer);
}

static char	*int_to_string(char *buffer, int x)
{ if (x < 0) { x = -x; *buffer++ = '-'; }
  return(int_to_string_aux(buffer, (unsigned int ) x));
}

static void report(char *format, ...)
{ va_list	args;
  int		length;  
  
  va_start(args, format);
  
  length = process_format("newuser: ", format, args);
  va_end(args);
  (void) Write(DiagnosticsStream, output_buffer, length, -1);
}

