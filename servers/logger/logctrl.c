/*------------------------------------------------------------------------
--                                                                      --
--			H E L I O S   S E R V E R S			--
--			---------------------------			--
--                                                                      --
--             Copyright (C) 1991, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- logctrl.c								--
--                                                                      --
--	Administrative utility to control the behaviour of the		--
--	error logger.							--
--                                                                      --
--	Author:  BLV 19.3.91						--
--                                                                      --
------------------------------------------------------------------------*/

/* $Header: /dsl/HeliosRoot/Helios/servers/logger/RCS/logctrl.c,v 1.2 1994/03/14 16:00:47 tony Exp $ */


#include <helios.h>
#include <stdio.h>
#include <stdlib.h>
#include <syslib.h>
#include <gsp.h>
#include <codes.h>
#include <nonansi.h>
#include <string.h>
#include "logger.h"

#ifndef eq
#define eq ==
#define ne !=
#endif

static void	usage(void);
static void	logger_redirect(Object  *);
static void	logger_clear(Object *);
static void	logger_revert(Object *);
static void	logger_abort(Object *);

int main(int argc, char **argv)
{ Object 	*logger		= Null(Object);
  char		*logname	= Null(char);
  bool		redirect	= FALSE;
  bool		revert		= FALSE;
  bool		clear		= FALSE;
  bool		abort		= FALSE;
  int		i;

  for (i = 1; i < argc; i++)
   { char *temp = argv[i];

     if (!strcmp(temp, "redirect"))
      { if (revert || clear || abort) usage();
        redirect = TRUE;
        continue; 
      }
     if (!strcmp(temp, "revert"))
      { if (redirect || clear || abort) usage();
        revert = TRUE;
        continue;
      }      
     if (!strcmp(temp, "clear"))
      { if (redirect || revert || abort) usage();
        clear = TRUE;
        continue; 
      }
     if (!strcmp(temp, "abort"))
      { if (redirect || revert || clear) usage();
        abort = TRUE;
        continue;
      }

     if (temp[0] eq '/')
      logname = temp;
     else
      usage();
   }

  if (logname eq Null(char)) logname = "/logger";
  if (!redirect && !revert && !clear && !abort) redirect = TRUE;
  
  logger = Locate(Null(Object), logname);
  if (logger eq Null(Object))
   { fprintf(stderr, "logctrl: failed to locate %s\n", logname);
     return(EXIT_FAILURE);
   }

  if   (redirect) logger_redirect(logger);
  elif (revert)   logger_revert(logger);
  elif (clear)    logger_clear(logger);
  elif (abort)	  logger_abort(logger);
  return(EXIT_SUCCESS);
}
 
static void usage(void)
{ fputs("logctrl : usage, logctrl [redirect | revert | clear | abort] <logger>\n",
		stderr);
  exit(EXIT_FAILURE);
}

/**
*** logger_redirect(). This sends a message to the specified logger requesting
*** it to redirect its standard output stream.
**/
static void	logger_redirect(Object *logger)
{ MCB	m;
  WORD	control[IOCMsgMax];
  BYTE	data[IOCDataMax];
  Port	reply_port = NewPort();
  WORD	rc;
  
  InitMCB(&m, MsgHdr_Flags_preserve, NullPort, reply_port, FC_GSP + FG_GetInfo);
  m.Control	= control;
  m.Data	= data;
  MarshalCommon(&m, logger, Null(char));
  MarshalWord(&m, Logger_Redirect);
  MarshalStream(&m, Heliosno(stdout));
  SendIOC(&m);
  m.MsgHdr.Dest = reply_port;
  rc = GetMsg(&m);
  FreePort(reply_port);
  if (rc ne Err_Null)
   { fprintf(stderr, "logctrl: error redirecting output of %s, fault 0x%08lx\n",
   		logger->Name, rc);
     exit(EXIT_FAILURE);
   }  
}

static void	logger_revert(Object *logger)
{ MCB	m;
  WORD	control[IOCMsgMax];
  BYTE	data[IOCDataMax];
  Port	reply_port = NewPort();
  WORD	rc;
  
  InitMCB(&m, MsgHdr_Flags_preserve, NullPort, reply_port, FC_GSP + FG_GetInfo);
  m.Control	= control;
  m.Data	= data;
  MarshalCommon(&m, logger, Null(char));
  MarshalWord(&m, Logger_Revert);

  SendIOC(&m);
  m.MsgHdr.Dest = reply_port;
  rc = GetMsg(&m);
  FreePort(reply_port);
  if (rc ne Err_Null)
   { fprintf(stderr, "logctrl: error reverting output of %s, fault 0x%08lx\n",
   		logger->Name, rc);
     exit(EXIT_FAILURE);
   }  
}

static void	logger_clear(Object *logger)
{ MCB	m;
  WORD	control[IOCMsgMax];
  BYTE	data[IOCDataMax];
  Port	reply_port = NewPort();
  WORD	rc;
  
  InitMCB(&m, MsgHdr_Flags_preserve, NullPort, reply_port, FC_GSP + FG_GetInfo);
  m.Control	= control;
  m.Data	= data;
  MarshalCommon(&m, logger, Null(char));
  MarshalWord(&m, Logger_Clear);

  SendIOC(&m);
  m.MsgHdr.Dest = reply_port;
  rc = GetMsg(&m);
  FreePort(reply_port);
  if (rc ne Err_Null)
   { fprintf(stderr, "logctrl: error reverting output of %s, fault 0x%08lx\n",
   		logger->Name, rc);
     exit(EXIT_FAILURE);
   }  
}

static void	logger_abort(Object *logger)
{ MCB	m;
  WORD	control[IOCMsgMax];
  BYTE	data[IOCDataMax];
  Port	reply_port = NewPort();
  WORD	rc;

  InitMCB(&m, MsgHdr_Flags_preserve, NullPort, reply_port, FC_GSP + FG_GetInfo);
  m.Control	= control;
  m.Data	= data;
  MarshalCommon(&m, logger, Null(char));
  MarshalWord(&m, Logger_Abort);

  SendIOC(&m);
  m.MsgHdr.Dest = reply_port;
  rc = GetMsg(&m);
  FreePort(reply_port);
  if (rc ne Err_Null)
   { fprintf(stderr, "logctrl: error aborting %s, fault 0x%08lx\n",
   		logger->Name, rc);
     exit(EXIT_FAILURE);
   }  
}




