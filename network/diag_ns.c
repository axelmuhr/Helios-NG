/*------------------------------------------------------------------------
--                                                                      --
--           H E L I O S   N E T W O R K I N G   S O F T W A R E	--
--           ---------------------------------------------------	--
--                                                                      --
--             Copyright (C) 1990, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- diag_ns.c								--
--                                                                      --
--	Enable/disable diagnostics in the Network Server		--
--                                                                      --
--	Author:  BLV 1/5/90						--
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Header: /hsrc/network/RCS/diag_ns.c,v 1.6 1993/08/11 10:28:07 bart Exp $*/

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
#include "netaux.h"

static void	show_help(void);
static void	usage(void);
static word	ns_private(Object *, int);
static void	ns_redirect(Object  *);
static void	ns_revert(Object *);
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
  Object 	*ns;
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

     if (!strncmp(temp, "boo", 3))
      { new_mask ^= dbg_Boot; continue; }
     if (!strncmp(temp, "exe", 3))
      { new_mask ^= dbg_Execute; continue; }
     if (!strncmp(temp, "allo", 4))
      { new_mask ^= dbg_Allocate; continue; }
     if (!strncmp(temp, "rel", 3))
      { new_mask ^= dbg_Release; continue; }
     if (!strncmp(temp, "mon", 3))
      { new_mask ^= dbg_Monitor; continue; }
     if (!strncmp(temp, "pro", 3))
      { new_mask ^= dbg_Problem; continue; }
     if (!strncmp(temp, "lin", 3))
      { new_mask ^= dbg_Links; continue; }
     if (!strncmp(temp, "ini", 3))
      { new_mask ^= dbg_Initialise; continue; }
     if (!strncmp(temp, "mem", 3))
      { new_mask |= dbg_Memory; continue; }
     if (!strncmp(temp, "nat", 3))
      { new_mask ^= dbg_Native; continue; }
     if (!strncmp(temp, "com", 3))
      { new_mask ^= dbg_Comms; continue; }
     if (!strncmp(temp, "loc", 3))
      { new_mask |= dbg_Lock; continue; }
     if (!strncmp(temp, "ioc", 3))
      { new_mask |= dbg_IOC; continue; }
     usage();
   }

  ns = Locate(Null(Object), "/ns");
  if (ns eq Null(Object))
   { fprintf(stderr, "diag_ns : failed to locate Network Server.\n");
     return(EXIT_FAILURE);
   }
   
  if (!interactive)
   { if (redirect) ns_redirect(ns);
     if (revert)   ns_revert(ns);

     if (disable)
      mask = 0;
     elif (most)
      mask = dbg_Boot | dbg_Execute | dbg_Allocate | dbg_Release | 
      	     dbg_Problem | dbg_Links | dbg_Initialise | dbg_Native;
     elif (all)
      mask = dbg_Boot | dbg_Execute | dbg_Allocate | dbg_Release |
      	     dbg_Problem | dbg_Monitor | dbg_Links | dbg_Initialise | 
      	     dbg_Native;
     else
      { mask = ns_private(ns, dbg_Inquire);
        if (mask < 0)
         { fprintf(stderr, "diag_ns : communication breakdown.\n");
           return(EXIT_FAILURE);
         }
      }
      
     mask ^= new_mask;
     if ((mask = ns_private(ns, mask)) < 0)
      { fprintf(stderr, "diag_ns: communication breakdown.\n");
        return(EXIT_FAILURE);
      }
     else
      return(EXIT_SUCCESS);
   }
   
  mask = ns_private(ns, dbg_Inquire);
  if (mask < 0)
   { fprintf(stderr, "diag_ns : communication breakdown.\n");
     return(EXIT_FAILURE);
   }

  init_terminal();   

  forever
   { 
     printf("\ndiag_ns : debugging %s\n", ns->Name);
     fputs("Current debugging options are :", stdout);
     if (mask & dbg_Boot)	fputs(" boot", stdout);
     if (mask & dbg_Execute)	fputs(" execute", stdout);
     if (mask & dbg_Allocate)	fputs(" allocate", stdout);
     if (mask & dbg_Release)	fputs(" release", stdout);
     if (mask & dbg_Monitor)	fputs(" monitor", stdout);
     if (mask & dbg_Problem)	fputs(" problem", stdout);
     if (mask & dbg_Links)	fputs(" links", stdout);
     if (mask & dbg_Initialise)	fputs(" initialise", stdout);
     if (mask & dbg_Native)	fputs(" native", stdout);
     if (mask & dbg_Comms)	fputs(" communications", stdout);
     if (mask eq 0)		fputs(" none", stdout);
     fputs("\n\n", stdout);
     
     fputs("1) boot     2) execute    3) allocate\n", stdout);
     fputs("4) release  5) monitor    6) problem\n", stdout);
     fputs("7) links    8) initialise 9) memory\n", stdout);
     fputs("0) native   Your choice (1-0, Q, ? ) ? ", stdout);
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
        case '1' : mask ^= dbg_Boot; 		break;
        case '2' : mask ^= dbg_Execute;		break;
        case '3' : mask ^= dbg_Allocate;	break;
        case '4' : mask ^= dbg_Release;		break;
        case '5' : mask ^= dbg_Monitor;		break;
        case '6' : mask ^= dbg_Problem;		break;
        case '7' : mask ^= dbg_Links;		break;
        case '8' : mask ^= dbg_Initialise;	break;
        case '9' : mask |= dbg_Memory;		break; 
        case '0' : mask ^= dbg_Native;		break;
      }

     mask = ns_private(ns, mask);
     if (mask < 0)
      { fprintf(stderr, "diag_ns : communications breakdown.\n");
        return(EXIT_FAILURE);
      }
   }
}
 
static void usage(void)
{ fputs("diag_ns : usage, diag_ns [all | most | none | redirect | revert]\n",
		stderr);
  fputs("        : or,    diag_ns boot | execute | release | monitor | ...\n",
  		stderr);
  exit(EXIT_FAILURE);
}

/**
*** This gives some help information to tell the user what the various
*** options do.
**/
static char *text1 = "\
Boot : this gives information when processors are being booted or rebooted.\n\
Some of the phases can be debugged in more detail using the other options.\n\n\
";
static char *text2 = "\
Execute : the network server frequently has to execute programs inside the\n\
network. For its own operations is has to execute a network agent. Other\n\
programs include ones specified in the resource map and programs invoked\n\
by device drivers.\n\n\
";
static char *text3 = "\
Allocate : one of the network server's main jobs is allocating processors\n\
to users. This option displays information about such allocations.\n\n\
";
static char *text4 = "\
Release : when users have finished with processors those processors are\n\
returned to the system pool, following some cleaning up. This debugging\n\
option reports when processors are returned and details of the clean-up\n\
process.\n\n\
";
static char *text5 = "\
Monitor : the network server continuously monitors every active processor\n\
in the Helios network, to check for crashed processors. This option gives\n\
debugging whenever a processor is monitored.\n\n\
";
static char *text6 = "\
Problem : occasionally a problem may be reported with a particular processor.\n\
Following such a report the network server may try to find out what the\n\
problem was, and if necessary reboot the processor. Not all problems require\n\
rebooting, for example the processor may simply have run out of memory.\n\n\
";
static char *text7 = "\
Links : this debugging option produces output whenever the network server\n\
has to manipulate the links between processors, for example to isolate a\n\
crashed processor.\n\n\
";
static char *text8 = "\
Initialise: this options gives information when the network server starts up.\
\n\n\
";
static char *text9 = "\
Memory : this causes the network server to report on its current memory\n\
usage.\n\n\
";
static char *text10 = "\
Native : this gives information about native network operation such as\n\
terminating processors and reconfiguring links.\n\n\
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
  fputs(text5,  output);
  fputs(text6,  output);
  fputs(text7,  output);
  fputs(text8,  output);
  fputs(text9,  output);
  fputs(text10, output);
  fflush(output);
  if (real_output) pclose(output);
  init_terminal();
}

/**
*** This routine performs a message transaction with the Network Server.
*** A private protocol is used (actually a Stream protocol, but who cares).
*** The Network Server should send back the new mask.
**/
static word	ns_private(Object *ns, int mask)
{ MCB		m;
  WORD		control[IOCMsgMax];
  BYTE		data[IOCDataMax];
  Port		reply_port = NewPort();
  WORD		rc;
  
  InitMCB(&m, MsgHdr_Flags_preserve, NullPort, reply_port, FC_GSP + FG_GetInfo);
  m.Control	= control;
  m.Data	= data;
  MarshalCommon(&m, ns, Null(char));
  MarshalWord(&m, mask);
  
  SendIOC(&m);
  m.MsgHdr.Dest	= reply_port;
  rc = GetMsg(&m);
  FreePort(reply_port);
  return(rc);
}

/**
*** ns_redirect(). This sends a message to the Network Server requesting
*** it to redirect its standard output stream.
**/
static void	ns_redirect(Object *ns)
{ MCB	m;
  WORD	control[IOCMsgMax];
  BYTE	data[IOCDataMax];
  Port	reply_port = NewPort();
  WORD	rc;
  
  InitMCB(&m, MsgHdr_Flags_preserve, NullPort, reply_port, FC_GSP + FG_GetInfo);
  m.Control	= control;
  m.Data	= data;
  MarshalCommon(&m, ns, Null(char));
  MarshalWord(&m, dbg_Redirect);
  MarshalStream(&m, Heliosno(stdout));
  SendIOC(&m);
  m.MsgHdr.Dest = reply_port;
  rc = GetMsg(&m);
  FreePort(reply_port);
  if (rc ne Err_Null)
   { fprintf(stderr, "diag_ns: error redirecting output of ns, fault 0x%08x\n",
   		rc);
     exit(EXIT_FAILURE);
   }  
}

static void	ns_revert(Object *ns)
{ MCB	m;
  WORD	control[IOCMsgMax];
  BYTE	data[IOCDataMax];
  Port	reply_port = NewPort();
  WORD	rc;
  
  InitMCB(&m, MsgHdr_Flags_preserve, NullPort, reply_port, FC_GSP + FG_GetInfo);
  m.Control	= control;
  m.Data	= data;
  MarshalCommon(&m, ns, Null(char));
  MarshalWord(&m, dbg_Revert);

  SendIOC(&m);
  m.MsgHdr.Dest = reply_port;
  rc = GetMsg(&m);
  FreePort(reply_port);
  if (rc ne Err_Null)
   { fprintf(stderr, "diag_ns: error reverting output of ns, fault 0x%08x",
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
   { fprintf(stderr, "diag_ns : not running interactively.\n");
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

