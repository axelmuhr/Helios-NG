/*------------------------------------------------------------------------
--                                                                      --
--           H E L I O S   N E T W O R K I N G   S O F T W A R E	--
--           ---------------------------------------------------	--
--                                                                      --
--             Copyright (C) 1990, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- stopns.c								--
--                                                                      --
--	Program to stop some or all of the Helios networking software	--
--                                                                      --
--	Author:  BLV 1/5/90						--
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Header: /hsrc/network/RCS/stopns.c,v 1.3 1993/08/11 10:51:35 bart Exp $*/

#include <stdio.h>
#include <syslib.h>
#include <message.h>
#include <stdlib.h>
#include <codes.h>
#include <gsp.h>
#include <task.h>

static void usage(void);
static void send_terminate(Object *);

int main(int argc, char **argv)
{ Object *obj;

  if (argc == 1) usage();
  for (argc--, argv++; argc > 0; argc--, argv++)
   { obj = Locate(Null(Object), *argv);
     if (obj == Null(Object))
      { fprintf(stderr, "Stopns : warning, failed to locate %s\n", *argv);
        continue;
      }  
     send_terminate(obj);
     Close(obj);
   }   
}

static void usage(void)
{
  fprintf(stderr, "Usage : stopns <server names>\n");
  exit(EXIT_FAILURE);
}

static void send_terminate(Object *server)
{
	word rc = Err_Null;
	MCB  mcb;
	WORD control_vec[IOCMsgMax];
	BYTE data_vec[IOCDataMax];
	Port reply;
	
        mcb.Control = control_vec;
        mcb.Data    = data_vec;
        
	reply = server->Reply;
	
	InitMCB(&mcb,MsgHdr_Flags_preserve,
		MyTask->IOCPort,reply, FC_GSP + FG_Terminate);

	MarshalCommon(&mcb,server,Null(char));

	if ((rc = PutMsg(&mcb)) < Err_Null) goto Done;

	mcb.Timeout = 5 * OneSec;
	mcb.MsgHdr.Dest = mcb.MsgHdr.Reply;
	if ((rc = GetMsg(&mcb)) < Err_Null) goto Done;
	
	rc = Err_Null;
    Done:

	if( mcb.MsgHdr.Reply != NullPort ) FreePort(mcb.MsgHdr.Reply);

	if (rc < Err_Null)
	 { if ((rc & EG_Mask) == EG_Protected)
	    fprintf(stderr, "Stopns : server %s will not terminate.\n", 
	    		server->Name);
	   else
	    fprintf(stderr,
	  "Stopns : message interaction failure with server %s, fault 0x%08x\n",
	           server->Name, rc);
	 }
	return;
}

