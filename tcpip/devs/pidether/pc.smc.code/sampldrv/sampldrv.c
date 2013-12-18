/******************************************************************************
*******************************************************************************
	A George Kalwitz Production, 1989
*******************************************************************************
******************************************************************************/

#include	"ieee8023.h"
#include	"cvars.h"
#include	"params.h"
#include	"board_id.h"
#include	"defs8390.h"
#include	"defs583.h"
#include	"intrupts.h"
#include	"timing.h"
#include	"micro_ch.h"
#include	"defs8013.h"
#include	"sdrvvars.h"
#include	"sys_info.h"
#include	"getcnfg.h"

#include	<stdio.h>
#include	<conio.h>
#include	<malloc.h>

extern	CNFG_Adapter	config_table;

/**************************************************************************

***************************************************************************/
main (argc, argv)
int	argc;
char	*argv[];
{
	init_params ();
	if (batcharg (argc, argv))	/* get command line params */
            exit (0);
	if (init_dependants ())
	{
		if (micro_chnl)
			printf ("\nNo board found in slot %d.\n",
						channel_pos + 1);
		else
			printf ("\nNo board found at base I/O %3X.\n",
						baseio);
		helpscr ();
		exit (0);
	}
	setup_screen ();		/* set color and draw screen */
	draw_short_stats ();		/* force update stats */
	handle_key_strokes ();		/* get user input */
	unset_screen ();		/* unset color and remove screen */
        disable_lan ();
	disable_ram ();
        if (board_id & BOARD_16BIT)
            reset_16bit_board ();
}

/**************************************************************************

this is called from the 'handle_key_strokes' routine

***************************************************************************/
drive_board()
{
    if (mode)
        become_responder ();		/* mode = 1 -> responder */
    else
        become_sender ();		/* mode = 0 -> initiator */
    draw_short_stats ();		/* when done force stats update */
}

/****************************************************************************

*****************************************************************************/
init_params ()
{
	char	far	*get_sys_info_ptr ();
	char	far	*sys_info_ptr;

	sys_info_ptr = get_sys_info_ptr ();
	feature_info_byte_1 = *(sys_info_ptr + FEATURE_INFO_BYTE_1_OFFSET);
	if (micro_channel ())
		micro_chnl = TRUE;
	else
		micro_chnl = FALSE;
	batch = 0;			/* flags any command line params */
	batch_type = B_NONE;		/* save specific command line params */
	clear_stats();			/* make sure none are left over */
	imr_hold = PRXE|PTXE|RXEE|TXEE|OVWE|CNTE;	/* our interrupts */
	RAMbase = 0xD0000000;		/* command line may change this */
	irq = 3;			/* command line may change this */
	baseio = 0x280;			/* command line may change this */
	RAMsize = 0x2000;		/* may change this later */
	check_method = 0;		/* dont compare received to sent */
	iterations = 1000;		/* this is changable by user */
	rcv_timeout = 3;		/* wait 3 seconds for response */
	max_retry = 2;			/* retry transmit 3 times */
	responder_found = 0;		/* look for a responder first */
	update_counter = 0;		/* update stats once every 10 */
}

/****************************************************************************

*****************************************************************************/
int	init_dependants ()
{
	config_table.cnfg_bus = micro_chnl;
	config_table.cnfg_base_io = baseio;
	config_table.cnfg_slot = channel_pos + 1;
	if (WDM_GetCnfg (&config_table) == -1)
		return (1);
	board_id = (((unsigned long) config_table.cnfg_extra_info << 16) |
			(unsigned long) config_table.cnfg_bid);
	if (board_id & MICROCHANNEL)
		get_POS_info ();
	else if (board_id & INTERFACE_CHIP)
		get_at_info ();
	initialize_pointers ();
	enable_ram ();				/* enable RAM on this board */
	setup_hardware ();
	num_of_fields = 5;
	return (0);
}

/****************************************************************************

this updates the sent and received statistics every 10 times or if the 
	caller forces the update

*****************************************************************************/
draw_short_stats ()
{
	position_cursor (10, 22);
	printf ("%15lu", frm_sent);
	position_cursor (10, 62);
	printf ("%15lu", frm_rcvd);
}

/******************************************************************************

this does everthing needed to become an initiator on a live network

******************************************************************************/
become_sender ()
{
    qck_init_send_screen ();		/* do any needed screen alterations */
    draw_short_stats ();		/* force update stats */
    send_messages ();			/* do the sending */
    undo_send_screen ();		/* remove any specifics from the scrn */
}

/******************************************************************************

this does everthing needed to become a responder on a live network

******************************************************************************/
become_responder ()
{
    qck_init_resp_screen ();		/* do any needed screen alterations */
    clear_stats ();			/* start with clean slate */
    draw_short_stats ();		/* force update stats */
    echo_messages ();			/* do the echoing */
    undo_resp_screen ();		/* remove any specifics from the scrn */
}

/******************************************************************************


******************************************************************************/
qck_init_send_screen ()
{
}

/******************************************************************************


******************************************************************************/
undo_send_screen ()
{
}

/******************************************************************************


******************************************************************************/
qck_init_resp_screen ()
{
    clear_responder_addr ();		/* dont leave a responder if initiate */
    show_responder_addr ();		/*   found one */
    responder_found = 0;		/* and make initiate find a new one */
}

/******************************************************************************


******************************************************************************/
undo_resp_screen ()
{
}

/******************************************************************************

this is called from the command line parser
	it places the version number of this executable on the screen

******************************************************************************/
verscpy ()
{
	printf ("\n\n\n");
	printf ("WD LAN ADAPTER SAMPLE DRIVER\n");
	printf ("       Version %s\n", version);
	printf ("\n\n\n");
}

/******************************************************************************

this is called from the command line parser
	it shows the user the available command line parameters

******************************************************************************/
helpscr ()
{
    printf ("\n");
    printf ("WD LAN ADAPTER SAMPLE DRIVER COMMAND LINE PARAMETERS\n");
    printf ("\n");
    printf ("   /base:aaa    Specifies base I/O address.\n");
    printf ("   /irq:n       Specifies interrupt level (IRQ).\n");
    printf ("   /ram:aaaaa   Specifies base address of the on-board RAM.\n");
    printf ("   /slot:n      Specifies micro-channel slot adapter to test.\n");
    printf ("   /v           Displays program version number.\n");
    printf ("   /?           Displays this help screen.\n");
    printf ("\n");
}

/******************************************************************************

this clears out the destination address after initiate found a responder

******************************************************************************/
clear_responder_addr ()
{
    int  count;

    for (count = 0; count < 6; count++)
        resp_addr[count] = 0;
}

