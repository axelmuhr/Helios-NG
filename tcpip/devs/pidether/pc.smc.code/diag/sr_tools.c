/******************************************************************************
*******************************************************************************
	A George Kalwitz Production, 1990
*******************************************************************************
******************************************************************************/

/******************************************************************************

this module contains tools for implementing send, receive, and echo

******************************************************************************/

#include	"time.h"
#include	"ieee8023.h"
#include	"ieee8022.h"
#include	"common.h"
#include	"cvars.h"
#include	"defs8390.h"
#include	"defs583.h"
#include 	"micro_ch.h"
#include	"board_id.h"
#include	"timing.h"
#include	"params.h"
#include	"err_cods.h"
#include	"diagdefs.h"
#include	"diagdata.h"
#include	"eth_data.h"

#if	defined(ROMDIAG)
#else
#include	"malloc.h"
#endif

#if	defined(DIAG)

#include	"cwuser.h"
#include	"diagmsg.mlh"
#include	"diaghlp.hlh"

#endif

int setup_int_vector (int);

/******************************************************************************

This portion determines if there is a board present by checking
the passed base I/O addresses for the 3 Node address bytes: 

	either 00 00 C0 or that entered by the OEM

*****************************************************************************/
int is_board_there (addr)
int  addr;
{
    if ((char) G_inp(addr+LAR) == (char) BISTRO_NODE_ADDR_0)
        if ((char) G_inp(addr+LAR2) == (char) BISTRO_NODE_ADDR_1)
            if ((char) G_inp(addr+LAR3) == (char) BISTRO_NODE_ADDR_2)
                return (1);
    if ((char) G_inp(addr+LAR) == (char) ALT_BISTRO_NODE_ADDR_0)
        if ((char) G_inp(addr+LAR2) == (char) ALT_BISTRO_NODE_ADDR_1)
            if ((char) G_inp(addr+LAR3) == (char) ALT_BISTRO_NODE_ADDR_2)
                return (1);
#if	defined(BISTRO)
#else
    if ((char) G_inp(addr+LAR) == (char) WD_NODE_ADDR_0)
        if ((char) G_inp(addr+LAR2) == (char) WD_NODE_ADDR_1)
            if ((char) G_inp(addr+LAR3) == (char) WD_NODE_ADDR_2)
                return (1);
#endif
#if	defined(DIAG)
    if ((char) G_inp(addr+LAR) == (char) *OEM_node_addr)
        if ((char) G_inp(addr+LAR2) == (char) *(OEM_node_addr+1))
            if ((char) G_inp(addr+LAR3) == (char) *(OEM_node_addr+2))
                return (1);
#endif
    return (0);
}

#if	defined(ROMDIAG)
#else
/******************************************************************************

all received packets will be moved into this local receive table until ready
	for processing

******************************************************************************/
get_rcv_tbl_spc ()
{
    table_ptr = (struct RCV_TBL far *) calloc (1, sizeof (struct RCV_TBL));
}

/******************************************************************************

all packets are built into this contiguous transmit buffer allowing a single
	string move to transfer the entire packet into the NIC's transmit
	RAM

******************************************************************************/
init_local_tx_buffer ()
{
    local_tx_buffer = (struct TBUF far *) calloc (1, sizeof (struct TBUF));
}
#endif

#if	defined(ROMDIAG)
#else
/******************************************************************************

this gives back the memory allocated for the transmit and receive buffers

******************************************************************************/
free_memory ()
{
    free ((char far *) table_ptr);
    free ((char far *) local_tx_buffer);
}
#endif

#if	defined(ROMDIAG)
#else
/******************************************************************************

this prepares to send and receive packets

******************************************************************************/
int init_snd_rcv()
{
    int  ret_code;

    board_int_off ();
    if (ret_code = setup_int_vector (FALSE))
        return (ret_code);
    if (ret_code = initialize_hardware ())
        return (ret_code);
    if (ret_code = get_snd_rcv_space ())
        return (ret_code);
    initialize_pointers ();
    board_int_on ();				/* allow NIC to interrupt */
    while (gota_packet () == PACKET_READY)	/* get rid of any pkts */
        update_head ();
#if	defined(TEST690)
	G_outp (baseio+IRR, (G_inp (baseio+IRR) | AINT));
#endif
    return (NO_ERROR);
}
#endif

/******************************************************************************


******************************************************************************/
int initialize_hardware ()
{
    int  ret_code;
    int  temp_batch_type;

    setup_hardware ();
#if	defined(DIAG)
    if (!(is_ram_enabled))
    {
        temp_batch_type = batch_type;
        batch_type = B_TEST;
        if (!(verify_ram_base ()))
            test_ram ();
        batch_type = temp_batch_type;
    }
#endif
#if	defined(ROMDIAG)
    init_NIC ();
#else
    upd_index = 0;				/* auto update */
    init_NIC();					/* setup the NIC */
#endif
    return (NO_ERROR);
}

#if	defined(ROMDIAG)
#else
/******************************************************************************

this undoes the batch preparation 

******************************************************************************/
undo_snd_rcv()
{
    board_int_off ();
#if	defined(ENGR)
#else
    disable_lan ();
#endif
    return_int_vector ();			/* put old ISR into table */
    free_memory ();				/* get rid of rcv/xmt table */
}
#endif

#if	defined(ROMDIAG)
int build_packet ()
{
    int     i;
    int far *tmp_ptr;

    tmp_ptr = (int far *) RAMbase;
    *tmp_ptr++ = 0x0008;
    *tmp_ptr++ = 0x005A;
    *tmp_ptr++ = 0x0000;
    *tmp_ptr++ = 0x0008;
    *tmp_ptr++ = 0x005A;
    *tmp_ptr++ = 0x0000;
/*
    *tmp_ptr++ = 0x0000;
    *tmp_ptr++ = 0x00C0;
    *tmp_ptr++ = 0x0000;
    *tmp_ptr++ = 0x0000;
    *tmp_ptr++ = 0x00C0;
    *tmp_ptr++ = 0x0000;
*/
    for (i = 0; i < 26; i++)
        *tmp_ptr++ = 0x0000;
    return (NO_ERROR);
}
#else
/******************************************************************************

this sets up the xmit buffer so that packets may be moved to the NIC's 
	transmit RAM with one string move

*****************************************************************************/
build_packet (finding_responder, dest_addr_ptr)
char	finding_responder;
char far *dest_addr_ptr;
{
    int        i;
    int	       data_len;
    int        pattern_offset;
#if	defined(ENGR)
    char  far  *tmp_ptr;
#endif

    data_len = frame_len - 18;    /* src = 6; dst = 6; len = 2; crc = 4 */
    movstr (dest_addr_ptr, local_tx_buffer->d_addr, 6);	/* move dest addr */
    movstr (&node_addr[0], local_tx_buffer->s_addr, 6);	/* move our addr */
    local_tx_buffer->len_1 = data_len >> 8;		/* data length */
    local_tx_buffer->len_0 = data_len & 0xFF;
    local_tx_buffer->d_sap = llc_dst_sap & 0xFF;	/* link layer stuff */
    local_tx_buffer->s_sap = llc_src_sap & 0xFF;
    local_tx_buffer->cntrl = llc_control_byte & 0xFF;
    if (finding_responder)
    {
        local_tx_buffer->info[0] = 0x81;
        local_tx_buffer->info[1] = 0x01;
        local_tx_buffer->info[2] = 0x00;
        pattern_offset = 3;
    }
    else
        pattern_offset = 0;
    for (i = 0; i < data_len/20+1; i++)			/* and the data */
        movstr (&pattern[0], &local_tx_buffer->info[pattern_offset + i*20], 20);
    size_hold = frame_len - 4;			/* dont count crc */
#if	defined(ENGR)
    if (force_crc_errors)
    {
        tmp_ptr = local_tx_buffer->d_addr;
        tmp_ptr += size_hold;
        for (i = 0; i < 4; i++)
        {
            *tmp_ptr = 0;
            tmp_ptr++;
        }
        size_hold += 4;			/* add in CRC */
    }
#endif
    if (check_method == CHECKSUM_METHOD)
        fill_checksum (local_tx_buffer->d_addr, size_hold);
#if	defined(ENGR)
    if ((quick_mode) && (send_only_flag))
        pkt_to_sh_mem (local_tx_buffer, tx_ptr, size_hold); /* pkt to tx area */
#endif
}
#endif

#if	defined(ROMDIAG)
#else
/******************************************************************************

this echoes a packet by moving the appropriate data from the receive table
	to the NIC's transmit RAM
	'my_head' points to the next packet to process

******************************************************************************/
int echo_packet()
{
    int         size;
#if	defined(ENGR)
    int		i;
    char   far  *tmp_ptr;
#endif

  #if	defined(ENGR)
    if (receive_only_flag)
        return (NO_ERROR);
  #endif
    size = (((int)my_head->count1 << 8) | ((int)my_head->count0 & 0xFF));
    size -= 4;					/* remove crc size */
#if	defined(ENGR)
    tmp_ptr = my_head->daddr;
    tmp_ptr += size;			/* point to CRC */
    for (i = 0; i < 4; i++)
        *tmp_ptr = 0xFF;
    if (force_crc_errors)
        size += 4;			/* add in CRC */
#endif
    movstr (my_head->saddr, my_head->daddr, 6);	/* make sender the dest */
    movstr (node_addr, my_head->saddr, 6);	/* make us the sender */
    my_head->dsap = my_head->ssap & 0xFF;	/* make dest now the src */
    my_head->ssap = (llc_src_sap | RESP_BIT) & 0xFF;
    my_head->control = (my_head->control | PF_BIT) & 0xFF;
    if (check_method == CHECKSUM_METHOD)
        fill_checksum (my_head->daddr, size);
    return (transmit_packet (my_head->daddr, size));
}

/***********************************************************************

this clears all the statistics

*************************************************************************/
clear_stats()
{
    int i;

    clear_local_stats();
    #if	defined(ENGR)
        for (i = 0; i < 17; i++)
            collision_table[i] = 0;
    #endif
}
#endif

/******************************************************************************

******************************************************************************/
int setup_int_vector (test_flag)
int test_flag;
{
    int  ret_code;

    ret_code = NO_ERROR;
    int_off ();
#if	defined(ROMDIAG)
    prepare_int_vector ();
    install_tlan (vector_no);
#else
  #if	defined(DIAG)
    if (test_flag)
    {
        prepare_int_vector ();
        install_tlan (vector_no);
    }
    else
    {
  #endif
        prepare_int_vector ();
       #if	defined(ENGR)
        if (quick_mode)
            install_qlan (vector_no);
        else
       #endif
            install_rlan (vector_no);
  #if	defined(DIAG)
    }
  #endif
#endif
    int_on ();
    return (ret_code);
}

#if	defined(ROMDIAG)
#else
/******************************************************************************

this initializes the parameters used by send and receive 

******************************************************************************/
int get_snd_rcv_space ()
{
    #if	defined(DIAG)
    int i;

    i = get_current_MSR();		/* get contents */
    if (!(i & MENB))			/* if not enabled */
    {
        if (!(batch_type & (B_SEND | B_RESP | B_TEST)))
            Alert (RAM_NOT_ENABLED, 24, 0);
        return (DISABLED_RAM_ERR);
    }
    #endif

    get_rcv_tbl_spc ();			/* allocate receive table space */
    if (table_ptr == NULL)
    {
#if	defined(DIAG)
        if (!(batch_type & (B_SEND | B_RESP | B_TEST)))
            Alert (NO_MEM, 24, 0);
#else
        my_alert ("Insufficient memory to run the sample driver", 15, 0);
#endif
        return (INSUFFICIENT_MEM_ERR);
    }
    init_rcv_table ();			/* put rcv table into known state */
    init_local_tx_buffer ();			/* allocate transmit buffer */
    if (local_tx_buffer == NULL)
    {
#if	defined(DIAG)
        if (!(batch_type & (B_SEND | B_RESP | B_TEST)))
            Alert (NO_MEM, 24, 0);
#else
        my_alert ("Insufficient memory to run the sample driver", 15, 0);
#endif
        return (INSUFFICIENT_MEM_ERR);
    }
    return (NO_ERROR);
}
#endif

#if	defined(ROMDIAG)
#else
/**************************************************************************


***************************************************************************/
initialize_pointers ()
{
    tx_ptr = (int far *) RAMbase;
    node_addr_ptr = (int far *) RAMbase;
#if	defined(DIAG)
#if	defined(ENGR)
    if ((large_frames) && (frame_len > CONTENTION_OFFSET))
      contention_ptr = (int far *)(RAMbase + (long)(CONTENTION_OFFSET + 0x200));
    else
      contention_ptr = (int far *) (RAMbase + (long)CONTENTION_OFFSET);
#else
    contention_ptr = (int far *) (RAMbase + (long)CONTENTION_OFFSET);
#endif
#endif
}
#endif

#if	defined(ROMDIAG)
#else
/******************************************************************************

******************************************************************************/
int is_xmt_pending ()
{
	return (xmt_pending);
}
#endif

#if	defined(ROMDIAG)
#else
/******************************************************************************

******************************************************************************/
show_tx_overlap_error ()
{
#if	defined(DIAG)
        if ((!(byp_index)) && (!(batch_type & (B_SEND | B_RESP | B_TEST))))
            Alert (TX_OVERLAP, 24, 0); 
#else
        my_alert ("Tried to xmit with the previous one still pending.", 15, 0);
#endif
}
#endif

#if	defined(ROMDIAG)
int transmit_packet ()
{
            return (initiate_xmit (60));
}
#else
/******************************************************************************

this handles everything to transmit a packet
	enter with the address of the local buffer and the size of the packet

******************************************************************************/
int transmit_packet (source_ptr, length)
int far *source_ptr;
int length;
{
    int		ret_code;
    time_t	start_time;
    time_t	time (time_t *);

    if (is_xmt_pending ())			/* was the last pkt sent??? */
    {
        overlapped++;
        start_time = time (NULL);		/* wait till not pending */
        while (time (NULL) < (start_time + OVERLAP_DELAY))
        {
            if (!(is_xmt_pending ()))
                break;
            if (was_keyboard_hit ())
                return (KB_HIT_ERR);		/* return with unique error */
        }
        if (is_xmt_pending ())
        {
            show_tx_overlap_error ();
            return (TX_OVERLAP_ERR);
        }
    }

#if	defined(ENGR)
    if ((quick_mode) && (send_only_flag))
    {
        if (ret_code = initiate_xmit (length))
            return (ret_code);
    }
    else
    {
#endif
        pkt_to_sh_mem (source_ptr, tx_ptr, length);	/* pkt to tx area */
        if (ret_code = initiate_xmit (length))
            return (ret_code);
        #if	defined(DIAG)	/* Diagnostic causes memory contention */
        if (ret_code = make_mem_contention ())
            return (ret_code);
        #endif
#if	defined(ENGR)
    }
       if (board_id & NIC_690_BIT)
       {
           if (contend_690_mode)
               contend_690 ();
       }
#endif
    return (NO_ERROR);				/* show no error */
}
#endif

#if	defined(ROMDIAG)
#else
/******************************************************************************

this will do either a memory to memory string move or a memory to I/O string
	move

******************************************************************************/
pkt_to_sh_mem (src, dst, size)
int far *src;
int far *dst;
int size;
{
	if (board_id & SLOT_16BIT)
        {
		check_16bit_access ();
		mv8013 (src, dst, size);
		recheck_16bit_access ();
        }
	else
		movstr (src, dst, size);
	return;
}
#endif

#if	defined(ROMDIAG)
#else
/**************************************************************************

this waits for a packet to be received by the NIC

**************************************************************************/
int wait_for_rcv ()
{
    int         ret_code;
    time_t      start_time;
    time_t      time (time_t *);

    ret_code = NO_ERROR;
    start_time = time (NULL);
    while (time (NULL) < (start_time + rcv_timeout))
    {
        if (was_keyboard_hit ())
            return (KB_HIT_ERR);
#if	defined(DIAG)
        if (ret_code = make_mem_contention ())
            return (ret_code);
#endif
#if	defined(ENGR)
       if (board_id & NIC_690_BIT)
       {
           if (contend_690_mode)
               contend_690 ();
       }
#endif
        ret_code = gota_packet ();
        if (ret_code == PACKET_READY)
            return (process_packet ());
    }
    return (RCV_TIME_OUT_ERR);
}

#endif

#if	defined(ROMDIAG)
#else
/******************************************************************************

this checks to see if a packet is ready for processing

******************************************************************************/
int gota_packet()
{
    int  ret_code;

    ret_code = NO_ERROR;
    if (buffers_used)
        ret_code = PACKET_READY;		/* show packet ready */
    return (ret_code);				/* show no packet */
}

/******************************************************************************

this will process a packet once it has been moved to local memory

******************************************************************************/
int process_packet()
{
    int  i;
    int  ret_code;

    ret_code = 0;
#if	defined(DIAG)
      if (mode == 3)
      {
          ret_code = 0;
          if (check_method == CHECKSUM_METHOD)
              ret_code = checksum_packet ();
          if (!ret_code)
              ret_code = echo_packet ();
      }
      if (mode == 0)
      {
          switch (check_method)
          {
              case NULL_METHOD:
                  break;
              case COMPARE_METHOD:
                  ret_code = compare_packet ();
                  break;
              case CHECKSUM_METHOD:
                  ret_code = checksum_packet ();
                  break;
              default:
                  break;
          }
      }
#else
      if (mode)
          ret_code = echo_packet ();
#endif
    update_head ();
    return (ret_code);
}
#endif

/******************************************************************************

this will set the enable interrupt bit for the appropriate board

******************************************************************************/
enable_board_int()
{
    int   temp;

    if ((board_id & INTERFACE_CHIP) && (board_id & MICROCHANNEL))
    {
        temp = G_inp (baseio+CCR);		/* get old contents */
        G_outp (baseio+CCR, temp | EIL);	/* and enable ints */
    }
    else if ((board_id & INTERFACE_CHIP) && (!(board_id & MICROCHANNEL)))
    {
        set_interrupt_status ();
    }
    else
        ;
}

/**************************************************************************

*************************************************************************/
set_interrupt_status ()
{
	int	temp;

	temp = G_inp (baseio+IRR);		/* get old int req reg */
	G_outp (baseio+IRR, temp | IEN);	/* and enable ints */
}

/**************************************************************************

*************************************************************************/
reset_interrupt_status ()
{
	int	temp;

	temp = G_inp (baseio+IRR);		/* get old int req reg */
	G_outp (baseio+IRR, temp & ~IEN);	/* and disable ints */
}

/**************************************************************************

this will disable the on board ram

*************************************************************************/
disable_ram()
{
    int  temp;

    temp = get_current_MSR();
    G_outp (baseio+MSR, (temp & ~(MENB|RST)));  /* disabled on board RAM */
#if	defined(DIAG)
    is_ram_enabled = 0;				/* show RAM disabled */
#elif	defined(ROMDIAG)
    is_ram_enabled = 0;				/* show RAM disabled */
#endif
}

/**************************************************************************

***************************************************************************/
enable_ram()
{
    int temp;

    temp = get_current_MSR();
    temp = temp & ~(MENB|RST);			/* mask unneeded bits */
    G_outp (baseio+MSR, temp | MENB);		/* enable memory */
#if	defined(DIAG)
    is_ram_enabled = 1;				/* show RAM enabled */
#elif	defined(ROMDIAG)
    is_ram_enabled = 1;				/* show RAM enabled */
#endif
}

/******************************************************************************

this gets the value which should be in the MSR
    for an 8003E type, it re-calculates the value
    for the other types, it reads the current contents of the register

******************************************************************************/
int get_current_MSR ()
{
    int value;

    value = (int)(RAMbase >> 25) & 0x3F; /* so find what it should be */
#if	defined(DIAG)
    if (is_ram_enabled)
        value = value | MENB;		/* show RAM is enabled */
#elif	defined(ROMDIAG)
    if (is_ram_enabled)
        value = value | MENB;		/* show RAM is enabled */
#endif
    return (value);
}

/******************************************************************************


******************************************************************************/
setup_hardware ()
{
    get_lan_addr ();
#if	defined (MCDGS)
#else
    setup_8013 ();
#endif
    enable_board_int ();
}

/******************************************************************************


******************************************************************************/
get_lan_addr()
{
    int i;

    for (i = 0; i < 6; i++)		/* get the node address of board */
        node_addr[i] = (char) G_inp (baseio+LAR+i);            
}

#if	defined(ROMDIAG)
#else
/******************************************************************************


******************************************************************************/
clear_lan_addr()
{
    int i;

    for (i = 0; i < 6; i++)
        node_addr[i] = 0;            
}
#endif

/******************************************************************************


*****************************************************************************/
int disable_lan ()
{
    G_outp (baseio+CR, STP|ABR|PS0);
    return (NO_ERROR);
}

#if	defined(ROMDIAG)
#else
/**************************************************************************

***************************************************************************/
int was_keyboard_hit ()
{
	int   input;

	if (kbhit ())
	{
		input = getch ();
		if (input == 0)
			getch ();
		return (TRUE);
	}
	else
		return (FALSE);
}
#endif

/**************************************************************************

***************************************************************************/
fill_checksum (char_pointer, size)
char	far	*char_pointer;
int	size;
{
    unsigned	int	i;
    char		checksum;

    checksum = 0;
    for (i = 1; i < size; i++)		/* one less than the length */
    {
        checksum += *char_pointer & 0xFF;
        char_pointer++;
    }
    checksum = (CHECKSUM_VALUE - checksum) & 0xFF;
    *char_pointer = checksum;
}

