/*------------------------------------------------------------------------
--                                                                      --
--           H E L I O S   N E T W O R K I N G   S O F T W A R E	--
--           ---------------------------------------------------	--
--                                                                      --
--             Copyright (C) 1990, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- diag_tfm.c								--
--                                                                      --
--	Enable/disable diagnostics in the Taskforce Manager		--
--                                                                      --
--	Author:  BLV 1/5/90						--
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Header: /users/nickc/RTNucleus/network/RCS/diag_tfm.c,v 1.7 1993/12/20 13:51:39 nickc Exp $*/

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
#include "session.h"
#include "private.h"
#include "tfmaux.h"

static void	show_help(void);
static void	usage(void);
static word	tfm_private(Object *, int);
static void	tfm_redirect(Object  *);
static void	tfm_revert(Object *);
static void	init_terminal(void);

/**
*** The command can be used in two different ways. First, it can be
*** given the argument all, most or none on the command line. In that
*** case the appropriate debugging options are enabled/disabled.
*** Other command line options include redirect, revert, and the first
*** couple of letters of the known debugging options.
*** Otherwise the program goes into a little menu system. Every time
*** around the loop the current debugging options are displayed,
*** and the user presses a key to decide what to do.
**/
int main(int argc, char **argv)
{ WORD		mask;
  WORD		new_mask;
  Object 	*tfm;
  int		c;
  bool		interactive	= TRUE;
  bool		disable		= FALSE;
  bool		most		= FALSE;
  bool		redirect	= FALSE;
  bool		revert		= FALSE;
  bool		all		= FALSE;
  int		i;
  char		*tfm_name	= Null(char);
    
  new_mask = 0;  
  for (i = 1; i < argc; i++) 
   { char *temp = argv[i];

     if (*temp eq '/')
      { tfm_name = temp; continue; }

     interactive = FALSE;

     if (!strcmp(temp, "all"))
      { all = TRUE; continue; }
     if (!strcmp(temp, "most"))
      { most = TRUE; continue; }
     if (!strcmp(temp, "none"))
      { disable = TRUE; continue; }
     if (!strcmp(temp, "redirect"))
      { redirect = TRUE; continue; }
     if (!strcmp(temp, "revert"))
      { revert = TRUE; continue; }
      
     if (!strncmp(temp, "cre", 3))
      { new_mask ^= dbg_Create; continue; }
     if (!strncmp(temp, "map", 3))
      { new_mask ^= dbg_Mapping; continue; }
     if (!strncmp(temp, "mon", 3))
      { new_mask ^= dbg_Monitor; continue; }
     if (!strncmp(temp, "env", 3))
      { new_mask ^= dbg_Environ; continue; }
     if (!strncmp(temp, "del", 3))
      { new_mask ^= dbg_Delete; continue; }
     if (!strncmp(temp, "sig", 3))
      { new_mask ^= dbg_Signal; continue; }
     if (!strncmp(temp, "allo", 4))
      { new_mask ^= dbg_Allocate; continue; }
     if (!strncmp(temp, "rel", 3))
      { new_mask ^= dbg_Release; continue; }
     if (!strncmp(temp, "mem", 3))
      { new_mask |= dbg_Memory; continue; }
     if (!strncmp(temp, "com", 3))
      { new_mask ^= dbg_Comms; continue; }
     if (!strncmp(temp, "lock", 3))
      { new_mask |= dbg_Lock; continue; }
     if (!strncmp(temp, "ioc", 3))
      { new_mask |= dbg_IOC; continue; }

     usage();
   }

  if (tfm_name eq Null(char))
   tfm = RmGetTfm();
  else
   { char	buf[128];
     strcpy(buf, tfm_name);
     strcat(buf, "/tfm");
     tfm = Locate(Null(Object), buf);
   }

  if (tfm eq Null(Object))
   { fprintf(stderr, "diag_tfm : failed to locate Taskforce Manager.\n");
     return(EXIT_FAILURE);
   }
   
  if (!interactive)
   { if (redirect) tfm_redirect(tfm);
     if (revert) tfm_revert(tfm);
     
     if (disable)
      mask = 0;
     elif (most)
      mask = dbg_Create | dbg_Mapping | dbg_Monitor | dbg_Delete | dbg_Signal |
      		dbg_Allocate | dbg_Release;
     elif (all)
      mask = dbg_Create | dbg_Mapping | dbg_Monitor | dbg_Environ |
      	     dbg_Delete | dbg_Signal | dbg_Allocate | dbg_Release;
     else
      { mask = tfm_private(tfm, dbg_Inquire);
        if (mask < 0)
         { fprintf(stderr, "diag_tfm: communication breakdown.\n");
           return(EXIT_FAILURE);
         }
      }
     
     mask ^= new_mask;
     if (tfm_private(tfm, (int)mask) < 0)
      { fprintf(stderr, "diag_tfm : communication breakdown.\n");
        return(EXIT_FAILURE);
      }
     else
      exit(EXIT_SUCCESS);
   }
   
  mask = tfm_private(tfm, dbg_Inquire);
  if (mask < 0)
   { fprintf(stderr, "diag_tfm : communication breakdown.\n");
     return(EXIT_FAILURE);
   }

  init_terminal();   

  forever
   { 
     printf("\ndiag_tfm : debugging %s\n", tfm->Name);
     fputs("Current debugging options are :", stdout);
     if (mask & dbg_Create)	fputs(" create", stdout);
     if (mask & dbg_Mapping)	fputs(" mapping", stdout);
     if (mask & dbg_Monitor)	fputs(" monitor", stdout);
     if (mask & dbg_Environ)	fputs(" environ", stdout);
     if (mask & dbg_Delete)	fputs(" delete", stdout);
     if (mask & dbg_Signal)	fputs(" signal", stdout);
     if (mask & dbg_Allocate)	fputs(" allocate", stdout);
     if (mask & dbg_Release)	fputs(" release", stdout);
     if (mask & dbg_Comms)	fputs(" communications", stdout);
     if (mask eq 0)		fputs(" none", stdout);
     fputs("\n\n", stdout);
     
     fputs("1) create   2) mapping    3) monitor\n", stdout);
     fputs("4) environ  5) delete     6) signal\n", stdout);
     fputs("7) memory   8) allocate   9) release\n", stdout);
     fputs("Your choice (1-9, Q, ? ) ? ", stdout);
     fflush(stdout);
     
     for ( c = getchar(); isspace(c); c = getchar());
     putchar('\n');
     switch(c)
      { case 'q' :
        case 'Q' : return(EXIT_SUCCESS);
        case '?' :
        case 'h' :
        case 'H' : show_help(); break;
	case 'c' :
	case 'C' : mask ^= dbg_Comms;		break;
	case 'l' :
	case 'L' : mask |= dbg_Lock;		break;
	case 'i' :
	case 'I' : mask |= dbg_IOC;		break;

        case '1' : mask ^= dbg_Create; 		break;
        case '2' : mask ^= dbg_Mapping;		break;
        case '3' : mask ^= dbg_Monitor;		break;
        case '4' : mask ^= dbg_Environ;		break;
        case '5' : mask ^= dbg_Delete;		break;
        case '6' : mask ^= dbg_Signal;		break;
        case '7' : mask |= dbg_Memory;		break;
	case '8' : mask ^= dbg_Allocate;	break;
	case '9' : mask ^= dbg_Release;		break;
      }
     /* while (c != '\n') c = getchar(); */

     mask = tfm_private(tfm, (int)mask);
     if (mask < 0)
      { fprintf(stderr, "diag_tfm : communications breakdown.\n");
        return(EXIT_FAILURE);
      }
   }
}
 
static void usage(void)
{ fputs("diag_tfm : usage, diag_tfm [all | most | none | redirect | revert]\n",
		stderr);
  fputs("         : or,    diag_tfm create | mapping | monitor | ...\n", stderr);
  exit(EXIT_FAILURE);
}

/**
*** This gives some help information to tell the user what the various
*** options do.
**/
static char *text1 = "\
Create : this gives information about the whole taskforce creation process.\n\
Some of the phases can be debugged in more detail using the other options.\n\n\
";
static char *text2 = "\
Mapping : this makes the mapping algorithms provide some debugging\n\
information.\n\n\
";
static char *text3 = "\
Monitor : the taskforce manager has separate processes to monitor every\n\
component task in a taskforce, and an additional process to monitor the\n\
taskforce as a whole. These processes generate debugging when individual\n\
tasks terminate.\n\n\
";
static char *text4 = "\
Environ : this gives information about the environments sent to all the\n\
component tasks in a taskforce.\n\n\
";
static char *text5 = "\
Delete : this gives information when the taskforce is being deleted, either\n\
because it has terminated or because of a user request such as\n\
rm /bart/tfm/xyz.5\n\n\
";
static char *text6 = "\
Signal : this gives information about all signals sent to tasks or taskforces\n\n\
";
static char *text7 = "\
Memory : this causes the taskforce manager to display its current memory\n\
usage.\n\n\
";
static char *text8 = "\
Allocate : this gives information when applications such as the domain\n\
program allocate processors.\n\n\
";
static char *text9 = "\
Release : this gives information when applications have finished with\n\
processors, which may result in those processors being returned to the\n\
system pool.\n\n\
";

static void show_help(void)
{ FILE	*output = popen("more", "w");
  bool	real_output = TRUE;
  
  if (output eq Null(FILE))
   { output = stdout; real_output = FALSE; }
  fputs(text1, output);   
  fputs(text2, output);
  fputs(text3, output);
  fputs(text4, output);
  fputs(text5, output);
  fputs(text6, output);
  fputs(text7, output);
  fputs(text8, output);
  fputs(text9, output);
  fflush(output);
  if (real_output) pclose(output);
  init_terminal();
}

/**
*** This routine performs a message transaction with the Taskforce Manager.
*** A private protocol is used (actually a Stream protocol, but who cares).
*** The Taskforce Manager should send back the new mask.
**/
static word	tfm_private(Object *tfm, int mask)
{ MCB		m;
  WORD		control[IOCMsgMax];
  BYTE		data[IOCDataMax];
  Port		reply_port = NewPort();
  WORD		rc;
  
  InitMCB(&m, MsgHdr_Flags_preserve, NullPort, reply_port, FC_GSP + FG_GetInfo);
  m.Control	= control;
  m.Data	= data;
  MarshalCommon(&m, tfm, Null(char));
  MarshalWord(&m, mask);
  
  SendIOC(&m);
  m.MsgHdr.Dest	= reply_port;
  rc = GetMsg(&m);
  FreePort(reply_port);
  return(rc);
}

/**
*** tfm_redirect(). This sends a message to the Taskforce Manager requesting
*** it to redirect its standard output stream.
**/
static void	tfm_redirect(Object *tfm)
{ MCB	m;
  WORD	control[IOCMsgMax];
  BYTE	data[IOCDataMax];
  Port	reply_port = NewPort();
  WORD	rc;
  
  InitMCB(&m, MsgHdr_Flags_preserve, NullPort, reply_port, FC_GSP + FG_GetInfo);
  m.Control	= control;
  m.Data	= data;
  MarshalCommon(&m, tfm, Null(char));
  MarshalWord(&m, dbg_Redirect);
  MarshalStream(&m, Heliosno(stdout));
  SendIOC(&m);
  m.MsgHdr.Dest = reply_port;
  rc = GetMsg(&m);
  FreePort(reply_port);
  if (rc ne Err_Null)
   { fprintf(stderr, "diag_tfm: error redirecting output of tfm, fault 0x%08lx\n",
   		rc);
     exit(EXIT_FAILURE);
   }  
}

static void	tfm_revert(Object *tfm)
{ MCB	m;
  WORD	control[IOCMsgMax];
  BYTE	data[IOCDataMax];
  Port	reply_port = NewPort();
  WORD	rc;
  
  InitMCB(&m, MsgHdr_Flags_preserve, NullPort, reply_port, FC_GSP + FG_GetInfo);
  m.Control	= control;
  m.Data	= data;
  MarshalCommon(&m, tfm, Null(char));
  MarshalWord(&m, dbg_Revert);

  SendIOC(&m);
  m.MsgHdr.Dest = reply_port;
  rc = GetMsg(&m);
  FreePort(reply_port);
  if (rc ne Err_Null)
   { fprintf(stderr, "diag_tfm: error reverting output of tfm, fault 0x%08lx",
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
   { fprintf(stderr, "diag_tfm : not running interactively.\n");
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

