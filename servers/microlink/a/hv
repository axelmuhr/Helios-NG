/* $Header: hermv.c,v 1.2 91/01/09 12:31:44 charles Locked $ */
/* $Source: /server/usr/users/b/charles/world/microlink/RCS/source/hermv.c,v $ */

/*------------------------------------------------------------------------*/
/*                                              microlink/source/hermv.c  */
/*------------------------------------------------------------------------*/

/* This file implements the Hermes version number part of the microlink   */
/*   interface module. This corresponds to the entry 'hermv' in the       */
/*   microlink directory. If you do a 'cat .microlink/hermv' this server  */
/*   asks the microcontroller for the version number and sends it back as */
/*   ASCII text. Alternatively a call to GetInfo() will do the same       */
/*   except return the information in binary format.                      */

/*------------------------------------------------------------------------*/
/*                                                          Header Files  */
/*------------------------------------------------------------------------*/

# include "microlink/private/microlink.h"
# include "microlink/private/hermv.h"
# include "microlink/general.h"

/*------------------------------------------------------------------------*/
/*                                               Look-ahead declarations  */
/*------------------------------------------------------------------------*/

void mlkAddHermvEntry          (DirNode *microlinkDir);
void mlkServeHermv             ( ServInfo        *si , 
                                 MicrolinkNode  *nde , 
                                 MsgBuf        *rply 
                               );
static void HermvRead          (MCB *msg,HermvContext *ctx);
static void HermvGetSize       (MCB *msg,HermvContext *ctx);
static void HermvSeek          (MCB *msg);
static void HermvDefault       (MCB *msg);
static void HermvGetInfo       (MCB *msg,HermvContext *ctx);

/*------------------------------------------------------------------------*/
/*                                                 mlkAddHermvEntry(...)  */
/*------------------------------------------------------------------------*/

void mlkAddHermvEntry(DirNode *microlinkDir)
/* Add the entry called 'hermv' into the microlnk directory.              */
{  static MicrolinkNode nde;

   InitNode((ObjNode*)&nde,"hermv",Type_File,0,DefFileMatrix);
   nde.objTyp = MlkHermv;
   Insert(microlinkDir,(ObjNode*)&nde.obj,TRUE);
}

/*------------------------------------------------------------------------*/
/*                                                   mlkDoOpenHermv(...)  */
/*------------------------------------------------------------------------*/

void mlkServeHermv( ServInfo *si , MicrolinkNode *nde , MsgBuf *rply )
/* This function called in response to any request to open a stream to    */
/*   the hermv part of the microlink server: It services all requests     */
/*   that come over the message-port allocated to correspond to that open */
/*   stream.                                                              */
{  MlkLinkRequestControl lr;
   HermvContext     ctx;
   MCB             *msg;
   Port            reqp;
   byte           *data;
   word             res;
   ubyte      txBuf[10];
   ubyte      rxBuf[10];

   UnLockTarget(si);                     /* Unlock target for other calls */
   PutMsg(&rply->mcb);                   /* Send reply                    */
   
   /* Time to ask the microcontroller for it's version number and load    */
   /*   the information into the context structure and also compute       */
   /*   a line of text describing the verion number in ASCII format.      */
   txBuf[0] = ASQhermv(1);
   rxBuf[0] = MLYhermv;
   lr.txBuf = txBuf; lr.rxBuf = rxBuf;
   MlkPerformLinkRequest(&lr);
   if(lr.status==General_Ok)
   {  ctx.info.min = MLYhermvMin(rxBuf);
      ctx.info.maj = MLYhermvMaj(rxBuf);
   } else
   {  ctx.info.min = -1;
      ctx.info.maj = -1;
   }
   sprintf
   (  ctx.txt,
      "The version number of Hermes is %d.%d\n",
      ctx.info.maj,ctx.info.min
   );
  
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
            HermvRead(msg,&ctx);         /*  Service it                   */
            break;                       /*  Get another request          */
         case FG_Close:                  /* Close request:                */
            goto hermvClose;             /*  Break out of service loop    */
         case FG_GetSize:                /* GetSize request:              */
            HermvGetSize(msg,&ctx);      /*  Service it                   */
            break;                       /*  Get another request          */
         case FG_Seek:                   /* Seek request:                 */
            HermvSeek(msg);              /*  Service it                   */
            break;                       /*  Get another request          */
         case FG_GetInfo:                /* Get information request       */
            HermvGetInfo(msg,&ctx);      /*  Get digitiser information    */
            break;                       /*  Get another request          */
         default:                        /* Unknown message ...           */
            HermvDefault(msg);           /*  Action on unknown message    */
            break;                       /*  Get another request          */
      }
   }

   hermvClose:;

   if(msg->MsgHdr.Reply!=NullPort) ErrorMsg(msg,(word)Err_Null);
}

/*------------------------------------------------------------------------*/
/*                                                        HermvRead(...)  */
/*------------------------------------------------------------------------*/

static void HermvRead(MCB *msg,HermvContext *ctx)
/* A call to read data along an open channel of the hermes stream: This   */
/*   function returns the appropriate package of data from it's internal  */
/*   ASCII buffer containing a description of the hermes version number   */
/*   as ascertained when the stream was opened.                           */
{  ReadWrite              *rcs;
   Port                  rport;
   word                    pos;
   word                    siz;
   word                    eof;

   rcs = (ReadWrite*)msg->Control;
   rport = msg->MsgHdr.Reply;
   pos = rcs->Pos;
   siz = rcs->Size;
   if(pos<0 || siz<0)
   {  ErrorMsg(msg,EC_Error|EG_Parameter|EO_Object);
      return;
   }
   eof = 0;
   if(pos+siz>strlen(ctx->txt)) siz=strlen(ctx->txt)-pos, eof=1;
   if(siz<0) siz=0;
   
   InitMCB
   (  msg,MsgHdr_Flags_preserve,
      rport,NullPort,(eof?ReadRc_EOF:ReadRc_EOD)
   );
   msg->Data            = ctx->txt+pos;
   msg->MsgHdr.DataSize = (int)siz;
   PutMsg(msg);
   FreePort(rport);
}

/*------------------------------------------------------------------------*/
/*                                                     HermvGetSize(...)  */
/*------------------------------------------------------------------------*/

static void HermvGetSize(MCB *msg,HermvContext *ctx)
{  InitMCB(msg,0,msg->MsgHdr.Reply,NullPort,(word)Err_Null);
   MarshalWord(msg,strlen(ctx->txt));
   PutMsg(msg);
}

/*------------------------------------------------------------------------*/
/*                                                        HermvSeek(...)  */
/*------------------------------------------------------------------------*/

static void HermvSeek(MCB *msg)
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
/*                                                     HermvDefault(...)  */
/*------------------------------------------------------------------------*/

static void HermvDefault(MCB *msg)
/* This function is called whenever an unimplemented function code is     */
/*    discovered over the open-stream request port.                       */
{  ErrorMsg(msg,EC_Error|EG_Unknown|EO_Object);  }

/*------------------------------------------------------------------------*/
/*                                                     HermvGetInfo(...)  */
/*------------------------------------------------------------------------*/

static void HermvGetInfo(MCB *msg,HermvContext *ctx)
{  
   /* This function returns the Hermes version number as obtained when    */
   /*   the stream was opened. It returns it in binary format as          */
   /*   specified in the file 'ext/hermv.h'                               */

   InitMCB
   (  msg,0,msg->MsgHdr.Reply,NullPort,(word)Err_Null  );
   msg->Control = (word*)&ctx->info;
   msg->MsgHdr.ContSize = sizeof(HermvInfo);
   PutMsg(msg);
}

