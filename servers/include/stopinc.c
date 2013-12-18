/*------------------------------------------------------------------------
--                                                                      --
--			H E L I O S   S E R V E R S			--
--			---------------------------			--
--                                                                      --
--             Copyright (C) 1991, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- stopinc.c								--
--                                                                      --
--	Program to terminate the include disk.				--
--                                                                      --
--	Author:  BLV 21.3.91						--
--                                                                      --
------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <helios.h>
#include <syslib.h>
#include <message.h>
#include <gsp.h>
#include <codes.h>

int main(void)
{ Object	*server = Locate(Null(Object), "/include");
  MCB		 m;
  WORD		 control[IOCMsgMax];
  BYTE		 data[IOCDataMax];
  Port		 reply_port;
  word		 rc;

  if (server == Null(Object))
   { fputs("stopinc: failed to locate /include server.\n", stderr);
     exit(EXIT_FAILURE);
   }

  reply_port = NewPort();
  InitMCB(&m, MsgHdr_Flags_preserve, NullPort, reply_port, FC_GSP + FG_Terminate);
  m.Control	= control;
  m.Data	= data;

  MarshalCommon(&m, server, Null(char));

  SendIOC(&m);
  m.MsgHdr.Dest	= reply_port;
  rc = GetMsg(&m);
  FreePort(reply_port);
  return (int)(rc);
}


