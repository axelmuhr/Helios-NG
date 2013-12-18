/*------------------------------------------------------------------------
--                                                                      --
--           H E L I O S   N E T W O R K I N G   S O F T W A R E	--
--           ---------------------------------------------------	--
--                                                                      --
--             Copyright (C) 1990, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- diag_sm.c								--
--                                                                      --
--	Enable/disable diagnostics in the Session Manager		--
--                                                                      --
--	Author:  BLV 15/4/92						--
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Header: /hsrc/network/RCS/diag_sm.c,v 1.2 1993/08/11 10:28:21 bart Exp $*/

#include <helios.h>
#include <stdio.h>
#include <stdlib.h>
#include <syslib.h>
#include <attrib.h>
#include <ctype.h>
#include <string.h>
#include <nonansi.h>
#include <posix.h>
#include <signal.h>
#include <sys/wait.h>
#include "private.h"
#include "rmlib.h"
#include "sessaux.h"

static void	show_help(void);
static void	usage(void);
static word	sm_private(Object *, int);
static void	sm_redirect(Object  *);
static void	sm_revert(Object *);
static void	init_terminal(void);

/**
*** The command can be used in two different ways. First, it can be
*** given the argument all, most or none on the command line. In that
*** case the appropriate debugging options are enabled/disabled.
*** Other command line options include redirect, revert, and bits of the
*** real debugging options. If no argument is given
*** the program goes into a little menu system. Every time
*** around the loop the current debugging options are displayed,
*** and the user presses a key to decide what to do.
**/

int main(int argc, char **argv)
{ WORD		mask, new_mask;
  Object 	*sm;
  int		c;
  bool		interactive	= TRUE;
  bool		all		= FALSE;
  bool		disable		= FALSE;
  bool		most		= FALSE;
  bool		redirect	= FALSE;
  bool		revert		= FALSE;
  int		i;

  if (argc > 1) interactive = FALSE;

  new_mask = 0;
    
  for (i = 1; i < argc; i++)
   { char *temp = argv[i];

     if (!strcmp(temp, "all"))   
      { all = TRUE; continue; }
     if (!strcmp(temp, "none"))
      { disable = TRUE; continue; }
     if (!strcmp(temp, "most"))
      { most = TRUE; continue; }
     if (!strcmp(temp, "redirect"))
      { redirect = TRUE; continue; }
     if (!strcmp(temp, "revert"))
      { revert = TRUE; continue; }      

     if (!strncmp(temp, "cre", 3))
      { new_mask ^= dbg_Create; continue; }
     if (!strncmp(temp, "mon", 3))
      { new_mask ^= dbg_Monitor; continue; }
     if (!strncmp(temp, "del", 3))
      { new_mask ^= dbg_Delete; continue; }
     if (!strncmp(temp, "mem", 3))
      { new_mask ^= dbg_Memory; continue; }
     if (!strncmp(temp, "ioc", 3))
      { new_mask ^= dbg_IOC; continue; }
     usage();
   }

  sm = Locate(Null(Object), "/sm");
  if (sm eq Null(Object))
   { fprintf(stderr, "diag_sm : failed to locate Session Manager.\n");
     return(EXIT_FAILURE);
   }
   
  if (!interactive)
   { if (redirect) sm_redirect(sm);
     if (revert)   sm_revert(sm);

     if (disable)
      mask = 0;
     elif (most)
      mask = dbg_Create | dbg_Delete;
     elif (all)
      mask = dbg_Create | dbg_Delete | dbg_Monitor;
     else
      { mask = sm_private(sm, dbg_Inquire);
        if (mask < 0)
         { fprintf(stderr, "diag_sm : communication breakdown.\n");
           return(EXIT_FAILURE);
         }
      }
      
     mask ^= new_mask;
     if ((mask = sm_private(sm, mask)) < 0)
      { fprintf(stderr, "diag_sm: communication breakdown.\n");
        return(EXIT_FAILURE);
      }
     else
      return(EXIT_SUCCESS);
   }
   
  mask = sm_private(sm, dbg_Inquire);
  if (mask < 0)
   { fprintf(stderr, "diag_sm : communication breakdown.\n");
     return(EXIT_FAILURE);
   }

  init_terminal();   

  forever
   { 
     printf("\ndiag_sm : debugging %s\n", sm->Name);
     fputs("Current debugging options are :", stdout);
     if (mask & dbg_Create)	fputs(" create", stdout);
     if (mask & dbg_Delete)	fputs(" delete", stdout);
     if (mask & dbg_Monitor)	fputs(" monitor", stdout);
     if (mask eq 0)		fputs(" none", stdout);
     fputs("\n\n", stdout);
     
     fputs("1) create   2) delete     3) monitor\n", stdout);
     fputs("4) memory\n", stdout);
     fputs("Your choice (1-4, Q, ? ) ? ", stdout);
     fflush(stdout);
     
     for ( c = getchar(); isspace(c); c = getchar());
     putchar('\n');
     switch(c)
      { case 'q' :
        case 'Q' : return(EXIT_SUCCESS);
        case '?' :
        case 'h' :
        case 'H' : show_help(); break;
	case 'i' :
	case 'I' : mask |= dbg_IOC;		break;
        case '1' : mask ^= dbg_Create; 		break;
        case '2' : mask ^= dbg_Delete;		break;
        case '3' : mask ^= dbg_Monitor;		break;
        case '4' : mask ^= dbg_Memory;		break;
      }

     mask = sm_private(sm, mask);
     if (mask < 0)
      { fprintf(stderr, "diag_sm : communications breakdown.\n");
        return(EXIT_FAILURE);
      }
   }
}
 
static void usage(void)
{ fputs("diag_sm : usage, diag_sm [all | most | none | redirect | revert]\n",
		stderr);
  fputs("        : or,    diag_sm create | delete | monitor | memory ...\n",
  		stderr);
  exit(EXIT_FAILURE);
}

/**
*** This gives some help information to tell the user what the various
*** options do.
**/
static char *text1 = "\
Create : this gives information whenever a window is registered or\n\
when a new session is started.\n\
";
static char *text2 = "\
Delete : when a session is aborted this option produces some diagnostics.\n\
";
static char *text3 = "\
Monitor : the Session Manager regularly checks those parts of the network\n\
it is responsible for. This option makes the Session Manager report all\n\
such activity.\n\
";
static char *text4 = "\
Memory : this causes the Session Manager to report on its current memory\n\
usage.\n\n\
";

static void show_help(void)
{ FILE	*output = popen("more", "w");
  bool	real_output = TRUE;
  
  if (output eq Null(FILE))
   { output = stdout; real_output = FALSE; }
  fputs(text1,  output);   
  fputs(text2,  output);
  fputs(text3,  output);
  fputs(text4,  output);
  fflush(output);
  if (real_output) pclose(output);
  init_terminal();
}

/**
*** This routine performs a message transaction with the Session Manager
*** A private protocol is used (actually a Stream protocol, but who cares).
*** The Session Manager should send back the new mask.
**/
static word	sm_private(Object *sm, int mask)
{ MCB		m;
  WORD		control[IOCMsgMax];
  BYTE		data[IOCDataMax];
  Port		reply_port = NewPort();
  WORD		rc;
  
  InitMCB(&m, MsgHdr_Flags_preserve, NullPort, reply_port, FC_GSP + FG_GetInfo);
  m.Control	= control;
  m.Data	= data;
  MarshalCommon(&m, sm, Null(char));
  MarshalWord(&m, mask);
  
  SendIOC(&m);
  m.MsgHdr.Dest	= reply_port;
  rc = GetMsg(&m);
  FreePort(reply_port);
  return(rc);
}

/**
*** sm_redirect(). This sends a message to the Session Manager requesting
*** it to redirect its standard output stream.
**/
static void	sm_redirect(Object *sm)
{ MCB	m;
  WORD	control[IOCMsgMax];
  BYTE	data[IOCDataMax];
  Port	reply_port = NewPort();
  WORD	rc;
  
  InitMCB(&m, MsgHdr_Flags_preserve, NullPort, reply_port, FC_GSP + FG_GetInfo);
  m.Control	= control;
  m.Data	= data;
  MarshalCommon(&m, sm, Null(char));
  MarshalWord(&m, dbg_Redirect);
  MarshalStream(&m, Heliosno(stdout));
  SendIOC(&m);
  m.MsgHdr.Dest = reply_port;
  rc = GetMsg(&m);
  FreePort(reply_port);
  if (rc ne Err_Null)
   { fprintf(stderr, "diag_sm: error redirecting output of sm, fault 0x%08x\n",
   		rc);
     exit(EXIT_FAILURE);
   }  
}

static void	sm_revert(Object *sm)
{ MCB	m;
  WORD	control[IOCMsgMax];
  BYTE	data[IOCDataMax];
  Port	reply_port = NewPort();
  WORD	rc;
  
  InitMCB(&m, MsgHdr_Flags_preserve, NullPort, reply_port, FC_GSP + FG_GetInfo);
  m.Control	= control;
  m.Data	= data;
  MarshalCommon(&m, sm, Null(char));
  MarshalWord(&m, dbg_Revert);

  SendIOC(&m);
  m.MsgHdr.Dest = reply_port;
  rc = GetMsg(&m);
  FreePort(reply_port);
  if (rc ne Err_Null)
   { fprintf(stderr, "diag_sm: error reverting output of sm, fault 0x%08x",
   		rc);
     exit(EXIT_FAILURE);
   }  
}

/**
*** Set the terminal to raw, single character mode
**/
static void init_terminal(void)
{ Attributes	Attr;
  static	bool first_time = TRUE;
  
  unless(isatty(fileno(stdin)))
   { fprintf(stderr, "diag_sm : not running interactively.\n");
     exit(EXIT_FAILURE); 
   } 

  if (first_time)
   { setvbuf(stdin, Null(char), _IONBF, 0);
    first_time = FALSE;
   }
  (void) GetAttributes(Heliosno(stdin), &Attr);
  AddAttribute(		&Attr, ConsoleRawInput);
  AddAttribute(		&Attr, ConsoleEcho);
  AddAttribute(		&Attr, ConsoleBreakInterrupt);
  RemoveAttribute(	&Attr, ConsoleIgnoreBreak);
  AddAttribute(		&Attr, ConsolePause);
  RemoveAttribute(	&Attr, ConsoleRawOutput);
  (void) SetAttributes(Heliosno(stdin), &Attr);
}

