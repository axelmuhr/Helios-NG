--------------------------------------------------------------------------
--                                                                      --
--                      H E L I O S   K E R N E L                       --
--                      -------------------------                       --
--                                                                      --
--             Copyright (C) 1987, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- event.m								--
--                                                                      --
--	Event Structure							--
--                                                                      --
--      Author: NHG 24-October-87					--
--                                                                      --
--	SCCS Id: %W% %G%						--
--------------------------------------------------------------------------

_report ['include event.m]
_def 'event.m_flag 1


include structs.m

struct Event [
	struct Node	Node		-- link in event chain
	word		Pri		-- priority in chain
	word		Code		-- event routine
	word		Data		-- data for this
	word		ModTab		-- pointer to module table
]

-- End of event.m
