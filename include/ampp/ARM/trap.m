-- File:	trap.m
-- Subsystem:	Helios-ARM header files.
-- Author:	P.A.Beskeen
-- Date:	Oct '92
--
-- Description: Helios-ARM trap (swi) definitions. Trap numbers are
--		encapsulated within the ARM swi instruction.
--		This file defines how this is encoding is applied.
--
--		Include this file in preference to ARM/trapdefs.m.
--
--		*WARNING* must be kept in step with <ARM/trap.h>
--
--		To call a Helios trap use: 'swi	TRAP_<function-name>'.
--
--
-- RcsId: $Id: trap.m,v 1.1 1993/08/05 13:10:57 paul Exp $
--
-- (C) Copyright 1993 Perihelion Software Ltd.
--     All Rights Reserved.


_report ['include trap.m]
_def 'ARM/trap.m_flag 1


-- Define SWI fields.

_def SWI_OP		0xef000000	-- swi instruction 

_def SWI_OS_RISCOS	0x00000000	-- RISCOS is OS 0 
_def SWI_OS_RISCIX	0x00100000	-- RISCiX is OS 1 
_def SWI_OS_ARX		0x00200000	-- ARX is OS 2    
_def SWI_OS_HELIOS	0x00300000	-- Helios is OS 3 

_def SWI_OS_MASK	0x00f00000	-- Mask for OS bits 
_def SWI_TRAPMASK	0x000000ff	-- Helios system call mask <= 255 fns 
_def SWI_UNUSEDMASK	0x000fff00	-- @@@ Maybe use later for flags


-- Define std bits to use whenever calling Helios swi's.

_def 'TRAP_STDHELIOS	SWI_OS_HELIOS


-- Call pseudo trap interface rather than standard SWI trap interface.

_defq PseudoTrap[trapnum] [
	mov	tmp, (trapnum & ~TRAP_STDHELIOS)
	mov	lr, pc
	ldr	pc, (r0, -r0)
]


-- Include automatically produced trap number defs:

include 'ARM/trapdefs.m


-- end of swi.m 
