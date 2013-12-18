--------------------------------------------------------------------------
--                                                                      --
--                      H E L I O S   K E R N E L                       --
--                      -------------------------                       --
--                                                                      --
--             Copyright (C) 1987, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- debug.m                                                              --
--                                                                      --
--	Debugging macros						--
--                                                                      --
--      Author NHG 20-May-87						--
--                                                                      --
--	SCCS Id: %W% %G%						--
--------------------------------------------------------------------------

_report ['include debug.m]
_def 'debug.m_flag 1

_test [_defp 'debugging]
[
        _defq 'Regs [call .PutRegs]
        _defq 'Mark [call .PutMark]
]
[
        _defq 'Regs []
        _defq 'Mark []
]


-- End of debug.m
