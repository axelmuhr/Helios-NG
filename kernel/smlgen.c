
/*{{{  InitSMLChan  */

#ifdef __STDC__
static WORD InitSMLChan
(
	SMLChannel	*sc,
	CBPtr		cb,
	WORD		cbsize
)
#else
static WORD InitSMLChan(sc,cb,cbsize)
SMLChannel	*sc;
CBPtr		cb;
WORD		cbsize;
#endif
{
	sc->Cb		= cb;
	sc->BufferMax	= cbsize - CBOverhead;
	sc->Waiter	= NULL;
	sc->Reason	= 0;
	sc->Size	= 0;
	sc->Buf		= 0;
	sc->Left	= 0;

	SetCBWord_(cb,DataReady,0);
	SetCBWord_(cb,Ack,0);
	SetCBWord_(cb,Size,0xfeed0dad);

	return 0;
}
/*}}}*/
/*{{{  SMLTx  */
/*--------------------------------------------------------
-- SMLTx						--
--							--
-- Transmit a buffer full of data via the link.		--
--							--
--------------------------------------------------------*/

#ifdef __STDC__
static WORD SMLTx(LinkTransferInfo *info)
#else
static WORD SMLTx(info)
LinkTransferInfo *info;
#endif
{
	SMLChannel *sc = (SMLChannel *)info->link->TxChan;
	CBPtr cb = sc->Cb;

#if USE_ABORT	
	if( info->size == 0 )		/* abort transmission		*/
	{
		SaveState *s = sc->Waiter;
		if( NullStateP(s) ) return (WORD)P_NullState;

		/* signal abort to transmitter				*/
		sc->Reason 	= SML_Aborted;

		/* reset state to initial values			*/
		sc->Waiter 	= P_NullState;
		sc->Size	= 0;
		SetCBWord_(cb,DataReady,0);
		SetCBWord_(cb,Ack,0);

		return (WORD)s;
	}
#endif

	/* save size and buffer for interrupt routine			*/
	sc->Size = info->size*MPSize;
	sc->Buf = info->buf;

	while( sc->Size > 0 )
	{
		WORD tfr = sc->Size;

		if( tfr > sc->BufferMax ) tfr = sc->BufferMax;

		SetCBData_(cb,sc->Buf,tfr);
		SetCBWord_(cb,Size,tfr);

		SetCBWord_(cb,DataReady,1);

		SendInterrupt(sc->IntInfo);		
		SendInterrupt(sc->IntInfo);		
		SendInterrupt(sc->IntInfo);		
		SendInterrupt(sc->IntInfo);		

		Await(sc,CBAck_(cb));

		if( sc->Reason != SML_Wakeup )
		  {
		    break;
		  }

		SetCBWord_(cb,Ack,0);

		sc->Size -= tfr;
		sc->Buf = MInc_(sc->Buf,tfr);

	}

	return 0;
}

/*}}}*/
/*{{{  SMLRx  */
/*--------------------------------------------------------
-- SMLRx						--
--							--
-- Receive a buffer full of data via the link.		--
--							--
--------------------------------------------------------*/

#ifdef __STDC__
static WORD SMLRx(LinkTransferInfo *info)
#else
static WORD SMLRx(info)
LinkTransferInfo *info;
#endif
{
	SMLChannel *sc = (SMLChannel *)info->link->RxChan;
	CBPtr cb = sc->Cb;


#if USE_ABORT		
	if( info->size == 0 )		/* abort transfer		*/
	{
		SaveState *s = sc->Waiter;
		if( NullStateP(s) ) return (WORD)P_NullState;


		/* signal abort to reciever				*/
		sc->Reason 	= SML_Aborted;

		/* reset state to initial values			*/
		sc->Waiter 	= P_NullState;
		sc->Size	= 0;
		SetCBWord_(cb,DataReady,0);
		SetCBWord_(cb,Ack,0);

		return (WORD)s;
	}
#endif

	/* save size and buffer for interrupt routine			*/
	sc->Size = info->size*MPSize;
	sc->Buf = info->buf;

	while( sc->Size > 0 )
	{
		WORD tfr;
		WORD pos;

		if( sc->Left != 0 )
		{
			tfr = sc->Left;
			pos = CBWord_(cb,Size) - tfr;
			if( tfr > sc->Size )
			{
				sc->Left = tfr - sc->Size;
				tfr = sc->Size;
			}
			else sc->Left = 0;

			SMData_(MtoC_(sc->Buf), cb, offsetof(ChannelBuffer,Buffer)+pos, tfr);

		}
		else
		{
			
			Await(sc,CBDataReady_(cb));

			if( sc->Reason != SML_Wakeup ) break;

			tfr = CBWord_(cb,Size);

			if( tfr > sc->Size )
			{
				sc->Left = tfr - sc->Size;
				tfr = sc->Size;
			}

			CBData_(sc->Buf,cb,tfr);

		}

		sc->Size -= tfr;
		sc->Buf = MInc_(sc->Buf,tfr);

		if( sc->Left == 0 )
		{
			SetCBWord_(cb,DataReady,0);
			SetCBWord_(cb,Ack,1);
			SendInterrupt(sc->IntInfo);
		}
		
	}

	return 0;
}
/*}}}*/
/*{{{  SMLTxRdy  */

/*--------------------------------------------------------
-- SMLTxRdy						--
--							--
-- Check channel for ready to transmit			--
--							--
--------------------------------------------------------*/

#ifdef __STDC__
static WORD SMLTxRdy(LinkInfo *link)
#else
static WORD SMLTxRdy(link)
LinkInfo *link;
#endif
{
#if 1
	SMLChannel *sc = (SMLChannel *)link->TxChan;
	CBPtr cb = sc->Cb;

	if (sc -> Left != 0 || CBWord_(cb, DataReady) != 0)
	{
		return FALSE;
	}
	else
	{
		return TRUE;
	}

/*	return (sc->Left != 0 || CBWord_(cb, DataReady) != 0); */
#else
	return TRUE;
#endif
}

/*}}}*/
/*{{{  SMLRxRdy  */
/*--------------------------------------------------------
-- SMLRxRdy						--
--							--
-- Check link for ready to recieve			--
--							--
--------------------------------------------------------*/

#ifdef __STDC__
static WORD SMLRxRdy(LinkInfo *link)
#else
static WORD SMLRxRdy(link)
LinkInfo *link;
#endif
{
	SMLChannel *sc = (SMLChannel *)link->RxChan;
	CBPtr cb = sc->Cb;

	return ( sc->Left != 0 || CBWord_(cb,DataReady) != 0 );
}
/*}}}*/
/*{{{  SMLInterrupt  */

#if USE_INTERRUPT

/*--------------------------------------------------------
-- SMLInterrupt						--
--							--
-- Handle a shared memory link interrupt.		--
--							--
--------------------------------------------------------*/

#ifdef __STDC__
static WORD SMLInterrupt(LinkInfo *link, WORD vector)
#else
static WORD SMLInterrupt(link,vector)
LinkInfo *link;
WORD vector;
#endif
{
	SMLChannel *sc = (SMLChannel *)(link->TxChan);
	CBPtr cb = sc->Cb;
#if TX_IN_INTERRUPT || RX_IN_INTERRUPT

	WORD tfr;
#endif
	WORD handled = FALSE;

	AcknowledgeInterrupt(link,vector);

	/* Transmit side...						*/
	if( !NullStateP(sc->Waiter) && CBWord_(cb,Ack) != 0 )
	{
		handled = TRUE;

#if TX_IN_INTERRUPT
		/* Complete the transfer...				*/

		SetCBWord_(cb,Ack,0);

		tfr = CBWord_(cb,Size);
		sc->Size -= tfr;
		sc->Buf = MInc_(sc->Buf,tfr);

		if( sc->Size <= 0 )
		{
			/* Signal "done" to SMLTx at end of data	*/
			sc->Reason = SML_Done;
			IntrResume(sc->Waiter);
			sc->Waiter = P_NullState;
		}
		else
		{
			/* Otherwise start a new transfer here		*/
			tfr = sc->Size;

			if( tfr > sc->BufferMax ) tfr = sc->BufferMax;

			SetCBData_(cb,sc->Buf,tfr);
			SetCBWord_(cb,Size,tfr);

			SetCBWord_(cb,DataReady,1);
			SendInterrupt(sc->IntInfo);
		}
#else
		sc->Reason = SML_Wakeup;
		IntrResume(sc->Waiter);
		sc->Waiter = P_NullState;
#endif
	}

	/* Receive side...						*/
	
	sc = (SMLChannel *)(link->RxChan);
	cb = sc->Cb;
	
	if( !NullStateP(sc->Waiter) && CBWord_(cb,DataReady) != 0 )
	{
		handled = TRUE;

#if RX_IN_INTERRUPT
		/* perform transfer					*/
		tfr = CBWord_(cb,Size);

		CBData_(sc->Buf,cb,tfr);

		sc->Size -= tfr;
		sc->Buf = MInc_(sc->Buf,tfr);

		SetCBWord_(cb,DataReady,0);

		SetCBWord_(cb,Ack,1);
		SendInterrupt(sc->IntInfo);		

		if( sc->Size <= 0 )
		{
			/* If all data come, wake up SMLRx		*/
			sc->Reason = SML_Done;
			IntrResume(sc->Waiter);
			sc->Waiter = P_NullState;			
		}
#else
		sc->Reason = SML_Wakeup;
		IntrResume(sc->Waiter);
		sc->Waiter = P_NullState;
#endif
	}

	return handled;

	vector=vector;
}

#endif
/*}}}*/


