
*******************************************************************************
	A George Kalwitz Production, 1989
*******************************************************************************
******************************************************************************/

/******************************************************************************

this module contains tools for implementing send, receive, and echo

******************************************************************************/

#include	"ieee8022.h"
#include	"ieee8023.h"
#include	"common.h"
#include	"cvars.h"
#include	"diagdata.h"
#include	"eth_data.h"

#include	"params.h"
#include	"defs8390.h"
#include	"defs583.h"

/******************************************************************************

this returns a pointer to the next available receive table page

******************************************************************************/
int far *get_tail()
{
    if (buffers_used == NUM_OF_BUFFS)
        return (NULL);
    tail_index++;			/* index next avail page */
    if (tail_index == NUM_OF_BUFFS)	/* handle wrap around */
        tail_index = 0;
    my_tail = &table_ptr->rcv_table[tail_index];/* point to next avail page */
    buffers_used++;			/* and update counter */
    return ((int far *) my_tail);
}

/**************************************************************************

this verifies that the received packet is meant for the Diagnostic
	or Sample Driver.

***************************************************************************/
int is_it_our_pkt_type (packet_ptr)
int  far  *packet_ptr;
{
    int       tmp_var;

  #if	defined(ENGR)
    if (promiscuous_mode)
        return (TRUE);
  #endif
    if (((*(packet_ptr + 3)) & 0xFF) != mult_addr[2] & 0xFF)
    {
        if (((*(packet_ptr + 3)) & 0xFF) != node_addr[2] & 0xFF)
            return (FALSE);
    }
    /* get packet size from MAC header */
    tmp_var = (((*(packet_ptr + 8) >> 8) & 0xFF) + (*(packet_ptr + 8) << 8));
#if	defined(ENGR)
    if (large_frames)
    {
        if (tmp_var > PACKET_DATA_SIZE)
            return (FALSE);
    }
    else
    {
        if ((tmp_var > 1500) || (tmp_var < 46))
            return (FALSE);
    }
#else
    if ((tmp_var > 1500) || (tmp_var < 46))
        return (FALSE);
#endif
    tmp_var = *(packet_ptr + 9);	/* get SAPs */
    if ((tmp_var & 0xFF) != (llc_src_sap & 0xFF))
        return (FALSE);
    tmp_var = *(packet_ptr + 10);	/* get LLC control byte */
    if (mode == 0)			/* if initiator */
    {
        tmp_var |= PF_BIT;		/* be compatible with old DIAGNOSE */
        if ((tmp_var & 0xFF) != ((llc_control_byte | PF_BIT) & 0xFF))
            return (FALSE);
    }
    else				/* if responder */
    {
        tmp_var &= ~PF_BIT;		/* be compatible with old DIAGNOSE */
        if (((tmp_var & 0xFF) != TST_CMND) && ((tmp_var & 0xFF) != XID_CMND))
            return (FALSE);
    }
    return (TRUE);
}

/**************************************************************************

***************************************************************************/
int any_deadly_errors (packet_ptr)
int  far  *packet_ptr;
{
	int	ret_code;
	int	tmp_var;

	ret_code = 0;
	tmp_var = *packet_ptr;
	tmp_var >>= 8;
	tmp_var &= 0xFF;
	if ((tmp_var < pstart_hold) || (tmp_var >= pstop_hold))
	{
		bad_nxt_pkt++;
		reinit_rcv_ring ();
		ret_code = 1;
	}
	if (*(packet_ptr + 1) < 64)
	{
		runt_packets++;
		reinit_rcv_ring ();
		ret_code = 1;
	}
	return (ret_code);
}

/**************************************************************************

***************************************************************************/
reinit_rcv_ring ()
{
    G_outp (baseio+CR, STP|PS0|ABR);		/* stop NIC | select page 0 */
    G_outp (baseio+BNRY, pstart_hold);		/* init to = PSTART */
    G_outp (baseio+CR, STP|PS1|ABR);		/* stop NIC | sel page 1 */
    G_outp (baseio+CURR, pstart_hold + 1);
    local_nxtpkt_ptr = pstart_hold + 1;		/* internal next pkt pointer */
    G_outp (baseio+CR, STA|PS0|ABR);		/* start NIC | select page 0 */
}

