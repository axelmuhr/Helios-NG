/*------------------------------------------------------------------------
--                                                                      --
--          H E L I O S   I N P U T / O U T P U T   S E R V E R         --
--          ---------------------------------------------------         --
--                                                                      --
--               Copyright (C) 1993, Perihelion Software Ltd.           --
--                          All Rights Reserved.                        --
--                                                                      --
--  qpc.c                                                               --
--                                                                      --
--  Author:  BLV 14/3/93                                                --
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Id# */
/* Copyright (C) 1993, Perihelion Software Ltd. 			*/

/**
*** This module contains the support for the Loughborough Sound Images Ltd.
*** boards, QPC C40 V1 and QPC C40 V2. These boards have a rather complicated
*** setup with several different sets of base addresses some of which have to
*** be set in software. 
**/

#include "../helios.h"

static int	HIOBASE		= 0x300;
static int	SLABASE		= 0x280;
static int	Version		= 1;
static int	Module		= 0;	/* primary, ...			*/
       int	QpcLinkData	= 0x284;/* 2, 4, 6 | 8 + SLABASE	*/
       int	QpcTxEmptyBit	= 0x08;	/* bit depends on module number	*/
       int	QpcRxFullBit	= 0x04;	/* ditto.			*/
       int	QpcStatus	= 0x292;/* 12 + SLABASE			*/
static int	ControlValue	= 0;
static int	BarValue	= 0;

#define ResetBits	0x0001		/* global reset only		*/

#define	ControlOffset	0x10		/* relative to HIOBASE		*/
#define BarOffset	0x18		/* relative to HIOBASE		*/
#define InterruptStatus	0x12		/* relative to SLABASE		*/
					/* == interrupt enable register	*/
#define InterruptMode	0x16		/* relative to SLABASE		*/
#define StatusOffset	0x10		/* relative to SLABASE		*/

#define ITERATIONS	32767		/* Timeout counter.		*/

/**
*** Reset involves writing two values to the control register, one to assert
*** the resets and one to clear it again. The interrupt status register is
*** read to clear out any junk, and RxFull and TxEmpty are reset.
***
*** Debugging: doing a complete reinitialisation every time rather than
*** just asserting processor and link reset means that the program does not
*** hang up.
**/
void qpc_reset(void)
{
	_outpw(HIOBASE + ControlOffset, 0);
	_outpw(HIOBASE + BarOffset, BarValue);
	_outpw(HIOBASE + ControlOffset, ControlValue | ResetBits);
	goto_sleep(100000);
	_outpw(HIOBASE + ControlOffset, ControlValue);
	_outpw(QpcStatus, QpcTxEmptyBit | QpcRxFullBit);

	(void) _inpw(QpcStatus);
}

/**
*** Byte I/O is illegal since C40 links always transfer words.
**/
int qpc_byte_to_link(int x)
{
	ServerDebug("qpc_byte_to_link: internal error, this routine cannot be called.");
	longjmp(exit_jmpbuf, 1);
}

int qpc_byte_from_link(char *where)
{
	ServerDebug("qpc_byte_from_link: internal error, this routine cannot be called.");
	longjmp(exit_jmpbuf, 1);
}

/**
*** rdrdy() and wrrdy() simply involve reading the interrupt status
*** register, storing the current values, 
**/
int qpc_rdrdy(void)
{
	int	x;

	x = _inpw(QpcStatus);
	if (x & QpcRxFullBit)
		return(TRUE);
	else
		return(FALSE);
}

int qpc_wrrdy(void)
{
	int	x;

	x = _inpw(QpcStatus);
	if (x & QpcTxEmptyBit)
		return(TRUE);
	else
		return(FALSE);
}

/**
*** fetch_block() and send_block() are implemented in assembler
**/

/**
*** Initialisation. This is rather complicated.
***
*** a) work out whether this is a version 1 or a version 2 board, using the
***    the box name.
*** b) get the HIOBASE address from the host.con file. This is link_base.
*** c) get the SLABASE address from the host.con file.
*** d) fill in the BAR register, and work out the correct value for the
***    control register. The real link adapter can now be identified.
*** e) get the module number from host.con (primary, secondary, tertiary,
***    or quaternary. Primary is the default.
*** f) check that there is a TIM module in that slot, using the status register
*** g) determine the address of the registers used for input and output,
***    which depend on the module number.
*** h) work out the bits representing TxEmpty and RxFull, and enable
***    these bits in the interrupt enable register.
**/
void qpc_init_link(char *name)
{
	word	 config_word;
	char	*config_text;
	int	 status_register;
	bool	 board_present;

	/* name == QPCV1 or QPCV2	*/
	Version = name[4] - '0';

	config_word	= get_int_config("link_base");
	if (config_word ne Invalid_config)
		HIOBASE	= (int) config_word;

	config_word	= get_int_config("qpc_slabase");
	if (config_word ne Invalid_config)
		SLABASE	= (int) config_word;

	_outpw(HIOBASE + ControlOffset, 0);

	if (Version eq 1)
	{
			/* The BAR register contains bits 5-11 of the	*/
			/* SLA address in bits 8-14. Bits 7 and 15 are 	*/
			/* reserved. Bits 0-6 hold the JTAG base address*/
			/* which is irrelevant.				*/
		BarValue	= (SLABASE  >> 5) & 0x7F;
		BarValue	= (BarValue << 8) & 0xFF00;
		_outpw(HIOBASE + BarOffset, BarValue);
	}
	else
	{
			/* The BAR register contains bits 5-11 of the	*/
			/* SLA address in bits 1-7. Bits 0 and 8 are	*/
			/* ignored, and bits 9-15 hold the JTAG address	*/
		BarValue	= (SLABASE >> 4) & 0x00FE;
		_outpw(HIOBASE + BarOffset, BarValue);
	}

		/* Control value. This is mostly 0. All the reset bits	*/
		/* are 0, PC interrupts are disabled, and the flags	*/
		/* should be 0. Bits 12 and 13 enable the SLABASE and	*/
		/* JTAGBase in some order.				*/
	if (Version eq 1)
		ControlValue	= 0x6000;	/* bit 13 == SLA	*/
	else
		ControlValue	= 0x5000;	/* bit 12 == SLA	*/
	_outpw(HIOBASE + ControlOffset, ControlValue | ResetBits);
	goto_sleep(100000);
	_outpw(HIOBASE + ControlOffset, ControlValue);

	config_text = get_config("qpc_module");
	if (config_text eq NULL)
		Module = 0;	/* default to primary	*/
	elif (!mystrcmp(config_text, "primary"))
		Module = 0;
	elif (!mystrcmp(config_text, "secondary"))
		Module = 1;
	elif (!mystrcmp(config_text, "tertiary"))
		Module = 2;
	elif (!mystrcmp(config_text, "quaternary"))
		Module = 3;
	else
	{
		ServerDebug("Invalid host.con option for qpc_module, it should be one of primary, \nsecondary, tertiary or quaternary.");
		longjmp(exit_jmpbuf, 1);
	}

	status_register = _inpw(SLABASE + StatusOffset);
	board_present	= FALSE;
	switch(Module)
	{
	case 0 : if ((status_register & 0x0004) == 0) board_present = TRUE;
		 break;

	case 1 : if ((status_register & 0x0010) == 0) board_present = TRUE;
		 break;

	case 2 : if ((status_register & 0x0200) == 0) board_present = TRUE;
		 break;

	case 3 : if ((status_register & 0x0800) == 0) board_present = TRUE;
		 break;
	}
	unless(board_present)
	{
		ServerDebug("QPC initialisation code, failed to find a board in the specied module.");
		longjmp(exit_jmpbuf, 1);
	}

		/* Work out the link data registers and the bits for	*/
		/* TxEmpty and RxFull, all of which depend on the	*/
		/* Module.						*/
	switch(Module)
	{
	case 0	:
		QpcLinkData	= SLABASE + 0x04;
		QpcTxEmptyBit	= 0x0008;
		QpcRxFullBit	= 0x0004;
		break;

	case 1 :
		QpcLinkData	= SLABASE + 0x0C;
		QpcTxEmptyBit	= 0x0800;
		QpcRxFullBit	= 0x0400;
		break;

	case 2 :
		QpcLinkData	= SLABASE + 0x00;
		QpcTxEmptyBit	= 0x0002;
		QpcRxFullBit	= 0x0001;
		break;

	case 3 :
		QpcLinkData	= SLABASE + 0x08;
		QpcTxEmptyBit	= 0x0200;
		QpcRxFullBit	= 0x0100;
		break;
	}

	QpcStatus	= SLABASE + InterruptStatus;

		/* Enable appropriate status lines.			*/
	_outpw(QpcStatus, QpcTxEmptyBit | QpcRxFullBit);
}





