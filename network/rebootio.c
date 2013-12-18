/*------------------------------------------------------------------------
--                                                                      --
--           H E L I O S   N E T W O R K I N G   S O F T W A R E	--
--           ---------------------------------------------------	--
--                                                                      --
--             Copyright (C) 1990, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- rebootio.c								--
--                                                                      --
--	Program to reboot an I/O Server					--
--                                                                      --
--	Author:  BLV 25/5/90						--
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Header: /hsrc/network/RCS/rebootio.c,v 1.3 1993/08/11 10:42:33 bart Exp $*/

#include <stdio.h>
#include <syslib.h>
#include <message.h>
#include <stdlib.h>
#include <codes.h>
#include <gsp.h>
#include <task.h>
#include <string.h>
#define eq ==
#define ne !=

#ifndef FG_Reboot
#define FG_Reboot	0x00002FF0
#endif

/**
*** This program takes zero or one arguments. If zero then the program
*** will attempt to reboot the I/O Server corresponding to processor /IO,
*** the default. Otherwise it will attempt to stop the specified I/O processor.
*** Sending a message to /IO is difficult because it will be interpreted
*** by the name table code in the local processor rather than being
*** passed on to the actual I/O Server. Hence the message is sent to the
*** error logger inside the I/O processor, which is assumed to exist at
*** all times.
**/

static void usage(void);
static void send_terminate(Object *);

int main(int argc, char **argv)
{ Object *obj;
  char	 *server, servname[NameMax];
  
  if (argc > 2) usage();

  if (argc > 1)
   { server = *(++argv);
     if (*server ne '/')
      { servname[0] = '/';
        strncpy(&(servname[1]), server, NameMax - 1);
        servname[NameMax - 1] = '\0';
        server = servname;
      }
   }
  else
   server = "/IO";
   
  obj = Locate(Null(Object), server);
  if (obj == Null(Object))
   { fprintf(stderr, "rebootio : failed to locate %s\n", server);
     exit(EXIT_FAILURE);
   }  
  send_terminate(obj);
  Close(obj);
  return(EXIT_SUCCESS);
}

static void usage(void)
{
  fprintf(stderr, "Usage : rebootio <I/O Processor name>\n");
  exit(EXIT_FAILURE);
}

static void send_terminate(Object *server)
{	Object	*logger;
	MCB  mcb;
	WORD control_vec[IOCMsgMax];
	BYTE data_vec[IOCDataMax];
	
	logger = Locate(server, "logger");
	if (logger eq Null(Object))
	 { fprintf(stderr, "rebootio : failed to locate %s/logger.\n",
	           server->Name);
	   exit(EXIT_FAILURE);
	 }
	 
        mcb.Control = control_vec;
        mcb.Data    = data_vec;
	
	InitMCB(&mcb,MsgHdr_Flags_preserve,
		MyTask->IOCPort,NullPort, FC_GSP + FG_Reboot);

	MarshalCommon(&mcb,logger,Null(char));

	(void) PutMsg(&mcb);
	Close(logger);
}

