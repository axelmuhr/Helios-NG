/* $Header: rawdig.c,v 1.1 91/01/31 13:52:26 charles Locked $ */
/* $Source: /server/usr/users/b/charles/world/microlink/RCS/source/rawdig.c,v $ */

/*------------------------------------------------------------------------*/
/*                                             microlink/source/rawDig.c  */
/*------------------------------------------------------------------------*/

/* Raw Digitiser version */

/*------------------------------------------------------------------------*/
/*                                                          Header Files  */
/*------------------------------------------------------------------------*/

# include "microlink/private/microlink.h"
# include "microlink/private/rawdig.h"

/*------------------------------------------------------------------------*/
/*                                               Look-ahead declarations  */
/*------------------------------------------------------------------------*/

void mlkAddRawDigEntry(DirNode *microlinkDir);
void mlkServeRawDig( ServInfo *si , MicrolinkNode *nde , MsgBuf *rply );
static void RawDigDefault(MCB *msg);
static void RawDigEnEv(MCB *msg,RawDigContext *ctx);
static void RawDigDisableEvents(RawDigContext *ctx);
static void RawDigTransmitEvents(RawDigContext *ctx);
static void RawDigPacketHandler(byte *buf,RawDigContext *ctx);
static void RawDigGetPacket ( RawDigEvent    *des,
                              RawDigContext  *ctx
                            );

/*------------------------------------------------------------------------*/
/*                                                    The RawDig handler  */
/*------------------------------------------------------------------------*/

/* This RawDig server uses one handler which is registered with the       */
/*    executive microlink driver. The handler structure is static and     */
/*    defined below. It is static so that the routine that traps the      */
/*    kill signal can detach it before the program is exited.             */

ML_MsgHandler RawDigMsgHandler;

/*------------------------------------------------------------------------*/
/*                                                mlkAddRawDigEntry(...)  */
/*------------------------------------------------------------------------*/

void mlkAddRawDigEntry(DirNode *microlinkDir)
/* Add an entry into the microlink main directory corresponding to the    */
/*    RawDig.                                                          */
{  static MicrolinkNode nde;

   InitNode((ObjNode*)&nde,"rawdig",Type_File,0,DefFileMatrix);
   nde.objTyp = MlkRawDig;
   Insert(microlinkDir,(ObjNode*)&nde.obj,TRUE);
}

/*------------------------------------------------------------------------*/
/*                                                   mlkServeRawDig(...)  */
/*------------------------------------------------------------------------*/

void mlkServeRawDig( ServInfo *si , MicrolinkNode *nde , MsgBuf *rply )
/* Usual open-file request server */
{  RawDigContext     ctx;
   RawDigEvent      *rpb;
   MCB              *msg;
   Port             reqp;
   byte            *data;
   word              res;

   if(nde->obj.Account>0)                /* Already opened?               */
   {  ErrorMsg(si->m,EC_Error|EG_InUse|EO_Object);
      UnLockTarget(si);                  /* Send error message, unlock    */
      return;                            /*   target and return           */
   }                                     /* .. End of Interlock code      */

   rpb = Malloc(RawDigBuffSize*sizeof(RawDigEvent));
   if(rpb==NULL)
   {  ErrorMsg(si->m,EC_Error|EG_NoMemory);
      UnLockTarget(si);
      return;
   }

   ctx.base        = rpb;
   ctx.lim         = rpb+RawDigBuffSize;
   ctx.rp          = rpb;
   ctx.wp          = rpb;
   InitSemaphore(&ctx.sem,0);
   ctx.maxsem      = RawDigBuffSize-1;
   ctx.tm          =   0;
   ctx.eventPort   = NullPort;
   ctx.req         =   0;
   InitSemaphore(&ctx.ack,0);
   ctx.rph         = &RawDigMsgHandler;
   ctx.rph->msgType= MLErawdg;
   ctx.rph->func   = (void(*)(byte*,word))RawDigPacketHandler;
   ctx.rph->arg    = (void*)&ctx;
   ctx.cyc         = 0;
   
   /* Here we register a function which is asynchronously entered         */
   /*   whenever a RawDig data-block comes in over the microlink: it      */
   /*   places the data-block into a buffer (trapping buffer overflow     */
   /*   conditions) and then returns. The data from the buffer is then    */
   /*   read out and processed on demand by the event handler loop.       */

   if(ML_RegisterHandler(ctx.rph)<0)
   {  ErrorMsg(si->m,EC_Error|EG_Broken|EO_Object);
      UnLockTarget(si);
      return;
   }

   nde->obj.Account++;                   /* Increment no. of users        */
   UnLockTarget(si);                     /* Unlock target for other calls */
   PutMsg(&rply->mcb);                   /* Send reply                    */
   reqp = rply->mcb.MsgHdr.Reply;        /* Dereference request-port      */
   msg  = si->m;                         /* MCB to accept replies on      */
   data = msg->Data;                     /* Where to put message data blk */

   for(;;)                               /* Loop serving requests         */
   {  msg->MsgHdr.Dest = reqp;           /* Going to wait on this port    */
      msg->Timeout = (word)(StreamTimeout);/* Set timeout waiting for reqs*/
      msg->Data        = data;           /* Set where to put data         */
      res = GetMsg(msg);                 /* Wait for message              */
      if(res==EK_Timeout)  break;        /* Trap timeouts.                */
      if(res <Err_Null) continue;        /* If another error - retry      */
      switch(msg->MsgHdr.FnRc&FG_Mask)   /* Switch according to request   */
      {  case FG_Close:                  /* Close request:                */
            goto RawDigClose;               /*  Break out of service loop    */
         case FG_EnableEvents:           /* Events Request:               */
            RawDigEnEv(msg,&ctx);     /*  Enable events                */
            break;                       /*  Get another request          */
         default:                        /* Unknown message ...           */
            RawDigDefault(msg);       /*  Action on unknown message    */
            break;                       /*  Get another request          */
      }
   }

   RawDigClose:;

   if(ctx.eventPort!=NullPort) 
      RawDigDisableEvents(&ctx); /* Disable events                        */
   ML_DetachHandler(ctx.rph);    /* Detach the raw packet handler         */

   Wait(&nde->obj.Lock);     /* Gain access to the RawDig node            */
   nde->obj.Account--;       /* Decrement no. of users.                   */
   Signal(&nde->obj.Lock);   /* Relinquish access.                        */

   if(msg->MsgHdr.Reply!=NullPort) ErrorMsg(msg,(word)Err_Null);
}

/*------------------------------------------------------------------------*/
/*                                                 RawDigDefault(...)  */
/*------------------------------------------------------------------------*/

static void RawDigDefault(MCB *msg)
/* This function is called whenever an unimplemented function code is     */
/*    discovered over the open-stream request port.                       */
{  ErrorMsg(msg,EC_Error|EG_Unknown|EO_Object);  }

/*------------------------------------------------------------------------*/
/*                                                       RawDigEnEv(...)  */
/*------------------------------------------------------------------------*/

static void RawDigEnEv(MCB *msg,RawDigContext *ctx)
/* This function to enable/disable RawDig events by forking a process     */
/*    to pick RawDigtiser packets out of the input buffer, processes them,*/
/*    and send them as "raw digitiser packet" events                      */
{  int en;

   en = ((byte*)msg->Control)[0];
   if(en==0)
   {  /* Here if request is to disable events ... */
      /* Only applies if already enabled ...      */
      if(ctx->eventPort!=NullPort) RawDigDisableEvents(ctx);
      /* Send reply message ...                   */
      InitMCB(msg,0,msg->MsgHdr.Reply,NullPort,(word)Err_Null);
      ((byte*)msg->Control)[0] = en;
      msg->MsgHdr.ContSize     =  1;
      PutMsg(msg);
   } else
   {  /* Here if request is to enable events ... */
      /* Only if not yet enabled ... */
      if(ctx->eventPort==NullPort)
      {  /* Set up event process port description and so-forth: */
         ctx->eventPort     = msg->MsgHdr.Reply;
         /* Send aknowledge message down event port: */
         InitMCB
         (  msg,
            MsgHdr_Flags_preserve,
            msg->MsgHdr.Reply,
            NullPort,
            (word)Err_Null
         );
         ((byte*)msg->Control)[0] =  1;
         msg->MsgHdr.ContSize     =  1;
         PutMsg(msg);
         /* Finally, fork the event transmitter: */
         Fork(ReqStackSize,RawDigTransmitEvents,4,ctx);
         /* Return to avoid sending another reply: */
         return;
      } else
      {  /* Here if attempt to enable another event stream: wrong */
         ErrorMsg(msg,EC_Error|EG_InUse|EO_Object);
         return;
      }
   }

}

/*------------------------------------------------------------------------*/
/*                                              RawDigDisableEvents(...)  */
/*------------------------------------------------------------------------*/

static void RawDigDisableEvents(RawDigContext *ctx)
{  /* This function should only be called once events have been enabled   */

   /* This function is used to disable events. Unfortunately, the event   */
   /*   generation goes on in a different thread, and it is not easy in   */
   /*   Helios to either kill a specific process or send a signal to it:  */
   /* The approach taken here is to set a flag requesting to the event    */
   /*   thread to tidy up and kill itself: The event thread polls this    */
   /*   several times during each event generation cycle. Unfortunately   */
   /*   this is not good enough as the event thread may be blocking in    */
   /*   one of two places: Either the place where it is waiting to send   */
   /*   the event back to the user or the place where it is waiting for   */
   /*   a new packet of raw RawDig information to come in on the          */
   /*   microlink. In the first instance the event process is blocking on */
   /*   a message port: This function 'aborts' that port in order to      */
   /*   unblock it. In the second case the process is waiting on a        */
   /*   hardened semaphore which gets signalled by RawDigRawPacketHandler */
   /*   whenever new raw information comes in. This process could signal  */
   /*   that semaphore in order to unblock the events.                    */
   /*  Hence if this process always gains access to 'ctx->acc' prior to   */
   /*   signalling the semaphore 'ctx->req' we avoid the above problem.   */
   /* Once we have set 'ctx->req', aborted 'ctx->eventPort' and signalled */
   /*   'ctx->sem' we wait for the event process to signal 'ctx->ack' to  */
   /*   indicate that it is about to terminate.                           */

   ctx->req = 1;                     /* Request event thread to terminate */
   AbortPort(ctx->eventPort,0);      /* Unblock events port               */
   HardenedSignal(&ctx->sem);        /* Signal ctx->sem                   */
   Wait(&ctx->ack);                  /* Wait for event thread aknowledge  */
   FreePort(ctx->eventPort);         /* Free events port                  */
   ctx->req       =        0;        /* Reset termination request         */
   ctx->eventPort = NullPort;        /* Reset events port descriptior     */
}

/*------------------------------------------------------------------------*/
/*                                             RawDigTransmitEvents(...)  */
/*------------------------------------------------------------------------*/

static void RawDigTransmitEvents(RawDigContext *ctx)
/* This function runs in a forked process: It takes raw RawDig data       */
/*   packages from the input buffer, processes them, and transmits them   */
/*   as events ...                                                        */
{  RawDigEvent   event;
   MCB             mcb;

   ctx->tm  = 0; /* Event time-stamp counter */

   for(;;)
   {  RawDigGetPacket(&event,ctx);
      if(ctx->req) break;  /* Trap kill-request */
      /* Set up event package to send to client ...                  */
      InitMCB
      (  &mcb,
         (word)MsgHdr_Flags_preserve,
         ctx->eventPort,
         NullPort,
         (word)EventRc_IgnoreLost
      );
      mcb.Data            = (byte*)&event;
      mcb.MsgHdr.DataSize = sizeof(RawDigEvent);
      PutMsg(&mcb);
      if(ctx->req) break;  /* Trap kill-request */
   }

   Signal(&ctx->ack);  /* Aknowledge kill-request */
}

/*------------------------------------------------------------------------*/
/*                                           RawDigRawPacketHandler(...)  */
/*------------------------------------------------------------------------*/

#pragma -s1 /* Disable stack checking in this function */

static void RawDigPacketHandler(byte *buf,RawDigContext *ctx)
/* This raw packet handler run in a very limited supervisor state so      */
/*    is hardly allowed to call any functions at all. In fact, it only    */
/*    copies the packet received into the buffer referred to by <ctx>.    */
{  RawDigEvent       *wp;

   ctx->cyc++;  /* Increment cycle number for each packet received */

   /* Check whether we are in an overflow situation (i.e. buffer full)    */
   /*    by checking the count on the semaphore which signals each        */
   /*    packet's entry into the buffer. Whilst this count is above a     */
   /*    threshold, no packets are entered into the buffer.               */
   if(ctx->sem.Count>=ctx->maxsem) return;
   
   /* Get pointer to next packet structure in buffer to which the raw     */
   /*    data can be copied:                                              */
   wp = ctx->wp;
   
   wp->pkt = *(MLErawdgPacket*)buf;

   /* Store the current cycle number with the complete x- and y- raw      */
   /*   packets stored.                                                   */
   wp->cyc = ctx->cyc;

   /* Increment buffer write pointer and cycle it if necessary:           */
   if(++wp>=ctx->lim) wp=ctx->base;
   ctx->wp = wp;

   /* Signal the entry of a new packet into the buffer. */
   HardenedSignal(&ctx->sem);
   
}

#pragma -s0

/*------------------------------------------------------------------------*/
/*                                                  RawDigGetPacket(...)  */
/*------------------------------------------------------------------------*/

static void RawDigGetPacket ( RawDigEvent    *des,
                              RawDigContext  *ctx
                            )
/* This function is supposed to wait until data becomes availiable in the */
/*   buffer containing raw RawDigitizer data. When data beomes availiable */
/*   it copies it into the destination                                    */
{  RawDigEvent *rp;

   if(ctx->req) return; /* Trap kill-request */

   /* Wait for a packet to be availiable using hardened wait: The         */
   /*    semaphore count tells how many packets are in the buffer.        */
   HardenedWait(&ctx->sem);
   
   if(ctx->req) return; /* Trap kill-request */

   rp = ctx->rp;
   *des = *rp;
   rp++;
   if(rp>=ctx->lim) rp=ctx->base;
   ctx->rp = rp;

}

