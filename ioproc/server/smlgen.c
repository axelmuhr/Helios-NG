
/*{{{  InitSMLChan  */

#ifdef __STDC__
static word InitSMLChan
(
	SMLChannel	*sc,
	CBPtr		cb,
	word		cbsize
)
#else
static word InitSMLChan(sc,cb,cbsize)
SMLChannel	*sc;
CBPtr		cb;
word		cbsize;
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
	SetCBWord_(cb,Size,0xdeadbeef);
	SetCBWord_(cb,Protocol,0);

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
static word SMLTx(LinkTransferInfo *info)
#else
static word SMLTx(info)
LinkTransferInfo *info;
#endif
{
	SMLChannel *sc = (SMLChannel *)info->link->TxChan;
	CBPtr cb = sc->Cb;

/*	ServerDebug ("SMLTx (LinkTransferInfo:[size: 0x%lx; buf; link:[%d, ...]])",
			info -> size, info -> link -> Unit); */

#if USE_ABORT	
	if( info->size == 0 )		/* abort transmission		*/
	{
		SaveState *s = sc->Waiter;
		if( NullStateP(s) ) return (word)P_NullState;

		/* signal abort to transmitter				*/
		sc->Reason 	= SML_Aborted;

		/* reset state to initial values			*/
		sc->Waiter 	= P_NullState;
		sc->Size	= 0;
		SetCBWord_(cb,DataReady,0);
		SetCBWord_(cb,Ack,0);

		return (word)s;
	}
#endif

	/* save size and buffer for interrupt routine			*/
	sc->Size = info->size*MPSize;
	sc->Buf = info->buf;

	while( sc->Size > 0 )
	{
		word tfr = sc->Size;

		if( tfr > sc->BufferMax )
		{
			tfr = sc->BufferMax;
		}

		SetCBData_(cb,sc->Buf,tfr);
		SetCBWord_(cb,Size,tfr);

		SetCBWord_(cb,DataReady,1);
		SendInterrupt(sc->IntInfo);		

		Await(sc,CBAck_(cb));

		if( sc->Reason != SML_Wakeup ) break;

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
static word SMLRx(LinkTransferInfo *info)
#else
static word SMLRx(info)
LinkTransferInfo *info;
#endif
{
	SMLChannel *sc = (SMLChannel *)info->link->RxChan;
	CBPtr cb = sc->Cb;

/*	ServerDebug ("SMLRx () - cb (0x%lx):[dr: %d, ack: %d, size: %d, ...]",
			(long)cb, CBWord_(cb, DataReady), CBWord_(cb, Ack), CBWord_(cb, Size)); */

/*	ServerDebug ("SMLRx () - info -> size = %d", info -> size); */

#if USE_ABORT		
	if( info->size == 0 )		/* abort transfer		*/
	{
		SaveState *s = sc->Waiter;
		if( NullStateP(s) ) return (word)P_NullState;

		/* signal abort to reciever				*/
		sc->Reason 	= SML_Aborted;

		/* reset state to initial values			*/
		sc->Waiter 	= P_NullState;
		sc->Size	= 0;
		SetCBWord_(cb,DataReady,0);
		SetCBWord_(cb,Ack,0);

		return (word)s;
	}
#endif

	/* save size and buffer for interrupt routine			*/
	sc->Size = info->size*MPSize;
	sc->Buf = info->buf;

	while( sc->Size > 0 )
	{
		word tfr;
		word pos;

/*
		ServerDebug ("SMLRx () - sc -> Size = %d", sc -> Size);
		ServerDebug ("SMLRx () - sc -> Left = %d", sc -> Left);
*/
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
/*			ServerDebug ("SMLRx () - about to call Await () ... "); */
			Await(sc,CBDataReady_(cb));
/*			ServerDebug ("SMLRx () - ... Await () finished, sc -> Reason = %d", sc -> Reason); */

			if( sc->Reason != SML_Wakeup )
			{
/*				ServerDebug ("SMLRx () - reason was SML_Wakeup, breaking"); */
				break;
			}

			tfr = CBWord_(cb,Size);

/* 			ServerDebug ("SMLRx () - tfr = %d", tfr); */

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
/*			ServerDebug ("SMLRx () - sc -> Left == 0"); */

			SetCBWord_(cb,DataReady,0);
			SetCBWord_(cb,Ack,1);
			SendInterrupt(sc->IntInfo);
		}
		
	}

/*	return ((sc -> Size > 0) ? SML_Aborted : SML_Done); */
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
static word SMLTxRdy(LinkInfo *link)
#else
static word SMLTxRdy(link)
LinkInfo *link;
#endif
{
	SMLChannel *sc = (SMLChannel *)link->TxChan;
	CBPtr cb = sc->Cb;

	if (CBWord_(cb, DataReady) != 0 && CBWord_(cb, DataReady) != 1)
	{
		ServerDebug ("Warning: suspected corruption in transmit link!!! (DataReady)");
		printf ("Warning: suspected corruption in transmit link!!! (DataReady)\n");
	}

	if (CBWord_(cb, Ack) != 0 && CBWord_(cb, Ack) != 1)
	{
		ServerDebug ("Warning: suspected corruption in transmit link!!! (Ack)");
		printf ("Warning: suspected corruption in transmit link!!! (Ack)\n");
	}

	if (sc -> Left != 0 || CBWord_(cb, DataReady) != 0)
	{
		ServerDebug ("Tx clash detected"); 
		return FALSE;
	}
	else
	{
		return TRUE;
	}
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
static word SMLRxRdy(LinkInfo *link)
#else
static word SMLRxRdy(link)
LinkInfo *link;
#endif
{
	SMLChannel *sc = (SMLChannel *)link->RxChan;
	CBPtr cb = sc->Cb;

	if (CBWord_(cb, DataReady) != 0 && CBWord_(cb, DataReady) != 1)
	{
		ServerDebug ("Warning: suspected corruption in receive link!!! (DataReady)");
		printf ("Warning: suspected corruption in receive link!!! (DataReady)\n");
	}

	if (CBWord_(cb, Ack) != 0 && CBWord_(cb, Ack) != 1)
	{
		ServerDebug ("Warning: suspected corruption in receive link!!! (Ack)");
		printf ("Warning: suspected corruption in receive link!!! (Ack)\n");
	}

/*
	int	reply = (sc -> Left != 0 || CBWord_(cb,DataReady) != 0);

	if (reply)
	{
		ServerDebug ("SMLRxRdy () - reply = %d", reply);
	}
*/

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
static word SMLInterrupt(LinkInfo *link, word vector)
#else
static word SMLInterrupt(link,vector)
LinkInfo *link;
word vector;
#endif
{
	SMLChannel *sc = (SMLChannel *)(link->TxChan);
	CBPtr cb = sc->Cb;
	word tfr;
	word handled = FALSE;

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


