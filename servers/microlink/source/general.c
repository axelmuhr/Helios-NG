/* $Header: general.c,v 1.2 91/01/09 12:30:40 charles Locked $ */
/* $Source: /server/usr/users/b/charles/world/microlink/RCS/source/general.c,v $ */

/*------------------------------------------------------------------------*/
/*                                            microlink/source/general.c  */
/*------------------------------------------------------------------------*/

/* This server routine implements the "general" part of the microlink     */
/*   protocol. This part allows other tasks to send arbitrary messages    */
/*   to the microlink and receive the reply. Also it allows the client    */
/*   to "enable events" ie. to make the server send an event for each     */
/*   message of a particular type down an event port. These general       */
/*   functions could be implemented directly by executive microlink       */
/*   functions however it is better to handle them via. a server for      */
/*   several resons:                                                      */
/*                                                                        */
/* 1. Device / Executive interface independence.                          */
/* 2. Client task can run on different processor from microlink           */
/* 3. Interlocking of communications from several clients.                */
/*                                                                        */
/* Of these the last point is highly significant.                         */

/*------------------------------------------------------------------------*/
/*                                                          Header Files  */
/*------------------------------------------------------------------------*/

# include "microlink/private/microlink.h"
# include "microlink/private/general.h"

/*------------------------------------------------------------------------*/
/*                                               Look-ahead declarations  */
/*------------------------------------------------------------------------*/

void        mlkAddGeneralEntry      ( DirNode *microlinkDir );
void        mlkServeGeneral         ( ServInfo        *si ,
                                      MicrolinkNode  *nde ,
                                      MsgBuf        *rply 
                                    );
static void GeneralDefault          ( MCB *msg );
static void GeneralSetInfo          ( MCB *msg,GeneralContext *ctx );
static void GeneralGetInfo          ( MCB *msg,GeneralContext *ctx );
static void GeneralEnEv             ( MCB *msg,GeneralContext *ctx );
static GeneralEventChannel *GeneralSetupEvent
                                    (  GeneralContext *ctx,
                                       MCB            *msg,
                                       int             typ
                                    );
static void GeneralDisableEvent     ( GeneralEventChannel *evt );
static void GeneralTransmitEvents   ( GeneralEventChannel *evt );
static void GeneralRawPacketHandler ( byte *buf,GeneralEventChannel *evt );

/*------------------------------------------------------------------------*/
/*                                               mlkAddGeneralEntry(...)  */
/*------------------------------------------------------------------------*/

void mlkAddGeneralEntry(DirNode *microlinkDir)
/* Add an entry into the microlink main directory corresponding to the    */
/*    general microlink protocol server.                                  */
{  static MicrolinkNode nde;

   InitNode((ObjNode*)&nde,"general",Type_File,0,DefFileMatrix);
   nde.objTyp = MlkGeneral;
   Insert(microlinkDir,(ObjNode*)&nde.obj,TRUE);
}

/*------------------------------------------------------------------------*/
/*                                                  mlkServeGeneral(...)  */
/*------------------------------------------------------------------------*/

void mlkServeGeneral( ServInfo *si , MicrolinkNode *nde , MsgBuf *rply )
/* This function is called whenever a stream is opened to the general     */
/*     microlink services accessed at "/microlink/general".               */
/* It is passed the server-info structure corresponding to the request to */
/*    open the stream, and also a MsgBuf structure pointer for the        */
/*    already set-up reply message that should get sent back indicatating */
/*    a request port over which to conduct further transfers.             */
{  GeneralContext       ctx;
   GeneralEventChannel *evt;
   GeneralEventChannel *evn;
   MCB                 *msg;
   Port                reqp;
   byte               *data;
   word                 res;

   /* Initialise most fields of the general microlink context here.       */

   UnLockTarget(si);                     /* Unlock target for other calls */
   PutMsg(&rply->mcb);                   /* Send reply                    */
   reqp = rply->mcb.MsgHdr.Reply;        /* Dereference request-port      */
   msg  = si->m;                         /* MCB to accept replies on      */
   data = msg->Data;                     /* Where to put message data blk */
   ctx.isInRq  = 0;                      /* Setup context ...             */
   ctx.evts    = NULL;                   /* ... continued                 */

   for(;;)                               /* Loop serving requests         */
   {  msg->MsgHdr.Dest = reqp;           /* Going to wait on this port    */
      msg->Timeout = (word)(StreamTimeout); /*Set timeout waiting for reqs*/
      msg->Data        = data;           /* Set where to put data         */
      res = GetMsg(msg);                 /* Wait for message              */
      if(res==EK_Timeout)  break;        /* Trap timeouts.                */
      if(res <Err_Null) continue;        /* If another error - retry      */
      switch(msg->MsgHdr.FnRc&FG_Mask)   /* Switch according to request   */
      {  case FG_Close:                  /* Close request:                */
            goto GeneralClose;           /*  Break out of service loop    */
         case FG_EnableEvents:           /* Events Request:               */
            GeneralEnEv(msg,&ctx);       /*  Enable events                */
            break;                       /*  Get another request          */
         case FG_GetInfo:                /* Get information request       */
            GeneralGetInfo(msg,&ctx);    /*  Get General information      */
            break;                       /*  Get another request          */
         case FG_SetInfo:                /* Set information request       */
             GeneralSetInfo(msg,&ctx);   /*  Set General information      */
             break;                      /*  Get another request          */
         default:                        /* Unknown message ...           */
            GeneralDefault(msg);         /*  Action on unknown message    */
            break;                       /*  Get another request          */
      }
   }

   GeneralClose:;

   for(evt=ctx.evts;evt;evt=evn)         /* Disable all events ...        */
   {  evn = evt->nxt;
      GeneralDisableEvent(evt);
      free(evt);
   }
   
   Wait(&nde->obj.Lock);             /* Gain access to the General node   */
   Signal(&nde->obj.Lock);           /* Relinquish access.                */

   if(msg->MsgHdr.Reply!=NullPort) ErrorMsg(msg,(word)Err_Null);
}

/*------------------------------------------------------------------------*/
/*                                                   GeneralDefault(...)  */
/*------------------------------------------------------------------------*/

static void GeneralDefault(MCB *msg)
/* This function is called whenever an unimplemented function code is     */
/*    discovered over the open-stream request port.                       */
{  ErrorMsg(msg,EC_Error|EG_Unknown|EO_Object);  }

/*------------------------------------------------------------------------*/
/*                                                   GeneralSetInfo(...)  */
/*------------------------------------------------------------------------*/

static void GeneralSetInfo(MCB *msg,GeneralContext *ctx)
{  GeneralMlkRequest *blk;
   
   /* Make sure a previous request (if applicable) has finished ...      */
   if(ctx->isInRq) Wait(&ctx->ctl.sem);
   
   /* Access the block passed with the SetInfo() call                    */
   blk = (GeneralMlkRequest*)msg->Data;

   /* If there is a message to transmit, set it up in the link-request   */
   /*    control block.                                                  */
   if(blk->txFlag) ctx->ctl.txBuf = ctx->tx.byt,
                   ctx->tx = blk->msg;
   else            ctx->ctl.txBuf = NULL;
   
   /* Set pointer in the link-request control block to accept the reply  */
   /*   message if applicable.                                           */
   if(blk->rxType!=0) ctx->ctl.rxBuf     = ctx->rx.msg.byt, 
                      ctx->rx.msg.byt[0] = (ubyte)blk->rxType;
   else               ctx->ctl.rxBuf     = NULL;

   /* Set the flag which indicates that a message is currently scheduled */
   /*   for transmission/reception, and schedule the message.            */
   ctx->isInRq = 1; MlkPerformLinkRequest(&ctx->ctl);

   /* Return appropriate message                                         */
   if(msg->MsgHdr.Reply!=NullPort) ErrorMsg(msg,(word)Err_Null);
}

/*------------------------------------------------------------------------*/
/*                                                   GeneralGetInfo(...)  */
/*------------------------------------------------------------------------*/

static void GeneralGetInfo(MCB *msg,GeneralContext *ctx)
{
   /* Check that a request was sent via. SetInfo() in the first place,    */
   /*   and also that the request involved expecting a reply.             */
   if(!ctx->isInRq||ctx->ctl.rxBuf==NULL)
   {  ErrorMsg(msg,EC_Error|EG_Broken|EO_Object);
      return;
   }

   /* Wait for the request that was initiated by SetInfo() to complete    */
   Wait(&ctx->ctl.sem);

   /* Copy status parmaters ...                                           */
   ctx->rx.status = ctx->ctl.status;
   ctx->rx.code   = ctx->rx.code;

   /* Set up message control block to return reply which consists of the  */
   /*   received reply from the microcontroller along with some status    */
   /*   information.                                                      */
   InitMCB
   (  msg,0,msg->MsgHdr.Reply,NullPort,(word)Err_Null  );
   msg->Control = (word*)&ctx->rx;
   msg->MsgHdr.ContSize = sizeof(GeneralMlkReply);

   /* Send the reply message.                                             */
   PutMsg(msg);
}

/*------------------------------------------------------------------------*/
/*                                                      GeneralEnEv(...)  */
/*------------------------------------------------------------------------*/

static void GeneralEnEv(MCB *msg,GeneralContext *ctx)
/* This "general" microlink stream uses the event-mechanism to translate  */
/*    microlink events into Helios events: Any client with an event-      */
/*    stream open to "/microlink/general" may set up any amount of event  */
/*    channels using "EnableEvents()" with as a parameter the             */
/*    message-type of the messages that would be received from the        */
/*    microcontroller and get converted into messages over the microlink. */
/* To disable an event, the client should issue an EnableEvents() command */
/*    with the corresponding message-type execpt negated: This results    */
/*    in all event channels for that stream with events of the            */
/*    appropriate type to be disabled.                                    */
/* A call of EnableEvents() with paramter 0 results in all events on that */
/*    channel being disabled.                                             */
{  GeneralEventChannel *evt,**evl;
   int typ;

   typ = ((byte*)msg->Control)[0];
   if(typ<=0)
   {  
      typ = -typ;
      /* Here if request is to disable events ...                        */
      /* Find which events have the appropriate type in this loop, and   */
      /*   disable them one by one, unlinking them one by one from the   */
      /*   queue of events.                                              */
      for(evl=&ctx->evts;(evt=*evl)!=NULL;)
      {  if(typ==0||evt->typ==typ)
         {  /* Here if an event channel was set up with the event-type   */
            *evl = evt->nxt;
            GeneralDisableEvent(evt);
            free(evt);
         } else evl=&evt->nxt;
      }

      /* Send reply message ...                   */
      InitMCB(msg,0,msg->MsgHdr.Reply,NullPort,(word)Err_Null);
      ((byte*)msg->Control)[0] = typ;
      msg->MsgHdr.ContSize     =  1;
      PutMsg(msg);

   } else
   {  
      /* Here if request is to enable an event. An event of a particular */
      /*   type may only be enabled if it is not already enabled ...     */
      for(evt=ctx->evts;evt;evt=evt->nxt) if(evt->typ==typ) break;
      if(evt)
      {  /* Here if tried to enable same event-type twice */
         ErrorMsg(msg,EC_Error|EG_InUse|EO_Object);
         return;
      } else
      {  evt = GeneralSetupEvent(ctx,msg,typ);
         if(evt==NULL) { return; }
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
         Fork(ReqStackSize,GeneralTransmitEvents,4,evt);
      }
   }
}

/*------------------------------------------------------------------------*/
/*                                               GeneralEnableEvent(...)  */
/*------------------------------------------------------------------------*/

static GeneralEventChannel *GeneralSetupEvent
                            (  GeneralContext *ctx,
                               MCB            *msg,
                               int             typ
                            )
/* This function sets up all the parameters in the event context          */
/*   structure and registers a raw-packet handler for that event so that  */
/*   the event buffer immediately starts to get filled with messages from */
/*   the microlink of the corresponding type. After this function has     */
/*   been called, it merely remains to call the GeneralTransmitEvents()   */
/*   function to read events from the buffer and translate them into      */
/*   event messages to be sent to the client.                             */
/* If the event staructure cannot be set-up, the routine sends an error   */
/*   message in reply to <msg>, and returns NULL.                         */
/* Otherwise the reply port is used to transmit messages.                 */
{  GeneralEventChannel *evt;
   GeneralEvent        *bse;
   word                 res;

   evt = (GeneralEventChannel*)calloc(1,sizeof(GeneralEventChannel));
   if(evt==NULL)
   {  ErrorMsg(msg,EC_Error|EG_NoMemory|EO_Object);
      return NULL;
   }
   
   bse = (GeneralEvent*)calloc(GeneralBufferSize,sizeof(GeneralEvent));
   if(bse==NULL)
   {  free(evt);
      ErrorMsg(msg,EC_Error|EG_NoMemory|EO_Object);
      return NULL;
   }
   
   evt->typ        = typ;
   evt->mh.msgType = (ubyte)typ;
   evt->mh.func    = GeneralRawPacketHandler;
   evt->mh.arg     = (void*)evt;
   evt->evp        = msg->MsgHdr.Reply;
   evt->bse        = bse;
   evt->rp         = bse;
   evt->wp         = bse;
   evt->lim        = bse + GeneralBufferSize;
   evt->maxsem     = GeneralBufferSize-1;
   InitSemaphore(&evt->sem,0);
   InitSemaphore(&evt->acc,1);
   evt->cyc        = 0;
   evt->ovf        = 0;
   evt->req        = 0;
   InitSemaphore(&evt->ack,0);
   
   res = ML_RegisterHandler(&evt->mh);
   if(res<0)
   {  free(evt); free(bse);
      ErrorMsg(msg,EC_Error|EG_NoMemory|EO_Object);
      return NULL;
   }
   
   evt->nxt  = ctx->evts;
   ctx->evts = evt;
   
   return evt;
}

/*------------------------------------------------------------------------*/
/*                                              GeneralDisableEvent(...)  */
/*------------------------------------------------------------------------*/

static void GeneralDisableEvent(GeneralEventChannel *evt)
/* This function may be called to disable events on any event channel: it */
/*   communicates with the process that transmits the events and causes   */
/*   it to terminate. It then detaches the handler that was set up for    */
/*   that type of event.                                                  */
/* This function frees subsidiary structures which hang off the event     */
/*   control structure but does not free the event structure itself.      */
{
   evt->req = 1;                     /* Request event thread to terminate */
   AbortPort(evt->evp,0);            /* Unblock events port               */
   Wait(&evt->acc);                  /* Gain access to ctx->sem           */
   HardenedSignal(&evt->sem);        /* Signal ctx->sem                   */
   Signal(&evt->acc);                /* Release access to ctx->sem        */
   Wait(&evt->ack);                  /* Wait for event thread aknowledge  */
   FreePort(evt->evp);               /* Free events port                  */
   ML_DetachHandler(&evt->mh);       /* Detach packet handler             */
   free(evt->bse);                   /* Free buffer which stores events   */
   evt->req =        0;              /* Reset termination request         */
   evt->evp = NullPort;              /* Reset events port descriptior     */
}

/*------------------------------------------------------------------------*/
/*                                            GeneralTransmitEvents(...)  */
/*------------------------------------------------------------------------*/

static void GeneralTransmitEvents(GeneralEventChannel *evt)
{  GeneralEvent    *rp;
   MCB             mcb;

   evt->cyc = 0; /* Event cycle number       */

   for(;;)
   {  /* Trap overflow and reset event buffer: */
      if(evt->ovf)
      {  Wait(&evt->acc); InitSemaphore(&evt->sem,0); Signal(&evt->acc);
         evt->rp = evt->wp = evt->bse;
         evt->ovf = 0;
      }


      if(evt->req) break;          /* Trap kill-request:                 */
      HardenedWait(&evt->sem);     /* Wait for a microlink pkt to arrive */
      if(evt->req) break;          /* Trap kill-request                  */
      rp = evt->rp;                /* Get pointer to event               */

      /* Initialise MCB to send a reply message to the client:           */
      InitMCB
      ( &mcb,
        (word)MsgHdr_Flags_preserve,
        evt->evp,
        NullPort,
        (word)EventRc_IgnoreLost
      );
      mcb.Data            = (byte*)rp;
      mcb.MsgHdr.DataSize = sizeof(GeneralEvent);
      PutMsg(&mcb);                /* Send event.                        */
      
      if(evt->req) break;          /* Trap kill-request                  */
   }

   Signal(&evt->ack);              /* Aknowledge kill-request            */
}

/*------------------------------------------------------------------------*/
/*                                          GeneralRawPacketHandler(...)  */
/*------------------------------------------------------------------------*/

#pragma -s1 /* Disable stack checking in this function */

static void GeneralRawPacketHandler(byte *buf,GeneralEventChannel *evt)
/* This raw message handler run in a very limited supervisor state so     */
/*    is hardly allowed to call any functions at all. In fact, it only    */
/*    copies the packet received into the buffer referred to by <evt>.    */
{  GeneralEvent       *wp;

   /* Trap the overflow flag:                                             */
   if(evt->ovf) return;
   
   /* Get pointer to next packet structure in buffer to which the raw     */
   /*    raw data can be copied:                                          */
   wp = evt->wp;
   
   /* Copy the stylus packet to the appropriate part of the destination   */
   /*   buffer                                                            */
   wp->Device.microlink = *(GeneralMessageBlock*)buf;
   wp->Type             = (ubyte)(buf[0]);
   wp->Counter          = evt->cyc++;

   /* Increment buffer write pointer and cycle it if necessary:           */
   if(++wp>=evt->lim) wp=evt->bse;
   evt->wp = wp;
   
   /* Check the semaphore. If there are too many semaphore signals        */
   /*   outstanding then set the 'overflow' flag, and return. Otherwise   */
   /*   signal another semaphore.                                         */
   if(TestSemaphore(&evt->sem)>=evt->maxsem) evt->ovf = 1;
   else                                      HardenedSignal(&evt->sem);
   
}

#pragma -s0
