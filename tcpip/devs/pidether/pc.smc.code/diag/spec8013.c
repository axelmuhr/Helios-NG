/*****************************************************************************
*******************************************************************************
	A George Kalwitz Production, 1989
*******************************************************************************
******************************************************************************/

/*****************************************************************************

this module contains routines specific to the WD8013 family of cards

******************************************************************************/

#include	"defs8013.h"
#include	"board_id.h"
#include	"params.h"

/*****************************************************************************

this sets bit 6 (0 justified) of register offset 0x05
	it will enable the lan controller to access shared RAM 16 bits at
	a time
in addition, this routine maintains address bit 19
	(previous cards assumed this bit high...we must do it manually)

	note: this is a write only register

******************************************************************************/
setup_8013 ()
{
    /* this will protect against any other software who may have
       set the LAN16ENB even though the adapter is in an 8 bit slot */
    if (board_id & BOARD_16BIT)
    {
        if (board_id & INTERFACE_CHIP)
            G_outp (baseio+LAAR, ((G_inp (baseio+LAAR) & LAAR_MASK) | LA19));
        else
            G_outp (baseio+LAAR, INIT_LAAR_VALUE);
    }
    if (board_id & SLOT_16BIT)
    {
        if (board_id & INTERFACE_CHIP)
            G_outp (baseio+LAAR,
                   ((G_inp (baseio+LAAR) & LAAR_MASK) | LAN16ENB));
        else
            G_outp (baseio+LAAR, (LAN16ENB | LA19));
    }
}

/*****************************************************************************

this sets bit 7 (0 justified) of register offset 0x05
	it will enable the host to access shared RAM 16 bits at
	a time
it will also maintain the LAN16BIT bit high
in addition, this routine maintains address bit 19
	(previous cards assumed this bit high...we must do it manually)

	note 1: this is a write only register

	note 2: this routine should be called only after interrupts are 
		disabled and they should remain disabled until after the
		routine 'dis_16bit_access' is called

******************************************************************************/
en_16bit_access ()
{
    if (board_id & SLOT_16BIT)
    {
        if (board_id & INTERFACE_CHIP)
            G_outp (baseio+LAAR,
                   ((G_inp (baseio+LAAR) & LAAR_MASK) | MEM16ENB | LAN16ENB));
        else
            G_outp (baseio+LAAR, (MEM16ENB | LAN16ENB | LA19));
    }
}

/*****************************************************************************

this resets bit 7 (0 justified) of register offset 0x05
	it will disable the host from accessing shared RAM 16 bits at
	a time
it will maintain the LAN16BIT bit high
in addition, this routine maintains address bit 19
	(previous cards assumed this bit high...we must do it manually)

	note: this is a write only register

******************************************************************************/
dis_16bit_access ()
{
    if (board_id & SLOT_16BIT)
    {
        if (board_id & INTERFACE_CHIP)
            G_outp (baseio+LAAR,
                   ((G_inp (baseio+LAAR) & LAAR_MASK) | LAN16ENB));
        else
            G_outp (baseio+LAAR, (LAN16ENB | LA19));
    }
}

/*****************************************************************************

this resets bit 6 (0 justified) of register offset 0x05
	it will disable the lan controller to access shared RAM 16 bits at
	a time
in addition, this routine maintains address bit 19
	(previous cards assumed this bit high...we must do it manually)

	note: this is a write only register

******************************************************************************/
reset_16bit_board ()
{
    if (board_id & SLOT_16BIT)
    {
        if (board_id & INTERFACE_CHIP)
            G_outp (baseio+LAAR, (G_inp (baseio+LAAR) & LAAR_MASK));
        else
            G_outp (baseio+LAAR, LA19);
    }
}

/**************************************************************************

***************************************************************************/
check_16bit_access ()
{
    if (board_id & SLOT_16BIT)
    {
        int_off ();
        en_16bit_access ();
    }
}

/**************************************************************************

***************************************************************************/
recheck_16bit_access ()
{
    if (board_id & SLOT_16BIT)
    {
        dis_16bit_access ();
        int_on ();
    }
}

