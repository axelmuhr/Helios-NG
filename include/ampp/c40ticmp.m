-- File:	c40ticmp.m
-- Subsystem:	'C40 Helios AMPP macros
-- Author:	Paul Beskeen
-- Date:	Dec '91
--
-- Description: Helios/C40 / Texas Instruments Assembler compatibility macros.
--
--
-- RcsId: $Id: c40ticmp.m,v 1.2 1992/10/22 19:14:41 craig Exp $
--
-- (C) Copyright 1992 Perihelion Software Ltd.


-- Apart from the different directives supported, the Helios and TI C40
-- assemblers also differ in the following respects:
--
--	1) Helios assembler only supports position independent code.
--	   All labels in constant expressions return the offset from the
--	   current position to the position of the label.
--
--	2) Hexadecimal constants are prefixed by '0x' in the Helios assembler
--	   and postfixed by 'H' in the TI assembler.
--
--	3) Labels must be terminated by a trailing ':' in the Helios assembler,
--	   but this is only optional in the TI assembler.
--
--	4) The C40 indirect addressing decrement operator '--' must be escaped
--	   when used in AMPP source (such as this file), otherwise it will
--	   be taken as the AMPP comment character sequence. to escape the
--	   decrement, prefix each hypen with the AMPP esacpe character (a
--	   single quote) e.g.
--		ldi	r0, *'-'-AR0
--


-- Error free directive conversions:

_defq '.byte [char]
_defq '.hword [short]
_defq '.int [int]
_defq '.long [int]
_defq '.float [c40float]
_defq '.ieee [ieee32]
_defq '.space [space]
_defq '.string [char]
_defq '.word [int]
_defq '.align [align 128]
_defq '.even [align]
_defq '.global [global]
_defq '.ref [global]
_defq '.end [end]
_defq '.emsg [error]
_defq '.wmsg [warning]
_defq '.mmsg [note]

_defq '.copy ['_include]	-- beware: you may have to strip quotes
_defq '.include ['_include]	-- beware: you may have to strip quotes


-- Directives we can safely ignore:

_defq '.text []
_defq '.fclist []
_defq '.fnoclist []
_defq '.length[pagelen] []
_defq '.list []
_defq '.mlist []
_defq '.mnolist []
_defq '.nolist []
_defq '.page []
_defq '.sslist []
_defq '.ssnolist []
_defq '.width [pwidth] []

_defq '.version['num] [
	_if _not _eq num 40 [
		_report '[.Version error - can only assemble code for a TMS320C40]
		error '[".Version error - Can only assemble code for a TMS320C40"]
	]
]


-- Currently unsupported directives:

_defq '.field [
	_report '[.field directive not currently supported]
	error '[".field directive not supported"]
	'.field]


-- COFF support macros cannot be directly converted:

_defq '.asect [
	_report '[.asect directive not supported]
	error '[".asect directive not supported"]
	'.asect ]

_defq '.bss [
	_report '[.bss directive not supported]
	error '[".bss directive not supported (use GHOF data)"]
	'.bss ]

_defq '.data [
	_report '[.data directive not supported]
	error ".data directive not supported (use GHOF data)"
	'.data ]

_defq '.sect [
	_report '[.sect directive not supported]
	error '[".sect directive not supported"]
	'.sect ]

_defq '.option [
	_report '[.option not supported]
	error '[".option directive not supported (just delete it)"]
 	'.option ]



-- Other directives we do not support or cannot convert:
--
-- These are mostly macro definition directives that should be
-- converted into their AMPP equivalents.

_defq '.mlib [
	_report '[.mlib directive not supported]
	error '[".mlib directive not supported"]
	'.mlib ]

_defq '.break [
	_report '[.break directive not supported]
	error '[".break directive not supported"]
	'.break ]

_defq '.if [
	_report '[.if directive not supported]
	error '[".if directive not supported"]
	'.if ]

_defq '.else [
	_report '[.else directive not supported]
	error '[".else directive not supported"]
	'.else ]

_defq '.elseif [
	_report '[.elseif directive not supported]
	error '[".elseif  directive not supported"]
	'.elseif ]

_defq '.endif [
	_report '[.endif directive not supported]
	error '[".endif directive not supported"]
	'.endif ]

_defq '.endloop [
	_report '[.endloop directive not supported]
	error '[".endloop directive not supported"]
	'.endloop ]

_defq '.loop [
	_report '[.loop directive not supported]
	error '[".loop directive not supported"]
	'.loop ]

_defq '.asg [
	_report '[.asg directive not supported]
	error '[".asg directive not supported"]
	'.asg ]

_defq '.eval [
	_report '[.eval directive not supported]
	error '[".eval directive not supported"]
	'.eval ]

_defq '.endstruct [
	_report '[.endstruct directive not supported]
	error '[".endstruct directive not supported"]
	'.endstruct ]

_defq '.label [
	_report '[.label directive not supported]
	error '[".label directive not supported"]
	'.label ]

_defq '.tag [
	_report '[.tag directive not supported]
	error '[".tag directive not supported"]
	'.tag ]



-- end of c40ticmp.m
