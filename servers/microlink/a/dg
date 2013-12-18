/* $Header: digitiser.c,v 1.7 91/01/31 13:51:05 charles Locked $ */
/* $Source: /server/usr/users/b/charles/world/microlink/RCS/source/digitiser.c,v $ */

/*------------------------------------------------------------------------*/
/*                                          microlink/source/digitiser.c  */
/*------------------------------------------------------------------------*/

/* This implements the digitiser part of the microlink protocol server    */
/* By opening a channel to '/microlink/digitiser' and enabling events it  */
/*   is possible to receive a message for each digitiser co-ordinate      */
/*   packet received. It is also possible to send configuration           */
/*   information to this digitiser server to configure various options    */
/*   about the digitiser including required sample-rates and so-forth     */
/* The digitiser is interlocked in such a way that it is impossible to    */
/*   open two streams to the digitiser entry at the same time.            */

/*------------------------------------------------------------------------*/
/*                                                          Header Files  */
/*------------------------------------------------------------------------*/

# include "microlink/private/microlink.h"
# include "microlink/private/digitiser.h"

/*------------------------------------------------------------------------*/
/*                                               Look-ahead declarations  */
/*------------------------------------------------------------------------*/

void mlkAddDigitiserEntry         (DirNode *microlinkDir);
void mlkServeDigitiser            ( ServInfo        *si , 
                                    MicrolinkNode  *nde , 
                                    MsgBuf        *rply 
                                  );
static void DigitiserRead         (MCB *msg,DigContext *ctx);
static void DigitiserGetSize      (MCB *msg);
static void DigitiserSeek         (MCB *msg);
static void DigitiserDefault      (MCB *msg);
static void DigGetAsciiLine       (DigContext *ctx);
static void DigitiserEnEv         (MCB *msg,DigContext *ctx);
static void DigTransmitEvents     (DigContext *ctx);
static void DigRawPacketHandler   (byte *buf,DigContext *ctx);
static void DigGetProcessedPacket (Stylus_Event *des,word *cyc,DigContext *ctx);
static void DigDisableEvents      (DigContext *ctx);
static int  DigGenerateButtonPacket(Stylus_Event *des,word *cyc,DigContext *ctx);
static void DigitiserGetInfo(MCB *msg,DigContext *ctx);
static void DigitiserSetInfo(MCB *msg,DigContext *ctx);
static int DigDecodeOrd           (MLEstyluPacket*,DigContext*,int*,int);
static int DigCubicInterp         (signed char w[5]);
static int DigQuadInterp          (signed char w[5]);

/*------------------------------------------------------------------------*/
/*                                                 The digitiser handler  */
/*------------------------------------------------------------------------*/

/* This digitiser server uses one handler which is registered with the    */
/*    executive microlink driver. The handler structure is static and     */
/*    defined below. It is static so that the routine that traps the      */
/*    kill signal can detach it before the program is exited.             */

ML_MsgHandler DigMsgHandler;

/*------------------------------------------------------------------------*/
/*                                                 Digitiser Information  */
/*------------------------------------------------------------------------*/

/* This local structure stores the digitiser co-ordinate transormation    */
/*   information which can be read or set by the client using GetInfo()   */
/*   and SetInfo(), and survive after the digitiser stream is closed.     */

static DigitiserInfo digInfo ;

/*------------------------------------------------------------------------*/
/*                                             mlkAddDigitiserEntry(...)  */
/*------------------------------------------------------------------------*/

void mlkAddDigitiserEntry(DirNode *microlinkDir)
/* Add an entry into the microlink main directory corresponding to the    */
/*    digitiser.                                                          */
{  static MicrolinkNode nde;

   digInfo.cXX        =    1240 ;
   digInfo.cXY        =       0 ;
   digInfo.cX1        = -250000 ;
   digInfo.cYX        =       0 ;
   digInfo.cYY        =    1240 ;
   digInfo.cY1        = -250000 ;
   digInfo.denom      =    4096 ;
   digInfo.xFilter    =       0 ;
   digInfo.yFilter    =       0 ;
   digInfo.filterBits = DigFilterBits;
   digInfo.xGlitch    = (1<<30);
   digInfo.yGlitch    = (1<<30);
   digInfo.minRawX    = 
   digInfo.minRawX    =  DigMinXCoarse   *DigCountsPerWire;
   digInfo.maxRawX    = (DigMaxXCoarse+1)*DigCountsPerWire;
   digInfo.minRawY    =  DigMinYCoarse   *DigCountsPerWire;
   digInfo.maxRawY    = (DigMaxYCoarse+1)*DigCountsPerWire;

   InitNode((ObjNode*)&nde,"digitiser",Type_File,0,DefFileMatrix);
   nde.objTyp = MlkDigitiser;
   Insert(microlinkDir,(ObjNode*)&nde.obj,(word)(TRUE));
}

/*------------------------------------------------------------------------*/
/*                                                mlkServeDigitiser(...)  */
/*------------------------------------------------------------------------*/

void mlkServeDigitiser( ServInfo *si , MicrolinkNode *nde , MsgBuf *rply )
/* This function is called whenever a stream is opened to the digitiser.  */
/* It is passed the server-info structure corresponding to the request to */
/*    open the stream, and also a MsgBuf structure pointer for the        */
/*    already set-up reply message that should get sent back indicatating */
/*    a request port over which to conduct further transfers.             */
{  DigContext     ctx;
   DigRawPacket  *rpb;
   MCB           *msg;
   Port          reqp;
   byte         *data;
   word           res;

   if(nde->obj.Account>0)                /* Already opened?               */
   {  ErrorMsg(si->m,EC_Error|EG_InUse|EO_Object);
      UnLockTarget(si);                  /* Send error message, unlock    */
      return;                            /*   target and return           */
   }                                     /* .. End of Interlock code      */

   rpb = Malloc(DigBuffSize*sizeof(DigRawPacket));
   if(rpb==NULL)
   {  ErrorMsg(si->m,EC_Error|EG_NoMemory);
      UnLockTarget(si);
      return;
   }
   ctx.base        = rpb;
   ctx.lim         = rpb+DigBuffSize;
   ctx.rp          = rpb;
   ctx.wp          = rpb;
   ctx.ord         = MLEstyluXord;
   InitSemaphore(&ctx.sem,0);
   ctx.maxsem      = DigBuffSize-1;
   ctx.tm          =   0;
   ctx.eventPort   = NullPort;
   ctx.req         =   0;
   InitSemaphore(&ctx.ack,0);
   ctx.readPhase   =   0;
   ctx.rph         = &DigMsgHandler;
   ctx.rph->msgType= MLEstylu;
   ctx.rph->func   = (void(*)(byte*,word))DigRawPacketHandler;
   ctx.rph->arg    = (void*)&ctx;
   ctx.was         = 0;
   ctx.is          = 0;
   ctx.lstX        = 0;
   ctx.lstY        = 0;
   ctx.ctt         = 0;
   ctx.cyc         = 0;
   
   /* Here we register a function which is asynchronously entered         */
   /*   whenever a digitiser data-block comes in over the microlink: it   */
   /*   places the data-block into a buffer (trapping buffer overflow     */
   /*   conditions) and then returns. The data from the buffer is then    */
   /*   read out and processed on demand by either this process (when it  */
   /*   services read requests for an ASCII report of the stylus position)*/
   /*   or by the event-handler process which is forked when events are   */
   /*   enabled, and sends encoded events for every digitiser packet.     */
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
      {  case FG_Read:                   /* Read request:                 */
            DigitiserRead(msg,&ctx);     /*  Service it                   */
            break;                       /*  Get another request          */
         case FG_Close:                  /* Close request:                */
            goto digClose;               /*  Break out of service loop    */
         case FG_GetSize:                /* GetSize request:              */
            DigitiserGetSize(msg);       /*  Service it                   */
            break;                       /*  Get another request          */
         case FG_Seek:                   /* Seek request:                 */
            DigitiserSeek(msg);          /*  Service it                   */
            break;                       /*  Get another request          */
         case FG_EnableEvents:           /* Events Request:               */
            DigitiserEnEv(msg,&ctx);     /*  Enable events                */
            break;                       /*  Get another request          */
         case FG_GetInfo:                /* Get information request       */
            DigitiserGetInfo(msg,&ctx);  /*  Get digitiser information    */
            break;                       /*  Get another request          */
         case FG_SetInfo:                /* Set information request       */
             DigitiserSetInfo(msg,&ctx); /*  Set digitiser information    */
             break;                      /*  Get another request          */
         default:                        /* Unknown message ...           */
            DigitiserDefault(msg);       /*  Action on unknown message    */
            break;                       /*  Get another request          */
      }
   }

   digClose:;

   if(ctx.eventPort!=NullPort) 
      DigDisableEvents(&ctx);    /* Disable events                        */
   ML_DetachHandler(ctx.rph);    /* Detach the raw packet handler         */

   Wait(&nde->obj.Lock);     /* Gain access to the digitiser node         */
   nde->obj.Account--;       /* Decrement no. of users.                   */
   Signal(&nde->obj.Lock);   /* Relinquish access.                        */

   if(msg->MsgHdr.Reply!=NullPort) ErrorMsg(msg,(word)Err_Null);
}

/*------------------------------------------------------------------------*/
/*                                                    DigitiserRead(...)  */
/*------------------------------------------------------------------------*/

static void DigitiserRead(MCB *msg,DigContext *ctx)
/* If the user reads from the digitiser channel as if it were some sort   */
/*   of character stream, the channel returns a sequence of lines, each   */
/*   of which contains in a reable but arbitrary text format reports of   */
/*   the digitiser position and button status and so-forth. This method   */
/*   of using the port is not permitted by this routine if events get     */
/*   enabled, because it would be too much hassle to control the reading  */
/*   of the input buffer of there were two clients in separate processes  */
/*   trying to use it.                                                    */
{  ReadWrite              *rcs;
   Port                  rport;
   word                    btt;
   word                    tts;
   word                    sta;
   word                    seq;

   /* Check events not enabled:                                           */
   if(ctx->eventPort!=NullPort)
   {  ErrorMsg(msg,EC_Error|EG_InUse|EO_Object);
      return;
   }
   
   rcs = (ReadWrite*)msg->Control;
   seq = 0;
   
   /* The 'readPhase' member of the context structure is non-zero if     */
   /*   there is still part of a previous line to be output on the       */
   /*   stream, in which case it gives the offset into the line-buffer   */
   /*   of the next line to be sent. If it is zero, we need to read in a */
   /*   new line ...                                                     */
   if(ctx->readPhase==0) DigGetAsciiLine(ctx);
   /* The line always has the same length (padded out with spaces), and  */
   /*    that length is called 'DigLineLength'.                          */
   
   rport = msg->MsgHdr.Reply;

   for(btt=rcs->Size;btt>0;btt-=tts)
   {  tts = DigLineLength-ctx->readPhase; if(tts>btt) tts=btt;
      if(tts>=btt) { sta=ReadRc_EOD; }  /* Mark packet as last-one         */
      else         { sta=ReadRc_More;}  /* Mark packet as 'more-to-follow' */
      InitMCB(msg,MsgHdr_Flags_preserve,rport,NullPort,sta|seq);
      msg->Data            = (byte*)ctx->asciiLine + ctx->readPhase;
      msg->MsgHdr.DataSize = (int)tts;
      PutMsg(msg);
      seq += ReadRc_SeqInc; 
      if((ctx->readPhase+=tts)>=DigLineLength)
      {  ctx->readPhase-=DigLineLength;
         DigGetAsciiLine(ctx);
      }
   }
   FreePort(rport);
}

/*------------------------------------------------------------------------*/
/*                                                 DigitiserGetSize(...)  */
/*------------------------------------------------------------------------*/

static void DigitiserGetSize(MCB *msg)
{  InitMCB(msg,0,msg->MsgHdr.Reply,NullPort,(word)Err_Null);
   MarshalWord(msg,0);
   PutMsg(msg);
}

/*------------------------------------------------------------------------*/
/*                                                    DigitiserSeek(...)  */
/*------------------------------------------------------------------------*/

static void DigitiserSeek(MCB *msg)
{  SeekRequest *requestBlock = (SeekRequest*)msg->Control;
   word newPosition;

   switch(requestBlock->Mode)
   {  case S_Beginning:
         newPosition = requestBlock->NewPos;
         break;
      case S_Relative:
         newPosition = requestBlock->CurPos+requestBlock->NewPos;
         break;
      case S_End:
         /* Since there is no concept of 'file end', this mode is */
         /*   not easy to implement sensibly.                     */
         newPosition = requestBlock->CurPos-requestBlock->NewPos;
         break;
   }

   InitMCB(msg,0,msg->MsgHdr.Reply,NullPort,(word)Err_Null);
   MarshalWord(msg,newPosition);
   PutMsg(msg);
}

/*------------------------------------------------------------------------*/
/*                                                 DigitiserDefault(...)  */
/*------------------------------------------------------------------------*/

static void DigitiserDefault(MCB *msg)
/* This function is called whenever an unimplemented function code is     */
/*    discovered over the open-stream request port.                       */
{  ErrorMsg(msg,EC_Error|EG_Unknown|EO_Object);  }

/*------------------------------------------------------------------------*/
/*                                                  DigGetAsciiLine(...)  */
/*------------------------------------------------------------------------*/

static void DigGetAsciiLine(DigContext *ctx)
{  Stylus_Event st;
   int           i;
   word        cyc;
   
   DigGetProcessedPacket(&st,&cyc,ctx);
   sprintf(ctx->asciiLine,"(%5d,%5d)   0x%08X",st.X,st.Y,st.Buttons);
   for(i=0;i<DigLineLength;i++) if(!ctx->asciiLine[i]) break;
   for(   ;i<DigLineLength;i++) ctx->asciiLine[i]=' ';
   ctx->asciiLine[DigLineLength-1] = '\n';
}

/*------------------------------------------------------------------------*/
/*                                                    DigitiserEnEv(...)  */
/*------------------------------------------------------------------------*/

static void DigitiserEnEv(MCB *msg,DigContext *ctx)
/* This function to enable/disable digitiser events by forking a process  */
/*    to pick digtiser packets out of the input buffer, processes them,   */
/*    and send them as stylus events down the message port supplied in    */
/*    the request.                                                        */
{  int en;

   en = ((byte*)msg->Control)[0];
   if(en==0)
   {  /* Here if request is to disable events ... */
      /* Only applies if already enabled ...      */
      if(ctx->eventPort!=NullPort) DigDisableEvents(ctx);
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
         (  msg,MsgHdr_Flags_preserve,
            msg->MsgHdr.Reply,NullPort,
            (word)Err_Null
         );
         ((byte*)msg->Control)[0] =  1;
         msg->MsgHdr.ContSize     =  1;
         PutMsg(msg);
         /* Finally, fork the event transmitter: */
         Fork(ReqStackSize,DigTransmitEvents,4,ctx);
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
/*                                                 DigitiserGetInfo(...)  */
/*------------------------------------------------------------------------*/

static void DigitiserGetInfo(MCB *msg,DigContext *ctx)
{  
   /* This function returns the current digitiser information. A client   */
   /*    may send this message via. the GetInfo(...) call. It will return */
   /*    a structure containing the current calibration system for the    */
   /*    digitiser, as specified in 'ext/digitiser.h'. This function      */
   /*    manually resets some of the fields before passing the structure  */

   digInfo.minRawX =  DigMinXCoarse   *DigCountsPerWire;
   digInfo.maxRawX = (DigMaxXCoarse+1)*DigCountsPerWire;
   digInfo.minRawY =  DigMinYCoarse   *DigCountsPerWire;
   digInfo.maxRawY = (DigMaxYCoarse+1)*DigCountsPerWire;
   digInfo.filterBits = DigFilterBits;
   InitMCB
   (  msg,0,msg->MsgHdr.Reply,NullPort,(word)Err_Null  );
   msg->MsgHdr.ContSize = 0;
   msg->Data            = (byte*)&digInfo;
   msg->MsgHdr.DataSize = sizeof(DigitiserInfo);

   PutMsg(msg);
}

/*------------------------------------------------------------------------*/
/*                                                 DigitiserSetInfo(...)  */
/*------------------------------------------------------------------------*/

static void DigitiserSetInfo(MCB *msg,DigContext *ctx)
{  
   /* This function allows the client to set the digitiser scaling and    */
   /*   general transformation information. The details of the scaling    */
   /*   and transormation structure is given in 'ext/digitiser.h'         */
   /* Strictly speaking, we should provide a semaphore to control access  */
   /*   to this structure so that it does not get altered by this routine */
   /*   at the time when the event transmit thread is running the         */
   /*   function 'DigGetProcessedPacket' which used the structure as well,*/
   /*   but this will only result in a false packet being sent: Just say  */
   /*   that the client should disable events before calling SetInfo(),   */
   /*   which is the sort of thing the client should do anyway otherwise  */
   /*   it will get confused as to which transformation is being used for */
   /*   the packets it is receiving.                                      */

   digInfo = *(DigitiserInfo*)msg->Data;
   if(digInfo.denom==0) digInfo.denom=1; /* Can't divide by zero */
   if(msg->MsgHdr.Reply!=NullPort) ErrorMsg(msg,(word)Err_Null);
#  if 0
   diag(("SetInfo() request was received. New values are ...\n"));
   diag(("%+10d     %+10d     %+10d\n",digInfo.cXX,digInfo.cXY,digInfo.cX1));
   diag(("%+10d     %+10d     %+10d\n",digInfo.cYX,digInfo.cYY,digInfo.cY1));
   diag(("Denom = %d\n",digInfo.denom));
#  endif
}

/*------------------------------------------------------------------------*/
/*                                                 DigDisableEvents(...)  */
/*------------------------------------------------------------------------*/

static void DigDisableEvents(DigContext *ctx)
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
   /*   a new packet of raw digitiser information to come in on the       */
   /*   microlink. In the first instance the event process is blocking on */
   /*   a message port: This function 'aborts' that port in order to      */
   /*   unblock it. In the second case the process is waiting on a        */
   /*   hardened semaphore which gets signalled by 'DigRawPacketHandler'  */
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
/*                                                DigTransmitEvents(...)  */
/*------------------------------------------------------------------------*/

static void DigTransmitEvents(DigContext *ctx)
/* This function runs in a forked process: It takes raw digitiser data    */
/*   packages from the input buffer, processes them, and transmits them   */
/*   as events ...                                                        */
{  IOEvent       event;
   MCB             mcb;

   ctx->tm  = 0; /* Event time-stamp counter */

   for(;;)
   {  *(int*)0x740010 = (int)0x000000FF;
      DigGetProcessedPacket(&event.Device.Stylus,&event.Counter,ctx);
      *(int*)0x740010 = (int)0x0000FF00;
      if(ctx->req) break;  /* Trap kill-request */
      /* Set up event package to send to client ...                  */
      event.Type    = (word)(Event_Stylus);
      event.Stamp   = ctx->tm;
      InitMCB
      (  &mcb,
         (word)(MsgHdr_Flags_preserve),
         ctx->eventPort,
         NullPort,
         (word)(EventRc_IgnoreLost)
      );
      mcb.Data            = (byte*)&event;
      mcb.MsgHdr.DataSize = sizeof(IOEvent);
      /* Message gets sent here. Used to set the overflow flag if the     */
      /*   message timed out but this is no longer done since that would  */
      /*   result in just dropping a whole load of perfectly good samples */
      (*(int*)0x740008)++;
      *(int*)0x740010 = (int)0x00FF0000;
      PutMsg(&mcb);
      *(int*)0x740010 = (int)0xFF000000;
      if(ctx->req) break;  /* Trap kill-request */
   }

   Signal(&ctx->ack);  /* Aknowledge kill-request */
}

/*------------------------------------------------------------------------*/
/*                                              DigRawPacketHandler(...)  */
/*------------------------------------------------------------------------*/

#pragma -s1 /* Disable stack checking in this function */

static void DigRawPacketHandler(byte *buf,DigContext *ctx)
/* This raw packet handler run in a very limited supervisor state so      */
/*    is hardly allowed to call any functions at all. In fact, it only    */
/*    copies the packet received into the buffer referred to by <ctx>.    */
{  MLEstyluPacket *des;
   DigRawPacket    *wp;
   int             ord;

   /* Download the ordiante code (indicating x- or y-) of the packet that */
   /*    we have received.                                                */
   ord = ((MLEstyluPacket*)buf)->ord;

   /* If we are receiveing a y-ordiante, then increment the cycle number  */
   /*   which is used to count the packets being received over the link.  */
   if(ord==MLEstyluYord) ctx->cyc++;

   /* Check whether we are in an overflow situation (i.e. buffer full)    */
   /*    by checking the count on the semaphore which signals each        */
   /*    packet's entry into the buffer. Whilst this count is above a     */
   /*    threshold, no packets are entered into the buffer.               */
   if(ctx->sem.Count>=ctx->maxsem) return;
   
   /* Check whether the packet we are receiving refers to the ordinate    */
   /*   (x- or y-) that we are expecting                                  */
   if(ord!=ctx->ord) return;
   
   /* Change the record of the ordinate that we expect to receive next    */
   /*   time                                                              */
   ctx->ord = ord^(MLEstyluXord^MLEstyluYord);
   
   /* Get pointer to next packet structure in buffer to which the raw     */
   /*    data can be copied:                                              */
   wp = ctx->wp;
   
   /* Copy the stylus packet to the appropriate part of the destination   */
   /*   buffer depending on whether it is x-information or y-information  */
   des=&wp->x; if(ord==MLEstyluYord) des=&wp->y;
   *des=*(MLEstyluPacket*)buf;

   /* Change ctx->ord to indicate that we are now to expect the other     */
   /*   type of packet. Also, if that was an x-packet then we return      */
   /*   immediately, waiting for a y-packet before incrementing the       */
   /*   pointer and getting a new block:                                  */
   if(ord!=MLEstyluYord) return;

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
/*                                            DigGetProcessedPacket(...)  */
/*------------------------------------------------------------------------*/

static void DigGetProcessedPacket ( Stylus_Event   *des,
                                    word           *cyc,
                                    DigContext     *ctx
                                  )
/* This function is supposed to wait until data becomes availiable in the */
/*   buffer containing raw digitizer data. When data beomes availiable,   */
/*   it reads the next raw packet, processes it to form co-ordinate       */
/*   information which it writes to <des>, and then returns.              */
{  DigRawPacket *rp,*nrp;
   int ix,iy,x,y,dx,dy;

   tryAgain:;
   (*(int*)0x740014)++;
   *(int*)0x740018 = 0x000000FF;
   /* Return to this point if the stylus is found to be out-of-proximity  */
   /*   or the co-ordaintes obtained are invalid, in which case we have   */
   /*   to trasmit button-change events and out-of/into-prox events if    */
   /*   applicable, or else get the next raw-packet and see if the        */
   /*   button is back in proximity.                                      */
   /* Also return to here if a co-ordinate is ready to be transmitted.    */
   
   /* Check for changes in the states of the buttons which haven't yet    */
   /*   been reported in a stylus event ...                               */
   if(ctx->was!=ctx->is) 
   {  *(int*)0x740018 = 0x0000FF00;
      if(DigGenerateButtonPacket(des,cyc,ctx)) 
      {  *(int*)0x740018 = 0x0000FF00;
         return;
      }
   }
   *(int*)0x740018 = 0x00FF0000;
   
   /* Check whether we are ready to transmit a co-ordinate:               */
   if(ctx->ctt)
   {  des->Buttons =           0;
      des->X       = ctx->  lstX;
      des->Y       = ctx->  lstY; 
      *cyc         = ctx->lstCyc; /* Download cycle number of last ord.  */
      ctx->ctt     =           0; /* Indicate co-ordinate transmitted.   */
      *(int*)0x740018 = (int)0xFF000000;
      return;
   }
   
   *(int*)0x740018 = 0x00000000;
   *(int*)0x74001C = 0x000000FF;

   /* If this process is running in the event transmission thread, and    */
   /*   the stream-serving event-disable process is requesting the        */
   /*   termination of this process it will set ctx->req, and then try to */
   /*   unblock anything this process might be waiting on. We sample      */
   /*   ctx->req here and if non-zero we return immediately.              */
   /* This satisfys the scenario where ctx->req was set and ctx->sem      */
   /*   forcefully signalled by disable-events just before the semaphore  */
   /*   was re-initialised in the above code fragment.                    */
   if(ctx->req) return;
   
   *(int*)0x74001C = 0x0000FF00;

   /* Wait for a packet to be availiable using hardened wait: The         */
   /*    semaphore count tells how many packets are in the buffer.        */
   HardenedWait(&ctx->sem);
   
   *(int*)0x74001C = 0x00FF0000;
   
   /* Sample ctx->req again here in case ctx->sem was just signalled by   */
   /*    events-disable in order to unblock this process.                 */
   if(ctx->req) return;

   *(int*)0x74001C = (int)0xFF000000;
   
   /* Now read from the buffer and process the data, writing the          */
   /*   processed data into 'des', which is the destination packet for    */
   /*   the raw data.                                                     */

   rp = ctx->rp;    /* Load pointer to raw data    */

   /* Now increment the read pointer to point to the next buffer, allow  */
   /*   it to cycle if necassary ...                                     */
   if((nrp=rp+1)>=ctx->lim) nrp=ctx->base;
   ctx->rp = nrp;

   /* Compute the button information:                                     */
   ctx->is    = DigProx; /* Assume in proximity */
   if(rp->x.bt&MLEstyluTip)    ctx->is |= DigTip;
   if(rp->x.bt&MLEstyluBarrel) ctx->is |= DigBarrel;
   if(rp->x.bt&MLEstyluLeft)   ctx->is |= DigLeft;
   if(rp->x.bt&MLEstyluMiddle) ctx->is |= DigMddl;
   if(rp->x.bt&MLEstyluRight)  ctx->is |= DigRght;

   /* Convert digitiser raw wire values into co-ordinates: The function   */
   /*  returns 0 if the values found indicate an invalid packet or out-   */
   /*  of-proximity values. If out-of-proximity, the routine also sets    */
   /*  the out-of-proximity flag in the ctx->is field to schedule an out- */
   /*  of-prox event to be generated at the head of this function.        */
   if(DigDecodeOrd(&rp->x,ctx,&ix,1)==0) { goto tryAgain; }
   if(DigDecodeOrd(&rp->y,ctx,&iy,0)==0) { goto tryAgain; }

   /* Apply the currently chosen linear transformation of the digitiser   */
   /*   co-ordainates into reported co-ordinates. This translation        */
   /*   typically rotates slightly due to skew between LCD and digitiser, */
   /*   and scales and adjusts the co-ordinate system appropriately to    */
   /*   origin bottom-left or top-right or whatever.                      */
   x=(digInfo.cXX*ix+digInfo.cXY*iy+digInfo.cX1*1)/digInfo.denom;
   y=(digInfo.cYX*ix+digInfo.cYY*iy+digInfo.cY1*1)/digInfo.denom;

   /* Apply a simple forward-averaging filter to reduce jitter in the     */
   /*   digitiser co-ordinates. The filtering co-oefficient is supplied   */
   /*   in the digitiser-info structure and can be changed by the client. */
   x = (ctx->lstX*digInfo.xFilter+x*((1<<DigFilterBits)-digInfo.xFilter))
       >> DigFilterBits;
   y = (ctx->lstY*digInfo.yFilter+y*((1<<DigFilterBits)-digInfo.yFilter))
       >> DigFilterBits;

   /* Compute difference between this reported value and and the last     */
   /*   reported value with a view to rejecting the sample if it is too   */
   /*   far away from the old sample.                                     */
   dx = x-ctx->lstX; if(dx<0) dx=-dx;
   dy = y-ctx->lstY; if(dy<0) dy=-dy;
  
   /* Record the currently-computed stylus co-ordinate in the context    */
   /*   structure and copy down the co-ordinate raw cycle number as well */
   ctx->lstCyc = rp->cyc;
   ctx->lstX   = x;
   ctx->lstY   = y;
   ctx->tm     = 0; /* Stub for the moment: This should be a timestamp    */

   /* Check to see if there is a large delta between the last reported    */
   /*   digitiser value and the digitiser value about to be reported.     */
   /* If the delta is small enough, mark this co-ordinate as valid for    */
   /*   transmission as an event.                                         */
   if(dx<=digInfo.xGlitch&&dy<=digInfo.yGlitch) ctx->ctt=1;

   /* Now indicate that a new co-ordinate is availiable for transmission:*/
   *(int*)0x74001C = 0x00000000;
   goto tryAgain;
}

/*------------------------------------------------------------------------*/
/*                                          DigGenerateButtonPacket(...)  */
/*------------------------------------------------------------------------*/

static int DigGenerateButtonPacket(Stylus_Event *des,word *cyc,DigContext *ctx)
/* This function is called by DigGetProcessedPacket(...) whenever the     */
/*   current state of the digitiser buttons is not consistent with the    */
/*   state of digitiser buttons as sent in the event stream. This code    */
/*   locates a button which has changed, and generates an appropriate     */
/*   Stylus_Event packet which contains the changed-button-code, and      */
/*   modifies it's internal idea of which button changes have been sent   */
/*   in events.                                                           */
/* Returns 1 if the change on status really does result in a bona-fide    */
/*   stylus event, otherwise zero.                                        */
{  ubyte del;
   WORD evr;
   int b;

   del = (ctx->is^ctx->was);
   for(b=0;b<8;b++) if(del&(1<<b)) break;
   if(ctx->is&(1<<b))
   {  switch(b)
      {  case DigTipBitPos:     evr = (word)Buttons_Tip_Down;    break;
         case DigBarrelBitPos:  evr = (word)Buttons_Barrel_Down; break;
         case DigProxBitPos:    evr = (word)Buttons_Into_Prox;   break;
         case DigLeftBitPos:    evr = (word)Buttons_Left_Down;   break;
         case DigMddlBitPos:    evr = (word)Buttons_Middle_Down; break;
         case DigRghtBitPos:    evr = (word)Buttons_Right_Down;  break;
         default:               evr = (word)0;                   break;
      }
   } else
   {  switch(b)
      {  case DigTipBitPos:     evr = (word)Buttons_Tip_Up;      break;
         case DigBarrelBitPos:  evr = (word)Buttons_Barrel_Up;   break;
         case DigProxBitPos:    evr = (word)Buttons_OutOf_Prox;  break;
         case DigLeftBitPos:    evr = (word)Buttons_Left_Up;     break;
         case DigMddlBitPos:    evr = (word)Buttons_Middle_Up;   break;
         case DigRghtBitPos:    evr = (word)Buttons_Right_Up;    break;
         default:               evr = (word)0;                   break;
      }
   }

   ctx->was = (ctx->was&~(1<<b))|(ctx->is&(1<<b));
   if(evr==0) return 0;

   des->Buttons = evr;
   des->X       = ctx->lstX;
   des->Y       = ctx->lstY;
   *cyc         = ctx->lstCyc;
   return 1;
}

/*----------------------------------------------------------------------*/
/*                                                   DigDecodeOrd(...)  */
/*----------------------------------------------------------------------*/

static int DigDecodeOrd ( MLEstyluPacket *pkt,
                          DigContext     *ctx, 
                          int            *ord,
                          int             isX
                        )
{  int MinCrs,MaxCrs;
   int PktStrt,PktLim;
   int cp,i;
 
   cp = pkt->cp;
 
#  if DgSwDiags
   if(pkt->bt&MLEstyluBarrel||dgDiag)
   {  diag
      (( "%s : %02X %02X %02X ; %+4d %+4d %+4d %+4d %+4d ; %3d ; %02X : %02X %02X ",
         isX?"X":"Y", pkt->hdr, pkt->len, pkt->ord,
                      pkt->w[0],pkt->w[1],pkt->w[2],pkt->w[3],pkt->w[4],
                      pkt->cp,pkt->bt,
                      pkt->d1,pkt->d2
      ));
   }
#  endif
      
   /* A course value of -1 indicates out-of-proximity:                  */
   if(pkt->cp==0) 
   {  
#     if DgSwDiags
      if(pkt->bt&MLEstyluBarrel||dgDiag)
      {  diag(("cp==0\n"));  }
#     endif
      ctx->is&=~DigProx; return 0;
   }
   
   /* Check the proximity condition by testing that there is sufficient */
   /*   change in the wire values between the values on either side of  */
   /*   the supposed crossing-point.                                    */
   if ((pkt->w[2]-pkt->w[3])<DigProxLimit) 
   {  
#     if DgSwDiags
      if(pkt->bt&MLEstyluBarrel||dgDiag) { diag(("Not steep\n")); }
#     endif
      ctx->is&=~DigProx; /* Clear proximity bit */
      return 0; 
   }
 
   /* Determine and test boundary conditions:                           */
   if(isX) MaxCrs=44,MinCrs=1; else MaxCrs=32,MinCrs=1;
   if(cp<MinCrs+2) PktLim  = 3+cp-MinCrs; else PktLim  = 5;
   if(cp>MaxCrs-2) PktStrt = cp-MaxCrs+2; else PktStrt = 0;

   /* Check that the wire values are monotonic decreasing. If they are  */
   /*   not the data is most probably duff so discard the packet.       */

   for(i=PktStrt;i<PktLim-1;i++)
   {  if(pkt->w[i]<pkt->w[i+1]) 
      {  
#        if DgSwDiags
         if(pkt->bt&MLEstyluBarrel||dgDiag) { diag(("Increases at %d\n",i)); }
#        endif
         return 0;
      }
   }

#  if DgSwDiags
   if(pkt->bt&MLEstyluBarrel||dgDiag) { diag(("\n")); }
#  endif
   
   if (PktLim-PktStrt<5)
   {  *ord = DigQuadInterp(pkt->w+PktStrt) 
           + (PktStrt-1+MaxCrs-cp)*DigCountsPerWire; 
   }
   else
   {  *ord = DigCubicInterp(pkt->w) 
           + (MaxCrs-cp)*DigCountsPerWire; 
   }

   return 1;

}

/*----------------------------------------------------------------------*/
/*                                                 DigCubicInterp(...)  */
/*----------------------------------------------------------------------*/

static int DigCubicInterp( signed char w[5] )
/* Cubic interpolation : Fit a 3rd degree polynomial into the five wire */
/*    values and find a zero of the polynomial using two Newton-Raphson */
/*    approximations.                                                   */
{  int w1,w2,w3,w4,w5;
   int L0,L1,L2,L3;
   int L13,L0L1L2,L02L3,num,denom,res;

   /* Download the five wire values into local variables:               */
   w1=w[0]; w2=w[1]; w3=w[2]; w4=w[3]; w5=w[4];
   
   /* The co-efficients of the polynomial (*6) are computed into the    */
   /*   variables L0 .. L3 where Li is the coefficient of x^i           */
   L0 = 6*w3;
   L1 = -2*w2 - 3*w3 + 6*w4 - w5;
   L2 = 3*w2 - 6*w3 + 3*w4;
   L3 = -w2 + 3*w3 - 3*w4 + w5;
  
   /* Compute various terms for two Newton-Raphson approximations:      */
   /* Round values as you go to avoid overflow ...                      */
   L13    = (((L1*L1)>>5)*L1)>>3;
   L0L1L2 = (((L0*L1)>>5)*L2)>>3;
   L02L3  = (((L0*L0)>>5)*L3)>>3;

   /* Compute the numerator and denominator of the fraction which       */
   /*   represents two Newton-Raphson approximations to the zero of the */
   /*   polynomial obtained.                                            */
   /* Round as you go along to avoid overflow ...                       */
   num   = -L0 * ((L13 -   L0L1L2 + 2*L02L3)>>12);
   denom =  L1 * ((L13 - 2*L0L1L2 + 3*L02L3)>>12);

   /* Call a special division routine : See 'divide.s': The routine     */
   /*    computed (2^16*num)/denom. If we computed this in 'C' the      */
   /*    first multiplication would overflow so double-arithmetic would */
   /*    be required. Hence we use a simpler piece of assembly to do    */
   /*    the job. If denom=0 (unlikely) the assembler will just return  */
   /*    some preposterous value which would result in a funny          */
   /*    co-ordinate.                                                   */
   /* As seen, the result is then scaled so that DigCountsPerWire       */
   /*    ordinates are reported between two coarse positions.           */
   res = (DigDivide(num,denom)*DigCountsPerWire) >> 16;

   return res;
}

/*----------------------------------------------------------------------*/
/*                                                  DigQuadInterp(...)  */
/*----------------------------------------------------------------------*/

static int DigQuadInterp( signed char w[5] )
{  int w1,w2,w3;
   int L0,L1,L2;
   int L12,L0L2,num,denom,res;
 
   /* This function similar to quad interp but with fewer wire-values    */
   /*   we have to perform a quadratic-fit instead of a cubic fit.       */
   
   /* Download the wire values:                                          */
   w1 = w[0];
   w2 = w[1];
   w3 = w[2];
   
   /* Compute the co-efficients of the polynomial which fits these       */
   /*    wires.                                                          */
   L0 = 2*w2;
   L1 = w3 - w1;
   L2 = w3 + w1 - 2*w2;
 
   /* Compute terms required for two Newton-Raphson approximations:      */
   L12  = L1*L1;
   L0L2 = L0*L2;
 
   /* Compute the numerator and denominator of the fraction which        */
   /*    represents the results of two Newton-Raphson approximations:    */
   /* Round as you go along to avoid overflow                            */
   num   = L0 * ((L0L2-   L12)>>2);
   denom = L1 * (( L12-2*L0L2)>>2);
 
   /* Do a special fixed-point division (see DigCubicInterp(...)) and    */
   /*   scale to DigCountsPerWire difference from one coarse position    */
   /*   to the next.                                                     */
   res = (DigDivide(num,denom)*DigCountsPerWire)>>16;

   return res;
}
