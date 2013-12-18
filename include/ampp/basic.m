--------------------------------------------------------------------------
--                                                                      --
--                      H E L I O S   K E R N E L                       --
--                      -------------------------                       --
--                                                                      --
--             Copyright (C) 1987, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- basic.m                                                              --
--                                                                      --
--      General purpose macros                                          --
--                                                                      --
--	Author: NHG 20/5/87						--
--                                                                      --
-- SccsId: %W% %G%                                                      --
--------------------------------------------------------------------------

_report ['include basic.m]
_def 'basic.m_flag 1

-- Conditional include

_defq 'include['file]
[
        _if [_not _defp [file$_flag]] [_include file]
]

_def 'debugging 1	-- set if we are debugging 

_def 'addon 1		-- set if assembling for the add on board

_defq 'sccsid[arg]
[
		[byte "arg", 0]
]

_defq 'rcsid[arg] []		-- null out RCS ids

_test _defp 'helios.TRAN
[
	_defq 'Null [ ldc 0 ]	-- was mint
][
	_defq 'Null	[0]
	_def 'NULL	[0]
	_def 'FALSE	[0]
	_def 'TRUE	[1]
]


-- End of basic.m
