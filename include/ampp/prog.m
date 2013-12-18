--------------------------------------------------------------------------
--                                                                      --
--                      H E L I O S   K E R N E L                       --
--                      -------------------------                       --
--                                                                      --
--             Copyright (C) 1987, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- prog.m								--
--                                                                      --
--	Program structure						--
--                                                                      --
--      Author: NHG 8/8/87						--
--                                                                      --
--	SCCS Id: %W% %G%						--
--------------------------------------------------------------------------

_report ['include prog.m]
_def 'prog.m_flag 1

include structs.m
include module.m

struct Program [
	struct Module	Module		-- start of module
	word		Stacksize	-- size of initial stack
	word		Heapsize	-- size of program heap
	word		Main		-- offset of main entry point
]

-- End of prog.m
