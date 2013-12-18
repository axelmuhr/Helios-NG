-- File:	tim40.m
-- Subsystem:	'C40 Helios executive
-- Author:	P.A.Beskeen
-- Date:	Jan '92
--
-- Description: TIM-40 and ID ROM description
--
--		The ID ROM is a serial access ROM that characterises a
--		TIM-40 module. TCLK0 must be pulsed in a fixed sequence to
--		start to access the serial (1 or 4 bit) ROM's contents.
--		The data is then read a bit at a time from word reads of the
--		local bus.
--
-- RcsId: $Id: tim40.m,v 1.1 1992/06/29 14:05:31 paul Exp $
--
-- (C) Copyright 1992 Perihelion Software Ltd.

include structs.m


-- Define TIM-40 IDROM contents and their structure offsets within its image.

struct IDROM [
	word	SIZE

	short	MAN_ID
	byte	CPU_ID
	byte	CPU_CLK
	short	MODEL_NO
	byte	REV_LVL
	byte	RESERVED

	word	GBASE0
	word	GBASE1
	word	LBASE0
	word	LBASE1

	word	GSIZE0
	word	GSIZE1
	word	LSIZE0
	word	LSIZE1

	word	FSIZE

	byte	WAIT_G
	byte	WAIT_L
	byte	PWAIT_G
	byte	PWAIT_L

	word	TIMER0_PERIOD
	word	TIMER1_PERIOD
	short	TIMER0_CTRL
	short	TIMER1_CTRL

	word	GBCR
	word	LBCR

	word	AINIT_SIZE
]


-- Initialisation Block contains size, address to load to and data to load.
-- a B_SIZE of zero ends the IDROM load sequence

struct IDROMBLOCK [
	word	B_SIZE
	word	B_ADDR
	word	B_DATASTART		-- many words of data
]


-- TIM-40 specifies that all additional memory mapped periperals are mapped
-- at these addresses or higher.

_def	'LocalPeriMem		0x70000000
_def	'GlobalPeriMem		0xb0000000



-- end of tim40.m


