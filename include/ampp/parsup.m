-- Macros for Parrallel support Library

-- CHRG	14:10:88

_report ['include parsup.m]
_def 'parsup.m_flag 1

-- sccsid  [@(#)parsup.m	1.1    15/9/89 Copyright (C) 1987, Perihelion Software Ltd.]


-- Alternate Structure Definition

struct AltStruct [
	word	Type		-- Type of guard
	word	Bool		-- Boolean 
	word	Guard		-- Guard 
]

_def 	AltStructSize	[3]

_def	AltStruct_Type	[0]
_def	AltStruct_Bool	[1]
_def	AltStruct_Guard [2]

-- Guard Types

_def	Alt_Skip   [0]
_def	Alt_Chan   [1]
_def	Alt_Timer  [2]

-- General

_def  bytesperword  [4]




--_defq 'loop['loop_var 'count 'loop_body]           -- while loop
--[
--deftemp [
--	align				-- force loop label to word boundary
--templab 1:
--
--	loop_body			-- body of loop
--
--
--	ptr  loop_var
--	ldc  templab 2 - templab 1
--	lend
--templab 2:
--        ]
--]

-- End parsup.m
