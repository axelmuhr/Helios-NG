-- File:	c40idrom.m
-- Subsystem:	`C40 Helios executive
-- Author:	P.A.Beskeen
-- Date:	March '94
--
-- Description:
--
-- ROMed copy of IDROM structure.
--
-- The following data defines the boards 'IDROM' structure. The IDROM
-- is a standard defined by Texas Instruments and used by Helios-C40 to 
-- adapt to the environment in which it find itself running. The
-- document "TIM-40 TMS320C4x Module Specification" defines the
-- standard and is available from TI. For a quick description of the
-- following items, refer to the Helios example I/O Server configuration
-- file "template.con" on the Helios-C40 distribution disk/tape. Its
-- structure is also defined in "/hsrc/include/ampp/tim40.m".
--
-- This information was originally placed in c40rombt.a, but for easier
-- modification by users has been removed, and replaced by an include
-- statement
--

_report ['include c40idrom.m]

BoardIDROM:
	-- Miscellaneous items. The following 7 items do not need to be set
	-- as they are not used by Helios.
	word	IDROM.sizeof / 4	-- SIZE - defined in words.
--	short	0xfffe		-- MAN_ID - manufacturers ID.
--	byte	0		-- CPU_ID (0 = C40).
--	byte	49		-- CPU_CLK (49 = 40Mhz, 39 = 50Mhz, 29 = 60Mhz)
	word	0x3100feff	-- [replaces above]
--	short	0		-- MODEL_NO - board type ID/
--	byte	0		-- REV_LVL - board revison level.
--	byte	0		-- RESERVED.
	word	0		-- [replaces above]

	-- Memory bank location and size. These values must be set for your
	-- board. The following example contents define 2 blocks of memory,
	-- local strobe 0 and global strobe 0, starting at addresses 0x300000
	-- and 0x80000000 respectively, both blocks containing 4MB's of RAM.
	-- ONLY RAM memory should be included in these descriptions, NOT ROM
	-- Helios initialises its memory pool bounds from these values.
	word	0x80000000	-- GBASE0.
	word	-1		-- GBASE1 (-1 = no memory).
	word	0x00300000	-- LBASE0.
	word	-1		-- LBASE1 (-1 = no memory).

	-- The following memory bank sizes are defined in terms of number of
	-- words, NOT bytes.
	word	0x00100000	-- GSIZE0 (4MB's).
	word	0		-- GSIZE1.
	word	0x00100000	-- LSIZE0 (4MB's).
	word	0		-- LSIZE1.

	word	0800		-- FSIZE (just onchip RAM).

	-- Speed of memory - not used by Helios.
--	byte	0x22		-- WAIT_G.
--	byte	0x22		-- WAIT_L.
--	byte	0x55		-- PWAIT_G.
--	byte	0x55		-- PWAIT_L.
	word	0x55552222	-- [replaces above]

	-- Timer values - TIMER0_PERIOD must be set for accurate Helios clock.

	-- For Hunt HEPC2
	word	0x203a		-- TIMER0_PERIOD
	word	0x80		-- TIMER1_PERIOD
--	short	0x282		-- TIMER0_CTRL
--	short	0x281		-- TIMER1_CTRL (Can be used for DRAM refresh).
	word	0x02810282	-- [replaces above]

	-- Memory control registers - These are actually set by the CPU
	-- boot loader header.

	-- For Hunt HEPC2
	word	0x1e4a4000	-- GBCR
	word	0x1e4a4000	-- LBCR

	-- Not used by Helios.
	word	1		-- AINIT_SIZE.

BoardIDROMEnd:

