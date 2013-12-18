-- File:	hw_glap.h
-- Subsystem:	Helios-ARM Executive
-- Author:	P.A.Beskeen
-- Date:	Nov '92
--
-- Description: Gnome Link Adapter Podule (GLAP) manifests.
--
--
-- RcsId: $Id$
--
-- (C) Copyright 1992 Perihelion Software Ltd.
--     All Rights Reserved.


_report ['include 'hw_glap.m]
_def 'hw_glap.m_flag 1


-- Gnome transputer link adapter is an Acorn expansion card.

include ARM/xcb.m


-- Base of link adapter registers in expansion card space.

_def 'GLAP_base		0x2000


-- Gnome Link Adapter Podule register set.

struct glap_regs [
	byte read_data		byte _pad0.1 byte _pad0.2 byte _pad0.3
	byte write_data		byte _pad1.1 byte _pad1.2 byte _pad1.3
	byte input_status	byte _pad2.1 byte _pad2.2 byte _pad2.3
	byte output_status	byte _pad3.1 byte _pad3.2 byte _pad3.3
	byte control_status	byte _pad4.1 byte _pad4.2 byte _pad4.3
]


-- Returns address of link adapter for given card slot.

_defq 'GLAP_LinkAdapter[ln] [
	(XCB_ADDRESS(FAST, (ln)) + GLAP_base)
]


-- Loads address of link adapter for given card slot.
-- First reg (slotreg) holds card number, second is where store address.

_defq 'LD_GLAP_LinkAdapter[slotreg reg] [
	mov	reg, XCB_base + (XCB_SPEED_FAST << 19)
	add	reg, GLAP_base
	add	reg, slotreg LSL 14
]


-- Control register bits.

_def 'GLAP_ResetOut		0x1
_def 'GLAP_AnalyseOut		0x2
_def 'GLAP_LinkSpeed20Mhz	0x4	-- 0 = 10Mhz
_def 'GLAP_ChipReset		0x8	-- Set link adapter rst (min. 1.6uS)
_def 'GLAP_ErrorIn		0x16	-- Read error pin value


-- Input status register bits.

_def 'GLAP_InputReady		0x1
_def 'GLAP_ReadIntrEnable	0x2


-- Output status register bits.

_def 'GLAP_OutputReady		0x1
_def 'GLAP_WriteIntrEnable	0x2



-- End of hw_glap.m
