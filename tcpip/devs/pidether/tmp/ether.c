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

/* RcsId: $Id: ether.c,v 1.4 1992/06/19 09:47:31 bart Exp $ */
/* Copyright (C) 1987, Perihelion Software Ltd. 			*/

#include "helios.h"

#define MAXETHERPKT 1514

PRIVATE bool     ethernet = false;      /* do we have ethernet */       
PRIVATE unsigned int etherbase;         /* ether io port     */
PRIVATE long ethermem;                  /* ether ram address */
PRIVATE int      ethersize;             /* size of ether ram */

PRIVATE unsigned char etheraddr[8];             /* ether net addr    */
PRIVATE unsigned char ethermult[8];             /* multicast address */

PRIVATE unsigned char etherrcr;         /* ether receive config */
PRIVATE int      etherlevel;            /* ether interrupt level */     
PRIVATE byte etherbuff[MAXETHERPKT];           /* place to store a mess */

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


OPEN and INIT  
   etherbase = 0xFF;	/* set ether port base address */

   ethermem = -1;	/* set ether memory base address */

   etherrcr = 0x04;	/* accept broadcasts */

   etherlevel = -1;	/* set ethernet interrupt level */

  /* initialise ethernet board */
  if (!etherboardp()) 
     ServerDebug("I/O Server : /ether device not found");
     ServerDebug("I/O Server : base %x, mem %lx, etherrcr %d, level %d",
             etherbase, ethermem, etherrcr, etherlevel);

#if 0
	/* setup interrupt handler */
      init_int_vector();
#endif


void Ether_GetAttr(Conode *myco)
{
  NetInfo* nip = (NetInfo*)(mcb->Data);

  nip->Mode = 0L;
  nip->State = 0L;
  memcpy(&nip->Addr[0],&etheraddr[0],8);
  
  mcb->MsgHdr.Dest = 0L;

  Request_Return(0L,0L,(word)sizeof(NetInfo));
  use(myco)
}

void Ether_SetAttr(Conode *myco)
{
  Request_Return(EC_Error + SS_IOProc + EG_WrongFn + EO_Server, 0L, 0L);
  use(myco)
}


void Ether_Write(myco)
Conode* myco;
{

  /* Ethernet has a limit of 1514 bytes on a pkt */
  if (asked > MAXETHERPKT || asked == 0)
  {
    Request_Return(EC_Error + SS_IOProc + EG_WrongSize +
                  EO_Message, 0L,0L);
    ServerDebug("/ether - write Bad pkt Size");
    return;
  }


  ether_something();

  if (! SendTxPkt(mcb->Data,actual) )
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

      ether_something();	/* poll int handler */
      if (SendTxPkt(etherbuff,actual)) break;
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
  word time_interval = divlong(mcb->Control[ReadTimeout_off],time_unit);
  word got;
  word toget = (mcb->Control)[ReadSize_off];
  
  Port reply_port = mcb->MsgHdr.Reply;

  AddTail(Remove(&(myco->node)),PollingCo);
  myco->timelimit = Now + time_interval;
  
  forever
  {
	ether_something();

    if (RxAvail())
    {
      got = GetRxPkt(mcb->Data,toget);
      break;
    }

    if (ResetMe) resetboard();

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

