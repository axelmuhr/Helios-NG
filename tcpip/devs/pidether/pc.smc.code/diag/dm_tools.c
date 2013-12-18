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
#include 	"intrupts.h"
#include	"defs583.h"
#include	"micro_ch.h"
#include	"timing.h"
#include	"board_id.h"
#include	"params.h"
#include	"err_cods.h"
#include	"sys_info.h"
#include	"diagdata.h"
#include	"eth_data.h"

#if	defined(DIAG)

#include	"cwuser.h"
#include	"diagdefs.h"
#include	"diagmsg.mlh"
#include	"diaghlp.hlh"

#endif

long get_vector (int);

/******************************************************************************

this initiates the transmit by loading the proper byte counts and setting the
    transmit bit

******************************************************************************/
int initiate_xmit (length)
int length;
{
#if	defined(ROMDIAG)
#else
    if ((*node_addr_ptr & 0xFF) == 0x01)	/* flag multicast or not */
        multicast = 1;
    else
        multicast = 0;
    if ((*node_addr_ptr & 0xFF) == 0xFF)	/* flag broadcast or not */
        broadcast = 1;
    else
        broadcast = 0;
#endif
    G_outp (baseio+CR, ABR|STA);		/* select page 0 */
    G_outp (baseio+TPSR, tpsr_hold);		/* xmit pg strt at 0 of RAM */
    G_outp (baseio+TBCR1, length >> 8);		/* upper byte of count */
    G_outp (baseio+TBCR0, length & 0xFF);	/* lower byte of count */
#if	defined(ROMDIAG)
#else
    xmt_pending = 1;				/* show pending before xmt */
#endif
    G_outp (baseio+CR, TXP|ABR|STA);		/* start transmission */
    return (NO_ERROR);
}

/******************************************************************************

this initializes the NIC for xmitting/receiving for dumb cards
	TPSR = 0    =>	first 256 byte page of NIC's memory space
	PSTART = 8  =>	leave 2k for transmit memory and start receive
			ring at the ninth 256 byte page of the NIC's
			memory space
	PSTOP		this is the first 256 byte page
			after the end of the NIC's memory space
			(e.g. RAM size = 0x2000...PSTOP = 0x20)
	CURR = 9    =>	many values will work, this tells the chip which
			256 byte page to write the next received packet
	BNRY = 8    =>	initialize this to the same value as PSTART
			the NIC will not write to this 256 byte page or 
			any 256 byte page after it

******************************************************************************/
init_NIC()
{
    int  i;				/* general purpose counter */
    int  temp;
    int  count;

#if	defined(ENGR)
    if ((manual_init_nic) && (nic_initialized))
        return (NO_ERROR);
    nic_initialized = 1;
#endif
    tpsr_hold = 0;			/* transmit page start hold */
#if	defined(ENGR)
    if (large_frames)
        pstart_hold = 0x0A;
    else
        pstart_hold = 0x08;
#endif
    G_outp (baseio+CR, STP|ABR|PS0);	/* soft reset and page 0 */
    G_outp (baseio+RBCR0, 0);		/* clear remote byte count */
    G_outp (baseio+RBCR1, 0);
    count = 0;				/* wait for reset to finish */
    while ((!(G_inp (baseio+ISR) & RST)) && (count < 0x1000))
        count++;
    temp = ((fifo_depth & 0x0C) << 3) | BMS;    /* fifo depth | not loopback */
    if (board_id & (MICROCHANNEL | SLOT_16BIT))
        temp |= WTS;			/* word xfer select (16 bit cards) */
    G_outp (baseio+DCR, temp);
#if	defined(ENGR)
      rcr_hold = AM|AB;
      if (promiscuous_mode)
         rcr_hold |= PRO;
      if (force_crc_errors)
          rcr_hold |= SEP;
      if (accept_err_packets)
          rcr_hold |= (AR | SEP);
      if ((quick_mode) && (send_only_flag))
        rcr_hold = 0;
#else
      rcr_hold = 0;
#endif
    G_outp (baseio+RCR, rcr_hold);
    G_outp (baseio+TCR, LB2);			/* loopback operation */
    G_outp (baseio+PSTART, pstart_hold);	/* rcv ring strts 2k into RAM */
    if (RAMsize == 0x10000)
        pstop_hold = 0xFF;
    else
        pstop_hold = (((int)RAMsize >> 8) & 0xFF); /* rcv page stop hold */
    G_outp (baseio+PSTOP, pstop_hold);		/* stop at last RAM loc */
    G_outp (baseio+BNRY, pstart_hold);		/* init to = PSTART */
    G_outp (baseio+ISR, 0xFF);			/* clear all int status bits */
    G_outp (baseio+IMR, 0);			/* no interrupts yet */
    G_outp (baseio+CR, STP|ABR|PS1);		/* maintain rst | sel page 1 */
    for (i = 0; i < 6; i++)
    {
	G_outp (baseio+PAR0+i, node_addr[i]);	/* load physical address */
        G_outp (baseio+MAR0+i, 0);		/* other mlticst bits are 0 */
    }
    G_outp (baseio+MAR6, 0);			/* there are more MAR's */
    G_outp (baseio+MAR7, 0);			/*   than PAR's */
    local_nxtpkt_ptr = pstart_hold + 1;		/* internal next pkt pointer */
    G_outp (baseio+CURR, local_nxtpkt_ptr);
    G_outp (baseio+CR, STA|PS0|ABR);		/* start NIC | select page 0 */
#if	defined(ENGR)
    if (force_crc_errors)
        G_outp (baseio+TCR, MCRC);		/* do manual crc */
    else
        G_outp (baseio+TCR, 0);			/* allow receiving pkts */
#else
    G_outp (baseio+TCR, 0);			/* allow receiving pkts */
#endif
#if	defined(ROMDIAG)
#else
    xmt_pending = 0;				/* show nothing pending */
#endif
}

#if	defined(ROMDIAG)
#else
/************************************************************************

this will reset the multicast bit and disable multicast reception
	in the NIC

*************************************************************************/
disable_multicast ()
{
    int   temp_cr;
    int   temp_rcr;

    temp_cr = G_inp (baseio+CR) & ~TXP;		/* save old CR contents */
    G_outp (baseio+CR, (temp_cr & 0x3F) | PS2);	/* select page 2 */
    temp_rcr = G_inp (baseio+RCR);
    G_outp (baseio+CR, temp_cr & 0x3F);		/* select page 0 */
    G_outp (baseio+RCR, temp_rcr & ~AM);	/* disable multicast rcv */
    G_outp (baseio+CR, (temp_cr & 0x3F) | PS1);	/* select page 1 */
    G_outp (baseio+MAR0+MCA_REG, 0);		/* reset multicast hash bit */
    G_outp (baseio+CR, temp_cr);		/* restore original CR */
}

/************************************************************************

this will set the multicast bit and enable multicast reception
	in the NIC

*************************************************************************/
enable_multicast ()
{
    int   temp_cr;
    int   temp_rcr;

    temp_cr = G_inp (baseio+CR) & ~TXP;		/* save old CR contents */
    G_outp (baseio+CR, (temp_cr & 0x3F) | PS2);	/* select page 2 */
    temp_rcr = G_inp (baseio+RCR);
    G_outp (baseio+CR, temp_cr & 0x3F);		/* select page 0 */
    G_outp (baseio+RCR, temp_rcr | AM);		/* enable multicast rcv */
    G_outp (baseio+CR, (temp_cr & 0x3F) | PS1);	/* select page 1 */
    G_outp (baseio+MAR0+MCA_REG, MCA_VAL);	/* set multicast hash bit */
    G_outp (baseio+CR, temp_cr);		/* restore original CR */
}

/***************************************************************************

this clears our local stats

****************************************************************************/
clear_local_stats()
{
    int   i;
    unsigned long  int far *ptr;

    ptr = (unsigned long int far *) &frm_sent; /* get ptr to first stat var */
    for (i = 0; i < STAT_LEN; i++)	/* and zero all of them out */
        *ptr++ = 0;
}
#endif

/******************************************************************************

this sets up the interrupt vector for intelligent cards
	1) calculates which 8259 chip to use
	2) calculates what interrupt vector number to use
	3) calculates the interrupt mask
	4) saves the old interrupt vector for this interrupt level
	5) saves the old interrupt mask register contents
	6) writes the new interrupt mask register value

******************************************************************************/
prepare_int_vector ()
{
    int i,
        mask_8259_bit;

    if ((irq == 2) && (feature_info_byte_1 & SECOND_8259_PRESENT))
        irq = 9;
    if (irq > 7)			/* if using second int controller */
    {
        vector_no = 0x70 + (irq - 8);
        maskreg = MR8259B;
        intreg = IR8259B;
        mask_8259_bit = 1 << (irq - 8);
    }
    else
    {
        vector_no = irq + 8;
        maskreg = MR8259A;
        intreg = IR8259A;
        mask_8259_bit = 1 << irq;
    }
    prv_vector = get_vector (vector_no);
    save_mask = G_inp (maskreg);	 	/* get old interrupt mask */
    if (irq > 7)
        G_outp (MR8259A, (G_inp (MR8259A) & 0xFB));
    G_outp (maskreg, (save_mask & ~mask_8259_bit)); 	/* unmask our irq */
    if (irq == 9)
        irq = 2;
}

/******************************************************************************

this puts the interrupt mask register back to its original value and
	installs the original interrupt vector into the vector table
	at the base page of memory

******************************************************************************/
int return_int_vector ()
{
    int_off ();
    set_vector (vector_no, prv_vector);	/* put interrupt vector back */
    G_outp (maskreg, save_mask);	/* put interrupt mask back */
    int_on ();
    return (NO_ERROR);
}

