                                                                                /*

  []-------------------------------------------------------------------------[]
   |                                                                         |
   |                 (c) 1989, 1991 by parsytec GmbH, Aachen                 |
   |                          All rights reserved.                           |
   |                                                                         |
   |-------------------------------------------------------------------------|
   |                                                                         |
   |  csync.c								     |
   |                                                                         |
   |-------------------------------------------------------------------------|
   |                                                                         |
   |  History:                                                               |
   |    2 - O. Imbusch  -  1 March   1991 - Update for the PFS v2.0-handbook |
   |    1 - H. J. Ermen -  6 April   1989 - Basic Version                    |
   |                                                                         |
  []-------------------------------------------------------------------------[]

                                                                                */

#include <codes.h>
#include <ctype.h>
#include <gsp.h>
#include <helios.h>
#include <message.h>
#include <stdio.h>
#include <stdlib.h>
#include <syslib.h>
#include <sys/types.h>

#include "misc.h"

#define FG_Private   0x000000F0
#define FO_ForceSync 0x00000006

#define SyncOK (0)

/** spec **********************************************************************/

PUBLIC INT Sync (IN STRING PathTo)

/******************************************************************************
**
**  PURPOSE:
**    Locates the file server with <PathTo> and sends a "private" message to the
**    server to signal a file system synchronisation request.
**
**  PARAMETERS:
**
**    In:
**      <PathTo>  Path to the file server
**
**  RETURN:
**    SyncOK      No error occured
**
**  EXAMPLE:
**
**    In:
**      <PathTo> = "/helios/bin/fs"
** 
**    Return:
**      SyncOK
**
*** endspec *******************************************************************/

{
  MCB   Mcb;
  word  E;
  Port  Reply;
  word  ControlV [IOCMsgMax];
  byte  DataV [IOCDataMax];
 
/******************************************************************************* 
**
**  -Get a port for the reply message
**  -Basic initialisation of the MessageControlBlock
**
*******************************************************************************/
 	
 Reply = NewPort ();			

 InitMCB (&Mcb, MsgHdr_Flags_preserve, MyTask->IOCPort, Reply, FC_GSP | SS_HardDisk | FG_Private | FO_ForceSync);

/******************************************************************************* 
**
**  -Preparing control and data vector
**
*******************************************************************************/

  Mcb.Control = ControlV;	
  Mcb.Data    = DataV; 	   
  MarshalCommon (&Mcb, Null (Object), PathTo);

/******************************************************************************* 
**
**  -Send the message to the server
**  -Expect the server's reply (infinite wait)
**  -Release the port
**  -Normal termination
**
*******************************************************************************/
					
 E = PutMsg (&Mcb);

 InitMCB (&Mcb, MsgHdr_Flags_preserve, Reply, NullPort, 0);
 Mcb.Timeout = MaxInt;
 GetMsg (&Mcb);

 FreePort (Reply);
 
 return (SyncOK);
}

/******************************************************************************* 
**
**  csync.c
**
*******************************************************************************/
