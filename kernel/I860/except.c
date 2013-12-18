/* to get around hncc -zr option problems */

/*--------------------------------------------------------
-- Exception						--
--							--
-- Send an exception message to a given port.		--
-- This is invoked both as a straight procedure and as	--
-- a kernel worker process.				--
-- SendException is the external interface.		--
--							--
--------------------------------------------------------*/

static void Exception(Code code, Port port)
{
	MCB mcb;
	word e;

	if( port == NullPort ) return;
	
	*(word *)&mcb = 0;		/* zero all of first word */
	
	mcb.MsgHdr.Flags = MsgHdr_Flags_exception;
	mcb.MsgHdr.Dest = port;
	mcb.MsgHdr.Reply = NullPort;
	mcb.MsgHdr.FnRc = code;
	mcb.Timeout = -1;		/* wait for ever !! */
	
	e = _PutMsg(&mcb);
}

