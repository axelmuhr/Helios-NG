/*------------------------------------------------------------------------
--                                                                      --
--                     H E L I O S			                --
--                     -----------                                      --
--                                                                      --
--             Copyright (C) 1989, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- timeio.c								--
--                                                                      --
--	Author:  BLV 22/3/90						--
--                                                                      --
------------------------------------------------------------------------*/

#include <helios.h>	/* standard header */

/*--------------------------------------------------------
-- 		     Include Files			--
--------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include <codes.h>
#include <syslib.h>
#include <servlib.h>
#include <nonansi.h>
#include <stdlib.h>
#include <time.h>

/**
*** This program opens a stream to /logger in the  I/O Server and
*** bounces private protocol messages of it. These protocol messages
*** take the following function codes :
*** 0 FC_Private request control_size data_size
*** request = 1, for simple bounce
***           2, return message header only
***           3, send reply message with the specified size
**/

#define LL_Bounce (1 << 24)
#define LL_Empty  (2 << 24)
#define LL_Fill   (3 << 24)
#define C_shift   16

PRIVATE Port connect_to_server(string);
PRIVATE WORD message_handler(Port, int, int, int, int, int);
PRIVATE WORD results[20];
PRIVATE BYTE data_vec[65536];
PRIVATE WORD word_vec[256];
#define LOOPS 100
  
int main(void)
{ Port myport = connect_to_server("/logger");

  printf("Timeio : times are for %d bounces.\n", LOOPS);
  
  results[0] = message_handler(myport, 0, 0, LL_Bounce, 0, 0);
  printf("Message header only : %3d.%2d seconds\n\n",
         results[0] / 100, results[0] % 100);

  results[1] = message_handler(myport, 0, 64, LL_Empty, 0, 0);
  printf("Header + 64 bytes, -> I/O    : %3d.%2d seconds, %d K/s, max %d K/s\n",
         results[1] / 100, results[1] % 100,
         10000 / (16 * results[1]),
         10000 / (16 *(results[1] - results[0]))
        );  
  results[2] = message_handler(myport, 0, 0, LL_Fill, 0, 64);
  printf("Header + 64 bytes, <- I/O    : %3d.%2d seconds, %d K/s, max %d K/s\n",
         results[2] / 100, results[2] % 100,
         10000 / (16 * results[2]),
         10000 / (16 *(results[2] - results[0])) );  
  results[3] = message_handler(myport, 0, 64, LL_Bounce, 0, 0);
  printf("Header + 64 bytes, both ways : %3d.%2d seconds, %d K/s, max %d K/s\n\n",
         results[3] / 100, results[3] % 100,
         20000 / (16 * results[3]),
         20000 / (16 *(results[3] - results[0])) );           

  results[4] = message_handler(myport, 0, 1024, LL_Empty, 0, 0);
  printf("Header + 1024 bytes, -> I/O    : %3d.%2d seconds, %d K/s, max %d K/s\n",
         results[4] / 100, results[4] % 100,
         10000 / results[4],
         10000 / (results[4] - results[0])
         );  
  results[5] = message_handler(myport, 0, 0, LL_Fill, 0, 1024);
  printf("Header + 1024 bytes, <- I/O    : %3d.%2d seconds, %d K/s, max %d K/s\n",
         results[5] / 100, results[5] % 100,
         10000 / results[5],
         10000 / (results[5] - results[0])
         );  
  results[6] = message_handler(myport, 0, 1024, LL_Bounce, 0, 0);
  printf("Header + 1024 bytes, both ways : %3d.%2d seconds, %d K/s, max %d K/s\n\n",
         results[6] / 100, results[6] % 100,
         20000 / results[6],
         20000 / (results[6] - results[0])
         );  

  results[7] = message_handler(myport, 0, 32768, LL_Empty, 0, 0);
  printf("Header + 32768 bytes, -> I/O    : %3d.%2d seconds, %d K/s, max %d K/s\n",
         results[7] / 100, results[7] % 100,
         (10000 * 32) / results[7],
         (10000 * 32 ) / (results[7] - results[0])
         );  
  results[8] = message_handler(myport, 0, 0, LL_Fill, 0, 32768);
  printf("Header + 32768 bytes, <- I/O    : %3d.%2d seconds, %d K/s, max %d K/s\n",
         results[8] / 100, results[8] % 100,
         (10000 * 32) / results[8],
         (10000 * 32) / (results[8] - results[0])
         );  
  results[9] = message_handler(myport, 0, 32768, LL_Bounce, 0, 0);
  printf("Header + 32768 bytes, both ways : %3d.%2d seconds, %d K/s, max %d K/s\n\n",
         results[9] / 100, results[9] % 100,
         (20000 * 32) / results[9],
         (20000 * 32) / (results[9] - results[0])
         );  

  return(0);
}

PRIVATE WORD message_handler(Port destport, int csize, int dsize, int fnrc,
        int rcsize, int rdsize)
{ MCB  mcb;
  int  count;
  clock_t start;
  Port    reply_port = NewPort();
  
  mcb.Control = word_vec;
  mcb.Data    = data_vec;

  start = clock();
     
  for (count = 0; count < LOOPS; count++)
   {
     mcb.MsgHdr.Dest        = destport;
     mcb.MsgHdr.Reply       = reply_port;
     mcb.Timeout            = -1;
     mcb.MsgHdr.ContSize    = csize;
     mcb.MsgHdr.DataSize    = dsize;
     mcb.MsgHdr.Flags       = MsgHdr_Flags_preserve;
     mcb.MsgHdr.FnRc        = FC_Private + fnrc + (rcsize < C_shift) + rdsize;
     
     PutMsg(&mcb);

     mcb.MsgHdr.Dest  = mcb.MsgHdr.Reply;
     mcb.Timeout      = -1;

     GetMsg(&mcb);     
   }
   
  start = clock() - start;
  FreePort(reply_port);
  return(start);
}

PRIVATE Port connect_to_server(string name)
{ Object *server = Locate(Null(Object), name);
  Stream *stream;

  if (server == Null(Object))
   return(NullPort);
   
  stream = Open(server, Null(char), O_ReadWrite);
  if (stream == Null(Stream))
   { Close(server);
     return(NullPort);
   }
   
  Close(server);
  return(stream->Server);
}

