/*------------------------------------------------------------------------
--                                                                      --
--          H E L I O S   I N P U T / O U T P U T   S E R V E R         --
--          ---------------------------------------------------         --
--                                                                      --
--               Copyright (C) 1987, Perihelion Software Ltd.           --
--                          All Rights Reserved.                        --
--                                                                      --
--  ether.c                                                             --
--                                                                      --
--  Author:   AC April 90                                               --
--                                                                      --
------------------------------------------------------------------------*/

/* RcsId: $Id: ether.c,v 1.3 1992/05/04 19:22:04 craig Exp $ */
/* Copyright (C) 1987, Perihelion Software Ltd. 			*/

#include "helios.h"


PRIVATE bool     ethernet = FALSE;      /* do we have ethernet */       
PRIVATE unsigned char etheraddr[8];             /* ether net addr    */

#include "esupport.c"

PRIVATE BYTE etherbuff[MAXETHERPKT];           /* place to store a mess */

typedef struct NetInfo
{
        word Mask;
        word Mode;
        word State;
        byte Addr[8];
} NetInfo;

#define NetInfo_Mask_Mode   1
#define NetInfo_Mask_State  2
#define NetInfo_Mask_Addr   4 

PRIVATE NetInfo mynetinfo;

/*
void Ether_InitServer(Conode *myco)
{
  use(myco)
}
*/

void Ether_TidyServer(Conode *myco)
{
  tidyboard();
  use(myco)
}


void Ether_Testfun(result)
WORD* result;
{ WORD temp;
  char *board;
  
  if (!get_config("ETHERNET"))
   { *result = 0L; return; }

  if ((temp = get_int_config (HOST_PKT_INT)) == Invalid_config)
    pkt_int_no = -1 ;
  else
    pkt_int_no = (int) temp;


  if (!clarksonp()) 
   { *result = 0L;
     return;
   }


  ethernet = TRUE;
  *result  = 1L;
}

void Ether_GetAttr(Conode *myco)
{
  NetInfo* nip = (NetInfo*)(mcb->Data);

  nip->Mode = 0L;
  nip->State = 0L;
  memcpy(&nip->Addr[0],&etheraddr[0],8);
  
  mcb->MsgHdr.Dest = 0L;

  Request_Return(0L,0L,(WORD)sizeof(NetInfo));
  use(myco)
}

/*
void Ether_SetAttr(Conode *myco)
{
  Request_Return(EC_Error + SS_IOProc + EG_WrongFn + EO_Server, 0L, 0L);
  use(myco)
}

*/

void Ether_Open(myco)
Conode *myco;
{
  if ((((mcb->Control)[OpenMode_off] & 0x0F) ne O_WriteOnly)&&
      (((mcb->Control)[OpenMode_off] & 0x0F) ne O_ReadOnly))
  {
    Request_Return(EC_Error + SS_IOProc + EG_WrongFn +
                   EO_Server, 0L, 0L);
    return;
  }

  NewStream(Type_File, Flags_Closeable, NULL, Ether_Handlers);
  use(myco)
}


void Ether_Close(myco)
Conode *myco;
{
  if (mcb->MsgHdr.Reply ne 0L)
    Request_Return(ReplyOK, 0L, 0L);
  Seppuku();
  use(myco)
}


void Ether_Write(myco)
Conode* myco;
{
  word reply_port = mcb->MsgHdr.Reply;
  word asked      = (mcb->Control)[WriteSize_off];
  word actual     = mcb->MsgHdr.DataSize;
  word time_interval = divlong(mcb->Control[WriteTimeout_off], time_unit);

  /* Ethernet has a limit of 1514 bytes on a pkt */

  if (asked > MAXETHERPKT)
  {
    Request_Return(EC_Error + SS_IOProc + EG_WrongSize +
                  EO_Message, 0L,0L);
    notify ("TX error - bad packet size (%d bytes)", asked);
    return;
  }

  /*
   *  if asked = 0 return immediately
   */

  if (asked == 0)
  {
    mcb->Control[Reply1_off] = 0L;
    Request_Return(WriteRc_Done, 1L, 0L);
    return;
  }

  /*
   *  if there is any data it is the easy case 
   *  with the total write < 512 bytes       
   */


  myco->timelimit = Now + time_interval;

  unless (actual > 0) 
  {

    /*
     *  ok so we must have > 512 to transmit
     *  so ask the othe side to send it
     */


     mcb->Control[Reply1_off] = MAXETHERPKT;
     mcb->Control[Reply2_off] = MAXETHERPKT;
 
     mcb->MsgHdr.Flags = MsgHdr_Flags_preserve;
  
     Request_Return(WriteRc_Sizes, 2L, 0L);

     myco->timelimit = Now + time_interval;

     Suspend();
  
     if (myco->type eq CoSuicide) Seppuku();
    
     if(myco->type eq CoTimeout) 
     {
       Request_Return(EC_Warn | SS_IOProc | EG_Timeout | EO_Message, 0L, 0L); 
       return;
     }  
     actual = mcb->MsgHdr.DataSize;
  }


  unless (SendTxPkt(mcb->Data, (USHORT) actual)) 

  {
    memcpy(etherbuff,mcb->Data,(size_t)actual);
    
    AddTail(Remove(&(myco->node)),PollingCo);

    forever
    {
      myco->type = CoReady;
 
      Suspend();

      if (myco->type eq CoSuicide) Seppuku();
    
      if (myco->type eq CoTimeout) 
      {
        actual = 0;
        break;
      }  


      if (SendTxPkt(etherbuff, (USHORT) actual)) break;
    }

    PostInsert(Remove(&(myco->node)), Heliosnode);
  }
    
  mcb->MsgHdr.Reply = reply_port;
  mcb->Control[Reply1_off] = actual;
  Request_Return(WriteRc_Done,1L,0L); 

  return;
}


void Ether_Read(myco)
Conode* myco;
{
  WORD time_interval = divlong(mcb->Control[ReadTimeout_off],time_unit);
  WORD got;
  WORD toget = (mcb->Control)[ReadSize_off];
  
  Port reply_port = mcb->MsgHdr.Reply;

  AddTail(Remove(&(myco->node)),PollingCo);
  myco->timelimit = Now + time_interval;
  
  forever
  {


/*
-- crf: not by any means the best way to handle rx errors. However, sufficient
-- for our needs.
*/
    if (pkt_too_long
#ifdef ETHER_DEBUG
			|| pkt_overflow
#endif /* ETHER_DEBUG */
)
	notify_rx_error () ;
    if (pkt_rcvd)
    {
      got = GetRxPkt(mcb->Data,toget);
      break;
    }


    myco->type = CoReady;

    Suspend();

    if (myco->type eq CoTimeout)
    {
      got = 0L;
      break;
    }

    if (myco->type eq CoSuicide)
    {
      Seppuku();
    }
  }

  PostInsert(Remove(&(myco->node)),Heliosnode);

  mcb->MsgHdr.Reply = reply_port;
  mcb->MsgHdr.Dest  = 0L;

  if (Server_errno == 0) Server_errno = ReadRc_EOD;

  Request_Return(Server_errno,0L,got);
  
}

