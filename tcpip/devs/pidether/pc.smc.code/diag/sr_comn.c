/******************************************************************************
*******************************************************************************
	A George Kalwitz Production, 1989
*******************************************************************************
******************************************************************************/

/******************************************************************************

this module contains tools for implementing send, receive, and echo

******************************************************************************/

#include	"stdio.h"

#include	"time.h"
#include	"ieee8023.h"
#include	"ieee8022.h"
#include	"common.h"
#include	"cvars.h"
#include	"defs8390.h"
#include	"board_id.h"
#include	"timing.h"
#include	"params.h"
#include	"err_cods.h"
#include	"eth_data.h"

#if	defined(DIAG)

#include	"cwuser.h"
#include	"diagdata.h"
#include	"diagdefs.h"
#include	"diagmsg.mlh"
#include	"diaghlp.hlh"

#endif

int with_pattern (int, unsigned long);

/**************************************************************************

this will find and connect with the first node to respond
	first a packet is sent to the multicast address requesting a 
	responder to reply.
	the first valid response packet is used to extract the address of
	the responder and use that address for later transmissions

***************************************************************************/
int qck_find_responder ()
{
    int  i;
    int  ret_code;

    ret_code = 0;
    if (responder_found)		/* dont bother if one has been found */
    {
        show_responder_addr ();		/* just show the responder */
        return (ret_code);		/* and exit with no error */
    }
    if (ret_code = spark_responders ())	/* otherwise send the multicast pkt */
        ;
    else
    {
        while (gota_packet () == PACKET_READY)	/* look at all rcvd pckts */
        {
            if (is_responder_pkt (my_head))	/* verify valid responder pkt */
            {
                responder_found = 1;		/* show responder found */
                for (i = 0; i < 6; i++)		/* copy responders addr */
                    resp_addr[i] = my_head->saddr[i];
                show_responder_addr ();		/* show user responder addr */
                update_head ();			/* and get rid of the packet */
                break;
            }
            else
                update_head ();			/* else go to next packet */
        }
        if (!(responder_found))			/* all done with rcvd packets */
        {
            #if	defined(DIAG)
            if (!(batch_type & (B_SEND | B_RESP | B_TEST)))
                Alert (NO_RESPONDER, 24, 0);
            #else
            my_alert ("No responder found.");
            #endif
            ret_code = NO_RESPONDER_ERR;
        }
    }
    while (gota_packet () == PACKET_READY)	/* get rid of unused pkts */
        update_head ();
    return (ret_code);
}

/**************************************************************************

this will send a 'find responder' packet to the multicast address and wait
	for packets to be received

***************************************************************************/
int spark_responders ()
{
    int         ret_code;
    int         spark_responder_portal;

    llc_control_byte = (XID_CMND & 0xFF);
    #if	defined(DIAG)
    if (!(batch_type & (B_SEND | B_RESP | B_TEST)))
        spark_responder_portal = DisplayInformation (NO_MESSAGE, FALSE, 15, 0,
                                 INFO_PALETTE, PLEASE_WAIT);
    #endif
    build_packet (TRUE, mult_addr);		/* build the packet to send */
    ret_code = look_for_packet ();	/* wait for response */
    #if	defined(DIAG)
    if (!(batch_type & (B_SEND | B_RESP | B_TEST)))
        DestroyPortal (spark_responder_portal);
    #endif
    if (ret_code == KB_HIT_ERR)
        return (KB_HIT_ERR);
    if (ret_code == MAX_RETRY_ERR)
    {
        #if	defined(DIAG)
        if (!(batch_type & (B_SEND | B_RESP | B_TEST)))
            Alert (RCV_TIMED_OUT, 24, 0, max_retry);
        #else
        my_alert ("No nodes are responding.");
        #endif
        return (NO_FRM_RCVD_ERR);
    }
    return (ret_code);
}

/**************************************************************************

this transmits the packet and waits for a responder to reply and 
	retries the transmission if the receive times out

***************************************************************************/
int look_for_packet ()
{
    int		ret_code;
    int		num_of_retries;
    long	prev_frm_sent;
    time_t	start_time;
    time_t	time (time_t *);

    prev_frm_sent = frm_sent;
    for (num_of_retries = 0; num_of_retries <= max_retry; num_of_retries++)
    {
        if (num_of_retries)	/* dont bump first time through */
            frm_rtry++;
        if (ret_code = transmit_packet (local_tx_buffer, size_hold))
            return (ret_code);
        start_time = time (NULL);
        while (time (NULL) < (start_time + RESPONDER_DELAY))
        {
            if (was_keyboard_hit ())
                return (KB_HIT_ERR);
        }
        start_time = time (NULL);
        while (time (NULL) < (start_time + rcv_timeout))
        {
            if (was_keyboard_hit ())
                return (KB_HIT_ERR);
            ret_code = get_packet_status ();
            if (ret_code) 			/* packet has arrived */
                return (0);			/* show packet arrived */
        }
    }
    if (frm_sent == prev_frm_sent)
    {
        tell_no_frm_sent ();
        return (NO_FRM_SENT_ERR);
    }
    return (MAX_RETRY_ERR);			/* show no packet arrived */
}

/**************************************************************************

checks to see if a packet has arrived or not

***************************************************************************/
int get_packet_status ()
{
    if (buffers_used)
        return (1);
    else
        return (0);
}

/**************************************************************************

this verifies that the received packet is actually a responder
	by checking the link layer protocol

***************************************************************************/
int is_responder_pkt (packet_ptr)
struct   RBUF   far  *packet_ptr;
{
    int    temp;

    temp = packet_ptr->control & 0xFF;
    temp |= PF_BIT;			/* be compatible with old DIAGNOSE */
    if (temp != ((llc_control_byte | PF_BIT) & 0xFF))
        return (FALSE);					/* show bad pkt */
    if ((packet_ptr->ssap & 0xFF) != ((llc_dst_sap | RESP_BIT) & 0xFF))
        return (FALSE);					/* show bad pkt */
    return (TRUE);					/* show good pkt */
}

/**************************************************************************

this sends a packet with no user intervention

***************************************************************************/
int send_messages ()
{
    int  i;
    int  ret_code;
    int  done_portal;
    int  initiating_portal;

    #if	defined(DIAG)
     #if defined(ENGR)
     #else
    if (ret_code = verify_baseio ())
        return (ret_code);
     #endif
    #endif
    if (ret_code = init_snd_rcv ())
        return (ret_code);
    if (ret_code = qck_find_responder ()) /* use the first responder */
        return (ret_code);
    llc_control_byte = (TST_CMND & 0xFF);
    build_packet (FALSE, resp_addr);	/* build the packet to xmt */
    #if	defined(DIAG)
    if (!(batch_type & (B_SEND | B_RESP | B_TEST)))
        initiating_portal = DisplayInformation (NO_MESSAGE, FALSE, 15, 0,
                    INFO_PALETTE, INITIATING);
    #endif
    ret_code = batch_state1 ();
    #if	defined(DIAG)
    if (!(batch_type & (B_SEND | B_RESP | B_TEST)))
        DestroyPortal (initiating_portal);
    #endif
    if ((ret_code == NO_ERROR) || (ret_code == KB_HIT_ERR))
    {
        #if	defined(DIAG)
        if (!(batch_type & (B_SEND | B_RESP | B_TEST)))
        {
            done_portal = DisplayInformation (NO_MESSAGE, FALSE, 15, 0,
                              INFO_PALETTE, DONE_INITIATING);
            show_info_perm ();
            DestroyPortal (done_portal);
        }
        #endif
    }
    else if (ret_code == MAX_RETRY_ERR)
    {
        #if	defined(DIAG)
         if (!(batch_type & (B_SEND | B_RESP | B_TEST)))
            Alert (RCV_TIMED_OUT, 24, 0, max_retry);
        #else
         my_alert ("Receive timed out.");
        #endif
    }
    undo_snd_rcv ();
    return (ret_code);
}

/**************************************************************************


***************************************************************************/
tell_no_frm_sent ()
{
    #if	defined(DIAG)
    if (!(batch_type & (B_SEND | B_RESP | B_TEST)))
        Alert (NO_FRM_SENT, 24, 0);
    #else
    my_alert ("No frame was sent.");
    #endif
}

/**************************************************************************

this receives a packet with no user intervention

***************************************************************************/
int echo_messages ()
{
    int     ret_code;
    int     responding_portal;

    ret_code = 0;
    #if	defined(DIAG)
     #if defined(ENGR)
     #else
    if (ret_code = verify_baseio ())
        return (ret_code);
     #endif
    #endif
    if (ret_code = init_snd_rcv ())
        return (ret_code);
    #if	defined(DIAG)
    if (!(batch_type & (B_SEND | B_RESP | B_TEST)))
        responding_portal = DisplayInformation (NO_MESSAGE, FALSE, 15, 0,
                            INFO_PALETTE, RESPONDING);
    #endif
    ret_code = forever_echo ();
    #if	defined(DIAG)
    if (!(batch_type & (B_SEND | B_RESP | B_TEST)))
        DestroyPortal (responding_portal);
    #endif
    undo_snd_rcv ();
    return (ret_code);
}

/**************************************************************************


***************************************************************************/
int forever_echo ()
{
    int  ret_code;
    char local_count;

    local_count = 0;
    if (auto_clr_index)
        clear_stats ();
    draw_short_stats ();			/* force update stats */
    enable_multicast ();		/* allow multicast receive */
    while (TRUE)				/* till keyboard is hit */
    {
        if (was_keyboard_hit ())
        {
            ret_code = KB_HIT_ERR;
            break;
        }
#if	defined(TEST690)
	if (deadly_event)
	{
            if ((!(batch_type & (B_SEND | B_RESP | B_TEST))) && (!(byp_index)))
	    {
		Alert (DEADLY_RCV_ERR_MSG, 24, 0);
		ret_code = DEADLY_RCV_ERR;
		deadly_event = 0;
		break;
	    }
	}
#endif
#if	defined(ENGR)
      if (!quick_mode)
      {
#endif
        ret_code = gota_packet ();
        if (ret_code == PACKET_READY)	/* if pkt has been received */
        {
            if (ret_code = process_packet ())	/*   then echo it */
                break;
            draw_short_stats ();		/* update stats */
        }
        local_count++;
        if (!local_count)
            draw_short_stats ();		/* force every now and then */
#if	defined(ENGR)
      }
#endif
    }						/* end while */
    draw_short_stats ();			/* force update stats */
    disable_multicast ();			/* disable multicast receive */
    return (ret_code);
}

/***************************************************************************

this is the batch version of send/receive

****************************************************************************/
int batch_state1 ()
{
    int            ret_code;
    unsigned long  count;
    unsigned int   delay;

    if (auto_clr_index)
        clear_stats ();
    draw_short_stats ();			/* force update stats */
#if	defined(TEST690)
#else
    disable_multicast ();
#endif
    count = iterations;				/* init loop counter */
    while ((iterations == 0) || (count > 0))
    {
        if (was_keyboard_hit ())
        {
            ret_code = KB_HIT_ERR;
            break;				/* and return control */
        }
#if	defined(TEST690)
	if (deadly_event)
	{
            if ((!(batch_type & (B_SEND | B_RESP | B_TEST))) && (!(byp_index)))
	    {
		Alert (DEADLY_RCV_ERR_MSG, 24, 0);
		ret_code = DEADLY_RCV_ERR;
		deadly_event = 0;
		break;
	    }
	}
#endif
	if (ret_code = batch_snd_rcv ())	/* send and wait for rcv */
                break;
	#if	defined(ENGR)
        if (!(quick_mode))
        #endif
            draw_short_stats ();		/* update stats */
        count--;				/* update loop counter */
        #if	defined(ENGR)
          if (tx_throttle)
          {
              for (delay = 0; delay < tx_throttle; delay++)
                  nothing ();
          }
        #endif
    }						/* end while */
    draw_short_stats ();			/* force update stats */
    return (ret_code);
}

/******************************************************************************

******************************************************************************/
nothing ()
{
    return;
}

/******************************************************************************


******************************************************************************/
int batch_snd_rcv ()
{
    int  count;
    int  ret_code;
    long prev_frm_sent;

    prev_frm_sent = frm_sent;
    for (count = 0; count <= max_retry; count++)
    {
        if (count)	/* dont bump first time through */
            frm_rtry++;
        if (ret_code = transmit_packet (local_tx_buffer, size_hold))
                return (ret_code);
    
      #if	defined(ENGR)
        if (send_only_flag)
        {
            if (gota_packet () == PACKET_READY)
                update_head ();
            return (NO_ERROR);
        }
        else
        {
      #endif
            ret_code = wait_for_rcv ();		/* and wait for rcv */
            if (ret_code != RCV_TIME_OUT_ERR)
                return (ret_code);
      #if	defined(ENGR)
        }
      #endif
    }
    if (frm_sent == prev_frm_sent)
    {
        tell_no_frm_sent ();
        return (NO_FRM_SENT_ERR);
    }
    return (MAX_RETRY_ERR);
}
