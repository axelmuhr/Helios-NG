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
#include	"defs8390.h"
#include	"defs583.h"
#include	"board_id.h"
#include	"intrupts.h"
#include	"params.h"
#include	"cvars.h"
#include	"eth_data.h"

#include	"diagdefs.h"
#include	"diagdata.h"
#include	"sys_info.h"

#if	defined(SAMPLDRV)
#else
/***************************************************************


****************************************************************/
lan_test_rtn ()
{
    G_outp (baseio+IMR, 0);
    G_outp (baseio+ISR, 0xFF);		/* clear int pending */
    if ((irq > 7) ||
             ((irq == 2) && (feature_info_byte_1 & SECOND_8259_PRESENT)))
        G_outp (IR8259B, EOI);		/* issue two non_specific EOI's */
    G_outp (IR8259A, EOI);		/* issue non_specific EOI */
    got_serviced = 1;			/* show main routine it got serviced */
#if	defined(ROMDIAG)
#else
    xmt_pending = 0;
#endif
}
#endif

/************************************************************************

this will let the board interrupt the host

*************************************************************************/
board_int_on ()
{
    G_outp (baseio+IMR, imr_hold);
}

/************************************************************************

this will stop the board from interrupting the host

*************************************************************************/
board_int_off ()
{
    G_outp (baseio+IMR, 0);
}

