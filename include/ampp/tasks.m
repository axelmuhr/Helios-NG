--------------------------------------------------------------------------
--                                                                      --
--                      H E L I O S   K E R N E L                       --
--                      -------------------------                       --
--                                                                      --
--             Copyright (C) 1987, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- tasks.m								--
--                                                                      --
--
--                                                                      --
--      Author: NHG 28-July-87						--
--                                                                      --
--	SCCS Id: %W% %G%						--
--------------------------------------------------------------------------

_report ['include tasks.m]
_def 'tasks.m_flag 1

include structs.m
include queue.m
include memory.m

struct Task [
	struct Node	Node		-- queueing node
	word		Program		-- pointer to program structure
	struct Pool	MemPool		-- task private memory pool
	word		Port		-- initial message port
	word		Parent		-- parent's message port
	word		IOCPort		-- tasks IOC port
	word		Flags		-- task control flags
	word		ExceptCode	-- exception routine
	word		ExceptData	-- data for same
	word		HeapBase	-- base of task heap area
	word		ModTab		-- module table pointer
	word		TaskEntry	-- procman control structure
]

-- End of tasks.m
