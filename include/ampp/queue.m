--------------------------------------------------------------------------
--                                                                      --
--                      H E L I O S   K E R N E L                       --
--                      -------------------------                       --
--                                                                      --
--             Copyright (C) 1987, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- queue.m                                                              --
--                                                                      --
--      Queue data structures and kernel jump table offsets             --
--                                                                      --
--      Author: NHG 20-May-87						--
--                                                                      --
--	SCCS Id: %W% %G%						--
--------------------------------------------------------------------------

_report ['include queue.m]
_def 'queue.m_flag 1

include structs.m

-- List structure

struct List [
        word    Head                    -- pointer to first node on list
        word    Earth                   -- always NULL
        word    Tail                    -- pointer to last node on list
]

struct Node [
        word    Next                    -- pointer to next node in list
        word    Prev                    -- pointer to previous node in list
]

-- End of queue.m
