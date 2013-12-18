/* $Id: netdev.c,v 1.8 1994/03/17 13:56:20 nickc Exp $ */
/*
 * PC-ETHER
 *
 * This is an ethernet device which simply sends all requests to a remote
 * /ether server. Such a server is currently implemented by the PC IO server
 * for Western Digital and DLink cards.
 *
 * All ethernet devices should comform to the external interface spec of this
 * example.
 *
 */
 
#include <device.h>
#include <syslib.h>
#include <nonansi.h>
#include <queue.h>
#include <sem.h>
#include <codes.h>
#ifdef __C40
#include <c40.h>
#endif

/* #define DEBUG */

/*
 * Extend the DCB structure to contain device specific data. Since
 * devices cannot have static data this is the ONLY place that persistent
 * data may be stored.
 */
typedef struct
  {
    DCB		dcb;		/* Common part of DCB structure	*/
    Semaphore	lock;		/* Concurrent access lock	*/
    Stream *	read;		/* Input stream			*/
    Stream *	write;		/* Output stream		*/
    List	readq;		/* List of pending read requests*/
    Semaphore	nreq;		/* Size of readq		*/
  }
NetDCB;

/*
 * DevOperate is called by the server each time a device operation is
 * requested.
 */

static void
DevOperate(
	   NetDCB *	dcb,
	   NetDevReq *	req )
{
  NetInfoReq *	ireq = (NetInfoReq *)req;
  NetInfo *  info = &ireq->NetInfo;

  
  switch (req->DevReq.Request)
    {
    case FG_Read:
      /* Queue read requests for Reader process. The server	*/
      /* currently sends us several of these when it starts.	*/
      /* Note that we must lock the DCB against concurrent 	*/
      /* access.					 	*/
      
      Wait( &dcb->lock );
      
      AddTail( &dcb->readq, &req->DevReq.Node );
      
      Signal( &dcb->nreq );
      Signal( &dcb->lock );
      return;
      
    case FG_Write:
#ifdef DEBUG
	{
	  int 		i;
	  char *	buf = (char *)req->Buf;
	  
	  IOdebug( "TX %d buf = %x [%", req->Size, buf );
	  for (i = 0;i < 24; i++)
	    IOdebug( "%x %", buf[ i ] );
	  IOdebug( "]" );
	}
#endif
      /* Write data to /ether server inline			*/
      
      req->Actual = Write( dcb->write, (char *) req->Buf, req->Size, -1 );
      
      req->DevReq.Result = Result2( dcb->write );
      break;
      
    case FG_SetInfo:
#ifdef DEBUG
      IOdebug( "Net SetInfo %x %x %x [%", info->Mask, info->Mode, info->State );
      
	{
	  int i;
	  
	  for (i = 0; i < 5; i++ )
	    IOdebug( "%d.%", info->Addr[ i ] );
	  
	  IOdebug( "%d]", info->Addr[ 5 ] );
	}
#endif
      /* Set options/address in ethernet device		*/
      /* The tcpip server will attempt to set the ethernet	*/
      /* address on startup.					*/
      /* SetInfo does nothing in this driver.			*/
      /* req->DevReq.Result = SetInfo(dcb->write,info,sizeof(NetInfo)); */
      
      req->DevReq.Result = 0;
      
      break;
      
    case FG_GetInfo:
      /* Get options/address from /ether device. Only the	*/
      /* ethernet address is used by tcpip server.		*/
      
      info->Mask = 7;
      
      req->DevReq.Result = GetInfo( dcb->read, (byte *)info );
      
#ifdef DEBUG
	{
	  int i;
	  
	  IOdebug( "Addr [%" );
	  for (i = 0; i < 6; i++ )
	    IOdebug( "%d %", info->Addr[ i ] );
	  IOdebug( "]" );
	}
#endif
      break;
    }
  
#if 0
  /* Unlock the DCB before returning request to server		*/
  Signal( &dcb->lock );
#endif
  
  /* return request to server by calling the Action routine in	*/
  /* the request. Note that the tcpip server will re-call 	*/
  /* DevOperate before returning from this routine when the	*/
  /* request was a Read.					*/
  
  (*req->DevReq.Action)( dcb, req );

  return;
  
} /* DevOperate */


/*
 * The Reader process simply waits for a request to arrive and then
 * reads a message from the /ether server.
 * It calls the Action routine, so the stack size here must be adequate
 * for the tcpip server to do its packet arrival processing.
 */
static void
Reader( NetDCB * dcb )
{
  word 		got;
  NetDevReq *	req;

  
  for (;;)
    {		
      /* wait for a request from server 			*/
      
      Wait( &dcb->nreq );
      
      /* remove it from list. Note that we must lock the DCB	*/
      /* before doing this.					*/
      
      Wait( &dcb->lock );
      
      req = (NetDevReq *)RemHead( &dcb->readq );
      
      Signal( &dcb->lock );
      
      if (req == NULL)
	continue;
      
      /* Read until we get some data, we hope that it	will	*/
      /* be a complete ether packet.				*/
    again:		
      got = Read( dcb->read, (char *) req->Buf, req->Size, -1 );
      
      if (got <= 0)
	{
#ifdef DEBUG
	  IOdebug( "Reader: Read failed, return code = %d, Result2 = %x", got, Result2( dcb->read ) );
#endif	  
	  goto again;
	}

#ifdef DEBUG
	{
	  int 		i;
	  char *	buf = (char *)req->Buf;
	  
	  IOdebug( "RX %d [%", got );
	  for (i = 0;i < 24; i++)
	    IOdebug( "%x %", buf[ i ] );
	  IOdebug( "]" );
	}
#endif
      req->Actual        = got;
      req->DevReq.Result = 0;
      
      /* Return request to server.				*/
      
      (*req->DevReq.Action)( dcb, req );	
    }

  return;
  
} /* Reader */


/*
 * DevClose - this is never called by the current tcpip server
 */
static word
DevClose( NetDCB * dcb )
{
  Wait( &dcb->lock );
  
  Close( dcb->read  );
  Close( dcb->write );
  
  Free( dcb );

#ifdef DEBUG
  IOdebug( "Ether: closed down ethernet device" );
#endif
  
  return 0;
}


/*
 * DevOpen - this is called to initialize the device. It must allocate
 * the DCB, inititiaize it and initialize the hardware.
 * The info parameter points to a structure which has been initialized from
 * a devinfo file netdevice entry.
   */
NetDCB *
DevOpen(
	MPtr		dev,
	NetDevInfo *	info
)
{
  NetDCB *		dcb;
  Object *		o;

  /* Allocate the DCB						*/

  dcb = (NetDCB *) Malloc( sizeof (NetDCB) );
  
  if (dcb == NULL)
    {
#ifdef DEBUG
      IOdebug( "Ether: Failed to allocate memory for DCB structure" );
#endif      
      return NULL;
    }
  
  /* Look for /ether device					*/

#ifdef __C40
  {
  	/* C40 devices cannot access string constants. The following	*/
  	/* is a work-around to get access to the string.		*/
  	char name[12];

  	SetString_(name,0,'/','e','t','h');
  	SetString_(name,1,'e','r','\0','\0');
	o = Locate( NULL, name );  	
  }
#else
  o = Locate( NULL, "/ether" );
#endif

  if (o != NULL)
    {
      /* We open two streams to the /ether device so that we	*/
      /* can both read and write simultaneously.		*/
      
      dcb->read  = Open( o, NULL, O_ReadOnly  );
      dcb->write = Open( o, NULL, O_WriteOnly );
      
      Close( o );
    }
#ifdef DEBUG
  else
    {
      IOdebug( "Ether: failed to locate /ether service" );      
    }
#endif  
  
  if (o          == NULL ||
      dcb->read  == NULL ||
      dcb->write == NULL )
    {
      Close( dcb->read );
      Close( dcb->write );
      
      Free( dcb );

#ifdef DEBUG
      IOdebug( "Ether: Failed to connect to /ether service" );
#endif
      return NULL;
    }
  
  /* Initialize the common DCB fields				*/
  
  dcb->dcb.Device  = dev;
  dcb->dcb.Operate = DevOperate;
  dcb->dcb.Close   = DevClose;
  
  /* Initialize our private fields				*/
  
  InitSemaphore( &dcb->lock, 1 );
  
  InitList( &dcb->readq );
  
  InitSemaphore( &dcb->nreq, 0 );		
  
  /* Start the Reader process.					*/
  
  if (Fork( 2000, Reader, 4, dcb ) == 0)
    {
      IOdebug( "Ether: failed to Fork Reader process" );      
    }  

#ifdef DEBUG
  IOdebug( "Ether: successfully contact /ether service" );
#endif
  
  return dcb;
  
  info=info; /* not used, this stops compiler moaning */
  
} /* DevOpen */

