#include <helios.h>
#include <link.h>
#include "adapter.h"
#include "linkio.h"
#include "mcdep.h"

#define ReadyForRx(l) ((l)->LA_IStat & i_rdy)
#define ReadyForTx(l) ((l)->LA_OStat & o_rdy)

#define TxByte(b,l) ((l)->LA_OData = b)
#define RxByte(l) ((l)->LA_IData)

#define DisableRxInts(l) ((l)->LA_IStat = 0)
#define DisableTxInts(l) ((l)->LA_OStat = 0)

#define EnableRxInts(l) ((l)->LA_IStat = i_inte)
#define EnableTxInts(l) ((l)->LA_OStat = o_inte)

#define POLL_LIMIT 1000

#define use(x) (x=x)

#define DEBUG

void Link_Int_Disable(void)
{
	DisableRxInts(ADAPTER);
	DisableRxInts(ADAPTER);
}

void LinkTx(word size, struct LinkInfo *link, void *buf)
{	EXECROOT *xroot = Execroot();
	int npolls = POLL_LIMIT;
	char *cbuf = (char *)buf;
	use(link);
#ifdef DEBUG1
	XIOdebug("LinkTx: started \n");
#endif
	while( npolls-- )
	{
		if( ReadyForTx(ADAPTER) )
		{
			TxByte(*cbuf++,ADAPTER);
			if( --size == 0 ) return;
			npolls = POLL_LIMIT;
		}
	}

	xroot->TxData = cbuf;
	xroot->TxSize = size;
	EnableTxInts(ADAPTER);
#ifdef DEBUG1
	XIOdebug("LinkTx: suspending \n");
#endif
	Suspend(&xroot->TxSaveState);
#ifdef DEBUG1
	XIOdebug("LinkTx: wakeup \n");
#endif
}

void LinkRx(word size, struct LinkInfo *link, void *buf)
{	EXECROOT *xroot = Execroot();
	int npolls = POLL_LIMIT;
	char *cbuf = (char *)buf;
#ifdef DEBUG1
	XIOdebug("LinkRx: started \n");
#endif
	use(link);

	while( npolls-- )
	{
		if( ReadyForRx(ADAPTER) )
		{
			*cbuf++ = RxByte(ADAPTER);
			if( --size == 0 ) return;
			npolls = POLL_LIMIT;
		}
	}
	xroot->RxData = cbuf;
	xroot->RxSize = size;
	EnableRxInts(ADAPTER);

	Suspend(&xroot->RxSaveState);
#ifdef DEBUG1
	XIOdebug("LinkRx: leaving \n");
#endif
}

void LinkTxP(word size, struct LinkInfo *link, void *buf)
{	
	char *cbuf = (char *)buf;
	use(link);
	while(1)
	{
#ifdef DEBUG1
	XIOdebug(" LTX1 ");
#endif
#if 1
		if( ReadyForTx(ADAPTER) )
#else
		while(!(ADAPTER->LA_OStat & o_rdy));
#endif
		{
#ifdef DEBUG1
	XIOdebug(" LTX2 ");
#endif
#if 1
			TxByte(*cbuf++,ADAPTER);
#else
			ADAPTER->LA_OData = *cbuf++;
#endif
			if( --size == 0 ) return;
		}
	}
}

void LinkRxP(word size, struct LinkInfo *link, void *buf)
{	
	char *cbuf = (char *)buf;
	use(link);
	while(1)
	{
		if( ReadyForRx(ADAPTER) )
		{
			*cbuf++ = RxByte(ADAPTER);
			if( --size == 0 ) return;
		}
	}

}

word  RxInterruptHandler(adapter *l)
{	EXECROOT *xroot = Execroot();
	char *data = xroot->RxData;
	word   n    = xroot->RxSize;
	int npolls = POLL_LIMIT;
#ifdef DEBUG1
	XIOdebug("RxInt: ");
#endif
	while( npolls-- )
	{
		if( ReadyForRx(l) )
		{
			*data++ = RxByte(l);
			if( --n == 0 )
			{
				DisableRxInts(l);
				if (!(NullStateP(xroot->RxSaveState)))
				{
					Restart(xroot->RxSaveState);
					xroot->RxSaveState = (SaveState *)MinInt;
				}
				break;
			}
			else
				npolls = POLL_LIMIT;
		}
	}
	xroot->RxSize = n;
	xroot->RxData = data;
	
	return 1;
}

word TxInterruptHandler(adapter *l)
{	EXECROOT *xroot = Execroot();
	char *data = xroot->TxData;
	word  n    = xroot->TxSize;
	int   npolls = POLL_LIMIT;

#ifdef DEBUG1
	XIOdebug("LinkTxInt: started \n");
#endif

	while(npolls--)
	{	if( ReadyForTx(l) )
		{
			TxByte(*data++,l);
			if( --n == 0 )
			{
				DisableTxInts(l);
				if (!(NullStateP(xroot->TxSaveState)))
				{
#ifdef DEBUG1
	XIOdebug("LinkTxInt : Restarted \n");
#endif
					Restart(xroot->TxSaveState);
					xroot->TxSaveState = (SaveState *)MinInt;
				}
				break;
			}
			else
				npolls = POLL_LIMIT;
		}
	}
	xroot->TxSize = n;
	xroot->TxData = data;
	
	return 1;
}

SaveState *AbortLinkTx(LinkInfo *link)
{
	EXECROOT *xroot = Execroot();	
	SaveState *x = xroot->TxSaveState;
	
	xroot->TxSaveState = (SaveState *)MinInt;
	
	return x;
	
}

SaveState *AbortLinkRx(LinkInfo *link)
{
	EXECROOT *xroot = Execroot();	
	SaveState *x = xroot->RxSaveState;
	
	xroot->RxSaveState = (SaveState *)MinInt;
	
	return x;
	
}


