/******************************************************************************
*******************************************************************************
	A George Kalwitz Production, 1989
*******************************************************************************
******************************************************************************/

/******************************************************************************

this module contains tools for implementing send, receive, and echo
	only for the dumb cards

******************************************************************************/

#include	"ieee8023.h"
#include	"common.h"
#include	"cvars.h"
#include	"defs8390.h"
#include	"defs583.h"
#include	"board_id.h"
#include	"intrupts.h"
#include	"params.h"
#include	"sys_info.h"
#include	"eth_data.h"

#if	defined(DIAG)

#include	"diagdefs.h"
#include	"diagdata.h"
#include	"diagmsg.mlh"
#include	"diaghlp.hlh"

#endif

extern   int  far *get_tail();

/******************************************************************************

this returns the value of the CURR register

******************************************************************************/
unsigned int get_curr()
{
    int  tmp_cr;
    int  ret_val;

    tmp_cr = G_inp (baseio+CR) & ~TXP;		/* get current CR value */
    G_outp (baseio+CR, ((tmp_cr & 0x3F) | PS1));/* select page 1 registers */
    ret_val = G_inp (baseio+CURR);		/* read CURR value */
    G_outp (baseio+CR, tmp_cr);			/* return to original value */
    return (ret_val & 0xFF);			/* and show CURR to caller */
}

/******************************************************************************

this updates the receive buffer ring of the NIC
	it updates the BNRY register so the chip may overwrite the packet
	we just read out of the NIC's receive ring

******************************************************************************/
update_rcv_ring()
{
    int far *memptr;	/* NIC tells where the next avail 256 byte page is */

    memptr = (int far *) (RAMbase +
	(((unsigned long)local_nxtpkt_ptr << 8) & 0xFFFF));
    local_nxtpkt_ptr = *memptr; 
    local_nxtpkt_ptr >>= 8;
    local_nxtpkt_ptr &= 0xFF;

    if (local_nxtpkt_ptr == pstart_hold)	  /* and update boundary */
        G_outp(baseio+BNRY, pstop_hold-1);
    else
        G_outp(baseio+BNRY, local_nxtpkt_ptr-1);
}

/******************************************************************************

this is the interrupt service routine for sending and receiving

******************************************************************************/
lan_service_rtn()
{
    G_outp (baseio+IMR, 0);			/* stop board interrupts */
    if ((irq > 7) ||
             ((irq == 2) && (feature_info_byte_1 & SECOND_8259_PRESENT)))
        G_outp (IR8259B, EOI); 			/* issue two non_spec EOI's */
    G_outp (IR8259A, EOI);			/* issue non_specific EOI */
    int_on ();
/*
    check_16bit_access ();
*/
if (frm_rcvd == 117)
    int_on ();
    while (isr_hold = G_inp (baseio+ISR))	/* while something in board */
    {
        G_outp (baseio+ISR, isr_hold);		/* clear ones that were set */
        rsr_hold = G_inp (baseio+RSR);
        tsr_hold = G_inp (baseio+TSR);
        if (isr_hold & OVW)			/* overwrite warning? */
            handle_overwrite();			/* take care of overwrite */
        if (isr_hold & PTX)			/* xmit with no errors? */
        {
            xmt_pending = 0;			/* show xmt not pending */
            frm_sent++;				/* and show pkt was sent */
            if (multicast)
                mc_sent++;
            if (broadcast)
                bc_sent++;
            if (tsr_hold & XMT_ERR_MASK)
                check_xmt_errors ();
#if	defined(ENGR)
            else
                collision_table[0]++;
#endif
        }
        if (isr_hold & TXE)			/* any xmt errors? */
        {
            xmt_pending = 0;			/* show xmt not pending */
            check_fatal_xmt_errors ();		/* update error statistics */
        }
#if	defined(ENGR)
        if (isr_hold & (PRX|RXE))		/* rcv with errors? */
#else
        if (isr_hold & PRX) 			/* rcv with no errors? */
#endif
        {
#if	defined(ENGR)
            if (!block_rcv_mode)
            {
#endif
                while (local_nxtpkt_ptr != get_curr ())
		{
                    suck_up_packet ();		/* copy pkt to local table */
#if	defined(TEST690)
		    if (deadly_event)
			    break;
#endif
		}
#if	defined(ENGR)
            }
#endif
        }
        if (isr_hold & RXE)
            check_rcv_errors (rsr_hold);	/* update error statistics */
        get_counter_stats ();		/* always read counter stats */
    }
/*
    recheck_16bit_access ();
*/
    int_off ();
    G_outp (baseio+IMR, imr_hold);		/* re-enable board interrupts */
}

/******************************************************************************

******************************************************************************/
handle_overwrite()
{
	if (board_id & NIC_690_BIT)
		handle_690_overwrite ();
	else
		handle_8390_overwrite ();
}

/******************************************************************************

this handles an overwrite condition
	1) resets the NIC
	2) waits for reset to complete
	3) puts the NIC into loopback mode (still operates but can't receive)
	4) empties the receive ring
	5) and hopefully reconnects to the LAN

******************************************************************************/
handle_8390_overwrite ()
{
    unsigned int count;

    G_outp (baseio+CR, STP|ABR|PS0);	/* soft reset and page 0 */
    G_outp (baseio+RBCR0, 0);		/* clear remote byte count */
    G_outp (baseio+RBCR1, 0);
    count = 0;				/* wait for reset to complete */
    while ((!(G_inp (baseio+ISR) & RST)) && (count < 0x1000))
        count++;
    G_outp (baseio+TCR, LB2);		/* keep in loopback mode */
    G_outp (baseio+CR, STA|ABR|PS0);	/* restart it */
    while (local_nxtpkt_ptr != get_curr()) /* while buff ring not empty */
    {
        suck_up_packet ();		/* get rcvd packets */
#if	defined(TEST690)
	if (deadly_event)
		break;
#endif
    }
    missed += G_inp (baseio+CNTR2);	/* see how many were missed */
    G_outp (baseio+BNRY, G_inp (baseio+BNRY));	/* write BNRY with itself */
    G_outp (baseio+RCR, rcr_hold);	/* take 690 out of monitor mode */
    G_outp (baseio+TCR, 0);		/* out of lpbk...buffer packets */
}

/******************************************************************************

******************************************************************************/
handle_690_overwrite ()
{
    while (local_nxtpkt_ptr != get_curr()) /* while buff ring not empty */
    {
        suck_up_packet ();		/* get rcvd packets */
#if	defined(TEST690)
	if (deadly_event)
		break;
#endif
    }
    missed += G_inp (baseio+CNTR2);	/* see how many were missed */
    G_outp (baseio+BNRY, G_inp (baseio+BNRY));	/* write BNRY with itself */
}

/******************************************************************************

this gets all of the NIC counter statistics from their appropriate registers

******************************************************************************/
get_counter_stats()
{
    align_errs += G_inp (baseio+CNTR0);
    crc_errs += G_inp (baseio+CNTR1);
    missed += G_inp (baseio+CNTR2);
}

/******************************************************************************

this is called by the interrupt service routine when a packet has been
	received

data received by the NIC will be copied into the local receive table in
	this routine

if hardware has been designed with the DMA clock to Network clock ratio
	greater that 4:1...NIC bugs may occur
the NIC creates a four byte header at the front of every received packet
	which contains:	1) receive status
			2) index of next available 256 byte page
			3) lower byte count of received packet
			4) upper byte count of received packet
the bug (with the 4:1 clock ratio) makes the chip copy the lower byte count
	into both the upper byte count position and the lower byte count
	position.
this means you cannot read the size of the packet directly from the receive
	ring data
as a software fix, the next packet pointer (item 2 above) may be used to 
	calculate the upper byte count and
	then the lower byte count may be added to it

more code is used here to make sure we wrap around to the beginning of the
	ring when reading received data

******************************************************************************/
suck_up_packet ()
{
    int   nic_overcount;	/* nic says 1 or 2 more than we want */
    int   pkt_size;		/* calculated size of the received data */
    int   wrap_size;		/* size of data before wrapping is needed */
    int   header_nxtpkt_ptr;	/* NIC's next pkt ptr in rcv header */
    int   low_byte_count;	/* low byte count of pkt read from rcv header */
    int   high_byte_count;	/* calculated high byte count */
    int   far  *dptr,		/* pointer to local receive table */
               *sptr;		/* pointer into shared RAM */

    sptr = (int far *) (RAMbase +
	(((unsigned long)local_nxtpkt_ptr << 8) & 0xFFFF));
    if (any_deadly_errors (sptr))
        return;
    if (is_it_our_pkt_type (sptr))
    {
        id_rcvd_addr (sptr);
        header_nxtpkt_ptr = *sptr;
        header_nxtpkt_ptr >>= 8;
        header_nxtpkt_ptr &= 0xFF;
        low_byte_count = *(sptr + 1);
        low_byte_count &= 0xFF;
        if ((low_byte_count + NIC_HEADER_SIZE) > NIC_PAGE_SIZE)
            nic_overcount = 2;
        else
            nic_overcount = 1;
        if (header_nxtpkt_ptr > local_nxtpkt_ptr)
        {
            wrap_size = 0;
            high_byte_count = header_nxtpkt_ptr - local_nxtpkt_ptr - 
                                                        nic_overcount;
        }
        else
        {
            wrap_size = (int)((pstop_hold - local_nxtpkt_ptr) << 8);
            high_byte_count = pstop_hold - local_nxtpkt_ptr +
                               header_nxtpkt_ptr - pstart_hold - nic_overcount;
        }
        pkt_size = (high_byte_count << 8) | (low_byte_count & 0xFF);
        *(sptr + 1) = pkt_size;		/* write size back into ring */
        if (wrap_size > pkt_size)	/* if pkt fits without wrap */
            wrap_size = 0;
        dptr = get_tail ();		/* get local rcv table space */
        if (dptr == NULL)		/* no space available */
        {
            missed++;			/* show it in a statistic */
            update_rcv_ring ();		/* and let the NIC go on receiving */
            return;
        }
        if (wrap_size)	/* packet wrapped, copy pre-wrap data */
        {
            pkt_to_loc_mem (sptr, dptr, wrap_size);
            sptr = (int far *) (RAMbase + (long)(pstart_hold << 8));
            dptr += wrap_size/2;	/* prepare for further copying */
            pkt_size -= wrap_size;
        }
        if (pkt_size)		/* if anything left to move... */
            pkt_to_loc_mem (sptr, dptr, pkt_size);
    }
    update_rcv_ring();			/* and make the NIC happy */
}

/******************************************************************************

this identifies transmit errors which do not abort the transmit

******************************************************************************/
check_xmt_errors()
{
    if (tsr_hold & COL)				/* collisions? */
    {
        collisions += G_inp (baseio+NCR);
       #if	defined(ENGR)
        collision_table[G_inp (baseio+NCR)]++;
       #endif
    }
#if	defined(ENGR)
    else
        collision_table[0]++;
#endif
/*  				dont worry about this error...something is
    if (tsr_hold & CRS)			funny in the 8390
        lost_crs++;
*/
    if (tsr_hold & CDH)				/* cd heartbeat? */
        cd_hearts++;
    if (tsr_hold & OWC)				/* out of window collision? */
        owc_errs++;
}

/******************************************************************************

this identifies fatal transmit errors which abort the transmit

******************************************************************************/
check_fatal_xmt_errors()
{
    if (tsr_hold & ABT)				/* xmit aborted? */
    {
        aborts++;
	collisions += 16;
       #if	defined(ENGR)
	collision_table[16]++;
       #endif
    }
    if (tsr_hold & FU)				/* fifo underrun? */
        underruns++;				/* update statistics */
}

/*****************************************************************************

this identifies receive errors

******************************************************************************/
check_rcv_errors (status)
int status;
{
    if (status & RCV_ERR_MASK)
    {
        if (status & CRC)			/* crc error? */
            crc_errs += G_inp (baseio+CNTR1);
        if (status & FAE)			/* frame alignment error? */
            align_errs += G_inp (baseio+CNTR0);
        if (status & FO)			/* fifo overrun? */
            overruns++;				/* update stats */
        if (status & MPA)			/* did we miss packets? */
            missed += G_inp (baseio+CNTR2);
        if (status & DIS)			/* is rcvr disabled? */
            rcvr_disabled++;
        if (status & DFR)
            deferring++;
    }
}

/******************************************************************************

this decides if the received packet was multicast or broadcast

******************************************************************************/
id_rcvd_addr (packet_ptr)
int  far  *packet_ptr;
{
    frm_rcvd++;
    if (*packet_ptr & PHY)
    {
        if ((*(packet_ptr + 2) & 0xFF) == 0xFF)
            bc_rcvd++;
        else
            mc_rcvd++;
    }
    check_rcv_errors (*packet_ptr);
}

/******************************************************************************

this will do either a memory to memory string move or a memory to I/O string
	move

******************************************************************************/
pkt_to_loc_mem (src, dst, size)
int far *src;
int far *dst;
int size;
{
	if (board_id & SLOT_16BIT)
	    mv8013 (src, dst, size);
	else
	    movstr (src, dst, size);
	return;
}

#if	defined(ENGR)
/******************************************************************************

this is the interrupt service routine for sending and receiving

******************************************************************************/
quick_service_rtn ()
{
    G_outp (baseio+IMR, 0);			/* stop board interrupts */
    if ((irq > 7) ||
             ((irq == 2) && (feature_info_byte_1 & SECOND_8259_PRESENT)))
        G_outp (IR8259B, EOI); 			/* issue two non_spec EOI's */
    G_outp (IR8259A, EOI);			/* issue non_specific EOI */
    while (isr_hold = G_inp (baseio+ISR))	/* while something in board */
    {
        G_outp (baseio+ISR, isr_hold);		/* clear ones that were set */
        rsr_hold = G_inp (baseio+RSR);
        tsr_hold = G_inp (baseio+TSR);
        if (isr_hold & OVW)			/* overwrite warning? */
            quick_handle_overwrite();		/* take care of overwrite */
        if (isr_hold & PTX)		/* xmit with no errors? */
        {
            xmt_pending = 0;			/* show xmt not pending */
            frm_sent++;				/* and show pkt was sent */
            if (tsr_hold & XMT_ERR_MASK)
                check_xmt_errors ();
            else
                collision_table[0]++;
        }
        if (isr_hold & TXE)			/* any xmt errors? */
	{
            xmt_pending = 0;			/* show xmt not pending */
            check_fatal_xmt_errors ();		/* update error statistics */
	}
        if (isr_hold & (PRX|RXE))		/* rcv with errors? */
        {
            while (local_nxtpkt_ptr != get_curr ())
            {
                quick_suck_up_packet ();
#if	defined(TEST690)
                if (deadly_event)
                    break;
#endif
            }
        }
        get_counter_stats ();			/* always read counter stats */
    }
    G_outp (baseio+IMR, imr_hold);		/* re-enable board interrupts */
}

/******************************************************************************

******************************************************************************/
quick_handle_overwrite()
{
	if (board_id & NIC_690_BIT)
		handle_690_overwrite ();
	else
		handle_8390_overwrite ();
}

/******************************************************************************

******************************************************************************/
quick_handle_8390_overwrite ()
{
    int count;

    G_outp (baseio+CR, STP|ABR|PS0);	/* soft reset and page 0 */
    G_outp (baseio+RBCR0, 0);		/* clear remote byte count */
    G_outp (baseio+RBCR1, 0);
    count = 0;				/* wait for reset to complete */
    while ((!(G_inp (baseio+ISR) & RST)) && (count < 0x1000))
        count++;
    G_outp (baseio+TCR, LB2);		/* keep in loopback mode */
    G_outp (baseio+CR, STA|ABR|PS0);	/* restart it */
    while (local_nxtpkt_ptr != get_curr()) /* while buff ring not empty */
    {
        quick_suck_up_packet ();
#if	defined(TEST690)
        if (deadly_event)
            break;
#endif
    }
    missed += G_inp (baseio+CNTR2);	/* see how many were missed */
    G_outp (baseio+BNRY, G_inp (baseio+BNRY));	/* write BNRY with itself */
    G_outp (baseio+RCR, rcr_hold);	/* take 690 out of monitor mode */
    G_outp (baseio+TCR, 0);		/* out of lpbk...buffer packets */
}

/******************************************************************************

******************************************************************************/
quick_handle_690_overwrite ()
{
    while (local_nxtpkt_ptr != get_curr()) /* while buff ring not empty */
    {
        quick_suck_up_packet ();
#if	defined(TEST690)
        if (deadly_event)
            break;
#endif
    }
    missed += G_inp (baseio+CNTR2);	/* see how many were missed */
    G_outp (baseio+BNRY, G_inp (baseio+BNRY));	/* write BNRY with itself */
}

/******************************************************************************

******************************************************************************/
quick_suck_up_packet ()
{
    int   far  *sptr;

    sptr = (int far *) (RAMbase +
	(((unsigned long)local_nxtpkt_ptr << 8) & 0xFFFF));
    if (any_deadly_errors (sptr))
        return;
    id_rcvd_addr (sptr);
    update_rcv_ring();			/* and make the NIC happy */
}

#endif

#if	defined(SPECIAL)
show_registers (col)
int	col;
{
	int	temp;
	int	row;
	char	string[20];

	row = 3;

	temp = G_inp (baseio+CR);
	sprintf (string, "CR %02X", temp);
	DisplayTextInPortal (row, col, string, VINTENSE);
	row++;
/*
	sprintf (string, "ISR %02X", isr_hold);
	DisplayTextInPortal (row, col, string, VINTENSE);
	row++;
	sprintf (string, "RSR %02X", rsr_hold);
	DisplayTextInPortal (row, col, string, VINTENSE);
	row++;
	sprintf (string, "TSR %02X", tsr_hold);
	DisplayTextInPortal (row, col, string, VINTENSE);
	row++;
*/
	sprintf (string, "CLDA0 %02X", G_inp (baseio+CLDA0));
	DisplayTextInPortal (row, col, string, VINTENSE);
	row++;
	sprintf (string, "CLDA1 %02X", G_inp (baseio+CLDA1));
	DisplayTextInPortal (row, col, string, VINTENSE);
	row++;
	G_outp (baseio+CR, ((temp & 0x3F) | PS1));
	sprintf (string, "CURR %02X", G_inp (baseio+CURR));
	DisplayTextInPortal (row, col, string, VINTENSE);
	row++;
	G_outp (baseio+CR, ((temp & 0x3F) | 0xC0));
	G_outp (baseio+0x11, (G_inp (baseio+0x11) | 0x01));
	G_outp (baseio+CR, ((temp & 0x3F) | PS1));
	sprintf (string, "RCNTL %02X", G_inp (baseio+0x1B));
	DisplayTextInPortal (row, col, string, VINTENSE);
	row++;
	sprintf (string, "RCNTH %02X", G_inp (baseio+0x1C));
	DisplayTextInPortal (row, col, string, VINTENSE);
	row++;
	G_outp (baseio+CR, ((temp & 0x3F) | 0xC0));
	G_outp (baseio+0x11, (G_inp (baseio+0x11) & 0xFE));
	G_outp (baseio+CR, temp);
	row++;
	sprintf (string, "BBC %04d", bad_byte_count);
	DisplayTextInPortal (row, col, string, VINTENSE);
/*
	G_outp (baseio+CR, temp);
	sprintf (string, "Ints %04d", num_of_ints);
	DisplayTextInPortal (row, col, string, VINTENSE);
*/
}

#endif

